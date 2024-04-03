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
 * @brief   This file represents the firmware interface component to the IO
 *          Expander chip 'PCAL6408A'. It ports the driver component with the
 *          actual configuration to operate the chip properly.
 *          It also export a useful interface level so that the other Firmware
 *          components can communicate with the IO expander easily
 * --------------------------------------------------------------------------- *
 */

/* --------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdio.h>          // for standard types, etc
#include <stdbool.h>        // for bool types

#include "pcal6408a.h"      // ioexp driver header
#include "ioexp.h"          // board level ioexp header

#define __log_subsystem     F1
#define __log_component     ioexp
#include "log_lib.h"        // for logging and debugging
__log_component_def(F1, ioexp, purple, 1, 0)

#include "utils_misc.h"     // for macros string concats
#include "utils_units.h"    // for macros time units macros
#include "utils_bitwise.h"

#include "driver/i2c.h"         // for i2c transfer
#include "driver/gpio.h"        // for interrupt and reset pin
#include "hal/i2c_ll.h"

#ifdef MICROPYTHON_BUILD
#define config_get_lte_modem_enable_on_boot() false
#else
#define config_get_lte_modem_enable_on_boot() false
#endif
#include "esp_event.h"

/* --------------------------------------------------------------------------- *
 * Configuration
 * --------------------------------------------------------------------------- *
 */
// -- init 'lte' compilation switch
#ifdef __feature_lte
#define __lte__     y
#else
#define __lte__     n
#endif

// -- init 'lora' compilation switch
#ifdef __feature_lora
#define __lora__    y
#else
#define __lora__    n
#endif

// -- init 'secure-element' compilation switch
#ifdef __feature_secure_element
#define __se__      y
#else
#define __se__      n
#endif

// -- init 'interrupt capability' compilation switch
#if defined(__feature_lte) || defined(__feature_lora)
#define __int__     y
#else
#define __int__     n
#endif

/* --------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */
enum {
    high = true,
    low  = false
};

/* --------------------------------------------------------------------------- *
 * access guard
 * --------------------------------------------------------------------------- *
 */
static bool s_is_access_initialized = false;
static SemaphoreHandle_t s_ioexp_access_mutex = NULL;
#define __ioexp_access_guard_init()                         \
    do {                                                    \
        s_ioexp_access_mutex = xSemaphoreCreateMutex();     \
        __log_assert(s_ioexp_access_mutex != NULL,          \
            "failed to create ioexp access guard mutex");   \
        s_is_access_initialized = true;                     \
    } while(0)
#define __ioexp_access_lock()       \
    if(s_is_access_initialized)     \
        xSemaphoreTake(s_ioexp_access_mutex, portMAX_DELAY)
#define __ioexp_access_unlock()     \
    if(s_is_access_initialized)     \
        xSemaphoreGive(s_ioexp_access_mutex)

/* --------------------------------------------------------------------------- *
 * Pins definitions
 * --------------------------------------------------------------------------- *
 */
/**
 * The following diagram from schematics:
 *
 * +-----------+                +-------------------+
 * |      GPIO9|<--- int_n <---o|INT              P0|-----------> lora_power
 * |           |  rst_n (NA))->o|RESET            P1|-----------> lte_power
 * |           |                |                 P2|-----------> lora_reset_n
 * |   ESP32   |         +3V3---|ADDR             P3|-----------> lte_reset_n
 * |           |                |    PCAL6408A    P4|-----------> secure_en
 * |           |                |                 P5|<----------- lora_int
 * |      GPIO7|--- I2C0_CLK ---|SCL              P6|<----------- lora_free
 * |      GPIO8|--- I2C0_DAT ---|SDA              P7|<----------- lte_ring
 * +-----------+                +-------------------+
 */
