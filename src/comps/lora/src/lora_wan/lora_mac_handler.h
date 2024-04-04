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
 * @brief   lora-mac handler interfacing sub-component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_MAC_HANDLER_H__
#define __LORA_MAC_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif
/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */

#include "lora.h"
#include "LoRaMac.h"

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */

typedef struct {
    LoRaMacEventInfoStatus_t status;
    uint8_t     ack_received;
    int8_t      tx_power;
    int8_t      data_rate;
    uint8_t     port;
    uint8_t     channel;
    uint32_t    ul_counter;
} lmh_tx_status_params_t;

typedef struct {
    LoRaMacEventInfoStatus_t status;
    int8_t      rssi;
    int8_t      snr;
    uint8_t     port;
    int8_t      rx_slot;
    int8_t      data_rate;
    uint8_t     len;
    uint8_t*    buf;
    uint32_t    dl_counter;
} lmh_rx_status_params_t;

typedef void lmh_cb_on_mac_tx_t(lmh_tx_status_params_t* p_tx_status);
typedef void lmh_cb_on_mac_rx_t(lmh_rx_status_params_t* p_rx_status);
typedef struct {
    lmh_cb_on_mac_tx_t * on_mac_tx;
    lmh_cb_on_mac_rx_t * on_mac_rx;
} lmh_callbacks_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
void lmh_init( void );

void lmh_reset(bool purge_nvm);

void lmh_join( void );

void lmh_process(void);

bool lmh_is_busy(void);

lora_error_t lmh_send(uint8_t * buf, uint8_t len, uint8_t port, bool confirm);

void lmh_callbacks(lmh_callbacks_t * p_callbacks);

void lmh_set_region(lora_region_t region);
lora_region_t lmh_get_region(void);

void lmh_set_class(lora_wan_class_t class);
lora_wan_class_t lmh_get_class(void);

void lmh_start_class_c_temp_session(void);
void lmh_stop_class_c_temp_session(void);

void lmh_change_class(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_MAC_HANDLER_H__ */
