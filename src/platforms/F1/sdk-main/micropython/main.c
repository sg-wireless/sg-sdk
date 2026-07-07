/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "tinyusb_freertos_compat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_task.h"
#include "esp_cpu.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_flash.h"

#include "esp_psram.h"

#include "py/cstack.h"
#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/persistentcode.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "extmod/modmachine.h"
#include "shared/readline/readline.h"
#include "shared/runtime/pyexec.h"
#include "shared/timeutils/timeutils.h"
#include "shared/tinyusb/mp_usbd.h"

#include "uart.h"
#include "uart_resource_guard.h"
#include "usb.h"
#include "usb_serial_jtag.h"
#include "modesp32.h"
#include "modmachine.h"
#include "modnetwork.h"

#if MICROPY_BLUETOOTH_NIMBLE
#include "extmod/modbluetooth.h"
#endif

#if MICROPY_PY_ESPNOW
#include "modespnow.h"
#endif

#ifdef CONFIG_SAFEBOOT_FEATURE_ENABLE
#include "boot_if.h"
#endif
#include "log_lib.h"
#if defined(CONFIG_LORA_DEINIT_ON_SOFT_RESET) || defined(CONFIG_LORA_LCT_OPERATE_AFTER_RESET)
#include "lora.h"
#endif

// Forward declaration for OTA cancellation on soft reset
extern void fuota_cancel_on_soft_reset(void);
extern void fuota_validate_ota_partition(void);

#ifdef CONFIG_SDK_CTRL_CLIENT_BOOT_ENABLE
static bool ctrl_on_boot_nvs_check(void) {
    nvs_handle_t handle;
    uint8_t val;
    esp_err_t err = nvs_open("ctrl", NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return true;
    }
    err = nvs_get_u8(handle, "ctrl", &val);
    nvs_close(handle);
    if (err != ESP_OK) {
        return true;
    }
    return (bool)val;
}
#endif

// Forward declaration for mbedtls platform time function
extern int mbedtls_platform_set_time(time_t (*time_func)(time_t *time));

// MicroPython runs as a task under FreeRTOS
#define MP_TASK_PRIORITY        (ESP_TASK_PRIO_MIN + 1)

typedef struct _native_code_node_t {
    struct _native_code_node_t *next;
    uint32_t data[];
} native_code_node_t;

static native_code_node_t *native_code_head = NULL;

static void esp_native_code_free_all(void);

int vprintf_null(const char *format, va_list ap) {
    // do nothing: this is used as a log target during raw repl mode
    return 0;
}

// Required by FreeRTOS port - provide a TCB cleanup function
void vPortCleanUpTCB(void *pxTCB) {
    // No special cleanup required for this implementation
    (void)pxTCB;
}

void set_custom_mac(void)
{
    uint8_t base_mac_addr[6] = { 0 };
    esp_err_t ret = ESP_OK;
    ret = esp_efuse_mac_get_custom(base_mac_addr);
    ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
    if (ret == ESP_OK) {
        ret = esp_base_mac_addr_set(base_mac_addr);
        ESP_ERROR_CHECK_WITHOUT_ABORT(ret);
    }
}

static void handle_lora_lct_mode(void)
{
    #ifdef CONFIG_LORA_LCT_OPERATE_AFTER_RESET

    lora_ctor();
    lora_mode_t mode;
    bool lct_state = false;
    lora_get_mode(&mode);
    if(mode == __LORA_MODE_WAN)
    {
        lora_ioctl(__LORA_IOCTL_LCT_MODE_GET, &lct_state);
    }

    if( lct_state )
    {
        __log_output("== lora lct mode started\n");
    }
    else
    {
        lora_dtor();
    }

    #endif /* CONFIG_LORA_LCT_OPERATE_AFTER_RESET */
}

time_t platform_mbedtls_time(time_t *timer) {
    // mbedtls_time requires time in seconds from EPOCH 1970

    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec + TIMEUTILS_SECONDS_1970_TO_2000;
}

