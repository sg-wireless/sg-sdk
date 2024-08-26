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
 * @brief   Implements the micropython C-module interface for the lora-stack.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <math.h>
#define __log_subsystem  lora
#define __log_component  mpy_lora
#include "log_lib.h"
#include "mp_lite_if.h"
// #include "mpirq.h"
#include "lora.h"
#include "lora_wan_duty.h"
#include "mphalport.h"
#include "mpy_lora_cb_if.h"

/** -------------------------------------------------------------------------- *
 * lora module name and constants
 * --------------------------------------------------------------------------- *
 */
__mp_mod_name(lora, LoRaMacHandler);
__mp_mod_include(lora, "lora.h");
__mp_declare_mod_obj(lora);

__mp_mod_class_const(lora, _commission, OTAA,       __LORA_COMMISSION_OTAA);
__mp_mod_class_const(lora, _commission, ABP,        __LORA_COMMISSION_ABP);

__mp_mod_class_const(lora, _version, VERSION_1_0_X, __LORA_WAN_VERSION_1_0_X);
__mp_mod_class_const(lora, _version, VERSION_1_1_X, __LORA_WAN_VERSION_1_1_X);

__mp_mod_class_const(lora, _mode,       WAN,            __LORA_MODE_WAN);
__mp_mod_class_const(lora, _mode,       RAW,            __LORA_MODE_RAW);

__mp_mod_class_const(lora, _bw,         BW_125KHZ,      __LORA_BW_125_KHZ);
__mp_mod_class_const(lora, _bw,         BW_250KHZ,      __LORA_BW_250_KHZ);
__mp_mod_class_const(lora, _bw,         BW_500KHZ,      __LORA_BW_500_KHZ);

__mp_mod_class_const(lora, _cr,         CODING_4_5,     __LORA_CR_4_5);
__mp_mod_class_const(lora, _cr,         CODING_4_6,     __LORA_CR_4_6);
__mp_mod_class_const(lora, _cr,         CODING_4_7,     __LORA_CR_4_7);
__mp_mod_class_const(lora, _cr,         CODING_4_8,     __LORA_CR_4_8);

__mp_mod_class_const(lora, _region,     REGION_AS923,   __LORA_REGION_AS923);
__mp_mod_class_const(lora, _region,     REGION_AU915,   __LORA_REGION_AU915);
__mp_mod_class_const(lora, _region,     REGION_CN470,   __LORA_REGION_CN470);
__mp_mod_class_const(lora, _region,     REGION_CN779,   __LORA_REGION_CN779);
__mp_mod_class_const(lora, _region,     REGION_EU433,   __LORA_REGION_EU433);
__mp_mod_class_const(lora, _region,     REGION_EU868,   __LORA_REGION_EU868);
__mp_mod_class_const(lora, _region,     REGION_KR920,   __LORA_REGION_KR920);
__mp_mod_class_const(lora, _region,     REGION_IN865,   __LORA_REGION_IN865);
__mp_mod_class_const(lora, _region,     REGION_US915,   __LORA_REGION_US915);
__mp_mod_class_const(lora, _region,     REGION_RU864,   __LORA_REGION_RU864);

__mp_mod_class_const(lora, _class, CLASS_A, __LORA_WAN_CLASS_A)
__mp_mod_class_const(lora, _class, CLASS_B, __LORA_WAN_CLASS_B)
__mp_mod_class_const(lora, _class, CLASS_C, __LORA_WAN_CLASS_C)

/** -------------------------------------------------------------------------- *
 * lora management layer methods
 * --------------------------------------------------------------------------- *
 */
__mp_mod_init(lora)(void)
{
    __log_info("init lora");
    lora_ctor();
    mpy_lora_callback_init();
    return mp_const_none;
}

__mp_mod_fun_0(lora, callback_stub_connect)(void) {
    lora_connect_callback_stub();
    return mp_const_none;
}

__mp_mod_fun_0(lora, callback_stub_disconnect)(void) {
    mpy_lora_callback_init();
    return mp_const_none;
}

__mp_mod_fun_var_between(lora, mode, 0, 1)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    if( __arg_n == 1 )
    {
        mp_obj_t obj = __arg_v[0];
        if(MP_OBJ_IS_INT(obj) == false) {
            __log_output(__red__"-- passing non int object"__default__);
        } else {
            mp_int_t mode = mp_obj_get_int(obj);
            if(mode != __LORA_MODE_RAW && mode != __LORA_MODE_WAN) {
                __log_output(__red__"-- passing incorrect mode(%d)"__default__,
                    mode);
            } else {
                lora_change_mode(mode);
                mpy_lora_callback_init();
            }
        }
        return mp_const_none;
    }
    else
    {
        lora_mode_t mode;
        lora_get_mode( & mode );
        __log_info("lora - %s\n", mode == __LORA_MODE_WAN ? "WAN" : "RAW");
        return MP_OBJ_NEW_SMALL_INT(mode);
    }
}

__mp_mod_fun_0( lora, stats )(void)
{
    lora_stats();
    return mp_const_none;
}

