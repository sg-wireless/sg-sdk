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
 * @brief   This file declare the adaptor specification of the sx126x lora chip
 *          with respect to the lora-stack
 * --------------------------------------------------------------------------- *
 */
#ifndef __SX126X_PORT_H__
#define __SX126X_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * LoRa SPI transaction definition:
 * -------------------------------
 * @code
 *                         [8:offset]
 *          TX    8:cmd   [16:address]                     [n:data]
 *              |--------|-------------|-------------|-------------------|
 *          RX                            [byte-1]         [n:data]
 *                                       [dummy-byte]
 * @endcode
 * --------------------------------------------------------------------------- *
 */
typedef struct {

    // -- command phase
    uint8_t     command;        // 8-bit command

    // -- address phase
    uint16_t    address;        // contains address or offset
    uint16_t    address_bits;   // 16:[address] , 8:[offset]

    // -- received byte-1 phase
    uint8_t     rx_byte1;

    // -- data phase read/write
    uint8_t*    tx_buffer; /* NULL if not transmitting data */
    uint8_t*    rx_buffer; /* NULL if not receiving data */
    uint32_t    size;

    // -- timing
    uint32_t    cs_enable_pretrans_ms;

    // -- flags
    uint32_t    flags;
    #define __sx126x_spi_trx_has_address          (1u<<0)
    #define __sx126x_spi_trx_has_rx_byte1         (1u<<1)
    #define __sx126x_spi_trx_has_dummy_rx_byte1   (1u<<2)
    #define __sx126x_spi_trx_has_cs_pretrans_ms   (1u<<3)

} sx126x_spi_trx_t;

typedef int sx126x_port_spi_trx_t ( sx126x_spi_trx_t * p_trx );
typedef void sx126x_port_wait_on_busy_signal_t ( void );
typedef void sx126x_port_send_reset_pulse_t ( void );
typedef bool sx126x_port_int_pin_state_t ( void );

typedef struct {
    sx126x_port_spi_trx_t *             p_spi_trx;
    sx126x_port_wait_on_busy_signal_t * p_wait_on_busy_signal;
    sx126x_port_send_reset_pulse_t *    p_send_reset_pulse;
    sx126x_port_int_pin_state_t *       p_get_int_pin_state;
} sx126x_port_t;

/**
 * @brief   to intialize and connect to the adaptor port of the pycom
 *          lora_network stack
 */
void sx126x_port_init( sx126x_port_t * p_port_params );

/**
 * @brief   This function shall ba called if the sx126x interrupt pin is raised 
 */
void sx126x_port_irq( void );

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __SX126X_PORT_H__ */