void mp_task(void *pvParameter) {
    volatile uint32_t sp = (uint32_t)esp_cpu_get_sp();
    #if MICROPY_PY_THREAD
    mp_thread_init(pxTaskGetStackStart(NULL), MICROPY_TASK_STACK_SIZE / sizeof(uintptr_t));
    #endif
    #if MICROPY_HW_ESP_USB_SERIAL_JTAG
    usb_serial_jtag_init();
    #elif MICROPY_HW_ENABLE_USBDEV
    usb_init();
    #endif
    #if MICROPY_HW_ENABLE_UART_REPL
    uart_stdout_init();
    #endif
    set_custom_mac();
    machine_init();

    // Configure time function, for mbedtls certificate time validation.
    mbedtls_platform_set_time(platform_mbedtls_time);

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK) {
        ESP_LOGE("esp_init", "can't create event loop: 0x%x\n", err);
    }

    void *mp_task_heap = MP_PLAT_ALLOC_HEAP(MICROPY_GC_INITIAL_HEAP_SIZE);
    if (mp_task_heap == NULL) {
        printf("mp_task_heap allocation failed!\n");
        esp_restart();
    }

    fuota_validate_ota_partition();

soft_reset:
    // Cancel any ongoing OTA upgrade before resetting Python environment
    fuota_cancel_on_soft_reset();

    // Clear dynamic log registrations so MicroPython modules can re-register
    log_dynamic_registry_clear();
    
    // initialise the stack pointer for the main thread
    mp_cstack_init_with_top((void *)sp, MICROPY_TASK_STACK_SIZE);
    gc_init(mp_task_heap, mp_task_heap + MICROPY_GC_INITIAL_HEAP_SIZE);
    mp_init();
    mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_lib));
    readline_init0();

    // initialise peripherals
    machine_pins_init();
    #if MICROPY_PY_MACHINE_I2S
    machine_i2s_init0();
    #endif

    // run boot-up scripts
    pyexec_frozen_module("_boot.py", false);
    #if CONFIG_SAFEBOOT_FEATURE_ENABLE
    bootif_safeboot_soft_reset_init();
    if( bootif_state_get() == __BOOTIF_STATE_SAFEBOOT_MODE )
    {
        __log_output("== micropython "__green__"safeboot"__default__" mode\n"
            __red__"-- skip 'boot.py', 'main.py'"__default__"\n");
        bootif_state_set(__BOOTIF_STATE_NORMAL_MODE);
        #ifdef CONFIG_SDK_CTRL_CLIENT_BOOT_ENABLE
        if (ctrl_on_boot_nvs_check()) {
            pyexec_file_if_exists("ctrl_client_safeboot.py");
        }
        #endif

    }
    else
    #endif
    {
        __log_output("== micropython "__yellow__"normal"__default__" mode\n");
        pyexec_file_if_exists("boot.py");

        if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
            #ifdef CONFIG_SDK_CTRL_CLIENT_BOOT_ENABLE
            if (ctrl_on_boot_nvs_check()) {
                pyexec_file_if_exists("ctrl_client_start.py");
            } else {
                pyexec_file_if_exists("ctrl_client_stubs.py");
            }
            #endif
            int ret = pyexec_file_if_exists("main.py");
            if (ret & PYEXEC_FORCED_EXIT) {
                goto soft_reset_exit;
            }
        } else {
            #ifdef CONFIG_SDK_CTRL_CLIENT_BOOT_ENABLE
            pyexec_file_if_exists("ctrl_client_safeboot.py");
            #endif
        }
    }

    handle_lora_lct_mode();

    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            vprintf_like_t vprintf_log = esp_log_set_vprintf(vprintf_null);
            if (pyexec_raw_repl() != 0) {
                break;
            }
            esp_log_set_vprintf(vprintf_log);
        } else {
            if (pyexec_friendly_repl() != 0) {
                break;
            }
        }
    }

