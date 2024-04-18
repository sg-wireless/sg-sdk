/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   This file represents a uPython c-module interface to the exported
 *          APIs of system information.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <string.h>
#include <stdbool.h>
#include "mp_lite_if.h"
#include "py/objstr.h"
#include "sysinfo.h"
#include "fw_version.h"

/* --- module functions definitions ----------------------------------------- */

__mp_mod_name(sysinfo, SystemInfo);


__mp_mod_fun_0(sysinfo, show_board) (void) {
    sysinfo_board();
    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, board) (void) {

    static const qstr board_dict_keys[] = {
        MP_QSTR_full_name,
        MP_QSTR_platform,
        MP_QSTR_module_name,
        MP_QSTR_module_number,
        MP_QSTR_shield
    };
    static MP_DEFINE_STR_OBJ(full_name_obj, SDK_BOARD);
    static MP_DEFINE_STR_OBJ(platform_obj, SDK_PLATFORM);
    static MP_DEFINE_STR_OBJ(module_name_obj, SDK_BOARD_NAME);
    static MP_DEFINE_STR_OBJ(module_number_obj, SDK_BOARD_NUMBER);
    static MP_DEFINE_STR_OBJ(shield_obj, SDK_BOARD_SHIELD);

    static MP_DEFINE_ATTRTUPLE(
        board_dict_obj,
        board_dict_keys,
        sizeof(board_dict_keys)/sizeof(board_dict_keys[0]),
        MP_ROM_PTR(&full_name_obj),
        MP_ROM_PTR(&platform_obj),
        MP_ROM_PTR(&module_name_obj),
        MP_ROM_PTR(&module_number_obj),
        MP_ROM_PTR(&shield_obj)
        );

    return MP_OBJ_FROM_PTR(&board_dict_obj);
}

__mp_mod_fun_0(sysinfo, show_flash) (void) {

    sysinfo_flash_stats();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, show_spiram) (void) {

    sysinfo_spiram_stats();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, show_version) (void) {

    sysinfo_version();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, version) (void) {

    static const qstr fw_version_dict_keys[] = {
        MP_QSTR_major,
        MP_QSTR_minor,
        MP_QSTR_patch,
        MP_QSTR_git_delta,
        MP_QSTR_git_tag,
        MP_QSTR_build_date,
        MP_QSTR_build_time,
        MP_QSTR_custom,
        MP_QSTR_release,
        MP_QSTR_build
    };

    static mp_obj_str_t git_tag_str_obj = {.base = {&mp_type_str}};
    static mp_obj_str_t date_str_obj = {.base = {&mp_type_str}};
    static mp_obj_str_t time_str_obj = {.base = {&mp_type_str}};
    static mp_obj_str_t custom_str_obj = {.base = {&mp_type_str}};
    static mp_obj_str_t release_str_obj = {.base = {&mp_type_str}};
    static mp_obj_str_t build_str_obj = {.base = {&mp_type_str}};

    static mp_obj_tuple_t fw_version_dict_obj = {
        .base = {&mp_type_attrtuple},
        .len = sizeof(fw_version_dict_keys)/sizeof(fw_version_dict_keys[0]),
        .items = { 
            0,
            0,
            0,
            0,
            MP_ROM_PTR(&git_tag_str_obj),
            MP_ROM_PTR(&date_str_obj),
            MP_ROM_PTR(&time_str_obj),
            MP_ROM_PTR(&custom_str_obj),
            MP_ROM_PTR(&release_str_obj),
            MP_ROM_PTR(&build_str_obj),
            MP_ROM_PTR((void *)fw_version_dict_keys)
        }
    };

    static bool initialized = false;

    if(! initialized)
    {
        git_tag_str_obj.data = (const byte *)fw_version_git_tag_short_str();
        git_tag_str_obj.len = strlen((const char*)git_tag_str_obj.data);

        date_str_obj.data = (const byte *)fw_version_date_str();
        date_str_obj.len = strlen((const char*)date_str_obj.data);

        time_str_obj.data = (const byte *)fw_version_time_str();
        time_str_obj.len = strlen((const char*)time_str_obj.data);

        custom_str_obj.data = (const byte *)fw_version_custom_str();
        custom_str_obj.len = strlen((const char*)custom_str_obj.data);

        release_str_obj.data = (const byte *)fw_version_release_str();
        release_str_obj.len = strlen((const char*)release_str_obj.data);

        build_str_obj.data = (const byte *)fw_version_string();
        build_str_obj.len = strlen((const char*)build_str_obj.data);

        fw_version_dict_obj.items[0] = MP_ROM_INT(fw_version_release_major());
        fw_version_dict_obj.items[1] = MP_ROM_INT(fw_version_release_minor());
        fw_version_dict_obj.items[2] = MP_ROM_INT(fw_version_release_patch());
        fw_version_dict_obj.items[3] = MP_ROM_INT(fw_version_git_delta());

        initialized = true;
    }


    return MP_OBJ_FROM_PTR(&fw_version_dict_obj);
}

__mp_mod_fun_0(sysinfo, show_efuses) (void) {

    sysinfo_efuses();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, show_all) (void) {

    sysinfo_board();
    sysinfo_version();
    sysinfo_efuses();
    sysinfo_flash_stats();
    sysinfo_spiram_stats();

    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
