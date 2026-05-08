/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
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
#include "esp_heap_caps.h"

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

__mp_mod_fun_0(sysinfo, show_memory) (void) {

    sysinfo_memory_stats();

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
    sysinfo_memory_stats();
    sysinfo_spiram_stats();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, memory) (void) {

    static const qstr memory_dict_keys[] = {
        MP_QSTR_total_heap,
        MP_QSTR_free_heap,
        MP_QSTR_used_heap,
        MP_QSTR_total_internal,
        MP_QSTR_free_internal,
        MP_QSTR_used_internal,
        MP_QSTR_total_psram,
        MP_QSTR_free_psram,
        MP_QSTR_used_psram,
        MP_QSTR_total_dma,
        MP_QSTR_free_dma,
        MP_QSTR_used_dma,
        MP_QSTR_largest_free_heap,
        MP_QSTR_largest_free_internal,
        MP_QSTR_largest_free_psram,
        MP_QSTR_largest_free_dma,
        MP_QSTR_internal_usage_percent,
        MP_QSTR_psram_usage_percent,
        MP_QSTR_dma_usage_percent,
    };

    static mp_obj_dict_t memory_dict_obj;
    static mp_map_elem_t memory_dict_table[MP_ARRAY_SIZE(memory_dict_keys)];
    static bool initialized = false;

    if (!initialized) {
        memory_dict_obj.base.type = &mp_type_dict;
        memory_dict_obj.map.alloc = MP_ARRAY_SIZE(memory_dict_table);
        memory_dict_obj.map.used = MP_ARRAY_SIZE(memory_dict_table);
        memory_dict_obj.map.table = memory_dict_table;

        for (size_t i = 0; i < MP_ARRAY_SIZE(memory_dict_keys); i++) {
            memory_dict_obj.map.table[i].key = MP_OBJ_NEW_QSTR(memory_dict_keys[i]);
        }

        initialized = true;
    }

    // Get current memory stats
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t used_heap = total_heap - free_heap;
    
    size_t total_internal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t used_internal = total_internal - free_internal;
    
    size_t total_spiram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_spiram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t used_spiram = total_spiram - free_spiram;
    
    size_t total_dma = heap_caps_get_total_size(MALLOC_CAP_DMA);
    size_t free_dma = heap_caps_get_free_size(MALLOC_CAP_DMA);
    size_t used_dma = total_dma - free_dma;
    
    size_t largest_free_heap = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    size_t largest_free_internal = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    size_t largest_free_spiram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    size_t largest_free_dma = heap_caps_get_largest_free_block(MALLOC_CAP_DMA);

    float internal_usage = (used_internal * 100.0f) / total_internal;
    float spiram_usage = total_spiram > 0 ? (used_spiram * 100.0f) / total_spiram : 0.0f;
    float dma_usage = (used_dma * 100.0f) / total_dma;

    // Update dictionary values
    memory_dict_obj.map.table[0].value = MP_ROM_INT(total_heap);
    memory_dict_obj.map.table[1].value = MP_ROM_INT(free_heap);
    memory_dict_obj.map.table[2].value = MP_ROM_INT(used_heap);
    memory_dict_obj.map.table[3].value = MP_ROM_INT(total_internal);
    memory_dict_obj.map.table[4].value = MP_ROM_INT(free_internal);
    memory_dict_obj.map.table[5].value = MP_ROM_INT(used_internal);
    memory_dict_obj.map.table[6].value = MP_ROM_INT(total_spiram);
    memory_dict_obj.map.table[7].value = MP_ROM_INT(free_spiram);
    memory_dict_obj.map.table[8].value = MP_ROM_INT(used_spiram);
    memory_dict_obj.map.table[9].value = MP_ROM_INT(total_dma);
    memory_dict_obj.map.table[10].value = MP_ROM_INT(free_dma);
    memory_dict_obj.map.table[11].value = MP_ROM_INT(used_dma);
    memory_dict_obj.map.table[12].value = MP_ROM_INT(largest_free_heap);
    memory_dict_obj.map.table[13].value = MP_ROM_INT(largest_free_internal);
    memory_dict_obj.map.table[14].value = MP_ROM_INT(largest_free_spiram);
    memory_dict_obj.map.table[15].value = MP_ROM_INT(largest_free_dma);
    memory_dict_obj.map.table[16].value = mp_obj_new_float(internal_usage);
    memory_dict_obj.map.table[17].value = mp_obj_new_float(spiram_usage);
    memory_dict_obj.map.table[18].value = mp_obj_new_float(dma_usage);

    return MP_OBJ_FROM_PTR(&memory_dict_obj);
}

/* --- end of file ---------------------------------------------------------- */