soft_reset_exit:

    #if MICROPY_BLUETOOTH_NIMBLE
    mp_bluetooth_deinit();
    #endif

    #if MICROPY_PY_ESPNOW
    espnow_deinit(mp_const_none);
    MP_STATE_PORT(espnow_singleton) = NULL;
    #endif

    #ifdef CONFIG_SDK_MPY_HOOK_MACHINE_VIRTUAL_TIMERS_ENABLE
    void hook_mpy_machine_timer_virtual_deinit_all(void);
    hook_mpy_machine_timer_virtual_deinit_all();
    #endif

    machine_timer_deinit_all();

    #if MICROPY_PY_ESP32_PCNT
    esp32_pcnt_deinit_all();
    #endif

    #if MICROPY_PY_THREAD
    mp_thread_deinit();
    #endif

    #if MICROPY_HW_ENABLE_USB_RUNTIME_DEVICE
    mp_usbd_deinit();
    #endif

    gc_sweep_all();

    // Free any native code pointers that point to iRAM.
    esp_native_code_free_all();

    mp_hal_stdout_tx_str("MPY: soft reboot\r\n");

    // deinitialise peripherals
    machine_pwm_deinit_all();
    // TODO: machine_rmt_deinit_all();
    machine_pins_deinit();
    #ifdef CONFIG_LORA_DEINIT_ON_SOFT_RESET
    // Tear down the LoRaWAN MAC stack (preserves NVM join credentials) so its
    // RX window timers and internal state are cleanly rebuilt on the next boot
    // via lora_ctor() when the lora module is imported again.
    lora_dtor();
    #endif
    #if MICROPY_PY_MACHINE_I2C_TARGET
    mp_machine_i2c_target_deinit_all();
    #endif
    machine_deinit();

    #if MICROPY_PY_SOCKET_EVENTS
    socket_events_deinit();
    #endif

    mp_deinit();
    /*
     * Release all UART reservations on soft reset to ensure a clean state.
     * This includes both regular user code (context=0) and LTE.py (context=1).
     * LTE.py will re-acquire UART1 when reloaded if needed.
     */
    uart_release_all();
    fflush(stdout);
    goto soft_reset;
}

void boardctrl_startup(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // Query the physical size of the SPI flash and store it in the size
    // variable of the global, default SPI flash handle.
    esp_flash_get_physical_size(NULL, &esp_flash_default_chip->size);

    // If there is no filesystem partition (no "vfs" or "ffat"), add a "vfs" partition
    // that extends from the end of the application partition up to the end of flash.
    if (esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "vfs") == NULL
        && esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "ffat") == NULL) {
        // No "vfs" or "ffat" partition, so try to create one.

        // Find the end of the last partition that exists in the partition table.
        size_t offset = 0;
        esp_partition_iterator_t iter = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
        while (iter != NULL) {
            const esp_partition_t *part = esp_partition_get(iter);
            offset = MAX(offset, part->address + part->size);
            iter = esp_partition_next(iter);
        }

        // If we found the application partition and there is some space between the end of
        // that and the end of flash, create a "vfs" partition taking up all of that space.
        if (offset > 0 && esp_flash_default_chip->size > offset) {
            size_t size = esp_flash_default_chip->size - offset;
            esp_partition_register_external(esp_flash_default_chip, offset, size, "vfs", ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, NULL);
        }
    }

    void init_log_system(void);
    init_log_system();

    #ifdef CONFIG_IOEXP_ENABLED
    /**
     * GPIO ISR service installation was originally done in machine_pin.c
     * but because it is needed by other interrupt intialisation such as
     * the IO-Expander interrupt pin, it will be done here earlier before all
     * system modules.
     */
    gpio_install_isr_service(0);

    void ioexp_init(void);
    ioexp_init();
    #endif /* CONFIG_IOEXP_ENABLED */
}

void app_main(void) {
    // Hook for a board to run code at start up.
    // This defaults to initialising NVS.
    MICROPY_BOARD_STARTUP();
    // Create and transfer control to the MicroPython task.
    xTaskCreatePinnedToCore(mp_task, "mp_task", MICROPY_TASK_STACK_SIZE / sizeof(StackType_t), NULL, MP_TASK_PRIORITY, &mp_main_task_handle, MP_TASK_COREID);
}

MP_WEAK void nlr_jump_fail(void *val) {
    printf("NLR jump failed, val=%p\n", val);
    esp_restart();
}

static void esp_native_code_free_all(void) {
    while (native_code_head != NULL) {
        native_code_node_t *next = native_code_head->next;
        heap_caps_free(native_code_head);
        native_code_head = next;
    }
}

void *esp_native_code_commit(void *buf, size_t len, void *reloc) {
    len = (len + 3) & ~3;
    size_t len_node = sizeof(native_code_node_t) + len;
    native_code_node_t *node = heap_caps_malloc(len_node, MALLOC_CAP_EXEC);
    #if CONFIG_IDF_TARGET_ESP32S2
    // Workaround for ESP-IDF bug https://github.com/espressif/esp-idf/issues/14835
    if (node != NULL && !esp_ptr_executable(node)) {
        free(node);
        node = NULL;
    }
    #endif // CONFIG_IDF_TARGET_ESP32S2
    if (node == NULL) {
        m_malloc_fail(len_node);
    }
    node->next = native_code_head;
    native_code_head = node;
    void *p = node->data;
    if (reloc) {
        mp_native_relocate(reloc, buf, (uintptr_t)p);
    }
    memcpy(p, buf, len);
    return p;
}
