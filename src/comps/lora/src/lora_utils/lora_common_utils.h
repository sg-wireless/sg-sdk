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
 * @brief   common lora modes utilities sub-component interface
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_COMMON_UTILS_H__
#define __LORA_COMMON_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * include
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>
#include "lora.h"
#include "LoRaMac.h"
#include "Region.h"

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
#define __lora_common_utils_invalid_api_region  ((lora_region_t)-1)
#define __lora_common_utils_invalid_mac_region  ((LoRaMacRegion_t)-1)

LoRaMacRegion_t get_lora_mac_region_enum_value(lora_region_t region);

lora_region_t get_lora_api_region_enum_value(LoRaMacRegion_t region);

const char* get_lora_region_string(lora_region_t region);

uint32_t get_lora_region_default_frequency(lora_region_t region);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_COMMON_UTILS_H__ */
