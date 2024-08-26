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
#include "utils_misc.h"

#define __read_from_cached_buffer

/** -------------------------------------------------------------------------- *
 * configuration
 * --------------------------------------------------------------------------- *
 */
#ifdef __efuse_lora_keys_enable

    #if   defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_EFUSE_KEY0)
        #define __lora_efuse_blk_n  4
        #define __lora_efuse_key_n  0
    #elif defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_EFUSE_KEY1)
        #define __lora_efuse_blk_n  5
        #define __lora_efuse_key_n  1
    #elif defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_EFUSE_KEY2)
        #define __lora_efuse_blk_n  6
        #define __lora_efuse_key_n  2
    #elif defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_EFUSE_KEY3)
        #define __lora_efuse_blk_n  7
        #define __lora_efuse_key_n  3
    #elif defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_EFUSE_KEY4)
        #define __lora_efuse_blk_n  8
        #define __lora_efuse_key_n  4
    #elif defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_EFUSE_KEY5)
        #define __lora_efuse_blk_n  9
        #define __lora_efuse_key_n  5
    #else
        #error "lora on efuses enabled but not associated to an eFuse KEY"
    #endif

    #define __lora_efuse_blk __concat(EFUSE_BLK, __lora_efuse_blk_n)
    #define __lora_efuse_key_desc __concat(ESP_EFUSE_KEY, __lora_efuse_key_n)

#endif /* __efuse_lora_keys_enable */

/** -------------------------------------------------------------------------- *
 * context variables
 * --------------------------------------------------------------------------- *
 */
#ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
static bool s_efuse_initialized = false;
typedef enum {
    __EFUSE_CACHE_USER_DATA,
    #ifdef __efuse_lora_keys_enable
    __EFUSE_CACHE_LORA_KEYS,
    #endif /* __efuse_lora_keys_enable */

    __EFUSE_CACHE_MAX_BLKS
} efuse_cache_blk_t;
static uint8_t s_efuse_cache[__EFUSE_CACHE_MAX_BLKS][32] = {0};

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

static bool efuse_read_info(
    efuse_cache_blk_t efuse_cache_blk,
    void* buf,
    uint32_t offset,
    uint32_t length);
#endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */

#ifdef __efuse_lora_keys_enable

static const esp_efuse_desc_t LORA_OTAA_APP_KEY[] = {
    {__lora_efuse_blk, 0, 128},
};
static const esp_efuse_desc_t LORA_OTAA_NWK_KEY[] = {
    {__lora_efuse_blk, 128, 128},
};

const esp_efuse_desc_t* ESP_EFUSE_LORA_OTAA_APP_KEY[] = {
    &LORA_OTAA_APP_KEY[0],
    NULL
};
const esp_efuse_desc_t* ESP_EFUSE_LORA_OTAA_NWK_KEY[] = {
    &LORA_OTAA_NWK_KEY[0],
    NULL
};

#define __lora_otaa_app_key_offset          0
#define __lora_otaa_app_key_length          16
#define __lora_otaa_nwk_key_offset          16
#define __lora_otaa_nwk_key_length          16

#endif /* __efuse_lora_keys_enable */

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
    uint8_t* buf = s_efuse_cache[__EFUSE_CACHE_USER_DATA];
    __esp_api_call( esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA, buf, 32*8),
        "failed to read efuse user-data",);
    __log_dump(buf, 32, 16, 0, __word_len_8);

    #ifdef __efuse_lora_keys_enable
    uint8_t* buf = s_efuse_cache[__EFUSE_CACHE_LORA_KEYS];
    __esp_api_call( esp_efuse_read_field_blob(__lora_efuse_key_desc, buf, 32*8),
        "failed to read efuse key"__stringify(__lora_efuse_key_desc)
        " (lora-keys)",);
    __log_dump(buf, 32, 16, 0, __word_len_8);
    #endif /* __efuse_lora_keys_enable */

    __log_info("efuse-if init done!");

    s_efuse_initialized = true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}

bool efuse_if_read_layout_version(efuse_layout_version_t* layout_ver)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info( __EFUSE_CACHE_USER_DATA, layout_ver,
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

#ifdef __feature_lora
bool efuse_if_read_lora_mac(efuse_lora_mac_t* lora_mac)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(__EFUSE_CACHE_USER_DATA, lora_mac,
        __user_data_lora_mac_offset,
        __user_data_lora_mac_length);
    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
    size_t size = esp_efuse_get_field_size(ESP_EFUSE_USER_DATA_LPWAN_MAC);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_USER_DATA_LPWAN_MAC,
            lora_mac, size),
        "failed to read efuse user-data lora mac", false);
    return true;
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}
#endif /* __feature_lora */

