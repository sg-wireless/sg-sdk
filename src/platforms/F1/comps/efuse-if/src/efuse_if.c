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
 * @brief   This file implements the interface component to the ESP32-S3 efuses
 * --------------------------------------------------------------------------- *
 */
#define __log_subsystem     F1
#define __log_component     efuse
#include "log_lib.h"
__log_component_def(F1, efuse, purple, 1, 0)

#include "sdkconfig.h"
#include "esp_efuse.h"
#include "esp_crc.h"
#include "esp_efuse_table.h"
#include "f1_efuse_table.h"
#include "efuse_if.h"

#define __read_from_cached_buffer

/** -------------------------------------------------------------------------- *
 * context variables
 * --------------------------------------------------------------------------- *
 */
#ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
static bool s_efuse_initialized = false;
static uint8_t s_user_data_blk[32] = {0};

#define __user_data_lora_mac_offset         0
#define __user_data_lora_mac_length         8

#define __user_data_serial_number_offset    8
#define __user_data_serial_number_length    6

#define __user_data_hw_id_offset            14
#define __user_data_hw_id_length            3

#define __user_data_project_id_offset       17
#define __user_data_project_id_length       3

#define __user_data_efuse_ver_offset        24
#define __user_data_efuse_ver_length        1

#define __user_data_wifi_mac_offset         25
#define __user_data_wifi_mac_length         6

#define __user_data_crc8_offset             31
#define __user_data_crc8_length             1

static bool efuse_read_info(void* buf, uint32_t offset, uint32_t length);
#endif

/** -------------------------------------------------------------------------- *
 * efuse interface component initialization
 * --------------------------------------------------------------------------- *
 */
#define __esp_api_call(__api_call, __err_msg, __ret)        \
    do {                                                    \
        esp_err_t err = __api_call;                         \
        if( err != ESP_OK ) {                               \
            __log_error("(err_code:%d)" __err_msg, err);    \
            return __ret;                                   \
        }                                                   \
    } while (0)

void efuse_if_init(void)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    if(s_efuse_initialized) {
        return;
    }

    // -- read the whole block and validate the CRC8
    uint8_t* buf = s_user_data_blk;
    __esp_api_call( esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA, buf, 32*8),
        "failed to read efuse user-data",);
    __log_dump(buf, 32, 16, 0, __word_len_8);

    __log_info("efuse-if init done!");

    s_efuse_initialized = true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}

bool efuse_if_read_layout_version(efuse_layout_version_t * layout_ver)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(layout_ver,
        __user_data_efuse_ver_offset,
        __user_data_efuse_ver_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_LAYOUT_VERSION);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_LAYOUT_VERSION,
            layout_ver, size),
        "failed to read efuse user-data lora mac", false);
    return true;
    #endif
}

bool efuse_if_read_lora_mac(efuse_lora_mac_t*lora_mac)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(lora_mac,
        __user_data_lora_mac_offset,
        __user_data_lora_mac_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_LPWAN_MAC);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_LPWAN_MAC,
            lora_mac, size),
        "failed to read efuse user-data lora mac", false);
    return true;
    #endif
}
bool efuse_if_read_serial_number(efuse_serial_number_t * serial_number)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(serial_number,
        __user_data_serial_number_offset,
        __user_data_serial_number_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_SERIAL_NUMBER);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_SERIAL_NUMBER,
            serial_number, size),
        "failed to read efuse user-data serial number", false);
    return true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}
bool efuse_if_read_hw_id(efuse_hw_id_t * hw_id)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(hw_id,
        __user_data_hw_id_offset,
        __user_data_hw_id_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_HW_ID);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_HW_ID,
            hw_id, size),
        "failed to read efuse user-data hw id", false);
    return true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}
bool efuse_if_read_project_id(efuse_project_id_t * project_id)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(project_id,
        __user_data_project_id_offset,
        __user_data_project_id_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_PROJECT_ID);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_PROJECT_ID,
            project_id, size),
        "failed to read efuse user-data project id", false);
    return true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}
bool efuse_if_read_wifi_mac(efuse_wifi_mac_t * wifi_mac)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(wifi_mac,
        __user_data_wifi_mac_offset,
        __user_data_wifi_mac_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_WIFI_MAC);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_WIFI_MAC,
            wifi_mac, size),
        "failed to read efuse user-data wifi mac", false);
    return true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}

#ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
static bool efuse_read_info(void* buf, uint32_t offset, uint32_t length)
{
    if(!s_efuse_initialized)
    {
        efuse_if_init();
    }

    memcpy(buf, s_user_data_blk + offset, length);

    return true;
}
#endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */

/* --- end of file ---------------------------------------------------------- */
