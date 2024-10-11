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
 * @brief   lora-wan processing sub-component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_WAN_PROCESS_H__
#define __LORA_WAN_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif
/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>
#include "lora_sync_obj.h"

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */
typedef enum {
    __LORA_WAN_PROCESS_COMMISSION,
    __LORA_WAN_PROCESS_JOIN_REQUEST,
    __LORA_WAN_PROCESS_REJOIN_REQUEST,
    __LORA_WAN_PROCESS_JOIN_DONE,
    __LORA_WAN_PROCESS_JOIN_FAIL,
    __LORA_WAN_PROCESS_JOIN_STATUS_REQ,

    __LORA_WAN_PROCESS_PROCESS_MAC,

    __LORA_WAN_PROCESS_TRX_DUTY_CYCLE,
    __LORA_WAN_PROCESS_MSG_TIMEOUT,

    __LORA_WAN_PROCESS_REQ_CLASS,
    __LORA_WAN_PROCESS_CLASS_CHANGED,

    __LORA_WAN_PROCESS_LCT_MODE_ENTER,
    __LORA_WAN_PROCESS_LCT_MODE_EXIT,
} lora_wan_process_request_t;

typedef struct {
    bool  is_joined;
    sync_obj_t sync_obj;
} lora_wan_join_status_req_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

void lora_wan_process_ctor(void);

void lora_wan_process_dtor(void);

void lora_wan_process_request(
    lora_wan_process_request_t request_type,
    void* trigger_data);

bool lora_wan_process_busy(void);

void lora_wan_enable_rx_listening(void);

void lora_wan_disable_rx_listening(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_WAN_PROCESS_H__ */