#define __ioexp_pin__lora_power     (pcal6408a_port_pin_0)
#define __ioexp_pin__lte_power      (pcal6408a_port_pin_1)
#define __ioexp_pin__lora_reset_n   (pcal6408a_port_pin_2)
#define __ioexp_pin__lte_reset_n    (pcal6408a_port_pin_3)
#define __ioexp_pin__secure_en      (pcal6408a_port_pin_4)
#define __ioexp_pin__lora_int       (pcal6408a_port_pin_5)
#define __ioexp_pin__lora_free      (pcal6408a_port_pin_6)
#define __ioexp_pin__lte_ring       (pcal6408a_port_pin_7)

#define __ioexp_pin( sig_name )     __concat( __ioexp_pin__, sig_name )

#define __esp32_ioexp_i2c_port      (I2C_NUM_0)
#define __esp32_ioexp_i2c_freq      __freq_khz(400)
#define __esp32_ioexp_i2c_timeout   __time_ms(10)

#define __esp32_ioexp_scl_pin       (GPIO_NUM_7)
#define __esp32_ioexp_sda_pin       (GPIO_NUM_8)
#define __esp32_ioexp_int_pin       (GPIO_NUM_9)

/* --------------------------------------------------------------------------- *
 * PCAL6408A pins handlers
 * --------------------------------------------------------------------------- *
 */
__opt_paste(__lora__, y,
    static pcal6408a_port_pin_handle ioexp_drv_handle__lora_power  ;
    static pcal6408a_port_pin_handle ioexp_drv_handle__lora_reset_n;
    static pcal6408a_port_pin_handle ioexp_drv_handle__lora_int    ;
    static pcal6408a_port_pin_handle ioexp_drv_handle__lora_free   ;
)
__opt_paste(__lte__, y,
    static pcal6408a_port_pin_handle ioexp_drv_handle__lte_power   ;
    static pcal6408a_port_pin_handle ioexp_drv_handle__lte_reset_n ;
    static pcal6408a_port_pin_handle ioexp_drv_handle__lte_ring    ;
)
__opt_paste(__se__, y,
    static pcal6408a_port_pin_handle ioexp_drv_handle__secure_en   ;
)

#define __ioexp_drv_handle( sig ) __concat( ioexp_drv_handle__, sig )

/* --------------------------------------------------------------------------- *
 * input pins interrupt handlers and callbacks
 * --------------------------------------------------------------------------- *
 */
#define __ioexp_sig_int_callback(sig) __concat(ioexp_sig_int_callback__, sig)

__opt_paste(__lora__, y,
    static volatile ioexp_callback_t __ioexp_sig_int_callback(lora_int ) = NULL;
    static volatile ioexp_callback_t __ioexp_sig_int_callback(lora_free) = NULL;
)
__opt_paste(__lte__, y,
    static volatile ioexp_callback_t __ioexp_sig_int_callback(lte_ring ) = NULL;
)

#define __ioexp_sig_int_handler(sig) __concat(ioexp_sig_int_handler__, sig)

#if __opt_test(__lora__, y)
static void __ioexp_sig_int_handler(lora_int)(bool pin_value) {
    if( __ioexp_sig_int_callback(lora_int) )
        __ioexp_sig_int_callback(lora_int)();
}
static void __ioexp_sig_int_handler(lora_free)(bool pin_value) {
    if( __ioexp_sig_int_callback(lora_free) )
        __ioexp_sig_int_callback(lora_free)();
}
#endif

#if __opt_test(__lte__, y)
static void __ioexp_sig_int_handler(lte_ring)(bool pin_value) {
    if( __ioexp_sig_int_callback(lte_ring) )
        __ioexp_sig_int_callback(lte_ring)();
}
#endif

/* --------------------------------------------------------------------------- *
 * output signals manipulations
 * --------------------------------------------------------------------------- *
 */
#define __activate_signal(sig)                                      \
    do{                                                             \
        __ioexp_access_lock();                                      \
        pcal6408a_error_t err = pcal6408a_activate_pin(             \
            __ioexp_drv_handle( sig ));                             \
        __ioexp_access_unlock();                                    \
        if(err != __pcal6408a_ok)                                   \
            __log_error("code(%d): ioexp activate signal "#sig"\n", \
            err);                                                   \
    } while(0)

