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
 * @brief   This file represents a uPython c-module interface to the exported
 *          APIs of ctrl nvs configuration.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "ctrl_config.h"
#include "esp_task_wdt.h"
#include "mp_lite_if.h"

/* --- module functions definitions ----------------------------------------- */

//__mp_mod_name(ctrl_cfg, CTRL_CFG);

__mp_mod_init(ctrl_cfg)(void) {
    ctrl_config_init0();
    return mp_const_none;
}

__mp_mod_fun_var_between(ctrl_cfg, wifi_ssid_sta, 0, 1)(size_t arg_n,
                                                        const mp_obj_t *arg_v) {
    if (arg_n) {
        if (arg_v[0] == mp_const_none) {
            config_set_wifi_sta_ssid(NULL, true);
        } else if (MP_OBJ_IS_STR(arg_v[0])) {
            config_set_wifi_sta_ssid((uint8_t *)(mp_obj_str_get_str(arg_v[0])),
                                     true);
        } else { /*Nothing*/
        }
    } else {
        uint8_t *ssid = (uint8_t *)malloc(33);
        mp_obj_t ssid_obj;
        if (config_get_wifi_sta_ssid(ssid)) {
            ssid_obj =
                mp_obj_new_str((const char *)ssid, strlen((const char *)ssid));
        } else {
            ssid_obj = mp_const_none;
        }
        free(ssid);
        return ssid_obj;
    }
    return mp_const_none;
}

__mp_mod_fun_var_between(ctrl_cfg, wifi_pwd_sta, 0, 1)(size_t arg_n,
                                                       const mp_obj_t *arg_v) {
    if (arg_n) {
        if (arg_v[0] == mp_const_none) {
            config_set_wifi_sta_pwd(NULL, true);
        } else if (MP_OBJ_IS_STR(arg_v[0])) {
            config_set_wifi_sta_pwd((uint8_t *)(mp_obj_str_get_str(arg_v[0])),
                                    true);
        } else { /*Nothing*/
        }
    } else {
        uint8_t *pwd = (uint8_t *)malloc(65);
        mp_obj_t pwd_obj;
        if (config_get_wifi_sta_pwd(pwd)) {
            pwd_obj =
                mp_obj_new_str((const char *)pwd, strlen((const char *)pwd));
        } else {
            pwd_obj = mp_const_none;
        }
        free(pwd);
        return pwd_obj;
    }
    return mp_const_none;
}

__mp_mod_fun_var_between(ctrl_cfg, ctrl_on_boot, 0, 1)(size_t arg_n,
                                                       const mp_obj_t *arg_v) {
    if (arg_n) {
        config_set_ctrl_enabled(mp_obj_is_true(arg_v[0]));
    } else {
        return mp_obj_new_bool(config_get_ctrl_enabled());
    }
    return mp_const_none;
}

__mp_mod_fun_var_between(ctrl_cfg, ztp_enabled, 0, 1)(size_t arg_n,
                                                      const mp_obj_t *arg_v) {
    if (arg_n) {
        config_set_ztp_enabled(mp_obj_is_true(arg_v[0]));
    } else {
        return mp_obj_new_bool(config_get_ztp_enabled());
    }
    return mp_const_none;
}

__mp_mod_fun_kw(ctrl_cfg, lte_config,
                0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {
        ARG_carrier,
        ARG_apn,
        ARG_cid,
        ARG_band,
        ARG_type,
        ARG_reset,
        ARG_mode,
        ARG_bands
    };
    STATIC const mp_arg_t allowed_args[] = {
        {MP_QSTR_carrier,
         MP_ARG_KW_ONLY | MP_ARG_OBJ,
         {.u_obj = mp_const_none}},
        {MP_QSTR_apn, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_cid, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_band, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_bands, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_type, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_reset, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},
        {MP_QSTR_mode, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none}},

    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
                     allowed_args, args);

    uint8_t carrier[129];
    uint8_t apn[129];
    uint8_t type[17];
    uint8_t bands[45];
    uint8_t cid;
    uint8_t band;
    uint8_t reset;
    uint8_t mode;

    if (!config_get_ctrl_lte_config(carrier, apn, type, bands, &cid, &band,
                                    &reset, &mode)) {
        return mp_const_none;
    }

    if (n_args == 0) {
        mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(8, NULL));

        if (strlen((const char *)carrier) == 0) {
            t->items[ARG_carrier] = mp_const_none;
        } else {
            t->items[ARG_carrier] = mp_obj_new_str(
                (const char *)carrier, strlen((const char *)carrier));
        }

        if (strlen((const char *)apn) == 0) {
            t->items[ARG_apn] = mp_const_none;
        } else {
            t->items[ARG_apn] =
                mp_obj_new_str((const char *)apn, strlen((const char *)apn));
        }
        if (cid == 0xFF) {
            t->items[ARG_cid] = mp_obj_new_int(1);
        } else {
            t->items[ARG_cid] = mp_obj_new_int(cid);
        }
        if (band == 0xFF) {
            t->items[ARG_band] = mp_const_none;
        } else {
            t->items[ARG_band] = mp_obj_new_int(band);
        }
        if (strlen((const char *)type) == 0) {
            t->items[ARG_type] = mp_const_none;
        } else {
            t->items[ARG_type] =
                mp_obj_new_str((const char *)type, strlen((const char *)type));
        }
        if (reset == 0xff) {
            t->items[ARG_reset] = mp_const_false;
        } else {
            t->items[ARG_reset] = mp_obj_new_bool(reset);
        }
        if (mode == 0xff) {
            t->items[ARG_mode] = 0;
        } else {
            t->items[ARG_mode] = mp_obj_new_int(mode);
        }
        if (strlen((const char *)bands) == 0) {
            t->items[ARG_bands] = mp_const_none;
        } else {
            t->items[ARG_bands] = mp_obj_new_str((const char *)bands,
                                                 strlen((const char *)bands));
        }

        return MP_OBJ_FROM_PTR(t);
    } else {
        nlr_raise(mp_obj_new_exception_msg(
            &mp_type_ValueError,
            "Error this functionality is not yet supported!"));
    }

    return mp_const_none;
}

