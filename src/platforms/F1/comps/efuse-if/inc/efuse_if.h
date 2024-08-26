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
 * @brief   This is an interface file to the board efuses.
 * --------------------------------------------------------------------------- *
 */
#ifndef __EFUSE_IF_H__
#define __EFUSE_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** -------------------------------------------------------------------------- *
 * configs
 * --------------------------------------------------------------------------- *
 */
#ifdef __feature_lora
    #if defined(CONFIG_SDK_BOARD_LORA_WAN_KEYS_ON_EFUSES)
        #define __efuse_lora_keys_enable
    #endif
#endif

/** -------------------------------------------------------------------------- *
 * typedes
 * --------------------------------------------------------------------------- *
 */
#define __efuse_lora_mac_size           8
#define __efuse_lora_app_key_size      16
#define __efuse_lora_nwk_key_size      16
#define __efuse_serial_number_size      6
#define __efuse_hw_id_size              3
#define __efuse_project_id_size         3
#define __efuse_wifi_mac_size           6

typedef uint8_t efuse_layout_version_t;
#ifdef __feature_lora
typedef uint8_t efuse_lora_mac_t[__efuse_lora_mac_size];
#endif /* __feature_lora */
#ifdef __efuse_lora_keys_enable
typedef uint8_t efuse_lora_app_key_t[__efuse_lora_app_key_size];
typedef uint8_t efuse_lora_nwk_key_t[__efuse_lora_nwk_key_size];
#endif /* __efuse_lora_keys_enable */
typedef uint8_t efuse_serial_number_t[__efuse_serial_number_size];
typedef uint8_t efuse_hw_id_t[__efuse_hw_id_size];
typedef uint8_t efuse_project_id_t[__efuse_project_id_size];
typedef uint8_t efuse_wifi_mac_t[__efuse_wifi_mac_size];

/** -------------------------------------------------------------------------- *
 * typedes
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief   init the efuse interface component
 */
void efuse_if_init(void);

/**
 * @brief   reads the eFuses user block layout version
 * 
 * @param   layout_ver a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_layout_version(efuse_layout_version_t * layout_ver);

#ifdef __feature_lora
/**
 * @brief   reads the lora mac (DevEUI) from efuses
 * 
 * @param   lora_mac a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_lora_mac(efuse_lora_mac_t * lora_mac);
#endif /* __feature_lora */

#ifdef __efuse_lora_keys_enable
/**
 * @brief   reads the lora OTAA activation AppKey from efuses
 * 
 * @param   app_key a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */

bool efuse_if_read_lora_app_key(efuse_lora_app_key_t* app_key);
/**
 * @brief   reads the lora OTAA activation NwkKey from efuses
 * 
 * @param   nwk_key a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_lora_nwk_key(efuse_lora_nwk_key_t* nwk_key);

#ifdef CONFIG_EFUSE_VIRTUAL
bool efuse_if_write_test_lora_keys(
    efuse_lora_app_key_t* app_key,
    efuse_lora_nwk_key_t* nwk_key);
#endif
#endif /* __efuse_lora_enable */

/**
 * @brief   reads the board serial number from efuses
 * 
 * @param   lora_mac a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_serial_number(efuse_serial_number_t * serial_number);

/**
 * @brief   reads the manufacturer defined HW ID from efuses
 * 
 * @param   lora_mac a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_hw_id(efuse_hw_id_t * hw_id);

/**
 * @brief   reads the project ID from efuses
 * 
 * @param   lora_mac a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_project_id(efuse_project_id_t * project_id);

/**
 * @brief   reads the WiFi Custom MAC address from efuses
 * 
 * @param   lora_mac a pointer at which the read value will be written
 * @return  true if read succeeded
 *          false if read failed
 */
bool efuse_if_read_wifi_mac(efuse_wifi_mac_t * wifi_mac);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __EFUSE_IF_H__ */