#define __deactivate_signal(sig)                                    \
    do{                                                             \
        __ioexp_access_lock();                                      \
        pcal6408a_error_t err = pcal6408a_deactivate_pin(           \
            __ioexp_drv_handle( sig ));                             \
        __ioexp_access_unlock();                                    \
        if(err != __pcal6408a_ok)                                   \
            __log_error("code(%d): ioexp activate signal "#sig"\n", \
            err);                                                   \
    } while(0)

#define __set_signal_value(sig, value)                              \
    do{                                                             \
        __ioexp_access_lock();                                      \
        pcal6408a_error_t err = pcal6408a_write(                    \
            __ioexp_drv_handle( sig ), value);                      \
        __ioexp_access_unlock();                                    \
        if(err != __pcal6408a_ok)                                   \
            __log_error("code(%d): ioexp write signal " #sig "\n",  \
            err);                                                   \
    } while(0)

#define __get_signal_value(sig, value)                              \
    do{                                                             \
        __ioexp_access_lock();                                      \
        pcal6408a_error_t err = pcal6408a_read(                     \
            __ioexp_drv_handle( sig ), &value);                     \
        __ioexp_access_unlock();                                    \
        if(err != __pcal6408a_ok)                                   \
            __log_error("code(%d): ioexp read signal " #sig "\n",   \
                err);                                               \
    } while(0)

/* --------------------------------------------------------------------------- *
 * i2c handling
 * --------------------------------------------------------------------------- *
 */
#define __esp_api_call(__api_call, __err_msg, __ret)        \
    do {                                                    \
        esp_err_t err = __api_call;                         \
        if( err != ESP_OK ) {                               \
            __log_error("(err_code:%d)" __err_msg, err);    \
            return __ret;                                   \
        }                                                   \
    } while (0)

static void pcal6408a_i2c_read_port(
    uint8_t dev_addr, uint8_t* buff, uint32_t cbytes)
{
    __log_debug("i2c [r] [addr: %02x] [len: %d] [byte1: %02x]"
        , dev_addr, cbytes, buff[0]);

    __esp_api_call( i2c_master_read_from_device(__esp32_ioexp_i2c_port,
        dev_addr, buff, cbytes, portMAX_DELAY), "i2c master read error", );
}

static void pcal6408a_i2c_write_port
    (uint8_t dev_addr, uint8_t* buff, uint32_t cbytes)
{
    __log_debug("i2c [w] [addr: %02x] [len: %d] [byte1: %02x]",
        dev_addr, cbytes, buff[0]);

    __esp_api_call( i2c_master_write_to_device(__esp32_ioexp_i2c_port,
        dev_addr, buff, cbytes, portMAX_DELAY), "i2c master write error", );
}

static void esp32_ioexp_i2c_ctor(void)
{
    i2c_config_t i2c_cfg = {
        .scl_io_num = __esp32_ioexp_scl_pin,
        .sda_io_num = __esp32_ioexp_sda_pin,
        .mode = I2C_MODE_MASTER,
        /**
         * there is already an external pull-up resistors.
         */
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = __esp32_ioexp_i2c_freq
    };

    __esp_api_call(i2c_param_config(__esp32_ioexp_i2c_port, &i2c_cfg),
        "i2c param config error", );

    int timeout = __time2cycles( __esp32_ioexp_i2c_timeout, I2C_APB_CLK_FREQ );
    __log_debug("i2c timeout : %d cycles", timeout);

    timeout = timeout > I2C_LL_MAX_TIMEOUT ? I2C_LL_MAX_TIMEOUT : timeout;

    __esp_api_call(i2c_set_timeout(__esp32_ioexp_i2c_port, timeout),
        "i2c set timeout error", );

    __esp_api_call( i2c_driver_install(__esp32_ioexp_i2c_port, I2C_MODE_MASTER,
        0, 0, 0), "i2c driver install error", );
}