__mp_mod_fun_0( lora, initialize )(void)
{
    lora_ctor();
    return mp_const_none;
}

__mp_mod_fun_0( lora, deinit )(void)
{
    lora_dtor();
    return mp_const_none;
}

/** -------------------------------------------------------------------------- *
 * lora callback handling
 * --------------------------------------------------------------------------- *
 */

#define __rx_buf_len  0xff
static uint8_t s_rx_buffer[__rx_buf_len];
// static uint8_t s_rx_len;

__mp_mod_fun_kw(lora, callback, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_trigger, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __MPY_LORA_CB_ON_ANY}},
        { MP_QSTR_port, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __port_any}},
        { MP_QSTR_handler, MP_ARG_KW_ONLY | MP_ARG_OBJ | MP_ARG_REQUIRED,
            {.u_obj  = mp_const_none}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_trigger_int   args[0].u_int
    #define __arg_port_int      args[1].u_int
    #define __arg_handler_obj   args[2].u_obj

    mpy_lora_callback_set(__arg_port_int, __arg_trigger_int, __arg_handler_obj);

    return mp_const_none;
    #undef __arg_trigger_int
    #undef __arg_port_int
    #undef __arg_handler_obj
}

/** -------------------------------------------------------------------------- *
 * lora main send/recv methods
 * --------------------------------------------------------------------------- *
 */
__mp_mod_fun_0(lora, is_tx_pending)(void)
{
    bool is_pending;
    lora_ioctl(__LORA_IOCTL_IS_PENDING_TX, &is_pending);
    if( is_pending )
        return MP_ROM_TRUE;
    else
        return MP_ROM_FALSE;
}

__mp_mod_fun_kw(lora, send, 1)(
        size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_,    MP_ARG_OBJ | MP_ARG_REQUIRED, {.u_obj = mp_const_none}},
        { MP_QSTR_confirm, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false}},
        { MP_QSTR_port,    MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int  = 1}},
        { MP_QSTR_retries, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int  = 0}},
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int  = 0}},
        { MP_QSTR_sync,    MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false}},
        { MP_QSTR_id,      MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int  = 0}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_buf_obj       args[0].u_obj
    #define __arg_confirm_bool  args[1].u_bool
    #define __arg_port_int      args[2].u_int
    #define __arg_retries_int   args[3].u_int
    #define __arg_timeout_int   args[4].u_int
    #define __arg_sync_bool     args[5].u_bool
    #define __arg_id_int        args[6].u_int

    mp_buffer_info_t tx_buf = {0};
    bool is_confirmed = __arg_confirm_bool;

    mp_get_buffer_raise(__arg_buf_obj, &tx_buf, MP_BUFFER_READ);

    lora_mode_t mode;
    lora_get_mode(&mode);
    if(mode == __LORA_MODE_RAW)
    {
        lora_raw_param_t param = {.type = __LORA_RAW_PARAM_PAYLOAD};
        lora_ioctl(__LORA_IOCTL_GET_PARAM, &param);

        if(param.param.payload < tx_buf.len)
        {
            mp_raise_TypeError(MP_ERROR_TEXT("payload exceeds allowed"));
        }
    }
    else if(mode == __LORA_MODE_WAN)
    {
        lora_wan_param_t param = {.type = __LORA_WAN_PARAM_PAYLOAD};
        lora_ioctl(__LORA_IOCTL_GET_PARAM, &param);

        if(param.param.payload < tx_buf.len)
        {
            mp_raise_TypeError(MP_ERROR_TEXT("payload exceeds allowed"));
        }
    }
    else
    {
        mp_raise_TypeError(MP_ERROR_TEXT("unknown lora mode detected"));
    }

    lora_tx_params_t tx_params = {
        .buf = tx_buf.buf,
        .len = tx_buf.len,
        .port = __arg_port_int,
        .confirm = is_confirmed,
        .retries = __arg_retries_int,
        .timeout = __arg_timeout_int,
        .sync = __arg_sync_bool,
        .msg_app_id = __arg_id_int
    };

    if( __arg_sync_bool )
    {
        MP_THREAD_GIL_EXIT();
    }

    lora_tx( & tx_params );

    if( __arg_sync_bool )
    {
        MP_THREAD_GIL_ENTER();
    }

    return mp_const_none;

    #undef __arg_buf_obj
    #undef __arg_confirm_bool
    #undef __arg_port_int
    #undef __arg_retries_int
    #undef __arg_timeout_int
    #undef __arg_sync_bool
    #undef __arg_id_int
}

