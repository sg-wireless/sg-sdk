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
 * @brief   Declares an extended Radio functionality needed specifically
 *          by lora-stack.
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_RADIO_EXT_H__
#define __LORA_RADIO_EXT_H__

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
 * typedefs
 * --------------------------------------------------------------------------- *
 */
typedef enum {
    __LORA_RADIO_IRQ_TX_DONE            = (1 << 0),
    __LORA_RADIO_IRQ_RX_DONE            = (1 << 1),
    __LORA_RADIO_IRQ_PREAMBLE_DETECTED  = (1 << 2),
    __LORA_RADIO_IRQ_SYNC_WORD_VALID    = (1 << 3),
    __LORA_RADIO_IRQ_HEADER_VALID       = (1 << 4),
    __LORA_RADIO_IRQ_HEADER_ERR         = (1 << 5),
    __LORA_RADIO_IRQ_CRC_ERR            = (1 << 6),
    __LORA_RADIO_IRQ_CAD_DONE           = (1 << 7),
    __LORA_RADIO_IRQ_CAD_DETECTED       = (1 << 8),
    __LORA_RADIO_IRQ_TIMEOUT            = (1 << 9)
} lora_radio_irq_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
uint32_t lora_radio_ext_get_irqs( void );

int8_t lora_radio_ext_get_max_tx_power( void );
int8_t lora_radio_ext_get_min_tx_power( void );

const char* lora_radio_ext_get_chip_name( void );

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_RADIO_EXT_H__ */