#ifdef __efuse_lora_keys_enable
bool efuse_if_read_lora_app_key(efuse_lora_app_key_t* lora_otaa_app_key)

{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE

    return efuse_read_info(__EFUSE_CACHE_LORA_KEYS, lora_otaa_app_key,
        __lora_otaa_app_key_offset,
        __lora_otaa_app_key_length);

    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */

    size_t size = esp_efuse_get_field_size(ESP_EFUSE_LORA_OTAA_APP_KEY);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_LORA_OTAA_APP_KEY,
            lora_otaa_app_key, size),
        "failed to read efuse key"__stringify(__efuse_lora_key)" lora app key",
        false);
    return true;

    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}

bool efuse_if_read_lora_nwk_key(efuse_lora_nwk_key_t* lora_otaa_nwk_key)

{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE

    return efuse_read_info(__EFUSE_CACHE_LORA_KEYS, lora_otaa_nwk_key,
        __lora_otaa_nwk_key_offset,
        __lora_otaa_nwk_key_length);

    #else /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */

    size_t size = esp_efuse_get_field_size(ESP_EFUSE_LORA_OTAA_NWK_KEY);
    __esp_api_call(
        esp_efuse_read_field_blob(ESP_EFUSE_LORA_OTAA_NWK_KEY,
            lora_otaa_nwk_key, size),
        "failed to read efuse key"__stringify(__efuse_lora_key)" lora nwk key",
        false);
    return true;

    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */
}

#ifdef CONFIG_EFUSE_VIRTUAL
bool efuse_if_write_test_lora_keys(
    efuse_lora_app_key_t* app_key,
    efuse_lora_nwk_key_t* nwk_key)
{
    __esp_api_call(esp_efuse_batch_write_begin(),
        "failed to begin efuse batch write", false);

    size_t size;

    size = esp_efuse_get_field_size(ESP_EFUSE_LORA_OTAA_APP_KEY);
    __esp_api_call(esp_efuse_write_field_blob(
        ESP_EFUSE_LORA_OTAA_APP_KEY, app_key, size),
        "failed to write efuse blob app_key", false);
    size = esp_efuse_get_field_size(ESP_EFUSE_LORA_OTAA_NWK_KEY);
    __esp_api_call(esp_efuse_write_field_blob(
        ESP_EFUSE_LORA_OTAA_NWK_KEY, nwk_key, size),
        "failed to write efuse blob nwk_key", false);
    esp_efuse_set_write_protect(__lora_efuse_blk);

    __esp_api_call(esp_efuse_set_key_purpose(
        __lora_efuse_blk, ESP_EFUSE_KEY_PURPOSE_USER),
        "failed to write key purpose", false);
    __esp_api_call(esp_efuse_set_keypurpose_dis_write(__lora_efuse_blk),
        "failed to write key purpose disable", false);
 
    __esp_api_call(esp_efuse_batch_write_commit(),
        "failed to commit efuse batch write", false);

    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    s_efuse_initialized = false;
    efuse_if_init();
    #endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */

    return true;
}
#endif /* CONFIG_EFUSE_VIRTUAL */

#endif /* __efuse_lora_keys_enable */

bool efuse_if_read_serial_number(efuse_serial_number_t* serial_number)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(__EFUSE_CACHE_USER_DATA, serial_number,
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

bool efuse_if_read_hw_id(efuse_hw_id_t* hw_id)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(__EFUSE_CACHE_USER_DATA, hw_id,
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

bool efuse_if_read_project_id(efuse_project_id_t* project_id)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(__EFUSE_CACHE_USER_DATA, project_id,
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

bool efuse_if_read_wifi_mac(efuse_wifi_mac_t* wifi_mac)
{
    #ifdef SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE
    return efuse_read_info(__EFUSE_CACHE_USER_DATA, wifi_mac,
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
static bool efuse_read_info(
    efuse_cache_blk_t efuse_cache_blk,
    void* buf,
    uint32_t offset,
    uint32_t length)
{
    if(!s_efuse_initialized)
    {
        efuse_if_init(false);
    }

    memcpy(buf, s_efuse_cache[efuse_cache_blk] + offset, length);

    return true;
}
#endif /* SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE */

/* --- end of file ---------------------------------------------------------- */
