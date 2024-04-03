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
 * @brief   micropython C module for system inspection utilities.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "driver/periph_ctrl.h"

#include "mp_lite_if.h"
#include "log_lib.h"

#ifdef CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_INTERFACE_ENABLE
/* --- module functions definitions ----------------------------------------- */

__mp_mod_ifdef(sys_inspect,
    CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_INTERFACE_ENABLE)

__mp_mod_name(sys_inspect, System_Inspection);

__mp_mod_fun_ifdef(sys_inspect, dump_memory,
    CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_MEMORY_DUMP_ENABLE);

__mp_mod_fun_ifdef(sys_inspect, periph_module_list,
    CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_PERIPH_POWER_ENABLE);
__mp_mod_fun_0(sys_inspect, periph_module_list) (void) {

    log_filter_save_state_t state = {.subsystem_name="default",
        .component_name="default"};
    log_filter_save_state(&state, true);
    void periph_module_list(void);
    periph_module_list();
    log_filter_restore_state(&state);

    return mp_const_none;
}

__mp_mod_fun_ifdef(sys_inspect, periph_power,
    CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_PERIPH_POWER_ENABLE);
__mp_mod_fun_2(sys_inspect, periph_power)(
    mp_obj_t o_periph_str, mp_obj_t o_enable) {

    const char* periph_name = mp_get_string(o_periph_str);

    if(! periph_name) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("arg 1 must be a string"));
    }

    if(! mp_obj_is_bool(o_enable)) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("arg 2 must be a bool"));
    }

    extern periph_module_t periph_module_get_id(const char* name);

    periph_module_t perif_id = periph_module_get_id(periph_name);

    if( mp_obj_is_true(o_enable) )
    {
        periph_module_enable(perif_id);
    }
    else
    {
        periph_module_disable(perif_id);
    }

    return mp_const_none;
}

__mp_mod_fun_ifdef(sys_inspect, dump_memory,
    CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_MEMORY_DUMP_ENABLE);
__mp_mod_fun_kw(sys_inspect, dump_memory, 0)(
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args) {
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_address, MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED, {0}},
        { MP_QSTR_size, MP_ARG_KW_ONLY | MP_ARG_INT | MP_ARG_REQUIRED, {0}},
        { MP_QSTR_wordsize,     MP_ARG_KW_ONLY | MP_ARG_INT,{.u_int = 32}},
        { MP_QSTR_linesize,     MP_ARG_KW_ONLY | MP_ARG_INT,{.u_int = 32}},
        { MP_QSTR_disp_text,    MP_ARG_KW_ONLY | MP_ARG_BOOL,{.u_bool = false}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_address_int       args[0].u_int
    #define __arg_size_int          args[1].u_int
    #define __arg_wordsize_int      args[2].u_int
    #define __arg_linesize_int      args[3].u_int
    #define __arg_disp_text_bool    args[4].u_bool

    uint32_t wl;
    switch(__arg_wordsize_int)
    {
        case 8:     wl = __word_len_8;  break;
        case 16:    wl = __word_len_16; break;
        case 32:    wl = __word_len_32; break;
        default:
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("unsupported word size"));
    }

    if(__arg_linesize_int < (__arg_wordsize_int >> 2))
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("line size can not be less than word size"));
    }

    __log_output("dumping memory starting at address: "
        __yellow__"0x%08x"__default__" and of length "__yellow__"%d\n",
        __arg_address_int, __arg_size_int);

    __log_output_dump(__arg_address_int, __arg_size_int, __arg_linesize_int,
        __arg_disp_text_bool ?
            __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs : 0
        , wl);

    __log_output("\n");

    return mp_const_none;
    #undef __arg_address_int
    #undef __arg_size_int
    #undef __arg_wordsize_int
    #undef __arg_linesize_int
    #undef __arg_disp_text_bool
}

/* --- end of file ---------------------------------------------------------- */
#endif /* CONFIG_SDK_PLATFORM_SYSTEM_INSPECTION_INTERFACE_ENABLE */