static bool is_empty(uint8_t *value, uint8_t size) {
    bool ret_val = true;
    for (int i = 0; i < size; i++) {
        if (value[i] != 0xFF) {
            ret_val = false;
        }
    }
    return ret_val;
}

static void bytes_to_str(uint8_t *bytes, uint32_t len, char *str) {
    while (len--) {
        uint8_t byte = *(bytes++);
        uint8_t nibble_r = byte & 0x0F;
        uint8_t nibble_l = (byte >> 4) & 0x0F;
        str[0] = nibble_l + (nibble_l > 9 ? 'a' - 10 : '0');
        str[1] = nibble_r + (nibble_r > 9 ? 'a' - 10 : '0');

        str += 2;
    }
    str[0] = 0;
}

__mp_mod_fun_kw(ctrl_cfg, lora_config,
                0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_activation, ARG_app_eui, ARG_dev_eui, ARG_app_key, ARG_net_key };
    STATIC const mp_arg_t allowed_args[] = {
        {MP_QSTR_lora_activation,
         MP_ARG_KW_ONLY | MP_ARG_OBJ,
         {.u_obj = mp_const_none}},
        {MP_QSTR_lora_app_eui,
         MP_ARG_KW_ONLY | MP_ARG_OBJ,
         {.u_obj = mp_const_none}},
        {MP_QSTR_lora_dev_eui,
         MP_ARG_KW_ONLY | MP_ARG_OBJ,
         {.u_obj = mp_const_none}},
        {MP_QSTR_lora_app_key,
         MP_ARG_KW_ONLY | MP_ARG_OBJ,
         {.u_obj = mp_const_none}},
        {MP_QSTR_lora_net_key,
         MP_ARG_KW_ONLY | MP_ARG_OBJ,
         {.u_obj = mp_const_none}},

    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
                     allowed_args, args);

    uint8_t activation;
    uint8_t app_eui[8];
    uint8_t dev_eui[8];
    uint8_t app_key[16];
    uint8_t net_key[16];

    if (!config_get_ctrl_lora_config(&activation, app_eui, dev_eui, app_key,
                                     net_key)) {
        return mp_const_none;
    }

    char arr[32 + 1];

    if (n_args == 0) {
        mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(5, NULL));
        t->items[ARG_activation] = mp_obj_new_int_from_uint(activation);
        bytes_to_str(app_eui, 8, arr);
        t->items[ARG_app_eui] = is_empty(app_eui, 8)
                                    ? mp_const_none
                                    : mp_obj_new_str(arr, strlen(arr));
        bytes_to_str(dev_eui, 8, arr);
        t->items[ARG_dev_eui] = is_empty(dev_eui, 8)
                                    ? mp_const_none
                                    : mp_obj_new_str(arr, strlen(arr));
        bytes_to_str(app_key, 16, arr);
        t->items[ARG_app_key] = is_empty(app_key, 16)
                                    ? mp_const_none
                                    : mp_obj_new_str(arr, strlen(arr));
        bytes_to_str(net_key, 16, arr);
        t->items[ARG_net_key] = is_empty(net_key, 16)
                                    ? mp_const_none
                                    : mp_obj_new_str(arr, strlen(arr));
        return MP_OBJ_FROM_PTR(t);
    } else {
        nlr_raise(mp_obj_new_exception_msg(
            &mp_type_ValueError,
            "Error this functionality is not yet supported!"));
    }

    return mp_const_none;
}

