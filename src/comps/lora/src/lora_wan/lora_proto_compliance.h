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
 * @brief   interfacing header to the Semtech compliance protocol component.
 * --------------------------------------------------------------------------- *
 */

#ifndef __LORA_PROTO_COMPLIANCE_H__
#define __LORA_PROTO_COMPLIANCE_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

#include <stdbool.h>

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief   initialize the lora compliance package component
 */
void lora_proto_compliance_init(void);

/**
 * @brief   to obtain the current state of the compliance test whether it is
 *          active or not for the lora certification test support
 */
bool lora_proto_compliance_get_state(void);

/**
 * @brief   to set the desired state of the lora compliance test. it implicitly
 *          open or close the lora test port 224
 * 
 * @return  true    if succedded to set the required state
 *          fasle   if failed to set the required state
 */
bool lora_proto_compliance_set_state(bool state);

/**
 * @brief   to perform DUT reset request
 */
void lora_compliance_reset_dut(void);

/**
 * @brief   to check if DUT reset requested before this power-cycle or not
 */
bool lora_compliance_reset_state(void);

/**
 * @brief   to be called to process the duty cycle during the compliance test.
 */
void lora_proto_compliance_process_duty_cycle(void);

/**
 * @brief   a notification callback to be triggered from the Semtech LoRa MAC
 *          upon reception of the port 224 disable from the TCL (Test Control
 *          Layer).
 */
void lora_proto_compliance_notify_fport_224_disable(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_PROTO_COMPLIANCE_H__ */
