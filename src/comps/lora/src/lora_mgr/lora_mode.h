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
 * @brief   lora-stack mode API declarations
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_MODE_H__
#define __LORA_MODE_H__

#ifdef __cplusplus
extern "C" {
#endif
/* --- includes ------------------------------------------------------------- */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "lora.h"

/* --- typedefs ------------------------------------------------------------- */

typedef struct {

    lora_error_t (* mode_ctor)(void);
    lora_error_t (* mode_dtor)(void);

    lora_error_t (* mode_stats)(void);

    lora_error_t (* mode_tx)(lora_tx_params_t * p_tx_params);
    lora_error_t (* mode_rx)(lora_rx_params_t * p_rx_params);

    lora_error_t (* mode_ioctl)(uint32_t ioctl, void* arg);

} lora_mode_interface_t;

/* --- APIs ----------------------------------------------------------------- */

lora_mode_interface_t* lora_raw_get_interface(void);

lora_mode_interface_t* lora_wan_get_interface(void);

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_MODE_H__ */
