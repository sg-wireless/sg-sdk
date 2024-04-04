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
 * @brief   This file declare the API of the event handler interface.
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_EVENT_HANDLER_H__
#define __LORA_EVENT_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */

typedef enum {
    __lora_evt_mac_process_notify,
    __lora_evt_loramac_handler_process_notify,
    __lora_evt_raw_process_cmd
} lora_evt_handler_id_t;

typedef void lora_evt_handler_t(void* data);

/** -------------------------------------------------------------------------- *
 * APIs Declarations
 * --------------------------------------------------------------------------- *
 */

void lora_event_handler_init(void);

void lora_event_handler_register(uint32_t evt_id,
    lora_evt_handler_t* p_evt_handler);

void lora_event_handler_deregister(uint32_t evt_id);

void lora_event_handler_issue(uint32_t evt_id, void* evt_data, uint32_t size);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_EVENT_HANDLER_H__ */
