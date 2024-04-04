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
 * @brief   lora-raw mode radio access sub-component interface
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_RAW_RADIO_IF_H__
#define __LORA_RAW_RADIO_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

#include "lora.h"

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

void lora_raw_radio_ctor(void);

void lora_raw_radio_dtor(void);

void lora_raw_radio_send(uint8_t* buf, uint8_t len);

void lora_raw_radio_recv(void);

lora_error_t lora_raw_radio_reset_params(void);

lora_error_t lora_raw_radio_set_param(lora_raw_param_t* param);

lora_error_t lora_raw_radio_get_param(lora_raw_param_t* param);

lora_error_t lora_raw_radio_get_default_region_param(lora_raw_param_t* param);

lora_error_t lora_raw_radio_verify_param(lora_raw_param_t* param);

lora_error_t lora_raw_radio_apply_params(void);

void lora_raw_radio_process_irqs(void);

void lora_raw_radio_sleep(void);

uint32_t lora_raw_radio_get_time_on_air(void);

void lora_raw_radio_tx_cont_wave(uint32_t freq, int8_t power,
    uint32_t time_msec);

void lora_raw_radio_stats(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_RAW_RADIO_IF_H__ */
