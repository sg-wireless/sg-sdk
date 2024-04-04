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
 * @brief   A micropython module file for exposing "lte.h" as a micropython
 *          module
 * --------------------------------------------------------------------------- *
 */
#include "mp_lite_if.h"
#define __log_subsystem     lte
#include "log_lib.h"
#include "lte.h"
#include "lte_helpers.h"
#include "lte_uart.h"

#define __mod_generic_buf_size   (6 * 1024)
// static char s_mod_generic_buf[__mod_generic_buf_size];
static int s_class_instance_counter = 0;
typedef struct {
    mp_obj_base_t   base;
    int             instance_number;
    char            generic_buf[__mod_generic_buf_size];
} lte_class_data_t;

/** -------------------------------------------------------------------------- *
 * module functions implementation
 * --------------------------------------------------------------------------- *
 */
__mp_mod_class_new(LTEC, LTE)(
    const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *all_args){

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_carrier, MP_ARG_OBJ,
            {.u_obj = MP_OBJ_NEW_QSTR(MP_QSTR_standard)}},
        { MP_QSTR_cid, MP_ARG_INT,{.u_int = __lte_init_default_cid}},
        { MP_QSTR_mode, MP_ARG_INT,{.u_int = __lte_init_default_mode}},
        { MP_QSTR_baudrate, MP_ARG_INT,{.u_int = __lte_init_default_baudrate}},
        { MP_QSTR_debug, MP_ARG_BOOL,{.u_bool = __lte_init_default_debug}},
    };

    mp_arg_val_t out_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, out_args);

    #define __arg_carrier_obj   out_args[0].u_obj
    #define __arg_cid_int       out_args[1].u_int
    #define __arg_mode_int      out_args[2].u_int
    #define __arg_baudrate_int  out_args[3].u_int
    #define __arg_debug_bool    out_args[4].u_bool

    const char* carrier = mp_get_string(__arg_carrier_obj);
    if(!carrier) {
        carrier = __lte_init_default_carrier;
    }
    
    if(lte_init(carrier, __arg_cid_int, __arg_mode_int,
        __arg_baudrate_int, __arg_debug_bool) != __LTE_OK )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Modem initialization failed!"));
    }

    lte_class_data_t *self = m_new_obj(lte_class_data_t);

    self->base.type = type;
    // self->generic_buf = s_mod_generic_buf;
    self->instance_number = s_class_instance_counter++;

    return MP_OBJ_FROM_PTR(self);

    #undef __arg_carrier_obj
    #undef __arg_cid_int
    #undef __arg_mode_int
    #undef __arg_baudrate_int
    #undef __arg_debug_bool

}

__mp_mod_class_const(LTEC, LTE, CATM1, 0);
__mp_mod_class_const(LTEC, LTE, NBIOT, 1);

__mp_mod_class_method_0(LTEC, LTE, check_power)(mp_obj_t __self)
{
    // lte_class_data_t* self = MP_OBJ_TO_PTR(__self);
    return lte_check_power() ? mp_const_true : mp_const_false;
}

__mp_mod_class_method_kw(LTEC, LTE, print_pretty_response, 1)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    // lte_class_data_t* self = MP_OBJ_TO_PTR(pos_args[0]);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_flush, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false}},
        { MP_QSTR_prefix, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj=mp_const_none}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-2, pos_args+2, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_flush_bool    args[0].u_bool
    #define __arg_prefix_obj    args[1].u_obj
    #define __arg_rsp_obj       pos_args[1]

    const char* rsp = mp_get_string(__arg_rsp_obj);
    const char* prefix = NULL;

    if(__arg_prefix_obj != mp_const_none) {
        prefix = mp_get_string(__arg_prefix_obj);
    }

    lte_print_pretty_response(rsp, __arg_flush_bool, prefix);

    return mp_const_none;

    #undef __arg_flush_bool
    #undef __arg_prefix_obj
    #undef __arg_rsp_obj
}

__mp_mod_class_method_1(LTEC, LTE, return_pretty_response)(
    mp_obj_t __self, mp_obj_t obj)
{
    lte_class_data_t* self = MP_OBJ_TO_PTR(__self);
    const char* rsp = mp_get_string(obj);
    lte_return_pretty_response(rsp, self->generic_buf, __mod_generic_buf_size);
    return mp_obj_new_str(self->generic_buf, strlen(self->generic_buf));
}

