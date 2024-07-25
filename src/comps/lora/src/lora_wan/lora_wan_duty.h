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
 * @brief   lora-wan duty-cycle processing sub-component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_WAN_DUTY_H__
#define __LORA_WAN_DUTY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

void lora_wan_duty_ctor(void);

void lora_wan_duty_dtor(void);

void lora_wan_duty_set(uint32_t period);

uint32_t lora_wan_duty_get(void);

void lora_wan_duty_start(void);

void lora_wan_duty_stop(void);

void lora_wan_duty_suspend(void);

void lora_wan_duty_resume(void);

void lora_wan_duty_compliance_switch(bool is_on);

void lora_wan_duty_compliance_tx_periodicity(uint32_t periodicity);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_WAN_DUTY_H__ */