static void esp32_ioexp_i2c_dtor(void)
{
    __log_info("dtor() -> i2c pins");

    __esp_api_call( i2c_driver_delete(__esp32_ioexp_i2c_port),
        "i2c driver delete error", );

    __esp_api_call(gpio_reset_pin(__esp32_ioexp_scl_pin),
        "gpio reset pin error", );
    __esp_api_call(gpio_reset_pin(__esp32_ioexp_sda_pin),
        "gpio reset pin error", );
}

/* --------------------------------------------------------------------------- *
 * ioexp gpio interrupt and task handlers
 * --------------------------------------------------------------------------- *
 */
#define __esp_call_assert(__api_call, __err_msg)    \
    do {                                            \
        esp_err_t err = __api_call;                 \
        __log_assert(err == ESP_OK,                 \
            "(err_code:%d)" __err_msg, err);        \
    } while (0)

#if __opt_test(__int__, y)
static SemaphoreHandle_t s_ioexp_sync_sem_handle = NULL;
static TaskHandle_t s_ioexp_task_handle = NULL;
static void esp32_ioexp_task(void * arg)
{
    while(1)
    {
        if(xSemaphoreTake(s_ioexp_sync_sem_handle, portMAX_DELAY)== pdTRUE)
        {
            __ioexp_access_lock();
            __log_debug("interrupt event");
            pcal6408a_interrupt_trigger_port();
            __ioexp_access_unlock();
        }
    }
}

static void esp32_ioexp_task_init(void)
{
    s_ioexp_sync_sem_handle = xSemaphoreCreateBinary();
    __log_assert(s_ioexp_sync_sem_handle != NULL,
        "failed to create ioexp sync sem");

    xTaskCreate(
        esp32_ioexp_task,       // task function
        "ioexp-task",           // task name
        4 * 1024,               // stack size
        NULL,                   // parameter to task function
        configMAX_PRIORITIES,   // priority
        &s_ioexp_task_handle    // handle to the task
        );
    configASSERT( s_ioexp_task_handle );
}

static void esp32_ioexp_int_handler(void * args)
{
    (void)args;
    static BaseType_t xHigherPriorityTaskWoken;
    if(xSemaphoreGiveFromISR(s_ioexp_sync_sem_handle,
        &xHigherPriorityTaskWoken) != pdTRUE)
    {
        __log_error("ioexp: failed to give sem from isr");
    }
}

static void esp32_ioexp_gpio_ctor(void)
{
    __log_info("ctor() -> int gpio");
    /* configure pin */
    gpio_config_t cfg = {
        .pin_bit_mask = 1u << __esp32_ioexp_int_pin,    /* pin mask */
        .mode = GPIO_MODE_INPUT,                /* input to esp32 */
        /* there is an external pull up resistor is connected,
         * hence no need to activate esp32 pull resistor */
        .pull_up_en = GPIO_PULLUP_DISABLE,      /* disable pull up resistor */
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  /* disable pull down resistor */
        .intr_type = GPIO_INTR_NEGEDGE        /* active low interrupt */
        };
    __esp_call_assert( gpio_config( & cfg ),
        "error: ioexp esp32 interrupt pin config");

    /* installing isr handler and service priority level */
    __esp_call_assert( gpio_isr_handler_add(__esp32_ioexp_int_pin,
            esp32_ioexp_int_handler, NULL),
        "error: ioexp esp32 isr add handler");

    /* enable wakeup capability */
    // err = gpio_wakeup_enable(__esp32_ioexp_int_pin, GPIO_INTR_LOW_LEVEL);
    // __log_assert(err == ESP_OK, "error: ioexp esp32 interrupt enable");

    /* enabling the interrupt */
    __esp_call_assert( gpio_intr_enable(__esp32_ioexp_int_pin),
        "error: ioexp esp32 interrupt enable");
}

static void esp32_ioexp_gpio_dtor(void)
{
    __log_info("dtor() -> int gpio");

    __esp_call_assert( gpio_isr_handler_remove(__esp32_ioexp_int_pin),
        "error: ioexp esp32 remove interrupt handler");

    __esp_call_assert( gpio_reset_pin(__esp32_ioexp_int_pin),
        "error: ioexp esp32 interrupt enable");
}
#endif /* __int__ */

