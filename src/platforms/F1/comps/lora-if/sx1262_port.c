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
 * @brief   This file defines the port interface component between the Semtech
 *          chip sx126x and the F1 platform.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "soc/soc_caps.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "ioexp.h"
#include "sx126x_port.h"

#define __log_subsystem     lora
#define __log_component     port_sx126x
#include "log_lib.h"

/* --- configs -------------------------------------------------------------- */

#define __enable_spi_single_byte_trx_impl   (0)

/* spi signal timing params */
#define __sx1262_esp32_spi_clk              (SPI_MASTER_FREQ_8M) /* upto 16M */
#define __sx1262_esp32_spi_cpol             (0)
#define __sx1262_esp32_spi_cpha             (0)
#define __sx1262_esp32_spi_mode             \
    ( (__sx1262_esp32_spi_cpol << 1) | __sx1262_esp32_spi_cpha )

/** -------------------------------------------------------------------------- *
 * This is the physical connections between the ESP32 and the LoRa Chip
 *      +------------------+                          +--------------------+
 *      |               34 |--------- lora_cs_n ----->| NSS                |
 *      |               35 |--------- lora_si --------| MOSI               |
 *      |               36 |--------- lora_sck -------| SCK    LoRa Chip   |
 *      |    ESP32s3    37 |--------- lora_miso ------| MISO    sx1262     |
 *      |                  |                          |                    |
 *      |                  |                          |  BUSY  ---------+  |
 *      |                  |<<========++              |  DIO1  ------+  |  |
 *      |                  |          ||              |  NRESET --+  |  |  |
 *      |                  |          ||              |  POWER-+  |  |  |  |
 *      +------------------+          ||              +--------+--+--+--+--+
 *                                    ||                       |  |  |  |
 *                                    ||                       |  |  |  |
 *                                    ||              +--------+--+--+--+--+
 *                                    ||              |        0  2  5  6  |
 *                                    ++============>>|                    |
 *                                                    |     IO-Expander    |
 *                                                    |                    |
 *                                                    +--------------------+
 * --------------------------------------------------------------------------- *
 */
#define __esp32_spi_pin_ss          (34)
#define __esp32_spi_pin_mosi        (35)
#define __esp32_spi_pin_sclk        (36)
#define __esp32_spi_pin_miso        (37)

/* --- sx1262 port main init porc ------------------------------------------- */

static void sx1262_spi_bus_ctor( void );
static void sx1262_spi_bus_dtor( void );
static void sx1262_gpios_ctor( void );
static void sx1262_gpios_dtor( void );
static int  sx1262_spi_trx( sx126x_spi_trx_t* p_trx );
static void sx1262_wait_on_busy( void );
static void sx1262_send_reset_pulse( void );
static bool sx126x_port_int_pin_state ( void );

void sgw3501_lora_sx1262_ctor(void)
{
    __log_info(__green__"ctor() -> sx1262 chip");

    sx1262_spi_bus_ctor();
    sx1262_gpios_ctor();

    sx126x_port_t port_params = {
        .p_spi_trx = sx1262_spi_trx,
        .p_wait_on_busy_signal = sx1262_wait_on_busy,
        .p_send_reset_pulse = sx1262_send_reset_pulse,
        .p_get_int_pin_state = sx126x_port_int_pin_state
    };
    sx126x_port_init( & port_params );
}

void sgw3501_lora_sx1262_dtor(void)
{
    __log_info(__green__"dtor() -> sx1262 chip");

    sx1262_gpios_dtor();
    sx1262_spi_bus_dtor();
}

/** -------------------------------------------------------------------------- *
 * ESP32 SPI Lora Integration
 * --------------------------------------------------------------------------- *
 */
#undef __log_subsystem
#undef __log_component
#define __log_subsystem     lora
#define __log_component     port_spi

#define __no_ret_value
#define __esp_api_call(__api_call, __err_msg, __ret)    \
do {                                                    \
    esp_err_t err = __api_call;                         \
    if( err != ESP_OK ) {                               \
        __log_error("(err_code:%d)" __err_msg, err);    \
        return __ret;                                   \
    }                                                   \
} while (0)

static spi_device_handle_t s_spi_dev_handle;

#define __spi_max_buf_size  SOC_SPI_MAXIMUM_BUFFER_SIZE

