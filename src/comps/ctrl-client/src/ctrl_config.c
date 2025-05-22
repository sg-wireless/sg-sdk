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
 * @brief   This is the interface for the ctrl configuration stored in the NVS
 * --------------------------------------------------------------------------- *
 */

#include "nvs.h"
#include "nvs_config.h"

static nvs_handle_t ro_ctrl_handle, rw_ctrl_handle;

bool ctrl_config_init0(void) {
    //printf("This is ctrl_config_init0\n");
    return nvs_config_init0(&ro_ctrl_handle, &rw_ctrl_handle, "ctrl");
}

bool config_get_ctrl_lte_config(uint8_t *carrier, uint8_t *apn, uint8_t *type,
                                uint8_t *bands, uint8_t *cid, uint8_t *band,
                                uint8_t *reset, uint8_t *mode) {
    if (nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "carrier",
                     (char *)carrier) &&
        nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "apn", (char *)apn) &&
        nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "type", (char *)type) &&
        nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "cid", cid)) {
        if (!nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "bands",
                          (char *)bands)) {
            bands = (uint8_t *)"";
        }
        if (!nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "band", band)) {
            *band = 0x0;
        }
        if (!nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "reset", reset)) {
            *reset = false;
        }
        if (!nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "mode", mode)) {
            *mode = 0x0;
        }
        return true;
    }
    return false;
}

bool config_get_ctrl_lora_config(uint8_t *activation, uint8_t *app_eui,
                                 uint8_t *dev_eui, uint8_t *app_key,
                                 uint8_t *net_key) {
    if (nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "lora_mode", activation) &&
        nvs_read_blob(ro_ctrl_handle, rw_ctrl_handle, "lora_app_eui", app_eui,
                      8) &&
        nvs_read_blob(ro_ctrl_handle, rw_ctrl_handle, "lora_dev_eui", dev_eui,
                      8) &&
        nvs_read_blob(ro_ctrl_handle, rw_ctrl_handle, "lora_app_key", app_key,
                      16) &&
        nvs_read_blob(ro_ctrl_handle, rw_ctrl_handle, "lora_net_key", net_key,
                      16)) {
        return true;
    } else {
        return false;
    }
}

bool config_get_ctrl_device_token(uint8_t *ctrl_device_token) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "dev_token",
                        (char *)ctrl_device_token);
}

bool config_set_ctrl_device_token(uint8_t *ctrl_device_token) {
    return nvs_write_str(rw_ctrl_handle, "dev_token",
                         (char *)ctrl_device_token);
}

bool config_get_ctrl_mqttServiceAddress(
    uint8_t *ctrl_mqttServiceAddress) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "mqtt_addr",
                        (char *)ctrl_mqttServiceAddress);
}

bool config_get_ctrl_userId(uint8_t *ctrl_userId) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "user_id",
                        (char *)ctrl_userId);
}

bool config_get_ctrl_network_preferences(
    uint8_t *ctrl_network_preferences) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "ntwk_pref",
                        (char *)ctrl_network_preferences);
}

bool config_get_ctrl_extra_preferences(uint8_t *ctrl_extra_preferences) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "extra_pref",
                        (char *)ctrl_extra_preferences);
}

bool config_set_ztp_enabled(uint8_t ztp) {
    return nvs_write_u8(rw_ctrl_handle, "ztp", ztp);
}

bool config_get_ztp_enabled(void) {
    uint8_t ret_val;
    if (nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "ztp", &ret_val)) {
        return ret_val;
    } else {
        return false;
    }
}

bool config_get_ctrl_ztp_url(uint8_t *ctrl_ztp_url) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "ztp_url",
                        (char *)ctrl_ztp_url);
}

bool config_set_ctrl_ztp_url(uint8_t *ctrl_ztp_url) {
    return nvs_write_str(rw_ctrl_handle, "ztp_url", (char *)ctrl_ztp_url);
}

bool config_get_ctrl_claim_token(uint8_t *ctrl_claim_token) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "claim_token",
                        (char *)ctrl_claim_token);
}

bool config_set_ctrl_claim_token(uint8_t *ctrl_claim_token) {
    return nvs_write_str(rw_ctrl_handle, "claim_token", (char *)ctrl_claim_token);
}

bool config_set_wifi_sta_ssid(const uint8_t *wifi_ssid, bool update_flash) {
    return nvs_write_str(rw_ctrl_handle, "wifi_sta_ssid", (char *)wifi_ssid);
}

bool config_get_wifi_sta_ssid(uint8_t *wifi_ssid) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "wifi_sta_ssid",
                        (char *)wifi_ssid);
}

bool config_set_wifi_sta_pwd(const uint8_t *wifi_pwd, bool update_flash) {
    return nvs_write_str(rw_ctrl_handle, "wifi_sta_pwd", (char *)wifi_pwd);
}

bool config_get_wifi_sta_pwd(uint8_t *wifi_pwd) {
    return nvs_read_str(ro_ctrl_handle, rw_ctrl_handle, "wifi_sta_pwd",
                        (char *)wifi_pwd);
}

bool config_get_ctrl_enabled(void) {
    uint8_t ret_val;
    if (nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "ctrl", &ret_val)) {
        return (bool)ret_val;
    } else {
        return true;
    }
}

bool config_set_lora_region(uint8_t lora_region)
{
    return nvs_write_u8(
        rw_ctrl_handle, "lora_region", lora_region);
}

uint8_t config_get_lora_region(void)
{
    uint8_t ret_val;
    if (nvs_read_u8(ro_ctrl_handle, rw_ctrl_handle, "lora_region",
                    &ret_val))
    {
        return ret_val;
    }
    else
    {
        return 255;
    }
}

bool config_set_ctrl_enabled(bool ctrl_enabled) {
    return nvs_write_u8(rw_ctrl_handle, "ctrl", ctrl_enabled);
}

