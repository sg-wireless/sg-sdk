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
 * --------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2022, Pycom Limited.
 * 
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * --------------------------------------------------------------------------- *
 * 
 * @author  Ahmed Sabry (Pycom, SG Wireless)
 * 
 * @brief   This file represents the logging utilities for Pycom specific
 *          Firmware components
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "mp_lite_if.h"
#include "log_lib.h"

#ifdef CONFIG_SDK_LOG_LIB_ENABLE
#ifdef CONFIG_SDK_LOG_LIB_MPY_CMOD_ENABLE

/* --- module functions definitions ----------------------------------------- */

__mp_mod_ifdef(logs, CONFIG_SDK_LOG_LIB_ENABLE)
__mp_mod_ifdef(logs, CONFIG_SDK_LOG_LIB_MPY_CMOD_ENABLE)

__mp_mod_fun_kw(logs, va_list_demo, 0) (
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_tab_str, MP_ARG_KW_ONLY | MP_ARG_OBJ,
            {.u_obj = MP_OBJ_NEW_QSTR(MP_QSTR_default_tab_str)} },
        { MP_QSTR_sep_str, MP_ARG_KW_ONLY | MP_ARG_OBJ,
            {.u_obj = mp_const_none} },
        { MP_QSTR_line_items, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 4} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
                    MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    bool subsystem_state = log_filter_subsystem_get_state("default");
    bool component_state = log_filter_component_get_state("default", "default");
    log_filter_subsystem("default", true);
    log_filter_component("default", "default", true);

    const char* tab_str = mp_get_string(args[0].u_obj);
    const char* sep_str = " , ";
    if(args[1].u_obj != mp_const_none)
        sep_str = mp_get_string(args[1].u_obj);
    int lines_items = (int)(args[2].u_int);

    __log_printf("tab_str '%s', lines_items [%d], sep_str '%s'\n",
        tab_str, lines_items, sep_str);

    struct{int id; void* addr;} arr[10] = {
        {3, (void*)0x08801000}, {5, (void*)0x08802000}, {9, (void*)0x08802000},
        {4, (void*)0x08801000}, {0, (void*)0x08802000}, {6, (void*)0x08802000},
        {7, (void*)0x08801000}, {1, (void*)0x08802000}, {2, (void*)0x08802000},
        {8, (void*)0x08801000}
    };
    (void)arr[0];

    #define __tab_width    (strlen(tab_str) + 1)

    __log_decorate_list_start( id_addr_list, lines_items, __tab_width, sep_str);
    __log_printf("%s ", tab_str);
    for(int i=0; i < 10; ++i) {
        __log_decorate_list_printf_item(id_addr_list, __green__ "%d"
            __default__ "(" __yellow__ "%p" __default__ ")",
            arr[i].id, arr[i].addr);
    }
    __log_endl();

    log_filter_component("default", "default", component_state);
    log_filter_subsystem("default", subsystem_state);
    return mp_const_none;
}

__mp_mod_fun_0(logs, filter_stats)(void) {
    log_filter_list_stats();
    return mp_const_none;
}

__mp_mod_fun_2(logs, filter_subsystem)(mp_obj_t subsys_obj, mp_obj_t state_obj) {

    const char* subsys_str = mp_get_string(subsys_obj);
    if(subsys_str) {
        if(mp_obj_is_bool(state_obj))
            log_filter_subsystem(subsys_str, state_obj == mp_const_true);
        else
            __log_error("passing non bool value");
    } else {
        __log_error("passing non string obj");
    }
    return mp_const_none;
}

__mp_mod_fun_3(logs, filter_component)(
    mp_obj_t subsys_obj, mp_obj_t comp_obj, mp_obj_t state_obj) {

    const char* subsys_str = mp_get_string(subsys_obj);
    const char* comp_str = mp_get_string(comp_obj);
    if(subsys_str && comp_str) {
        if(mp_obj_is_bool(state_obj))
            log_filter_component(subsys_str, comp_str,
                state_obj == mp_const_true);
        else
            __log_error("passing non bool value");
    } else if(!subsys_str) {
        __log_error("passing subsystem non string obj");
    } else {
        __log_error("passing component non string obj");
    }
    return mp_const_none;
}
__mp_mod_fun_2(logs, filter_header)(
    mp_obj_t header_item_obj, mp_obj_t state_obj) {

    const char* header_item_str = mp_get_string(header_item_obj);
    if(header_item_str) {
        if(mp_obj_is_bool(state_obj))
            log_filter_header(header_item_str, state_obj == mp_const_true);
        else
            __log_error("passing non bool value");
    } else
        __log_error("passing non header item name string obj");
    return mp_const_none;
}
__mp_mod_fun_2(logs, filter_log_type)(
    mp_obj_t log_type_item_obj, mp_obj_t state_obj) {

    const char* log_type_item_str = mp_get_string(log_type_item_obj);
    if(log_type_item_str) {
        if(mp_obj_is_bool(state_obj))
            log_filter_type(log_type_item_str, state_obj == mp_const_true);
        else
            __log_error("passing non bool value");
    } else
        __log_error("passing non log type name string obj");
    return mp_const_none;
}

__mp_mod_fun_2(logs, header_reorder)(
    mp_obj_t header_seg, mp_obj_t new_order) {

    const char* log_type_item_str = mp_get_string(header_seg);
    if(log_type_item_str) {
        if(mp_obj_is_small_int(new_order)) {
            int order_value = MP_OBJ_SMALL_INT_VALUE(new_order);
            log_filter_header_reorder(log_type_item_str, order_value);
        } else
            __log_error("passing non int value for new order");
    } else
        __log_error("passing non log header segment name");
    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
#endif /* CONFIG_SDK_LOG_LIB_MPY_CMOD_ENABLE */
#endif /* CONFIG_SDK_LOG_LIB_ENABLE */