__mp_mod_fun_kw(lora, recv, 0)(
        size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buf, MP_ARG_KW_ONLY | MP_ARG_OBJ,{.u_obj = mp_const_none}},
        { MP_QSTR_port,    MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int  = -1}},
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 0}},
        { MP_QSTR_sync,    MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_buf_obj       args[0].u_obj
    #define __arg_port_int      args[1].u_int
    #define __arg_timeout_int   args[2].u_int
    #define __arg_sync_bool     args[3].u_bool

    mp_buffer_info_t rx_buf = {0};

    uint8_t* buf;
    uint8_t  len;
    uint8_t* p_len = &len;
    if(__arg_buf_obj != mp_const_none) {
        mp_get_buffer_raise(__arg_buf_obj, &rx_buf, MP_BUFFER_WRITE);
        buf = rx_buf.buf;
        len = rx_buf.len;
    }
    else
    {
        buf = s_rx_buffer;
        len = __rx_buf_len;
    }

    lora_rx_params_t rx_params = {
        .buf = buf,
        .p_len = p_len,
        .timeout = __arg_timeout_int,
        .sync = __arg_sync_bool,
    };

    lora_rx( & rx_params );

    // lora_recv(buf, p_len, p_port, __arg_timeout_int, __arg_sync_bool);

    if( __arg_buf_obj == mp_const_none && __arg_sync_bool )
    {
        return mp_obj_new_bytearray(len, s_rx_buffer);
    }

    return mp_const_none;

    #undef __arg_buf_obj
    #undef __arg_port_int
    #undef __arg_timeout_int
    #undef __arg_sync_bool
}

/** -------------------------------------------------------------------------- *
 * lora wan specific methods
 * --------------------------------------------------------------------------- *
 */
__mp_mod_fun_kw(lora, commission, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args) {

    static const mp_arg_t allowed_args[] = {
        #define __init_allowed_obj_kw(kw) \
            { MP_QSTR_##kw, MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj = mp_const_none}}
        { MP_QSTR_type, MP_ARG_KW_ONLY|MP_ARG_REQUIRED|MP_ARG_INT,{.u_int = 0}},
        __init_allowed_obj_kw(DevEUI),
        __init_allowed_obj_kw(JoinEUI),
        __init_allowed_obj_kw(AppKey),
        __init_allowed_obj_kw(NwkKey),
        { MP_QSTR_DevAddr, MP_ARG_KW_ONLY|MP_ARG_INT, {.u_int = 0}},
        __init_allowed_obj_kw(AppSKey),
        __init_allowed_obj_kw(NwkSKey),
        #undef __init_allowed_obj_kw
        { MP_QSTR_version, MP_ARG_KW_ONLY|MP_ARG_REQUIRED|MP_ARG_INT,
            {.u_int = 0}},
        { MP_QSTR_verify, MP_ARG_KW_ONLY|MP_ARG_BOOL, {.u_bool = false}}
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                    MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    #define __arg_type_int              args[0].u_int
    #define __arg_dev_eui_obj           args[1].u_obj
    #define __arg_join_eui_obj          args[2].u_obj
    #define __arg_app_key_obj           args[3].u_obj
    #define __arg_nwk_key_obj           args[4].u_obj
    #define __arg_dev_addr_int          args[5].u_int
    #define __arg_app_s_key_obj         args[6].u_obj
    #define __arg_nwk_s_key_obj         args[7].u_obj
    #define __arg_version_int           args[8].u_int
    #define __arg_verify_bool           args[9].u_bool

    lora_commission_params_t params;

    if( __arg_type_int == __LORA_COMMISSION_OTAA ) {
        if( __arg_dev_eui_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT("OTAA: missing devEui"));
        }
        if( __arg_join_eui_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT("OTAA: missing joinEui"));
        }
        if( __arg_app_key_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT("OTAA: missing appKey"));
        }
        if( __arg_version_int == __LORA_WAN_VERSION_1_0_X &&
            __arg_nwk_key_obj != mp_const_none) {
            mp_raise_TypeError(MP_ERROR_TEXT(
                "OTAA: nwkKey used with lora-wan version 1.1.x only"));
        }
        if( __arg_version_int == __LORA_WAN_VERSION_1_1_X &&
            __arg_nwk_key_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT(
                "OTAA: nwkKey is mandatory for lora-wan version 1.1.x"));
        }

        mp_buffer_info_t dev_eui;
        mp_buffer_info_t join_eui;
        mp_buffer_info_t app_key;
        mp_buffer_info_t nwk_key;
        mp_get_buffer_raise(__arg_dev_eui_obj, &dev_eui, MP_BUFFER_READ);
        mp_get_buffer_raise(__arg_join_eui_obj, &join_eui, MP_BUFFER_READ);
        mp_get_buffer_raise(__arg_app_key_obj, &app_key, MP_BUFFER_READ);
        if(__arg_nwk_key_obj != mp_const_none)
        {
            mp_get_buffer_raise(__arg_nwk_key_obj, &nwk_key, MP_BUFFER_READ);
        }
        __log_debug("-- OTAA commissioninig params --");
        __log_printf(" - devEUI:  ");
        __log_printf_hex_upper(dev_eui.buf, dev_eui.len);
        __log_endl();
        __log_printf(" - joinEUI: ");
        __log_printf_hex_upper(join_eui.buf, join_eui.len);
        __log_endl();
        __log_printf(" - appKey:  ");
        __log_printf_hex_upper(app_key.buf, app_key.len);
        __log_endl();
        if(__arg_nwk_key_obj != mp_const_none)
        {
            __log_printf(" - nwkKey:  ");
            __log_printf_hex_upper(nwk_key.buf, nwk_key.len);
            __log_endl();
        }
        params.type = __LORA_COMMISSION_OTAA;
        params.version = __arg_version_int;
        params.otaa.dev_eui = dev_eui.buf;
        params.otaa.join_eui = join_eui.buf;
        params.otaa.app_key = app_key.buf;
        if(__arg_nwk_key_obj != mp_const_none)
        {
            params.otaa.nwk_key = nwk_key.buf;
        }
    } else if ( __arg_type_int == __LORA_COMMISSION_ABP ) {
        if( __arg_dev_addr_int == 0 ) {
            mp_raise_TypeError(MP_ERROR_TEXT("ABP: missing DevAddr"));
        }
        if( __arg_dev_eui_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT("ABP: missing devEui"));
        }
        if( __arg_app_s_key_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT("ABP: missing AppSKey"));
        }
        if( __arg_nwk_s_key_obj == mp_const_none ) {
            mp_raise_TypeError(MP_ERROR_TEXT("ABP: missing NwkSKey"));
        }
        mp_buffer_info_t dev_eui;
        mp_buffer_info_t app_s_key;
        mp_buffer_info_t nwk_s_key;
        uint32_t dev_addr = __arg_dev_addr_int;
        mp_get_buffer_raise(__arg_dev_eui_obj, &dev_eui, MP_BUFFER_READ);
        mp_get_buffer_raise(__arg_app_s_key_obj, &app_s_key, MP_BUFFER_READ);
        mp_get_buffer_raise(__arg_nwk_s_key_obj, &nwk_s_key,MP_BUFFER_READ);

        params.type = __LORA_COMMISSION_ABP;
        params.version = __arg_version_int;
        params.abp.dev_addr = dev_addr;
        params.abp.dev_eui = dev_eui.buf;
        params.abp.app_s_key = app_s_key.buf;
        params.abp.nwk_s_key = nwk_s_key.buf;

        __log_debug("== ABP commissioninig params:");
        __log_printf(" - DevAddr:      %08X\n", dev_addr);
        __log_printf(" - devEUI:  ");
        __log_printf_hex_upper(dev_eui.buf, dev_eui.len);
        __log_endl();
        __log_printf(" - AppSKey:      ");
        __log_printf_hex_upper(app_s_key.buf, app_s_key.len);
        __log_endl();
        __log_printf(" - NwkSKey:      ");
        __log_printf_hex_upper(nwk_s_key.buf, nwk_s_key.len);
        __log_endl();
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("unknown activation type"));
    }

    if(__arg_verify_bool)
    {
        return lora_ioctl(__LORA_IOCTL_CHECK_COMMISSION, &params) == __LORA_OK
            ? mp_const_true : mp_const_false;
    }

    lora_ioctl(__LORA_IOCTL_SET_COMMISSION, &params);

    return mp_const_none;

    #undef __arg_type_int
    #undef __arg_dev_eui_obj
    #undef __arg_join_eui_obj
    #undef __arg_app_key_obj
    #undef __arg_nwk_key_obj
    #undef __arg_dev_addr_int
    #undef __arg_app_s_key_obj
    #undef __arg_nwk_s_key_obj
    #undef __arg_version_int
    #undef __arg_verify_bool
}

