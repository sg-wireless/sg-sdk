/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   uart interface sub-component
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "esp_system.h"
#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "string.h"
#include "driver/gpio.h"

#define __log_subsystem     lte
#define __log_component     uart
#include "log_lib.h"
__log_component_def(lte, uart, green, 1, 1);

/** -------------------------------------------------------------------------- *
 * LTE GM02S Chip connection configuration with the ESP32-S3
 * --------------------------------------------------------------------------- *
 */

// -- uart0 default configs
#define __gm02s_uart0_default_baud_rate     (115200)
#define __gm02s_uart0_default_data_bits     (UART_DATA_8_BITS)
#define __gm02s_uart0_default_parity        (UART_PARITY_DISABLE)
#define __gm02s_uart0_default_stop_bits     (UART_STOP_BITS_1)
#define __gm02s_uart0_default_flow_ctrl     (UART_HW_FLOWCTRL_CTS_RTS)

// -- uart0 signals
#define __gm02s_sig_txd0                    (GPIO_NUM_47)
#define __gm02s_sig_rxd0                    (GPIO_NUM_48)
#define __gm02s_sig_rts0                    (GPIO_NUM_33)
#define __gm02s_sig_cts0                    (GPIO_NUM_6)

// -- mcu configs
#define __gm02s_uart                        1
#define __gm02s_uart_num                    __concat(UART_NUM_, __gm02s_uart)
#define __gm02s_uart_dev                    __concat(UART, __gm02s_uart)
#define __gm02s_uart_rx_buf_size            (SOC_UART_FIFO_LEN * 2)
#define __gm02s_uart_tx_buf_size            (256)

#if __gm02s_uart_rx_buf_size && (__gm02s_uart_rx_buf_size <= SOC_UART_FIFO_LEN)
    #error "-- uart rx buf len must be greater than SOC_UART_FIFO_LEN"
#endif
#if __gm02s_uart_tx_buf_size && (__gm02s_uart_tx_buf_size <= SOC_UART_FIFO_LEN)
    #error "-- uart rx buf len must be greater than SOC_UART_FIFO_LEN"
#endif


#define __esp_api_call(__api_call, __err_msg, __ret)        \
    do {                                                    \
        esp_err_t err = __api_call;                         \
        if( err != ESP_OK ) {                               \
            __log_error("(err_code:%d)" __err_msg, err);    \
            return __ret;                                   \
        }                                                   \
    } while (0)

static bool s_initialized = false;

/** -------------------------------------------------------------------------- *
 * APIs implementation
 * --------------------------------------------------------------------------- *
 */

void lte_uart_init(uint32_t baudrate)
{
    if(s_initialized)
    {
        return;
    }
    __log_info("ctor() -> lte uart");

    uart_config_t uart_config = {
        .baud_rate  = baudrate ? baudrate : __gm02s_uart0_default_baud_rate,
        .data_bits  = __gm02s_uart0_default_data_bits,
        .parity     = __gm02s_uart0_default_parity,
        .stop_bits  = __gm02s_uart0_default_stop_bits,
        .flow_ctrl  = __gm02s_uart0_default_flow_ctrl,
        .source_clk = UART_SCLK_APB,
        .rx_flow_ctrl_thresh = UART_FIFO_LEN - UART_FIFO_LEN / 4
    };

    __esp_api_call(uart_param_config(__gm02s_uart_num, &uart_config),
        "failed to config uart",);

    __esp_api_call(uart_driver_install(__gm02s_uart_num,
        SOC_UART_FIFO_LEN * 2, 0, 0, NULL, 0),
        "failed to install uart driver",);

    __esp_api_call(uart_set_pin(__gm02s_uart_num,
        __gm02s_sig_txd0, __gm02s_sig_rxd0,
        __gm02s_sig_rts0, __gm02s_sig_cts0),
        "failed to set uart pins",);
    
    s_initialized = true;
}

void lte_uart_deinit(void)
{
    if(!s_initialized)
    {
        return;
    }
    __log_info("dtor() -> lte uart");

    __esp_api_call(uart_driver_delete(__gm02s_uart_num),
        "failed to delete uart driver",);
}

int lte_uart_read(uint8_t*buf, uint32_t len, uint32_t timeout_ms)
{
    if(!s_initialized)
    {
        __log_error("uart driver interface is not initialized");
        return 0;
    }

    TickType_t ticks = 0;
    if(timeout_ms) {
        ticks = pdMS_TO_TICKS(timeout_ms);
    }

    int bytes = uart_read_bytes(__gm02s_uart_num, buf, len, ticks);
    if(bytes < 0) {
        __log_error("error occurred in reading operation");
        return 0;
    }
    return bytes;
}

int lte_uart_write(const uint8_t*buf, uint32_t len)
{
    if(!s_initialized) {
        __log_error("uart driver interface is not initialized");
        return 0;
    }

    int bytes = uart_write_bytes(__gm02s_uart_num, buf, len);
    if(bytes < 0) {
        __log_error("error occurred in writing operation");
        return 0;
    }
    return bytes;
}

int lte_uart_any(void)
{
    if(!s_initialized) {
        __log_error("uart driver interface is not initialized");
        return 0;
    }

    size_t size;
    uart_get_buffered_data_len(__gm02s_uart_num, &size);
    return size;
}

int lte_uart_flush(void)
{
    uint32_t baudrate;
    uart_get_baudrate(__gm02s_uart_num, &baudrate);
    uint32_t timeout = (3 ) * 13000 * 2 / baudrate;
    if(uart_wait_tx_done(__gm02s_uart_num, timeout) == ESP_OK) {
        return 0;
    }
    return -1;
}

/* --- end of file ---------------------------------------------------------- */
