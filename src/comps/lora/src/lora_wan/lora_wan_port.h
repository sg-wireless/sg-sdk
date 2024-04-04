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
 * @brief   lora-wan transactions Ports header.
 *          Transactions are specified by the Tx or Rx on specified port number.
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_WAN_PORT_H__
#define __LORA_WAN_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdbool.h>
#include "lora_sync_obj.h"
#include "adt_list.h"
#include "buffers_chain.h"

#include "system/timer.h"
#include "lora.h"

/** -------------------------------------------------------------------------- *
 * data struct
 * --------------------------------------------------------------------------- *
 */

typedef struct {
    adt_list_t  list_links;

    // -- port identification
    uint8_t     port_num;

    // -- msg identification
    uint32_t    msg_seq_counter;

    // -- tx controls
    buf_chain_t tx_buf_chain;
    void*       tx_timeout_timer;

    // -- rx controls
    buf_chain_t rx_buf_chain;
    bool        is_rx_pending;
    void*       rx_timeout_timer;

    // -- indications controls
    buf_chain_t ind_buf_chain;

    lora_event_callback_t* callback;

} lora_wan_port_t;

typedef struct {
    uint32_t    port_num    : 8;
    uint32_t    retries     : 8;
    uint32_t    len         : 8;
    uint32_t    has_timeout : 1;
    uint32_t    sync        : 1;
    uint32_t    confirm     : 1;
    uint32_t    expire_timestamp;
    sync_obj_t  sync_obj;
    uint32_t    msg_seq_num;
    uint32_t    msg_app_id;
} lora_wan_port_tx_msg_t;

typedef enum {
    __IND_TX_TIMEOUT,
    __IND_TX_DONE,
    __IND_TX_FAIL,
    __IND_TX_CONFIRM,
    __IND_RX_DONE,
    __IND_RX_TIMEOUT,
} lora_wan_port_ind_t;

typedef struct {
    lora_wan_port_ind_t type;
    union{
        struct {
            uint32_t    msg_seq_num;
            uint32_t    msg_app_id;
            uint32_t    ul_frame_counter;
            int8_t      tx_power;
            int8_t      data_rate;
        } tx;
        struct {
            uint32_t    dl_frame_counter;
            int8_t      rssi;
            int8_t      snr;
            int8_t      data_rate;
        } rx;
    } ind_params;
} lora_wan_port_ind_msg_t;

typedef struct {
    enum {
        __MSG_TX_CONFIRMED,
        __MSG_TX_TIMEOUT,
        __MSG_SENT
    } status;
    uint32_t    msg_seq_num;
    uint32_t    msg_api_id;
} lora_wan_port_tx_status_msg_t;

typedef enum {
    __PORT_OK               =  0,
    __PORT_UNNOWN_ERROR     = -1,
    __PORT_IN_USE           = -2,
    __PORT_BAD_NUM          = -3,
    __PORT_NOT_OPENED       = -4,
    __PORT_NO_TX_DATA       = -5,
    __PORT_NO_RX_DATA       = -6,
    __PORT_NO_MEMORY        = -7,
} lora_port_error_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
lora_port_error_t lora_wan_port_open(lora_wan_port_t* p_port, int num);
lora_port_error_t lora_wan_port_close(lora_wan_port_t* p_port);

lora_port_error_t lora_port_close_all(void);

lora_port_error_t lora_wan_port_tx(
    lora_wan_port_t*    p_port,
    lora_tx_params_t*   p_tx_params
    );

lora_port_error_t lora_wan_port_rx(
    lora_wan_port_t*    p_port,
    lora_rx_params_t*   p_rx_params
    );

lora_port_error_t lora_wan_get_tx_data(
    uint8_t*    buf,
    uint8_t*    p_len
    );

bool lora_wan_is_pending_tx(void);

lora_port_error_t lora_wan_port_rx_indication(
    int         port_num,
    lora_wan_port_ind_msg_t * p_ind_msg,
    uint8_t*    buf,
    uint8_t     len
    );

lora_port_error_t lora_wan_port_indication(
    int         port_num,
    lora_wan_port_ind_msg_t * p_ind_msg
    );

lora_port_error_t lora_wan_port_get_ind_params(
    lora_wan_ind_params_t * p_ind_param);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_WAN_PORT_H__ */
