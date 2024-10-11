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
 * @brief   common lora modes utilities sub-component
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */
#define __log_subsystem  lora
#include "log_lib.h"

#include "lora.h"
#include "LoRaMac.h"

/** -------------------------------------------------------------------------- *
 * lora region utilites
 * --------------------------------------------------------------------------- *
 */
static struct {
    LoRaMacRegion_t     lora_mac_region;
    lora_region_t       lora_api_region;
    const char*         name_str;
} s_region_translator[] = {
    {LORAMAC_REGION_AS923, __LORA_REGION_AS923, "AS923"},
    {LORAMAC_REGION_AU915, __LORA_REGION_AU915, "AU915"},
    {LORAMAC_REGION_CN470, __LORA_REGION_CN470, "CN470"},   
    {LORAMAC_REGION_CN779, __LORA_REGION_CN779, "CN779"},
    {LORAMAC_REGION_EU433, __LORA_REGION_EU433, "EU433"},
    {LORAMAC_REGION_EU868, __LORA_REGION_EU868, "EU868"},
    {LORAMAC_REGION_KR920, __LORA_REGION_KR920, "KR920"},
    {LORAMAC_REGION_IN865, __LORA_REGION_IN865, "IN865"},
    {LORAMAC_REGION_US915, __LORA_REGION_US915, "US915"},
    {LORAMAC_REGION_RU864, __LORA_REGION_RU864, "RU864"}
};
#define __region_translator_arr_size    \
    sizeof(s_region_translator)/sizeof(s_region_translator[0])

LoRaMacRegion_t get_lora_mac_region_enum_value(lora_region_t region)
{
    int i;
    for(i = 0; i < __region_translator_arr_size; ++i)
    {
        if( region == s_region_translator[i].lora_api_region)
        {
            return s_region_translator[i].lora_mac_region;
        }
    }
    __log_error("invalid region %d", region);
    return (LoRaMacRegion_t)-1;
}

lora_region_t get_lora_api_region_enum_value(LoRaMacRegion_t region)
{
    int i;
    for(i = 0; i < __region_translator_arr_size; ++i)
    {
        if( region == s_region_translator[i].lora_mac_region)
        {
            return s_region_translator[i].lora_api_region;
        }
    }
    __log_error("invalid region %d", region);
    return (lora_region_t)-1;
}

const char* get_lora_region_string(lora_region_t region)
{
    int i;
    for(i = 0; i < __region_translator_arr_size; ++i)
    {
        if( region == s_region_translator[i].lora_api_region)
        {
            return s_region_translator[i].name_str;
        }
    }
    __log_error("invalid region %d", region);
    return "unknown";
}

uint32_t get_lora_region_default_frequency(lora_region_t region)
{
    switch (region) {
    case __LORA_REGION_AS923: return __freq_mhz(923);
    case __LORA_REGION_AU915: return __freq_mhz(915);
    case __LORA_REGION_CN470: return __freq_mhz(470);
    case __LORA_REGION_CN779: return __freq_mhz(779);
    case __LORA_REGION_EU433: return __freq_mhz(433);
    case __LORA_REGION_EU868: return __freq_mhz(868);
    case __LORA_REGION_KR920: return __freq_mhz(920);
    case __LORA_REGION_IN865: return __freq_mhz(865);
    case __LORA_REGION_US915: return __freq_mhz(915);
    case __LORA_REGION_RU864: return __freq_mhz(864);
    }

    __log_error("invalid region: %d", region);
    return 0;
}

/* --- end of file ---------------------------------------------------------- */
