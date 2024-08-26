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
 * @brief   micropython module for eFuses interface component.
 * --------------------------------------------------------------------------- *
 */

#include "mp_lite_if.h"
#define __log_subsystem     F1
#define __log_component     efuse
#include "log_lib.h"
#include "efuse_if.h"

/** -------------------------------------------------------------------------- *
 * module function
 * --------------------------------------------------------------------------- *
 */
__mp_mod_include(efuse_if, "efuse_if.h")
__mp_mod_init(efuse_if)(void)
{
    efuse_if_init();
    return mp_const_none;
}

__mp_mod_fun_0(efuse_if, layout_version)(void)
{
    efuse_layout_version_t ver;
    efuse_if_read_layout_version(&ver);
    return mp_obj_new_bytearray(sizeof(ver), &ver);
}

#ifdef __feature_lora
__mp_mod_fun_ifdef(efuse_if, lora_mac, __feature_lora)
__mp_mod_fun_0(efuse_if, lora_mac)(void)
{
    efuse_lora_mac_t lora_mac;
    efuse_if_read_lora_mac(&lora_mac);
    return mp_obj_new_bytearray(sizeof(lora_mac), &lora_mac);
}
#endif /* __feature_lora */

#ifdef __efuse_lora_keys_enable
__mp_mod_fun_ifdef(efuse_if, lora_app_key, __efuse_lora_keys_enable)
__mp_mod_fun_0(efuse_if, lora_app_key)(void)
{
    efuse_lora_app_key_t lora_app_key;
    efuse_if_read_lora_app_key(&lora_app_key);
    return mp_obj_new_bytearray(sizeof(lora_app_key), &lora_app_key);
}

__mp_mod_fun_ifdef(efuse_if, lora_nwk_key, __efuse_lora_keys_enable)
__mp_mod_fun_0(efuse_if, lora_nwk_key)(void)
{
    efuse_lora_nwk_key_t lora_nwk_key;
    efuse_if_read_lora_nwk_key(&lora_nwk_key);
    return mp_obj_new_bytearray(sizeof(lora_nwk_key), &lora_nwk_key);
}

#ifdef CONFIG_EFUSE_VIRTUAL
__mp_mod_fun_ifdef(efuse_if, write_test_lora_keys, CONFIG_EFUSE_VIRTUAL)
__mp_mod_fun_kw(efuse_if, write_test_lora_keys, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        #define __init_allowed_obj_kw(kw) \
            { MP_QSTR_##kw, MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj = mp_const_none}}
        __init_allowed_obj_kw(AppKey),
        __init_allowed_obj_kw(NwkKey),
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                    MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    #define __arg_app_key_obj           args[0].u_obj
    #define __arg_nwk_key_obj           args[1].u_obj



    mp_buffer_info_t app_key;
    mp_buffer_info_t nwk_key;
    mp_get_buffer_raise(__arg_app_key_obj, &app_key, MP_BUFFER_READ);
    mp_get_buffer_raise(__arg_nwk_key_obj, &nwk_key, MP_BUFFER_READ);

    if(app_key.len != __efuse_lora_app_key_size ||
        nwk_key.len != __efuse_lora_nwk_key_size)
    {
        __log_output("error: passing wrong key length\n");
        return mp_const_none;
    }

    efuse_if_write_test_lora_keys(app_key.buf, nwk_key.buf);

    return mp_const_none;
}
#endif /* CONFIG_EFUSE_VIRTUAL */

#endif /* __efuse_lora_keys_enable */

__mp_mod_fun_0(efuse_if, serial_number)(void)
{
    efuse_serial_number_t serial_number;
    efuse_if_read_serial_number(&serial_number);
    return mp_obj_new_bytearray(sizeof(serial_number), &serial_number);
}

__mp_mod_fun_0(efuse_if, hw_id)(void)
{
    efuse_hw_id_t hw_id;
    efuse_if_read_hw_id(&hw_id);
    return mp_obj_new_bytearray(sizeof(hw_id), &hw_id);
}

__mp_mod_fun_0(efuse_if, project_id)(void)
{
    efuse_project_id_t project_id;
    efuse_if_read_project_id(&project_id);
    return mp_obj_new_bytearray(sizeof(project_id), &project_id);
}

__mp_mod_fun_0(efuse_if, wifi_mac)(void)
{
    efuse_wifi_mac_t wifi_mac;
    efuse_if_read_wifi_mac(&wifi_mac);
    return mp_obj_new_bytearray(sizeof(wifi_mac), &wifi_mac);
}

/* --- end of file ---------------------------------------------------------- */