/* --------------------------------------------------------------------------- *
 * ioexp esp32 level power management
 * --------------------------------------------------------------------------- *
 */
typedef enum {   
    __opt_paste(__lte__, y,  __IOEXP_LTE_POWER_ON,)
    __opt_paste(__lte__, y,  __IOEXP_LTE_POWER_OFF,)
    __opt_paste(__lora__, y, __IOEXP_LORA_POWER_ON,)
    __opt_paste(__lora__, y, __IOEXP_LORA_POWER_OFF,)
    __opt_paste(__se__, y,   __IOEXP_SE_POWER_ON,)
    __opt_paste(__se__, y,   __IOEXP_SE_POWER_OFF,)
    __IOEXP_MPY_I2C_ON,
    __IOEXP_MPY_I2C_OFF,
    __IOEXP_I2C_ON,
    __IOEXP_I2C_OFF,
} ioexp_power_manage_t;

__opt_paste(__lte__, y,  static bool s_lte_on = false;)
__opt_paste(__lora__, y, static bool s_lora_on = false;)
__opt_paste(__se__, y,   static bool s_se_on = false;)
static bool s_mpy_i2c_on = false;
static bool s_i2c_on = false;

__opt_paste(__int__,y, static bool s_esp32_gpio_is_on = false;)
static bool s_esp32_i2c_is_on = false;

static void ioexp_manage_power(ioexp_power_manage_t req)
{
    __ioexp_access_lock();

    switch(req)
    {
    __opt_paste(__lte__, y,
        case __IOEXP_LTE_POWER_ON:      s_lte_on = true;      break;
        case __IOEXP_LTE_POWER_OFF:     s_lte_on = false;     break;
    )
    __opt_paste(__lora__, y,
        case __IOEXP_LORA_POWER_ON:     s_lora_on = true;     break;
        case __IOEXP_LORA_POWER_OFF:    s_lora_on = false;    break;
    )
    __opt_paste(__se__, y,
        case __IOEXP_SE_POWER_ON:       s_se_on = true;       break;
        case __IOEXP_SE_POWER_OFF:      s_se_on = false;      break;
    )
    case __IOEXP_MPY_I2C_ON:        s_mpy_i2c_on = true;  break;
    case __IOEXP_MPY_I2C_OFF:       s_mpy_i2c_on = false; break;
    case __IOEXP_I2C_ON:            s_i2c_on = true;      break;
    case __IOEXP_I2C_OFF:           s_i2c_on = false;     break;
    }

    if( 0
        __opt_paste(__lte__, y, || s_lte_on)
        __opt_paste(__lora__, y,|| s_lora_on)
        __opt_paste(__se__, y,  || s_se_on ))
    {
        goto __ctor_all;
    }
    else if( s_mpy_i2c_on || s_i2c_on )
    {
        goto __ctor_i2c;
    }
    goto __dtor_all;

    __ctor_all:
        #if __opt_test(__int__, y)
        if(!s_esp32_gpio_is_on)
        {
            esp32_ioexp_gpio_ctor();
            s_esp32_gpio_is_on = true;
        }
        #endif
        goto __ctor_i2c_only;
    __ctor_i2c:
        #if __opt_test(__int__, y)
        if(s_esp32_gpio_is_on)
        {
            esp32_ioexp_gpio_dtor();
            s_esp32_gpio_is_on = false;
        }
        #endif
    __ctor_i2c_only:
        if(!s_esp32_i2c_is_on)
        {
            esp32_ioexp_i2c_ctor();
            s_esp32_i2c_is_on = true;
        }
        goto __end__;

    __dtor_all:
        #if __opt_test(__int__, y)
        if(s_esp32_gpio_is_on)
        {
            esp32_ioexp_gpio_dtor();
            s_esp32_gpio_is_on = false;
        }
        #endif
        if(s_esp32_i2c_is_on)
        {
            esp32_ioexp_i2c_dtor();
            s_esp32_i2c_is_on = false;
        }

    __end__:
        __ioexp_access_unlock();
}

