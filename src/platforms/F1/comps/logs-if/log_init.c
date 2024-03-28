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
 * @brief   This file contains the initialization of the log library.
 *          It contains also the hooks definitions of the micropython stdout
 *          accessors.
 * --------------------------------------------------------------------------- *
 */

/* -- includes -------------------------------------------------------------- */
#include "log_lib.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "driver/uart.h"

#include "esp_timer.h"
#include "hal/cpu_hal.h"
#include "esp_rom_uart.h"
#include "driver/uart.h"
#include "soc/uart_periph.h"

/** -------------------------------------------------------------------------- *
 * port definition for the log-lib
 * --------------------------------------------------------------------------- *
 */
static uint32_t log_get_timestamp(void)
{
    return esp_timer_get_time()/1000;
}

#ifndef MICROPYTHON_BUILD
    static void log_init_uart(void);
    #define log_access_lock     NULL
    #define log_access_unlock   NULL
    #define __log_access_guard_init()
    #define __log_access_lock()
    #define __log_access_unlock()
#else

static SemaphoreHandle_t s_log_access_mutex = NULL;
static volatile bool s_is_mutex_init = false;

#define __log_access_guard_init()                           \
    do {                                                    \
        s_log_access_mutex = xSemaphoreCreateMutex();       \
        __log_assert(s_log_access_mutex != NULL,            \
            "failed to create ioexp access guard mutex");   \
        s_is_mutex_init = true;                             \
    } while(0)
#define __log_access_lock()    \
    if(s_is_mutex_init)xSemaphoreTake(s_log_access_mutex, portMAX_DELAY)
#define __log_access_unlock()  \
    if(s_is_mutex_init)xSemaphoreGive(s_log_access_mutex)

static void log_access_lock(void)
{
    __log_access_lock();
}
static void log_access_unlock(void)
{
    __log_access_unlock();
}
#endif

/**
 * log_lib uses this function to serialize its logs, it is the same micropython
 * function uart_stdout_tx_strn() which are implemented in 
 *      micropython/ports/esp32
 */
static void log_serial_output(uint8_t* str, uint32_t len)
{
    size_t remaining = len;
    // TODO add a timeout
    for (;;) {
        int ret = uart_tx_chars(UART_NUM_0, (const char*)str, remaining);
        if (ret == -1) {
            return;
        }
        remaining -= ret;
        if (remaining <= 0) {
            break;
        }
        str += ret;

        #ifdef MICROPYTHON_BUILD
        ulTaskNotifyTake(pdFALSE, 1);
        #endif
    }
    return;
}

static const char* get_current_task_name(void)
{
    return pcTaskGetName(NULL);
}
static int get_current_core_id(void)
{
    return cpu_hal_get_core_id();
}

void init_log_system(void)
{
    #ifndef MICROPYTHON_BUILD
    log_init_uart();
    #endif

    log_init_params_t init_params = {
        .get_timestamp = log_get_timestamp,
        .mutex_lock = log_access_lock,
        .mutex_unlock = log_access_unlock,
        .serial_out = log_serial_output,
        .get_core_id = get_current_core_id,
        .get_task_name = get_current_task_name
    };

    __log_access_guard_init();

    log_init( & init_params );
}

/** -------------------------------------------------------------------------- *
 * definition of the micropython stdout accessors hooks
 * --------------------------------------------------------------------------- *
 */
void hook_mpy_uart_stdout_access_lock(void)
{
    __log_access_lock();
}

void hook_mpy_uart_stdout_access_unlock(void)
{
    __log_access_unlock();
}

/** -------------------------------------------------------------------------- *
 * uart functionality in case of native c application
 * --------------------------------------------------------------------------- *
 */
#ifndef MICROPYTHON_BUILD
static void log_init_uart(void)
{
    uart_config_t uartcfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0
    };
    uart_param_config(UART_NUM_0, &uartcfg);

    uart_driver_install(UART_NUM_0, 129, 0, 0, NULL, 0);
}
#endif

/* -- end of file ----------------------------------------------------------- */
