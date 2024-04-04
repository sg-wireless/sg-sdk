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
 * @brief   lora-raw mode processing sub-component interface
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_RAW_PROCESS_H__
#define __LORA_RAW_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>
#include "lora.h"

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */
typedef enum {
    __LORA_RAW_PROCESS_TX_REQUEST,
    __LORA_RAW_PROCESS_TX_DONE,
    __LORA_RAW_PROCESS_TX_TIMEOUT,
    __LORA_RAW_PROCESS_RX_REQUEST,
    __LORA_RAW_PROCESS_RX_DONE,
    __LORA_RAW_PROCESS_RX_TIMEOUT,
    __LORA_RAW_PROCESS_RX_ERROR,
    __LORA_RAW_PROCESS_RX_CONT_START,
    __LORA_RAW_PROCESS_RX_CONT_STOP,
    __LORA_RAW_PROCESS_CAD_DONE,
    __LORA_RAW_PROCESS_RADIO_IRQ,
    __LORA_RAW_PROCESS_RADIO_CONFIG,
    __LORA_RAW_PROCESS_RADIO_TOA_EXPIRE,
    __LORA_RAW_PROCESS_OPERATION_TIMEOUT,
    __LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE,
    __LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE_END,
} lora_raw_process_event_t;

typedef struct {
    void*   buf;
    uint8_t len;
} lora_raw_process_tx_request_t;

typedef struct {
    void*       buf;
    uint8_t*    p_len;
    uint32_t    timeout;
    bool        sync;
} lora_raw_process_rx_request_t;

typedef struct {
    enum {
        __PROCESS_MSG_PAYLOAD_TX_REQ,
        __PROCESS_MSG_PAYLOAD_RX_REQ,
        __PROCESS_MSG_PAYLOAD_RX_DONE,
        __PROCESS_MSG_PAYLOAD_TX_CONT_WAVE,
    } type;

    union {
        struct {
            uint8_t*    buf;
            uint8_t     buf_len;
            uint8_t*    p_len;
            uint32_t    timeout;
        } rx_payload;

        struct {
            uint8_t*    buf;
            uint8_t     len;
            uint32_t    timeout;
        } tx_payload;
        struct {
            uint8_t*    buf;
            uint8_t     len;
            int8_t      rssi;
            int8_t      snr;
        } rx_done_payload;
        struct {
            uint32_t    freq;
            int8_t      power;
            uint32_t    timeout;
        } tx_cont_wave;
    };
} lora_raw_process_event_payload_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

void lora_raw_process_ctor(void);

void lora_raw_process_dtor(void);

void lora_raw_process_register_callback(lora_event_callback_t * p_callback);

/**
 * @details     send a processing request for lora raw events
 * 
 * @param   event   the event to be served
 * @param   event_data  the specific event data
 * @param   sync    true:  caller context waits until fulfilling the operation
 *                  false  caller context returns immediately and the operation
 *                          response will return in the registered callback
 */
void lora_raw_process_event(
    lora_raw_process_event_t            event,
    lora_raw_process_event_payload_t*   event_data,
    bool                                sync
    );

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_RAW_PROCESS_H__ */