/* --------------------------------------------------------------------------- *
 * ioexp module and driver init
 * --------------------------------------------------------------------------- *
 */
static void ioexp_driver_ctor(void);

void ioexp_init(void)
{
    static bool s_ioexp_initialized = false;

    if( ! s_ioexp_initialized )
    {
        __ioexp_access_guard_init();

        __opt_paste(__int__, y,
            esp32_ioexp_task_init();
        )

        ioexp_manage_power(__IOEXP_I2C_ON);
        ioexp_driver_ctor();
        ioexp_manage_power(__IOEXP_I2C_OFF);

        s_ioexp_initialized = true;
    }
}

static void ioexp_driver_ctor(void)
{
    __log_info("ctor() -> ioexp driver");

    /* init ioexp pcal6408a driver */
    pcal6408a_init_config_t cfg = {
        .is_addr_pin_connected_to_vdd = true,
        .is_open_drain_output = false,
        .i2c_read = pcal6408a_i2c_read_port,
        .i2c_write = pcal6408a_i2c_write_port
    };

    __log_assert( pcal6408a_init( & cfg ) == __pcal6408a_ok,
        "ioexp init failed" );

    /* init all wired pins */
    #define __config_ioexp_output_pin(sig, cap_level)                   \
        __ioexp_drv_handle( sig ) =                                     \
            pcal6408a_configure_output_port_pin (                       \
                __ioexp_pin( sig ),                                     \
                __concat( pcal6408a_pull_resistor_, none ),             \
                __concat( pcal6408a_drive_capability_level_, cap_level),\
                #sig                                                    \
                );                                                      \
        __log_assert( __ioexp_drv_handle( sig ) != NULL,                \
            "ioexp " #sig "pin init failed")

    __opt_paste(__lora__, y,
        __config_ioexp_output_pin(lora_power,   0);
        __config_ioexp_output_pin(lora_reset_n, 0);
    )
    __opt_paste(__lte__, y,
        __config_ioexp_output_pin(lte_power,    0);
        __config_ioexp_output_pin(lte_reset_n,  0);
    )
    __opt_paste(__se__, y,
        __config_ioexp_output_pin(secure_en,    0);
    )
    #undef __config_ioexp_output_pin

    #define __config_ioexp_input_pin(sig, pull, inv ,is_latch)          \
        __ioexp_drv_handle( sig ) =                                     \
            pcal6408a_configure_input_port_pin(                         \
                __ioexp_pin( sig ),                                     \
                __concat( pcal6408a_pull_resistor_, pull ),             \
                inv, is_latch,                                          \
                __ioexp_sig_int_handler( sig ),                         \
                #sig                                                    \
                );                                                      \
        __log_assert( __ioexp_drv_handle( sig ) != NULL,                \
            "ioexp " #sig "pin init failed")

    // --------------------- signal     pull-resistor inv    latched-input
    __opt_paste(__lora__, y,
        __config_ioexp_input_pin(lora_int,  down,         false,  true);
        __config_ioexp_input_pin(lora_free, down,         true,   true);
    )
    __opt_paste(__lte__, y,
        __config_ioexp_input_pin(lte_ring,  down,         false,  true);
    )
    #undef __config_ioexp_input_pin

    // -- chips initialization
    #if __opt_test(__lte__, y)
    if( config_get_lte_modem_enable_on_boot() )
    {
        ioexp_lte_chip_power_on();
    }
    else
    {
        ioexp_lte_chip_power_off();
    }
    #endif

    /* the lora and secure chips will be kept off until there is a certain
     * customer use-case needs to keep any of them on during the RST or POR
     */

    __opt_paste(__lora__, y, ioexp_lora_chip_power_off();)
    __opt_paste(__se__, y,   ioexp_secure_chip_disable();)
}

void ioexp_reset(void)
{
    __log_info("ioexp reset");
    pcal6408a_reset();
}

bool ioexp_micropython_req_i2c_init(int port, int scl, int sda,
    uint32_t freq, uint32_t timeout_ms)
{
    __log_info("I2C(%d): SCL(%d), SDA(%d), Freq(%d kHz), Bus-timeout(%d ms)\n",
        port, scl, sda, __to_khz(freq), timeout_ms);

    if( port == __esp32_ioexp_i2c_port &&
        scl == __esp32_ioexp_scl_pin &&
        sda == __esp32_ioexp_sda_pin &&
        (freq == 0 || freq == __esp32_ioexp_i2c_freq) &&
        (timeout_ms == 0 || timeout_ms == __to_ms(__esp32_ioexp_i2c_timeout)) )
    {
        ioexp_manage_power(__IOEXP_MPY_I2C_ON);
        return true;
    }

    __log_output("error, io-expander uses the same i2c bus with "
        "I2C("__cyan__"%d"__default__"): "
        "SCL("__yellow__"%d"__default__"), SDA("__yellow__"%d"__default__"), "
        "Freq("__green__"%d kHz"__default__"), "
        "Bus-timeout("__purple__"%d ms"__default__")\n",
        __esp32_ioexp_i2c_port, __esp32_ioexp_scl_pin, __esp32_ioexp_sda_pin,
        __to_khz(__esp32_ioexp_i2c_freq), __to_ms(__esp32_ioexp_i2c_timeout));

    return false;
}

void ioexp_micropython_req_i2c_deinit(void)
{
    ioexp_manage_power(__IOEXP_MPY_I2C_OFF);
}


/* --------------------------------------------------------------------------- *
 * ioexp connected chips exported controls
 * --------------------------------------------------------------------------- *
 */
/**
 * lora chip ioexp controls
 */
#if __opt_test(__lora__, y)
void ioexp_lora_chip_power_on(void)
{
    __log_info("lora on");
    ioexp_manage_power(__IOEXP_LORA_POWER_ON);

    // -- activate the associated lora chip signals
    __activate_signal( lora_power );
    __activate_signal( lora_reset_n );
    __activate_signal( lora_int );
    __activate_signal( lora_free );

    // -- init the reset signal
    __set_signal_value( lora_reset_n, high );

    // -- power on the chip
    __set_signal_value( lora_power, high );
}
void ioexp_lora_chip_power_off(void)
{
    __log_info("lora off");

    // -- deactivate interrupts signals before lora powering down, to prevent
    //    spontanuous interrupts during chip power off
    __deactivate_signal( lora_int );
    __deactivate_signal( lora_free );
    __deactivate_signal( lora_reset_n );

    // -- power off the chip
    __set_signal_value( lora_power, low );

    // -- deactivate the associated signals
    __deactivate_signal( lora_power );

    ioexp_manage_power(__IOEXP_LORA_POWER_OFF);
}
bool ioexp_lora_chip_power_status(void)
{
    return s_lora_on == true;
}
void ioexp_lora_chip_reset(void)
{
    __log_info("lora reset");

    if(!ioexp_lora_chip_power_status())
    {
        __log_output(__red__
            "-- lora is powered down, try power up first --"__default__"\n");
        return;
    }

    __set_signal_value( lora_reset_n, low );
    /**
     * according to the datasheet of the LoRa chip sx1262, the NRESET signal
     * shall be held low for typically 100 μs for the reset to happen
     * we will apply a delay of about 110 μs
     */
    #define __lora_chip_reset_delay (110)
    vTaskDelay(__lora_chip_reset_delay / (1000 * portTICK_PERIOD_MS) );
    // mp_hal_delay_us( __lora_chip_reset_delay );
    __set_signal_value( lora_reset_n, high );
}
void ioexp_lora_chip_set_int_signal_callback( ioexp_callback_t cb )
{
    __ioexp_sig_int_callback(lora_int) = cb;
}
void ioexp_lora_chip_set_busy_signal_callback( ioexp_callback_t cb )
{
    __ioexp_sig_int_callback(lora_free) = cb;
}
bool ioexp_lora_chip_is_busy(void)
{
    if(!ioexp_lora_chip_power_status())
    {
        __log_output(__red__
            "-- lora is powered down, try power up first --"__default__"\n");
        return false;
    }

    bool value = false;
    __get_signal_value( lora_free, value );
    return !value;
}
bool ioexp_lora_chip_read_int_pin( void )
{
    bool value = false;
    __get_signal_value( lora_int, value );
    return value;
}
#endif

/**
 * lte chip ioexp controls
 */
#if __opt_test(__lte__, y)
void ioexp_lte_chip_power_on(void)
{
    __log_info("lte on");
    ioexp_manage_power(__IOEXP_LTE_POWER_ON);

    // -- activate associated lte signals
    __activate_signal( lte_ring );
    __activate_signal( lte_reset_n );
    __activate_signal( lte_power );

    // -- init the reset signal
    __set_signal_value( lte_reset_n, high );

    // -- power on the chip
    __set_signal_value( lte_power, high );
}

void ioexp_lte_chip_power_off(void)
{
    __log_info("lte off");

    // -- power off the chip
    __set_signal_value( lte_power, low );

    // -- deactivate associated lte signals
    __deactivate_signal( lte_power );
    __deactivate_signal( lte_reset_n );
    __deactivate_signal( lte_ring );

    ioexp_manage_power(__IOEXP_LTE_POWER_OFF);
}
bool ioexp_lte_chip_power_status(void)
{
    return s_lte_on == true;
}
void ioexp_lte_chip_reset(void)
{
    __log_info("lte reset");

    if(!ioexp_lte_chip_power_status())
    {
        __log_output(__red__
            "-- lte is powered down, try power up first --"__default__"\n");
        return;
    }

    __set_signal_value( lte_reset_n, low );
    #define __lte_chip_reset_delay (100)
    vTaskDelay(__lte_chip_reset_delay / (1000 * portTICK_PERIOD_MS) );
    // mp_hal_delay_us( __lte_chip_reset_delay );
    __set_signal_value( lte_reset_n, high );
}
void ioexp_lte_chip_set_ring_signal_callback( ioexp_callback_t cb )
{
    __ioexp_sig_int_callback(lte_ring) = cb;
}
#endif

/**
 * secure chip ioexp controls
 */
#if __opt_test(__se__, y)
void ioexp_secure_chip_enable(void)
{
    __log_info("SE on");
    ioexp_manage_power(__IOEXP_SE_POWER_ON);

    // -- activate the associated secure chip signals
    __activate_signal( secure_en );

    // -- power on the chip
    __set_signal_value( secure_en, high );
}

void ioexp_secure_chip_disable(void)
{
    __log_info("SE off");

    // -- power off the secure chip
    __set_signal_value( secure_en, low );

    // -- deactivate the associated secure chip signals
    __deactivate_signal( secure_en );

    ioexp_manage_power(__IOEXP_SE_POWER_OFF);
}
bool ioexp_secure_chip_status(void)
{
    return s_se_on == true;
}
#endif

void ioexp_stats(void)
{
    __log_output_header("current running entities", 65, '=');
    __opt_paste(__lora__, y,
        __log_output("  >> %-20s %s\n", "lora", g_on_off[s_lora_on]);
    )
    __opt_paste(__lte__, y,
        __log_output("  >> %-20s %s\n", "lte", g_on_off[s_lte_on]);
    )
    __opt_paste(__se__, y,
        __log_output("  >> %-20s %s\n", "secure element", g_on_off[s_se_on]);
    )
    __log_output("  >> %-20s %s\n", "uPython i2c req", g_on_off[s_mpy_i2c_on]);
    __log_output("  >> %-20s %s\n", "ioexp i2c only", g_on_off[s_i2c_on]);
    __log_output("\n");
    __log_output("  == I2C      %s\n", g_on_off[s_esp32_i2c_is_on]);
    __opt_paste(__int__, y,
        __log_output("  == INT GPIO %s\n", g_on_off[s_esp32_gpio_is_on]);
    )

    ioexp_manage_power(__IOEXP_I2C_ON);
    pcal6408a_stats();
    ioexp_manage_power(__IOEXP_I2C_OFF);
}

/* --- end of file ---------------------------------------------------------- */