__mp_mod_fun_var_between(ctrl_cfg, device_token, 0, 1)(size_t arg_n,
                                                       const mp_obj_t *arg_v) {
    if (arg_n) {
        uint8_t *token_ptr;
        token_ptr = (uint8_t *)mp_obj_str_get_str(arg_v[0]);

        if (!config_set_ctrl_device_token(token_ptr)) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError,
                                               "Failed to write Device Token"));
        }

        return mp_const_none;
    } else {
        uint8_t ctrl_device_token[39];
        if (config_get_ctrl_device_token(ctrl_device_token)) {
            return mp_obj_new_str((const char *)ctrl_device_token,
                                  strlen((const char *)ctrl_device_token));
        } else {
            return mp_const_none;
        }
    }
}

__mp_mod_fun_var_between(ctrl_cfg, ztp_url, 0, 1)(size_t arg_n,
                                                  const mp_obj_t *arg_v) {
    if (arg_n) {
        uint8_t *token_ptr;
        token_ptr = (uint8_t *)mp_obj_str_get_str(arg_v[0]);

        if (!config_set_ctrl_ztp_url(token_ptr)) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError,
                                               "Failed to write ZTP URL!"));
        }

        return mp_const_none;
    } else {
        uint8_t ctrl_ztp_url[256];
        if (config_get_ctrl_ztp_url(ctrl_ztp_url)) {
            return mp_obj_new_str((const char *)ctrl_ztp_url,
                                  strlen((const char *)ctrl_ztp_url));
        } else {
            return mp_const_none;
        }
    }
}

__mp_mod_fun_var_between(ctrl_cfg, claim_token, 0, 1)(size_t arg_n,
                                                  const mp_obj_t *arg_v) {
    if (arg_n) {
        uint8_t *token_ptr;
        token_ptr = (uint8_t *)mp_obj_str_get_str(arg_v[0]);

        if (!config_set_ctrl_claim_token(token_ptr)) {
            nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError,
                                               "Failed to write Claim Token!"));
        }

        return mp_const_none;
    } else {
        uint8_t ctrl_claim_token[256];
        if (config_get_ctrl_claim_token(ctrl_claim_token)) {
            return mp_obj_new_str((const char *)ctrl_claim_token,
                                  strlen((const char *)ctrl_claim_token));
        } else {
            return mp_const_none;
        }
    }
}

__mp_mod_fun_var_between(ctrl_cfg, mqttServiceAddress, 0,
                         1)(size_t arg_n, const mp_obj_t *arg_v) {
    uint8_t ctrl_mqttServiceAddress[39];
    if (config_get_ctrl_mqttServiceAddress(ctrl_mqttServiceAddress)) {
        return mp_obj_new_str((const char *)ctrl_mqttServiceAddress,
                              strlen((const char *)ctrl_mqttServiceAddress));
    } else {
        return mp_const_none;
    }
}

__mp_mod_fun_var_between(ctrl_cfg, userId, 0, 1)(size_t arg_n,
                                                 const mp_obj_t *arg_v) {
    uint8_t ctrl_userId[254];
    if (config_get_ctrl_userId(ctrl_userId)) {
        return mp_obj_new_str((const char *)ctrl_userId,
                              strlen((const char *)ctrl_userId));
    } else {
        return mp_const_none;
    }
}

__mp_mod_fun_var_between(ctrl_cfg, network_preferences, 0,
                         1)(size_t arg_n, const mp_obj_t *arg_v) {
    uint8_t ctrl_network_preferences[54];
    if (config_get_ctrl_network_preferences(ctrl_network_preferences)) {
        return mp_obj_new_str((const char *)ctrl_network_preferences,
                              strlen((const char *)ctrl_network_preferences));
    } else {
        return mp_const_none;
    }
}

__mp_mod_fun_var_between(ctrl_cfg, extra_preferences, 0,
                         1)(size_t arg_n, const mp_obj_t *arg_v) {
    uint8_t ctrl_extra_preferences[99];
    if (config_get_ctrl_extra_preferences(ctrl_extra_preferences)) {
        return mp_obj_new_str((const char *)ctrl_extra_preferences,
                              strlen((const char *)ctrl_extra_preferences));
    } else {
        return mp_const_none;
    }
}

__mp_mod_fun_0(ctrl_cfg, wdt_disable)(void) {
    (void)esp_task_wdt_delete(NULL);
    return mp_const_none;
}

__mp_mod_fun_var_between(ctrl_cfg, lora_region, 0, 1)(size_t arg_n,
                                                       const mp_obj_t *arg_v) {
    if (arg_n) {
        config_set_lora_region(mp_obj_get_int(arg_v[0]));
    } else {
        uint8_t region = config_get_lora_region();
        if (region < 255) {
            return mp_obj_new_int_from_uint(region);
        }
    }
    return mp_const_none;
}