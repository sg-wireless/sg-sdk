/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @author  Ahmed Sabry (SG Wireless)
 * 
 * @brief   This file represents the porting of the Pycom LoRa stack to the ESP32
 *          IDF environment.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <sys/time.h>

#include "system/timer.h"
#include "system/systime.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs.h"
#include "esp_random.h"
#include "esp_crc.h"
#include "ioexp.h"
#include "lora_port.h"

#define __log_subsystem     lora
#define __log_component     port_system
#include "log_lib.h"

/** -------------------------------------------------------------------------- *
 * Pycom LoRa Stack System Port Definition
 * --------------------------------------------------------------------------- *
 */
// -- init methods for ports that needs initialization
static int esp32_nvm_port_ctor(void);
static int esp32_nvm_port_dtor(void);
static int esp32_mutex_port_ctor(void);
static int esp32_mutex_port_dtor(void);

// -- non-volatile memory (nvm)
static bool nvm_check(const char* key);
static int nvm_load(const char* key, uint8_t* buf, uint32_t size);
static int nvm_store(const char* key, uint8_t* buf, uint32_t size);
static int nvm_sync(void);
static int nvm_clear(const char* key);

// -- system timers
static void* timer_init(const char* name, void* arg, void(*p_callback)(void*));
static void timer_del(void* handle);
static void timer_start(void* handle);
static void timer_stop(void* handle);
static void timer_set_period(void* handle, uint32_t msec);

// -- system time
static uint32_t get_timestamp_msec(void);
static void delay_msec(uint32_t msec);

// -- access mutex
static void* mutex_new(void);
static void mutex_lock(void* handle);
static void mutex_unlock(void* handle);

// -- sync semaphore
static void* sem_new(void);
static void sem_wait(void* handle);
static void sem_signal(void* handle);

// -- optional utilities ( optional )
static uint32_t crc32_calc(uint32_t initial_crc, uint8_t * buf, uint32_t len);

void lora_board_ctor(void)
{
    __log_info("ctor() -> lora board");

    // -- system initialization

    // -- init ports
    esp32_nvm_port_ctor();
    esp32_mutex_port_ctor();

    // -- init lora stack
    lora_port_params_t init_params = {
        .nvm_check = nvm_check,
        .nvm_load = nvm_load,
        .nvm_store = nvm_store,
        .nvm_sync = nvm_sync,
        .nvm_clear = nvm_clear,
        .timer_init = timer_init,
        .timer_delete = timer_del,
        .timer_start = timer_start,
        .timer_stop = timer_stop,
        .timer_set_period = timer_set_period,
        .get_timestamp_msec = get_timestamp_msec,
        .delay_msec = delay_msec,
        .mutex_new = mutex_new,
        .mutex_lock = mutex_lock,
        .mutex_unlock = mutex_unlock,
        .sem_new = sem_new,
        .sem_wait = sem_wait,
        .sem_signal = sem_signal,
        .crc32_calc = crc32_calc
    };
    lora_port_init( &init_params );

    // -- hardware initialization
    ioexp_lora_chip_power_on();

    void sgw3501_lora_sx1262_ctor(void);
    sgw3501_lora_sx1262_ctor();
}

void lora_board_dtor(void)
{
    __log_info("dtor() -> lora board");

    ioexp_lora_chip_power_off();

    void sgw3501_lora_sx1262_dtor(void);
    sgw3501_lora_sx1262_dtor();

    esp32_nvm_port_dtor();
    esp32_mutex_port_dtor();
}

/** -------------------------------------------------------------------------- *
 * nvm ports implementation
 * --------------------------------------------------------------------------- *
 */
static const char* s_nvs_namespace = "lora-stack";
static nvs_handle_t s_nvs_handle;

static int esp32_nvm_port_ctor(void)
{
    __log_debug("");
    esp_err_t ret;
    ret = nvs_open(s_nvs_namespace, NVS_READWRITE, & s_nvs_handle);
    if(ret != ESP_OK) {
        __log_error("-- failed --");
        return -1;
    }
    return 0;
}

static int esp32_nvm_port_dtor(void)
{
    __log_debug("");
    nvs_close(s_nvs_handle);
    return 0;
}

void esp32_lora_nvm_clear(void)
{
    __log_debug("clear all lora nvm");
    esp_err_t ret;
    ret = nvs_erase_all(s_nvs_handle);
    if(ret != ESP_OK)
        __log_error("-- failed --");
}

static bool nvm_check(const char* key)
{
    __log_debug("check record '%s'", key);
    nvs_iterator_t it = nvs_entry_find(NVS_DEFAULT_PART_NAME, s_nvs_namespace,
        NVS_TYPE_ANY);
    while (it != NULL) {
            nvs_entry_info_t info;
            nvs_entry_info(it, &info);
            if( strcmp(key, info.key) == 0 ) {
                return true;
            }
            it = nvs_entry_next(it);
    };
    return false;
}

static int nvm_load(const char* key, uint8_t* buf, uint32_t size)
{
    __log_debug("load record '%s'", key);
    esp_err_t ret;
    size_t ret_size = size;
    ret = nvs_get_blob(s_nvs_handle, key, buf, &ret_size);
    if(ret != ESP_OK || ret_size != size) {
        __log_error("-- failed -- errcode(%d) size: %d, ret_size: %d",
            ret, size, ret_size);
        return -1;
    }
    return 0;
}

