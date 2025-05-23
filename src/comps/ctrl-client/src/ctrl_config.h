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
 * 
 * @author  Christian Ehlers (SG Wireless)
 *
 * @brief   This is the interface for the configuration stored in the NVS
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/******************************************************************************
 DEFINE CONSTANTS
 ******************************************************************************/

/******************************************************************************
 DEFINE TYPES
 ******************************************************************************/

/******************************************************************************
 DECLARE PUBLIC FUNCTIONS
 ******************************************************************************/

bool ctrl_config_init0(void);

bool config_get_ctrl_lte_config(uint8_t *carrier, uint8_t *apn, uint8_t *type,
                                uint8_t *bands, uint8_t *cid, uint8_t *band,
                                uint8_t *reset, uint8_t *mode);

bool config_get_ctrl_lora_config(uint8_t *activation, uint8_t *app_eui,
                                 uint8_t *dev_eui, uint8_t *app_key,
                                 uint8_t *net_key);

bool config_get_ctrl_device_token(uint8_t *ctrl_device_token);

bool config_set_ctrl_device_token(uint8_t *ctrl_device_token);

bool config_get_ctrl_mqttServiceAddress(uint8_t *ctrl_mqttServiceAddress);

bool config_get_ctrl_userId(uint8_t *ctrl_userId);

bool config_get_ctrl_network_preferences(
    uint8_t *ctrl_network_preferences);

bool config_get_ctrl_extra_preferences(uint8_t *ctrl_extra_preferences);

bool config_set_ztp_enabled(uint8_t ztp);

bool config_get_ztp_enabled(void);

bool config_set_ctrl_ztp_url(uint8_t *ctrl_ztp_url);

bool config_get_ctrl_ztp_url(uint8_t *ctrl_ztp_url);

bool config_set_ctrl_claim_token(uint8_t *ctrl_claim_token);

bool config_get_ctrl_claim_token(uint8_t *ctrl_claim_token);

bool config_set_wifi_sta_ssid(const uint8_t *wifi_ssid, bool update_flash);

bool config_get_wifi_sta_ssid(uint8_t *wifi_ssid);

bool config_set_wifi_sta_pwd(const uint8_t *wifi_pwd, bool update_flash);

bool config_get_wifi_sta_pwd(uint8_t *wifi_pwd);

bool config_get_ctrl_enabled(void);

bool config_set_ctrl_enabled(bool ctrl_enabled);

bool config_set_lora_region(uint8_t lora_region);

uint8_t config_get_lora_region(void);