static void sx1262_spi_bus_ctor( void )
{
    __log_info("ctor() -> spi bus");
    spi_bus_config_t bus_cfg = {
        .miso_io_num = __esp32_spi_pin_miso,
        .mosi_io_num = __esp32_spi_pin_mosi,
        .sclk_io_num = __esp32_spi_pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    __esp_api_call(
        spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO),
        "esp32 spi bus init failed", __no_ret_value);

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = __sx1262_esp32_spi_clk,
        .mode = __sx1262_esp32_spi_mode,
        .spics_io_num = __esp32_spi_pin_ss,
        .queue_size = 1,
        .flags = 0,
        .command_bits = 8,
        .cs_ena_pretrans = 0
    };

    __esp_api_call(
        spi_bus_add_device(SPI2_HOST, &dev_config, &s_spi_dev_handle),
        "error: sx1262 port: esp32 spi device add", __no_ret_value);
}

static void sx1262_spi_bus_dtor( void )
{
    __log_info("dtor() -> spi bus");

    __esp_api_call(
        spi_bus_remove_device(s_spi_dev_handle),
        "error: sx1262 port: esp32 spi device remove", __no_ret_value);
    s_spi_dev_handle = NULL;
    
    __esp_api_call(spi_bus_free(SPI2_HOST),
        "esp32 spi bus free failed", __no_ret_value);
}

static int sx1262_spi_trx( sx126x_spi_trx_t* p_trx )
{
    bool has_address  = p_trx->flags & __sx126x_spi_trx_has_address;
    bool has_rx_byte1 = p_trx->flags & __sx126x_spi_trx_has_rx_byte1;
    bool has_rx_dummy = p_trx->flags & __sx126x_spi_trx_has_dummy_rx_byte1;
    bool has_pretrans = p_trx->flags & __sx126x_spi_trx_has_cs_pretrans_ms;
    uint8_t *tx_buf = p_trx->tx_buffer;
    uint8_t *rx_buf = p_trx->rx_buffer;

    __log_assert( (tx_buf == NULL) ^ (rx_buf == NULL),
        "sx1262 spi transaction should be half duplex");

    __log_printf("spi trx "
                "["__red__"%c"__default__"] "
                "[cmd: "__yellow__"%02x"__default__"] "
                "[addr: "__green__,
        rx_buf || has_rx_byte1 ? 'R' : tx_buf ? 'W' : '-', p_trx->command);
    if(has_address) {
        if(p_trx->address_bits == 16)
            __log_printf("%04X", p_trx->address);
        else if(p_trx->address_bits == 8)
            __log_printf("--%02X", p_trx->address);
    } else
        __log_printf("----");
    __log_printf(__default__"] [size: "__green__"%3d"__default__"]",
        p_trx->size);
    if(tx_buf)
        __log_dump(tx_buf, p_trx->size, 16, __log_dump_flag_hide_address,
            __word_len_8);

    __esp_api_call( spi_device_acquire_bus(s_spi_dev_handle, portMAX_DELAY),
        "spi bus acquire", !0);

    spi_transaction_ext_t trx_desc = {
        .base = {
            .cmd = p_trx->command,  /* command byte */
            .rx_buffer = rx_buf,    /* rx buffer if any */
            .tx_buffer = tx_buf,    /* tx buffer if any */
            .flags = 0
        }
    };
    spi_transaction_t * p_base = & trx_desc.base;
    uint32_t rem_size = p_trx->size;
    uint32_t offset = 0;
    uint32_t first_trx_bytes = 0;

    // -- prepare first bus transaction
    if( has_pretrans ) {
        gpio_set_level(__esp32_spi_pin_ss, 0);
        vTaskDelay(p_trx->cs_enable_pretrans_ms / portTICK_PERIOD_MS);
    }

    if( has_address ) {
        p_base->flags |= SPI_TRANS_VARIABLE_ADDR;
        trx_desc.address_bits = p_trx->address_bits;
        p_base->addr = p_trx->address;
    }

    if(has_rx_dummy) {
        p_base->flags |= SPI_TRANS_VARIABLE_DUMMY;
        trx_desc.dummy_bits = 8;
    }

    if( has_rx_byte1 ) {
        p_base->flags |= SPI_TRANS_USE_RXDATA;
        p_base->length = 8;
    }

    if( rem_size ) {
        if( rx_buf ) {
            p_base->flags |= SPI_TRANS_USE_RXDATA;
            uint8_t rx_window = 4 - (has_rx_byte1 == true);
            first_trx_bytes = rem_size < rx_window ? rem_size : rx_window;
        }else if(tx_buf) {
            p_base->flags |= SPI_TRANS_USE_TXDATA;
            first_trx_bytes = rem_size < 4 ? rem_size : 4;
            memcpy( p_base->tx_data, tx_buf, first_trx_bytes );
        }
        p_base->length += first_trx_bytes << 3;
        rem_size -= first_trx_bytes;
        offset += first_trx_bytes;
    }

    if( rem_size ) {
        p_base->flags |= SPI_TRANS_CS_KEEP_ACTIVE;
    }

    __log_debug("1st cycle => len: %d , rem_size: %d, has_rx_byte1: %d, "
        "has_dummy: %d", first_trx_bytes, rem_size,
        has_rx_byte1 == true, has_rx_dummy == true);

    __esp_api_call(
        spi_device_polling_transmit(s_spi_dev_handle, &trx_desc.base),
        "spi trx polling", !0);

    if( has_rx_byte1 ) {
        p_trx->rx_byte1 = p_base->rx_data[0];
    }

    if( rx_buf && first_trx_bytes ) {
        memcpy( rx_buf, &p_base->rx_data[ has_rx_byte1 == true ],
            first_trx_bytes );
    }

    if(rem_size) {
        trx_desc.command_bits = 0;
        trx_desc.address_bits = 0;
        trx_desc.dummy_bits = 0;
        p_base->flags |= SPI_TRANS_VARIABLE_CMD;

        uint32_t len;
        do {
            p_base->rxlength = 0;
            len = 0;

            if(rem_size >= __spi_max_buf_size) {
                len = __spi_max_buf_size;
            } else if(rem_size < 4) {
                len = rem_size;
            } else {
                // len = (rem_size >> 2) << 2;
                len = rem_size & ~(uint32_t)3u;
            }
            rem_size -= len;

            __log_debug("trx cycle => len: %d , rem_size: %d", len, rem_size);

            if( rem_size )
                p_base->flags |= SPI_TRANS_CS_KEEP_ACTIVE;
            else
                p_base->flags &= ~SPI_TRANS_CS_KEEP_ACTIVE;

            p_base->length = len << 3;

            p_base->tx_buffer = NULL;
            p_base->rx_buffer = NULL;
            p_base->flags &= ~(SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA);
            if(len < 4) {
                if(tx_buf) {
                    p_base->flags |= SPI_TRANS_USE_TXDATA;
                    memcpy(p_base->tx_data, tx_buf + offset, len);
                } else if(rx_buf) {
                    p_base->flags |= SPI_TRANS_USE_RXDATA;
                }
            } else {
                if(tx_buf)
                    p_base->tx_buffer = tx_buf + offset;
                else if(rx_buf)
                    p_base->rx_buffer = rx_buf + offset;
            }

            // -- perform the trx
            __esp_api_call(
                spi_device_polling_transmit(s_spi_dev_handle, &trx_desc.base),
                    "spi trx polling", !0);

            if(len < 4 && rx_buf) {
                memcpy(rx_buf + offset, p_base->rx_data, len);
            }

            offset += len;

        } while(rem_size);
    }


    spi_device_release_bus(s_spi_dev_handle);

    if(rx_buf)
        __log_dump(rx_buf, p_trx->size, 16, __log_dump_flag_hide_address,
            __word_len_8);

    return 0;
}