static int nvm_store(const char* key, uint8_t* buf, uint32_t size)
{
    __log_debug("store record '%s'", key);
    esp_err_t ret;
    ret = nvs_set_blob(s_nvs_handle, key, buf, size);
    if(ret != ESP_OK) {
        __log_error("-- failed --");
        return -1;
    }
    return 0;
}

static int nvm_clear(const char* key)
{
    __log_debug("clear record '%s'", key);
    esp_err_t ret;
    ret = nvs_erase_key(s_nvs_handle, key);
    if(ret != ESP_OK) {
        __log_error("-- failed --");
        return -1;
    }
    return 0;
}

static int nvm_sync(void)
{
    __log_debug("sync lora nvm storage");
    esp_err_t ret;
    ret = nvs_commit(s_nvs_handle);
    if(ret != ESP_OK) {
        __log_error("-- failed --");
        return -1;
    }
    return 0;
}

/** -------------------------------------------------------------------------- *
 * timers ports implementation
 * --------------------------------------------------------------------------- *
 */
static void* timer_init(const char* name, void* arg, void(*p_callback)(void*))
{
    TimerHandle_t handle = xTimerCreate("lora-port", 0,
        pdFALSE, arg, (void*)p_callback);
    __log_assert(handle != NULL, "timer create failed");

    __log_debug("ctor() -> new timer '%s' , handle:%p", name, handle);
    return (void*)handle;
}
static void timer_del(void* handle)
{
    __log_debug("~dtor() -> delete timer of handle:%p", handle);
    xTimerDelete(handle, 0);
}
static void timer_start(void* handle)
{
    __log_debug("start timer of handle:%p", handle);
    BaseType_t ret;
    ret = xTimerStart(handle, 0);
    __log_assert(ret == pdPASS, "-- failed --");
}
static void timer_stop(void* handle)
{
    __log_debug("stop timer of handle:%p", handle);
    BaseType_t ret;
    ret = xTimerStop(handle, portMAX_DELAY);
    __log_assert(ret == pdPASS, "-- failed --");
}
static void timer_set_period(void* handle, uint32_t msec)
{
    __log_debug("set period (%d msec) for timer of handle:%p", msec, handle);
    BaseType_t ret;
    ret = xTimerChangePeriod(handle, msec / portTICK_PERIOD_MS, 0);
    __log_assert(ret == pdPASS, "-- failed --");
}

/** -------------------------------------------------------------------------- *
 * system time ports implementation
 * --------------------------------------------------------------------------- *
 */
static uint32_t get_timestamp_msec(void)
{
    return esp_timer_get_time() / 1000U;
}
static void delay_msec(uint32_t msec)
{
    vTaskDelay(msec / portTICK_PERIOD_MS);
}

/** -------------------------------------------------------------------------- *
 * mutex ports implementation
 * --------------------------------------------------------------------------- *
 */
// static SemaphoreHandle_t s_board_cs_mutex = NULL;
// static SemaphoreHandle_t s_sync_symaphore = NULL;
static int esp32_mutex_port_ctor(void)
{
    // s_board_cs_mutex = xSemaphoreCreateMutex();
    // __log_assert(s_board_cs_mutex != NULL, "CS mutex create failed");

    // s_sync_symaphore = xSemaphoreCreateBinary();
    // __log_assert(s_sync_symaphore != NULL, "sync sem create failed");
    return 0;
}

static int esp32_mutex_port_dtor(void)
{
    return 0;
}

static void* mutex_new(void)
{
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    __log_assert(mutex != NULL, "mutex create failed");

    __log_debug("ctor() -> new mutex with handle:%p", mutex);

    return (void*)mutex;
}
static void mutex_lock(void* handle)
{
    __log_debug("lock mutex with handle:%p", handle);
    BaseType_t ret;
    ret = xSemaphoreTake(handle, portMAX_DELAY);
    __log_assert(ret == pdTRUE, "-- failed --");
}
static void mutex_unlock(void* handle)
{
    __log_debug("unlock mutex with handle:%p", handle);
    BaseType_t ret;
    ret = xSemaphoreGive(handle);
    __log_assert(ret == pdTRUE, "-- failed --");
}

static void* sem_new(void)
{
    SemaphoreHandle_t sem = xSemaphoreCreateBinary();
    __log_assert(sem != NULL, "semaphore create failed");

    __log_debug("ctor() -> new semaphore with handle:%p", sem);

    return (void*)sem;
}

static void sem_wait(void* handle)
{
    __log_debug("wait for semaphore with handle:%p", handle);
    BaseType_t ret;
    ret = xSemaphoreTake(handle, portMAX_DELAY);
    __log_assert(ret == pdTRUE, "-- failed --");
}
static void sem_signal(void* handle)
{
    __log_debug("signal the semaphore with handle:%p", handle);
    BaseType_t ret;
    ret = xSemaphoreGive(handle);
    __log_assert(ret == pdTRUE, "-- failed --");
}

/** -------------------------------------------------------------------------- *
 * crc32 port implementation
 * --------------------------------------------------------------------------- *
 */
static uint32_t crc32_calc(uint32_t initial_crc, uint8_t * buf, uint32_t len)
{
    uint32_t crc = esp_crc32_le(initial_crc, buf, len);

    __log_debug("calc crc32 ,initial_val:%d ,buf: %p ,len: %4d --> crc: %08x",
        initial_crc, buf, len, crc);

    return crc;
}

/* --- end of file ---------------------------------------------------------- */
