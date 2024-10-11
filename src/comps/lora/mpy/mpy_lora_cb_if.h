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
 * @brief   API declarations for callback organizer for the micropython C-module
 *          of the lora-stack
 * --------------------------------------------------------------------------- *
 */
#ifndef __MPY_LORA_CALLBACK_IF_H__
#define __MPY_LORA_CALLBACK_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include "mp_lite_if.h"

/** -------------------------------------------------------------------------- *
 * configs
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_LORA_WAN_MAX_APP_LAYER_USED_PORTS
    #define __max_supported_simultaneous_ports  \
        CONFIG_LORA_WAN_MAX_APP_LAYER_USED_PORTS
#else
    #define __max_supported_simultaneous_ports  10
#endif

/** -------------------------------------------------------------------------- *
 * typoedef
 * --------------------------------------------------------------------------- *
 */
typedef enum {
    __MPY_LORA_CB_ON_TX_DONE        = (1u << 0),
    __MPY_LORA_CB_ON_TX_TIMEOUT     = (1u << 1),
    __MPY_LORA_CB_ON_TX_FAIL        = (1u << 2),
    __MPY_LORA_CB_ON_TX_CONFIRM     = (1u << 3),
    __MPY_LORA_CB_ON_RX_DONE        = (1u << 4),
    __MPY_LORA_CB_ON_RX_TIMEOUT     = (1u << 5),
    __MPY_LORA_CB_ON_RX_FAIL        = (1u << 6),

    __MPY_LORA_CB_ON_ANY            = 0xFFFF
} mpy_lora_callback_type_t;

#define __mpy_lora_max_callback_events_count    (7)

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
void mpy_lora_callback_init(void);

void mpy_lora_callback_set(
    int port,
    mpy_lora_callback_type_t on_event,
    mp_obj_t cb_fun_obj);

void mpy_lora_callback_unset(int port);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __MPY_LORA_CALLBACK_IF_H__ */