__mp_mod_class_method_kw(LTEC, LTE, read_rsp, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    lte_class_data_t* self = MP_OBJ_TO_PTR(pos_args[0]);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __lte_read_rsp_default_timeout}},
        { MP_QSTR_wait_ok_error, MP_ARG_KW_ONLY | MP_ARG_BOOL,
            {.u_bool = __lte_read_rsp_default_wait_ok_error}},
        { MP_QSTR_check_error, MP_ARG_KW_ONLY | MP_ARG_BOOL,
            {.u_bool = __lte_send_at_cmd_def_check_error}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_timeout_int           args[0].u_int
    #define __arg_wait_ok_error_bool    args[1].u_bool
    #define __arg_check_error_bool      args[2].u_bool

    self->generic_buf[0] = 0;
    if(lte_read_rsp(__arg_timeout_int, __arg_wait_ok_error_bool,
        __arg_check_error_bool, self->generic_buf, __mod_generic_buf_size)
            != __LTE_OK)
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("error in read AT response"));
    }

    return mp_obj_new_str(self->generic_buf, strlen(self->generic_buf));

    #undef __arg_timeout_int
    #undef __arg_wait_ok_error_bool
    #undef __arg_check_error_bool
}

__mp_mod_class_method_kw(LTEC, LTE, send_at_cmd, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    lte_class_data_t* self = MP_OBJ_TO_PTR(pos_args[0]);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cmd, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __lte_send_at_cmd_def_timeout}},
        { MP_QSTR_wait_ok_error, MP_ARG_KW_ONLY | MP_ARG_BOOL,
            {.u_bool = __lte_send_at_cmd_def_wait_ok_error}},
        { MP_QSTR_check_error, MP_ARG_KW_ONLY | MP_ARG_BOOL,
            {.u_bool = __lte_send_at_cmd_def_check_error}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_cmd_obj               args[0].u_obj
    #define __arg_timeout_int           args[1].u_int
    #define __arg_wait_ok_error_bool    args[2].u_bool
    #define __arg_check_error_bool      args[3].u_bool

    const char* cmd = __lte_send_at_cmd_def_cmd;
    if(__arg_cmd_obj != mp_const_none) {
        cmd = mp_get_string(__arg_cmd_obj);
    }

    self->generic_buf[0] = 0;
    if(lte_send_at_cmd(cmd, __arg_timeout_int, __arg_wait_ok_error_bool,
        __arg_check_error_bool, self->generic_buf, __mod_generic_buf_size)
            != __LTE_OK)
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("AT command returned ERROR!"));
    }

    return mp_obj_new_str(self->generic_buf, strlen(self->generic_buf));

    #undef __arg_cmd_obj
    #undef __arg_timeout_int
    #undef __arg_wait_ok_error_bool
    #undef __arg_check_error_bool
}

__mp_mod_class_method_kw(LTEC, LTE, attach, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    // lte_class_data_t* self = MP_OBJ_TO_PTR(pos_args[0]);
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_apn, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_type, MP_ARG_KW_ONLY | MP_ARG_OBJ,
            {.u_obj = mp_const_none}},
        { MP_QSTR_cid, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __lte_attach_default_cid}},
        { MP_QSTR_band, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __lte_attach_default_band}},
        { MP_QSTR_bands, MP_ARG_KW_ONLY | MP_ARG_OBJ,
            {.u_obj = mp_const_none}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_apn_obj       args[0].u_obj
    #define __arg_type_obj      args[1].u_obj
    #define __arg_cid_int       args[2].u_int
    #define __arg_band_int      args[3].u_int
    #define __arg_bands_obj     args[4].u_obj

    const char* apn = __lte_attach_default_apn;
    if(__arg_apn_obj != mp_const_none) {
        apn = mp_get_string(__arg_apn_obj);
    }
    const char* type = __lte_attach_default_type;
    if(__arg_type_obj != mp_const_none) {
        type = mp_get_string(__arg_type_obj);
    }
    int* bands = __lte_attach_default_bands;
    size_t bands_count = 0;
    if(__arg_bands_obj != mp_const_none) {
        mp_obj_t* bands_objs;
        mp_obj_get_array(__arg_bands_obj, &bands_count, &bands_objs);
        bands = m_malloc(bands_count * sizeof(int));
        if(bands == 0) {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("memory alloc fail"));
        }
        for(int i=0; i < bands_count; ++i) {
            if( MP_OBJ_IS_STR(bands_objs[i]) ) {
                if( ! helper_str_read_int(
                        mp_get_string(bands_objs[i]), &bands[i]) ) {
                    goto free_and_exit;
                }
            } else if( MP_OBJ_IS_INT(bands_objs[i]) ) {
                bands[i] = mp_obj_get_int(bands_objs[i]);
            } else {
                goto free_and_exit;
            }

            continue;
            free_and_exit:
            m_free(bands);
            mp_raise_msg(&mp_type_ValueError,
                MP_ERROR_TEXT("bands array should be of integers only"
                " or strings of plain integers"));
        }
    }

    if(lte_attach(apn, type, __arg_cid_int, __arg_band_int,
        bands, bands_count)!= __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte attach fail"));
    }

    if(bands) {
        m_free(bands);
    }

    return mp_const_none;

    #undef __arg_apn_obj
    #undef __arg_type_obj
    #undef __arg_cid_int
    #undef __arg_band_int
    #undef __arg_bands_obj
}