__mp_mod_fun_0( lora, join )(void)
{

    lora_ioctl(__LORA_IOCTL_JOIN, NULL);
    __log_debug("-- join requested --");

    return mp_const_none;
}

__mp_mod_fun_0( lora, is_joined )(void)
{
    bool status;
    lora_ioctl(__LORA_IOCTL_JOIN_STATUS, &status);
    if(status == true)
        return mp_const_true;
    else
       return mp_const_false;
}

__mp_mod_fun_kw(lora, wan_params, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_region, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __LORA_REGION_EU868}},
        { MP_QSTR_lwclass,  MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __LORA_WAN_CLASS_A}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                    MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    #define __arg_region_int        args[0].u_int
    #define __arg_class_int         args[1].u_int

    lora_wan_param_t param;

    #define __check_and_set_param(_p, _P, _a, _d, _t)           \
        do{                                                     \
            if( _a || __arg_##_p##_##_t != _d ) {               \
                param.type = __LORA_WAN_PARAM_ ## _P;           \
                lora_ioctl(__LORA_IOCTL_GET_PARAM, &param);     \
                if(param.param._p != __arg_##_p##_##_t) {       \
                    param.param._p = __arg_##_p##_##_t;         \
                    lora_ioctl(__LORA_IOCTL_SET_PARAM, &param); \
                }                                               \
            }                                                   \
        } while(0)

    __check_and_set_param(region,   REGION, 1, 0, int);
    __check_and_set_param(class,    CLASS,  1, 0, int);

    #undef __check_and_set_param

    return mp_const_none;

    #undef __arg_region_int
    #undef __arg_class_int
}

/** -------------------------------------------------------------------------- *
 * lora raw specific methods
 * --------------------------------------------------------------------------- *
 */