#undef __log_subsystem
#undef __log_component
#define __log_subsystem     lora
#define __log_component     port_sx126x

/** -------------------------------------------------------------------------- *
 * LoRa GPIOs and Interrupt handling Integration
 * --------------------------------------------------------------------------- *
 */

static void sx126x_chip_free_signal(void)
{
    __log_debug("int signal --> sx126x free");
}

static void sx126x_irq_signal(void)
{
    __log_debug("int signal --> sx126x irq");
    sx126x_port_irq();
}

static void sx1262_gpios_ctor( void )
{
    __log_info("ctor() -> sx126x 'busy' and 'irq' callbacks");
    ioexp_lora_chip_set_busy_signal_callback(sx126x_chip_free_signal);
    ioexp_lora_chip_set_int_signal_callback(sx126x_irq_signal);
}

static void sx1262_gpios_dtor( void )
{
    __log_info("~dtor() -> sx126x 'busy' and 'irq' callbacks");
    ioexp_lora_chip_set_busy_signal_callback(NULL);
    ioexp_lora_chip_set_int_signal_callback(NULL);
}

static void sx1262_wait_on_busy( void )
{
    bool busy;
    do {
        busy = ioexp_lora_chip_is_busy();
    } while (busy);
}

static void sx1262_send_reset_pulse( void )
{
    __log_info("reset sx126x chip");
    ioexp_lora_chip_reset();
}

static bool sx126x_port_int_pin_state ( void )
{
    return ioexp_lora_chip_read_int_pin();
}

/* --- end of file ---------------------------------------------------------- */