__mp_mod_class_method_0(LTEC, LTE, is_attached)(mp_obj_t __self)
{
    // lte_class_data_t* self = MP_OBJ_TO_PTR(__self);
    return mp_obj_new_bool(lte_is_attached());
}

__mp_mod_class_method_0(LTEC, LTE, reset)(mp_obj_t __self)
{
    if(lte_reset() != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte reset fail"));
    }
    return mp_const_none;
}

__mp_mod_class_method_kw(LTEC, LTE, connect, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cid, MP_ARG_INT, {.u_int = __lte_connect_default_cid}}
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_cid_int       args[0].u_int

    if(lte_connect(__arg_cid_int) != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte connect fail"));
    }

    return mp_const_none;
}

__mp_mod_class_method_kw(LTEC, LTE, mode, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_new_mode, MP_ARG_INT, {.u_int = -1}}
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_new_mode_int       args[0].u_int

    if(__arg_new_mode_int == -1) {
        int mode = 0;
        if(lte_get_mode(&mode) != __LTE_OK) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte get mode fail"));
        }
        return MP_OBJ_NEW_SMALL_INT(mode);
    } else {
        if(lte_mode(__arg_new_mode_int) != __LTE_OK) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte set mode fail"));
        }
    }

    return mp_const_none;
}

__mp_mod_class_method_0(LTEC, LTE, ifconfig)(mp_obj_t __self)
{
    lte_ppp_ifconfig_t ifconfig_struct;

    lte_ifconfig(&ifconfig_struct);

    char ip_str[16];
    mp_uint_t ip_len;
    mp_obj_t tuple[4];

    uint8_t *ip = ifconfig_struct.ip_addr;
    ip_len = snprintf(ip_str, 16, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);
    tuple[0] = mp_obj_new_str(ip_str, ip_len);

    ip = ifconfig_struct.gw;
    ip_len = snprintf(ip_str, 16, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);
    tuple[1] = mp_obj_new_str(ip_str, ip_len);

    ip = ifconfig_struct.netmask;
    ip_len = snprintf(ip_str, 16, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);
    tuple[2] = mp_obj_new_str(ip_str, ip_len);

    ip = ifconfig_struct.dns;
    ip_len = snprintf(ip_str, 16, "%u.%u.%u.%u", ip[3], ip[2], ip[1], ip[0]);
    tuple[3] = mp_obj_new_str(ip_str, ip_len);

    return mp_obj_new_tuple(4, tuple);
}

__mp_mod_class_method_0(LTEC, LTE, disconnect)(mp_obj_t __self)
{
    if(lte_disconnect() != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte disconnect fail"));
    }
    return mp_const_none;
}


__mp_mod_class_method_0(LTEC, LTE, is_connected)(mp_obj_t __self)
{
    return mp_obj_new_bool(lte_is_connected());
}

__mp_mod_class_method_0(LTEC, LTE, isconnected)(mp_obj_t __self)
{
    return mp_obj_new_bool(lte_isconnected());
}

__mp_mod_class_method_0(LTEC, LTE, isattached)(mp_obj_t __self)
{
    return mp_obj_new_bool(lte_isattached());
}

__mp_mod_class_method_0(LTEC, LTE, detach)(mp_obj_t __self)
{
    if(lte_detach() != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte detach fail"));
    }
    return mp_const_none;
}

__mp_mod_class_method_0(LTEC, LTE, deinit)(mp_obj_t __self)
{
    lte_deinit();
    return mp_const_none;
}

__mp_mod_class_method_kw(LTEC, LTE, power_on, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_wait_ok, MP_ARG_BOOL, {.u_int = true}}
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_wait_ok_bool       args[0].u_bool

    return mp_obj_new_bool(lte_power_on(__arg_wait_ok_bool));

    #undef __arg_wait_ok_bool
}

__mp_mod_class_method_kw(LTEC, LTE, power_off, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_force, MP_ARG_BOOL, {.u_int = false}}
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args-1, pos_args+1, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_force_bool       args[0].u_bool

    if(lte_power_off(__arg_force_bool) != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte power off fail"));
    }
    return mp_const_none;
    #undef __arg_force_bool
}

__mp_mod_class_method_0(LTEC, LTE, pause_ppp)(mp_obj_t __self)
{
    if(lte_pause_ppp() != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte pause ppp fail"));
    }
    return mp_const_none;
}

__mp_mod_class_method_0(LTEC, LTE, resume_ppp)(mp_obj_t __self)
{
    if(lte_resume_ppp() != __LTE_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("lte resume ppp fail"));
    }
    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