__mp_mod_fun_0( lora, recv_cont_start )(void)
{
    lora_ioctl(__LORA_IOCTL_RX_CONT_START, NULL);
    return mp_const_none;
}

__mp_mod_fun_0( lora, recv_cont_stop )(void)
{
    lora_ioctl(__LORA_IOCTL_RX_CONT_STOP, NULL);
    return mp_const_none;
}

__mp_mod_fun_kw(lora, radio_params, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    // -- reject the method call if mode is not lora RAW

    lora_mode_t mode;
    lora_get_mode(&mode);
    if( mode != __LORA_MODE_RAW )
    {
        mp_raise_TypeError(MP_ERROR_TEXT("this method for lora RAW only"));
        return mp_const_none;
    }

    // -- prepare allowed argument

    lora_raw_param_t param;

    #define __idx_reset_all     0
    #define __idx_region        1
    #define __idx_freq          2
    #define __idx_freq_khz      3
    #define __idx_freq_mhz      4
    #define __idx_tx_power      5
    #define __idx_antenna_gain  6
    #define __idx_sf            7
    #define __idx_cr            8
    #define __idx_preamble      9
    #define __idx_bw            10
    #define __idx_tx_inv_iq     11
    #define __idx_rx_inv_iq     12
    #define __idx_crc_on        13
    #define __idx_tx_timeout    14
    #define __idx_rx_timeout    15
    #define __idx_payload       16

    static mp_arg_t allowed[] = {
        #define __init(_idx, _kw, _type)    \
            [_idx] = {MP_QSTR_##_kw, MP_ARG_KW_ONLY | MP_ARG_##_type, {0}}
            __init(__idx_reset_all,     reset_all,      BOOL ),
            __init(__idx_region,        region,         INT  ),
            __init(__idx_freq,          frequency,      INT  ),
            __init(__idx_freq_khz,      freq_khz,       OBJ  ),
            __init(__idx_freq_mhz,      freq_mhz,       OBJ  ),
            __init(__idx_tx_power,      tx_power,       INT  ),
            __init(__idx_antenna_gain,  antenna_gain,   OBJ  ),
            __init(__idx_sf,            sf,             INT  ),
            __init(__idx_cr,            coding_rate,    INT  ),
            __init(__idx_preamble,      preamble,       INT  ),
            __init(__idx_bw,            bandwidth,      INT  ),
            __init(__idx_tx_inv_iq,     tx_iq,          BOOL ),
            __init(__idx_rx_inv_iq,     rx_iq,          BOOL ),
            __init(__idx_crc_on,        crc_on,         BOOL ),
            __init(__idx_tx_timeout,    tx_timeout,     INT  ),
            __init(__idx_rx_timeout,    rx_timeout,     INT  ),
            __init(__idx_payload,       payload,        INT  ),
        #undef __init
    };

    // -- loading default values

    #define __def_reset_all_bool    allowed[__idx_reset_all   ].defval.u_bool
    #define __def_region_int        allowed[__idx_region      ].defval.u_int
    #define __def_freq_int          allowed[__idx_freq        ].defval.u_int
    #define __def_freq_khz_obj      allowed[__idx_freq_khz    ].defval.u_obj
    #define __def_freq_mhz_obj      allowed[__idx_freq_mhz    ].defval.u_obj
    #define __def_tx_power_int      allowed[__idx_tx_power    ].defval.u_int
    #define __def_antenna_gain_obj  allowed[__idx_antenna_gain].defval.u_obj
    #define __def_sf_int            allowed[__idx_sf          ].defval.u_int
    #define __def_cr_int            allowed[__idx_cr          ].defval.u_int
    #define __def_preamble_int      allowed[__idx_preamble    ].defval.u_int
    #define __def_bw_int            allowed[__idx_bw          ].defval.u_int
    #define __def_tx_inv_iq_bool    allowed[__idx_tx_inv_iq   ].defval.u_bool
    #define __def_rx_inv_iq_bool    allowed[__idx_rx_inv_iq   ].defval.u_bool
    #define __def_crc_on_bool       allowed[__idx_crc_on      ].defval.u_bool
    #define __def_tx_timeout_int    allowed[__idx_tx_timeout  ].defval.u_int
    #define __def_rx_timeout_int    allowed[__idx_rx_timeout  ].defval.u_int
    #define __def_payload_int       allowed[__idx_payload     ].defval.u_int

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed)];
    #define __arg_reset_all_bool    args[__idx_reset_all   ].u_bool
    #define __arg_region_int        args[__idx_region      ].u_int
    #define __arg_freq_int          args[__idx_freq        ].u_int
    #define __arg_freq_khz_obj      args[__idx_freq_khz    ].u_obj
    #define __arg_freq_mhz_obj      args[__idx_freq_mhz    ].u_obj
    #define __arg_tx_power_int      args[__idx_tx_power    ].u_int
    #define __arg_antenna_gain_obj  args[__idx_antenna_gain].u_obj
    #define __arg_sf_int            args[__idx_sf          ].u_int
    #define __arg_cr_int            args[__idx_cr          ].u_int
    #define __arg_preamble_int      args[__idx_preamble    ].u_int
    #define __arg_bw_int            args[__idx_bw          ].u_int
    #define __arg_tx_inv_iq_bool    args[__idx_tx_inv_iq   ].u_bool
    #define __arg_rx_inv_iq_bool    args[__idx_rx_inv_iq   ].u_bool
    #define __arg_crc_on_bool       args[__idx_crc_on      ].u_bool
    #define __arg_tx_timeout_int    args[__idx_tx_timeout  ].u_int
    #define __arg_rx_timeout_int    args[__idx_rx_timeout  ].u_int
    #define __arg_payload_int       args[__idx_payload     ].u_int


    // -- parse the argument for the first time to pick-up the new region if any
    param.type = __LORA_RAW_PARAM_REGION;
    lora_ioctl(__LORA_IOCTL_GET_PARAM, &param);
    __def_region_int = param.region;

    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed), allowed, args);

    __def_reset_all_bool = false;
    __def_freq_khz_obj = mp_const_none;
    __def_freq_mhz_obj = mp_const_none;
    __def_antenna_gain_obj = mp_const_none;

    float    __def_antenna_gain_float;

    // set parameter to the parsed region value
    param.region = __arg_region_int;

    // set the default loader ioctl
    uint32_t ioctl_loader = __arg_region_int == __def_region_int
        ? __LORA_IOCTL_GET_PARAM : __LORA_IOCTL_GET_DEFAULT_REGION_PARAM;

    #define __load_default_value(_p, _P, _t)                \
        param.type = __LORA_RAW_PARAM_ ## _P;               \
        lora_ioctl(ioctl_loader, &param);                   \
        __def_##_p##_##_t = param.param._p;

    __load_default_value(freq,          FREQ,           int   );
    __load_default_value(tx_power,      TX_POWER,       int   );
    __load_default_value(antenna_gain,  ANTENNA_GAIN,   float );
    __load_default_value(sf,            SF,             int   );
    __load_default_value(cr,            CR,             int   );
    __load_default_value(preamble,      PREAMBLE,       int   );
    __load_default_value(bw,            BW,             int   );
    __load_default_value(tx_inv_iq,     TX_INV_IQ,      bool  );
    __load_default_value(rx_inv_iq,     RX_INV_IQ,      bool  );
    __load_default_value(crc_on,        CRC_ON,         bool  );
    __load_default_value(tx_timeout,    TX_TIMEOUT,     int   );
    __load_default_value(rx_timeout,    RX_TIMEOUT,     int   );
    __load_default_value(payload,       PAYLOAD,        int   );

    // -- second time argument parsing
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed), allowed, args);

    float   __arg_antenna_gain_float = __def_antenna_gain_float;

    // -- if reset all requested, reset then quit
    if(__arg_reset_all_bool)
    {
        lora_ioctl(__LORA_IOCTL_RESET_RADIO_PARAMS, NULL);
        return mp_const_none;
    }

    // -- refine the frequency argument
    if( __arg_freq_int == __def_freq_int )
    {
        if( __arg_freq_khz_obj != mp_const_none )
        {
            uint32_t freq =
                lround(mp_obj_get_float(__arg_freq_khz_obj) * 1000.0f);
            if(freq != __def_freq_int)
                __arg_freq_int = freq;
        }
        else if( __arg_freq_mhz_obj != mp_const_none )
        {
            uint32_t freq =
                lround(mp_obj_get_float(__arg_freq_mhz_obj)*1000000.0f);
            if(freq != __def_freq_int)
                __arg_freq_int = freq;
        }
    }

    // -- refine the antenna_gain argument
    if(__arg_antenna_gain_obj != mp_const_none)
    {
        float gain = mp_obj_get_float(__arg_antenna_gain_obj);
        if(gain != __def_antenna_gain_float)
        {
            __arg_antenna_gain_float = gain;
        }
    }

    // -- parameter verification step
    bool verified = true;
    param.region = __arg_region_int;
    if(__arg_region_int != __def_region_int)
    {
        param.type = __LORA_RAW_PARAM_REGION;
        if( lora_ioctl(__LORA_IOCTL_VERIFY_PARAM, &param) != __LORA_OK)
        {
            __log_output("error: invalid argument value '"
                __red__"region"__default__"'\n");
            verified = false;
        }
    }

    #define __verify_param(_p, _P, _t)                              \
        do {                                                        \
            if(__arg_##_p##_##_t != __def_##_p##_##_t)              \
            {                                                       \
                param.type = __LORA_RAW_PARAM_##_P;                 \
                param.param._p = __arg_##_p##_##_t;                 \
                if( lora_ioctl(__LORA_IOCTL_VERIFY_PARAM, &param)   \
                    != __LORA_OK) {                                 \
                    __log_output("error: invalid value for '"       \
                        __red__ "%s" __default__"'\n",              \
                        qstr_str(allowed[__idx_##_p].qst));         \
                    verified = false;                               \
                }                                                   \
            }                                                       \
        } while(0)

    __verify_param(freq,         FREQ,          int);
    __verify_param(tx_power,     TX_POWER,      int);
    __verify_param(antenna_gain, ANTENNA_GAIN,  float);
    __verify_param(sf,           SF,            int);
    __verify_param(cr,           CR,            int);
    __verify_param(preamble,     PREAMBLE,      int);
    __verify_param(bw,           BW,            int);
    __verify_param(tx_inv_iq,    TX_INV_IQ,     bool);
    __verify_param(rx_inv_iq,    RX_INV_IQ,     bool);
    __verify_param(crc_on,       CRC_ON,        bool);
    __verify_param(tx_timeout,   TX_TIMEOUT,    int);
    __verify_param(rx_timeout,   RX_TIMEOUT,    int);
    __verify_param(payload,      PAYLOAD,       int);

    if( ! verified )
    {
        return mp_const_none;
    }

    // -- parameter setting step
    bool change_detected = false;

    if(__arg_region_int != __def_region_int)
    {
        param.type = __LORA_RAW_PARAM_REGION;
        lora_ioctl(__LORA_IOCTL_SET_PARAM, &param);
        change_detected = true;
    }

    #define __update_param(_p, _P, _t)                          \
        do {                                                    \
            if(__arg_##_p##_##_t != __def_##_p##_##_t)          \
            {                                                   \
                param.type = __LORA_RAW_PARAM_##_P;             \
                param.param._p = __arg_##_p##_##_t;             \
                lora_ioctl(__LORA_IOCTL_SET_PARAM, &param);     \
                change_detected = true;                         \
            }                                                   \
        } while(0)
    
    __update_param(freq,         FREQ,          int);
    __update_param(antenna_gain, ANTENNA_GAIN,  float);
    __update_param(tx_power,     TX_POWER,      int);
    __update_param(sf,           SF,            int);
    __update_param(cr,           CR,            int);
    __update_param(preamble,     PREAMBLE,      int);
    __update_param(bw,           BW,            int);
    __update_param(tx_inv_iq,    TX_INV_IQ,     bool);
    __update_param(rx_inv_iq,    RX_INV_IQ,     bool);
    __update_param(crc_on,       CRC_ON,        bool);
    __update_param(tx_timeout,   TX_TIMEOUT,    int);
    __update_param(rx_timeout,   RX_TIMEOUT,    int);
    __update_param(payload,      PAYLOAD,       int);

    if(change_detected)
    {
        lora_ioctl(__LORA_IOCTL_RECONFIG_RADIO, NULL);
    }

    #undef __idx_reset_all
    #undef __idx_region
    #undef __idx_freq
    #undef __idx_freq_khz
    #undef __idx_freq_mhz
    #undef __idx_tx_power
    #undef __idx_antenna_gain
    #undef __idx_sf
    #undef __idx_cr
    #undef __idx_preamble
    #undef __idx_bw
    #undef __idx_tx_inv_iq
    #undef __idx_rx_inv_iq
    #undef __idx_crc_on
    #undef __idx_tx_timeout
    #undef __idx_rx_timeout
    #undef __idx_payload

    #undef __def_reset_all_bool
    #undef __def_region_int
    #undef __def_freq_int
    #undef __def_freq_khz_obj
    #undef __def_freq_mhz_obj
    #undef __def_tx_power_int
    #undef __def_sf_int
    #undef __def_cr_int
    #undef __def_preamble_int
    #undef __def_bw_int
    #undef __def_tx_inv_iq_bool
    #undef __def_rx_inv_iq_bool
    #undef __def_crc_on_bool
    #undef __def_tx_timeout_int
    #undef __def_rx_timeout_int
    #undef __def_payload_int

    #undef __arg_reset_all_bool
    #undef __arg_region_int
    #undef __arg_freq_int
    #undef __arg_freq_khz_obj
    #undef __arg_freq_mhz_obj
    #undef __arg_tx_power_int
    #undef __arg_antenna_gain_obj
    #undef __arg_sf_int
    #undef __arg_cr_int
    #undef __arg_preamble_int
    #undef __arg_bw_int
    #undef __arg_tx_inv_iq_bool
    #undef __arg_rx_inv_iq_bool
    #undef __arg_crc_on_bool
    #undef __arg_tx_timeout_int
    #undef __arg_rx_timeout_int
    #undef __arg_payload_int

    return mp_const_none;
}

__mp_mod_fun_kw(lora, tx_continuous_wave_start, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_frequency,    MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __freq_mhz(868)}},
        { MP_QSTR_tx_power,     MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED, 
            {.u_int = 0}},
        { MP_QSTR_timeout,      MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 10000}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                    MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    #define __arg_freq_int          args[0].u_int
    #define __arg_tx_power_int      args[1].u_int
    #define __arg_timeout_int       args[2].u_int

    lora_raw_tx_cont_wave_params_t params = {
        .freq  = __arg_freq_int,
        .power = __arg_tx_power_int,
        .time  = __arg_timeout_int
    };
    lora_ioctl(__LORA_IOCTL_TX_CONT_WAVE_START, &params);

    return mp_const_none;

    #undef __arg_freq_int
    #undef __arg_tx_power_int
    #undef __arg_timeout_int
}

__mp_mod_fun_0(lora, tx_continuous_wave_stop)(void){
    lora_ioctl(__LORA_IOCTL_TX_CONT_WAVE_STOP, NULL);
    return mp_const_none;
}

__mp_mod_fun_0(lora, raw_state_machine)(void)
{
    void lora_raw_sm_print(void);
    lora_raw_sm_print();
    return mp_const_none;
}

__mp_mod_fun_1(lora, duty_set)(mp_obj_t obj)
{
    if( MP_OBJ_IS_SMALL_INT(obj)) {
        uint32_t period = MP_OBJ_SMALL_INT_VALUE(obj);
        if(period > 0)
            lora_ioctl(__LORA_IOCTL_DUTY_CYCLE_SET, &period);
        else
            __log_output("error: provided duty-cycle period is zero");
    } else {
        __log_output("error: no provided duty-cycle period");
    }
    return mp_const_none;
}
__mp_mod_fun_0(lora, duty_get)(void)
{
    uint32_t period = 0;
    lora_ioctl(__LORA_IOCTL_DUTY_CYCLE_GET, &period);
    return MP_OBJ_NEW_SMALL_INT(period);
}
__mp_mod_fun_0(lora, duty_start)(void)
{
    lora_ioctl(__LORA_IOCTL_DUTY_CYCLE_START, NULL);
    return mp_const_none;
}
__mp_mod_fun_0(lora, duty_stop)(void)
{
    lora_ioctl(__LORA_IOCTL_DUTY_CYCLE_STOP, NULL);
    return mp_const_none;
}

__mp_mod_fun_0(lora, enable_rx_listening)(void)
{
    lora_ioctl(__LORA_IOCTL_ENABLE_RX_LISTENING, NULL);
    return mp_const_none;
}
__mp_mod_fun_0(lora, disable_rx_listening)(void)
{
    lora_ioctl(__LORA_IOCTL_DISABLE_RX_LISTENING, NULL);
    return mp_const_none;
}

__mp_mod_fun_1(lora, port_open)(mp_obj_t obj)
{
    if( MP_OBJ_IS_SMALL_INT(obj)) {
        uint32_t port_num = MP_OBJ_SMALL_INT_VALUE(obj);
        lora_ioctl(__LORA_IOCTL_PORT_OPEN, &port_num);
        /* no need to acquire a port callbacks resource
           it will be automatically acquired with lora.set_callback() */
    } else {
        __log_output("error: no provided port number");
    }
    return mp_const_none;
}
__mp_mod_fun_1(lora, port_close)(mp_obj_t obj)
{
    if( MP_OBJ_IS_SMALL_INT(obj)) {
        uint32_t port_num = MP_OBJ_SMALL_INT_VALUE(obj);
        if(lora_ioctl(__LORA_IOCTL_PORT_CLOSE, &port_num) == __LORA_OK )
        {
            /* detach the attached callbacks of this port */
            mpy_lora_callback_unset(port_num);
        }
    } else {
        __log_output("error: no provided port number");
    }
    return mp_const_none;
}

__mp_mod_fun_var_between(lora, list_region_params, 0, 1)(
    size_t __arg_n, const mp_obj_t * __arg_v
) {

    if(__arg_n == 1)
    {
        void lora_list_region_params(lora_region_t region);
        lora_list_region_params( mp_obj_get_int(__arg_v[0]) );
    }
    else
    {
        lora_mode_t mode;
        lora_get_mode(&mode);
        if( mode == __LORA_MODE_WAN )
        {
            void lora_list_mac_channels_status(void);
            lora_list_mac_channels_status();
        }
    }

    return mp_const_none;
}

__mp_mod_fun_0(lora, dr_stats)(void)
{
    void lora_dr_stats(void);
    lora_dr_stats();
    return mp_const_none;
}

__mp_mod_fun_ifdef(lora, certification_mode, CONFIG_LORA_LCT_CONTROL_API)
__mp_mod_fun_var_between(lora, certification_mode, 0, 1)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    lora_mode_t mode;
    lora_get_mode(&mode);
    if( mode != __LORA_MODE_WAN )
    {
        __log_output("This command is available only for lora-wan mode");
        return mp_const_none;
    }

    bool lct_state = false;
    if(__arg_n == 0)
    {
        lora_ioctl(__LORA_IOCTL_LCT_MODE_GET, &lct_state);

        __log_output("LCT mode is %s\n", g_on_off[lct_state == true]);
        return lct_state ? mp_const_true : mp_const_false;
    }
    else
    {
        if( ! mp_obj_is_bool(__arg_v[0]) )
        {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("passing non bool object"));
        }
        else
        {
            lct_state = __arg_v[0] == mp_const_true;
            lora_ioctl(__LORA_IOCTL_LCT_MODE_SET, &lct_state);
        }
    }
    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
