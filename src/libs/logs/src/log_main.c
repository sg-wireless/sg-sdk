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
 * @brief   It implements the main log function implementation. It implements 
 *          the log lib filteration capabilities as well.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "utils_misc.h"
#include "utils_fs_path.h"
#include "utils_bitwise.h"

#include "log_lib.h"
#include "log_buf_mgr.h"
#include "log_provider.h"
#include "log_obj.h"
#include "log_colors_defs.h"

__log_subsystem_def(log, default, 1, 1)

/* --- log design approach -------------------------------------------------- */

/** -------------------------------------------------------------------------- *
 * log line shape:
 * ===============
 * 
 * | <subsys> | <comp> | <file> | <lineno> | <log_type> | <func> | <msg>
 * 
 * <subsys>  attributes
 *      - log_color
 * <comp>   attributes
 *      - log_color
 * <log_type>
 *      - info
 *      - error
 *      - warn
 *      - assert
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * log usage approach:
 * ===================
 * = there is a default subsystem and component
 *      = default subsystem "default"
 *      = default component "default"
 * 
 * = any new subsystem will contain a 'default' component automatically
 * 
 * = subsystem "abc" has two sub-components "aa" and "bb"
 *      = in logs/log_subsys_comp.h, the following definition shall be occurred
 *          __log_subsystem_def(abc, green, 1, 1);
 *          __log_component_def(abc, aa, default, 1, 1);
 *          __log_component_def(abc, bb, yellow, 1, 1);
 * 
 * = subsystem "xyz" has three sub-components "aa", "dd", "gg"
 *      = in any file, the following definition shall be occurred
 *      = in logs/log_subsys_comp.h, the following definition shall be occurred
 *          __log_subsystem_def(xyz, blue, 1, 1);
 *          __log_component_def(xyz, aa, default, 1, 1);
 *          __log_component_def(xyz, dd, purple, 1, 1);
 *          __log_component_def(xyz, gg, default, 1, 1);
 * 
 * = logging use cases:
 *      #include "logs.h" // subsystem "default", component "default"
 *      void func_0 () {
 *          __log("-- subsystem [default] component[default] --\n");
 *      }
 * 
 *      #undef  __log_subsystem
 *      #define __log_subsystem     xyz
 *      void func_1 () {
 *          __log("-- subsystem [xyz    ] component[default] --\n");
 *      }
 * 
 *      #undef  __log_component
 *      #define __log_component     aa
 *      void func_1 () {
 *          __log("-- subsystem [xyz    ] component[aa     ] --\n");
 *      }
 * 
 *      #undef  __log_subsystem
 *      #define __log_subsystem     abc
 *      void func_1 () {
 *          __log("-- subsystem [abc    ] component[aa     ] --\n");
 *      }
 * 
 *      #undef  __log_component
 *      #define __log_component     gg
 *      void func_1 () {
 *          // -- the following log will cause compile error
 *          //    because there is no 'gg' component attached to 'abc' subsys
 *          __log("-- subsystem [xyz    ] component[gg     ] --\n");
 *      }
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * generated macros
 * ================
 *      for each subsystem the following macros are generated:
 *          __log_subsystem_<subsys>_id     // specifies the subsystem id
 *          __log_subsystem_<subsys>_on     // specifies default enable flag
 *          __log_subsystem_<subsys>_cc     // specifies the compile flag
 *          __log_subsystem_<subsys>_cl     // specifies the default color
 *          example
 *              #define __log_subsystem_lora_id     1
 *              #define __log_subsystem_lora_on     1
 *              #define __log_subsystem_lora_cc     1
 *              #define __log_subsystem_lora_cl     green
 *      for each component the following macros are generated:
 *          __log_component_<subsys>_<comp>_id  // specifies the component id
 *          __log_component_<subsys>_<comp>_on  // specifies default enable flag
 *          __log_component_<subsys>_<comp>_cc  // specifies the compile flag
 *          __log_component_<subsys>_<comp>_cl  // specifies the logs color
 *          example
 *              #define __log_component_lora_spi_id     2
 *              #define __log_component_lora_spi_on     1
 *              #define __log_component_lora_spi_cc     1
 *              #define __log_component_lora_spi_cl     blue
 * 
 * generated structs
 * =================
 *      static uint8_t s_log_subsystem_info [] = {
 *          // -- the index of the entry is the subsystem id itself
 *          // -- each entry contains the following subsystem info
 *          // b0:     < enable-flag >
 *          // b1:     < compile-flag >
 *          // b2:     < color-info-0 >
 *          // b3:     < color-info-1 >
 *          // b4:     < color-info-2 >
 *          // b5:     < color-info-3 >
 *          // b6:     < reserved >
 *          // b7:     < reserved >
 *      };
 *      static const char* s_log_subsystem_names [] = {
 *          "default",
 *          "lora"
 *      };
 *      static uint16_t s_log_component_info [] = {
 *          // -- each entry index represents the component id itself
 *          // -- entry info is as follows
 *          // b00:     < enable-flag >
 *          // b01:     < color-info-0 >
 *          // b02:     < color-info-1 >
 *          // b03:     < color-info-2 >
 *          // b04:     < color-info-3 >
 *          // b05:     < subsystem-id-0 >
 *          // b06:     < subsystem-id-1 >
 *          // b07:     < subsystem-id-2 >
 *          // b08:     < subsystem-id-3 >
 *          // b09:     < subsystem-id-4 >
 *          // b10:     < subsystem-id-5 >
 *          // b11:     < compile-flag >
 *          // b12:     < reserved >
 *          // b13:     < reserved >
 *          // b14:     < reserved >
 *          // b15:     < reserved >
 *      };
 *      static const char* s_log_component_names [] = {
 *          "default",
 *          "spi",
 *          "chip"
 *      };
 * 
 * auto-generated files
 * ====================
 *  - logs_gen_comp_ids.hh
 *      -> it contains the following macros definitions
 *          __log_subsystem_<subsys>_cc
 *          __log_component_<subsys>_<comp>_id
 *          __log_component_<subsys>_<comp>_cc
 *      -> it should be included by the logs.h to be visible across all logs
 *  - logs_gen_structs.cc
 *      -> it contains the following layout
 *          #include logs_gen_comp_ids.hh
 *          static uint8_t s_log_subsystem_info [] = {
 *              // -- subsystem '<subsys>'
 *              #define __log_subsystem_<subsys>_id     <gen-id>
 *              #define __log_subsystem_<subsys>_on     <enable_flag>
 *              #define __log_subsystem_<subsys>_cl     <default-color>
 *              [__log_subsystem_<subsys>_id] = __subsys_info_init(<subsys>),
 *              .
 *          };
 *          static const char* s_log_subsystem_names [] = {
 *              [__log_subsystem_<subsys>_id] = "<subsys>",
 *              .
 *          };
 *          static uint16_t s_log_component_info [] = {
 *              #define __log_component_<subsys>_<comp>_on  <enable-flag>
 *              #define __log_component_<subsys>_<comp>_cl  <color>
 *              [__log_component_<subsys>_<comp>_id] =
 *                  __comp_info_init(<subsys>, <comp>),
 *              .
 *          };
 *          static const char* s_log_component_names [] = {
 *              [__log_component_<subsys>_<comp>_id] = "<comp>",
 *              .
 *          };
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * macro to derive:
 *      __log_subsystem_<subsys>_id 
 *      __log_subsystem_<subsys>_on 
 *      __log_subsystem_<subsys>_cc 
 *      __log_subsystem_<subsys>_cl 
 * --------------------------------------------------------------------------- *
 */
#define __subsys__(_s, _m)  __tricat(__log_subsystem_, _s, __concat(_, _m))

/** -------------------------------------------------------------------------- *
 * macro to derive:
 *      __log_component_<subsys>_<comp>_id
 *      __log_component_<subsys>_<comp>_on
 *      __log_component_<subsys>_<comp>_cc
 *      __log_component_<subsys>_<comp>_cl
 * --------------------------------------------------------------------------- *
 */
#define __comp__(_s, _c, _m)                \
    __concat(                               \
        __tricat(__log_component_, _s, _),  \
        __tricat(_c, _, _m) )

/** -------------------------------------------------------------------------- *
 * info structs info packing and extracting
 * --------------------------------------------------------------------------- *
 */
#define __pack__(x, msk, pos)   ( (x & msk) << pos )
#define __extr__(x, msk, pos)   ( (x >> pos) & msk )

/** -------------------------------------------------------------------------- *
 * macros to control the subsystem info packing and extracting
 *        7  6  5  4  3  2  1  0
 *      |      color      |cc|on|
 * --------------------------------------------------------------------------- *
 */
#define __subsys_on_msk     1u
#define __subsys_on_pos     0u
#define __subsys_cc_msk     1u
#define __subsys_cc_pos     1u
#define __subsys_cl_msk     0x3Fu
#define __subsys_cl_pos     2u
#define __subsys_msk__(_f)   __tricat(__subsys_, _f, _msk)
#define __subsys_pos__(_f)   __tricat(__subsys_, _f, _pos)
#define __subsys_pack__(_s,_f)  (uint8_t) \
    __pack__(__subsys__(_s, _f), __subsys_msk__(_f), __subsys_pos__(_f))
#define __subsys_extr__(_i, _f)  \
    __extr__( _i, __subsys_msk__(_f), __subsys_pos__(_f) )
#define __subsys_info_init(_s)  \
    (__subsys_pack__(_s, on)|__subsys_pack__(_s, cc)|__subsys_pack__(_s, cl))

/** -------------------------------------------------------------------------- *
 * macros to control the component info packing and extracting
 *        15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 *        |res |      subsys-id  |     color       |cc|on|
 * --------------------------------------------------------------------------- *
 */
#define __comp_on_msk       1u      // -- on    enable-flag
#define __comp_on_pos       0u
#define __comp_cc_msk       1u      // -- cc    compile-flag
#define __comp_cc_pos       1u
#define __comp_cl_msk       0x3Fu   // -- cl    color
#define __comp_cl_pos       2u
#define __comp_ss_msk       0x3Fu   // -- ss    subsys-id
#define __comp_ss_pos       8u
#define __comp_msk__(_f)    __tricat(__comp_, _f, _msk)
#define __comp_pos__(_f)    __tricat(__comp_, _f, _pos)
#define __comp_pack__(_s, _c, _f)   (uint16_t) \
    __pack__(__comp__(_s, _c, _f), __comp_msk__(_f), __comp_pos__(_f))
#define __comp_extr__(_i, _f)  __extr__(_i, __comp_msk__(_f), __comp_pos__(_f))
#define __comp_info_init(_s, _c)                                            \
    (   __comp_pack__(_s, _c, on) | __comp_pack__(_s, _c, cc) |             \
        __comp_pack__(_s, _c, cl) |                                         \
        __pack__(__subsys__(_s, id), __comp_msk__(ss), __comp_pos__(ss)) )

/** -------------------------------------------------------------------------- *
 * - inject the autogenerated logs information arrays
 * - the file "logs_gen_structs.cc" contains the following arrays definitions
 * 
 *      // -- the used names in subsystems and components
 *      const char* s_used_names []
 * 
 *      // -- the subsystems info array
 *      uint8_t s_log_subsystem_info []
 * 
 *      // -- the subsystems names indexing array
 *      uint8_t s_log_subsystem_names_indices []
 * 
 *      // -- the components info array
 *      uint16_t s_log_component_info []
 * 
 *      // -- the components names indexing array
 *      uint8_t s_log_component_names_indices []
 * - the injection of the file "logs_gen_structs.cc" must be stay at this
 *   location because it depends on previous macros definitions
 * --------------------------------------------------------------------------- *
 */
#include "logs_gen_structs.cc"

#define __subsystem_name(id) s_used_names[s_log_subsystem_names_indices[id]]
#define __component_name(id) s_used_names[s_log_component_names_indices[id]]

/** -------------------------------------------------------------------------- *
 * type registeration procedures
 * --------------------------------------------------------------------------- *
 */
static log_type_info_t* s_registry_head = NULL;
void log_type_register(log_type_info_t * p_log_type_info)
{
    if( s_registry_head == NULL ) {
        s_registry_head = p_log_type_info;
    } else {
        log_type_info_t* p_type = s_registry_head;
        while(p_type->priv_data) {
            p_type = p_type->priv_data;
        }
        p_type->priv_data = p_log_type_info;
    }
    p_log_type_info->priv_data = NULL;
}
#define __registry_loop_begin(iter) \
    do {log_type_info_t* iter = s_registry_head;while(iter) {                               \

#define __registry_loop_end() \
    iter = iter->priv_data;}} while (0)

/** -------------------------------------------------------------------------- *
 * logs filter operations
 * --------------------------------------------------------------------------- *
 */
/* === log filter component regiseration ==================================== */
__log_component_def(log, filter, default, 1, 1)
#undef  __log_subsystem
#define __log_subsystem log
#undef  __log_component
#define __log_component filter

/* === common data ========================================================== */
static const char* s_onoff[] = {
    __red__"off"__default__,
    __green__"on"__default__};

/* === log header filter opertions ========================================== */
typedef struct {
    const char *    name;
    void (*log_provider)(log_info_base_t* p_basic_info);
    uint8_t         cc;
    uint8_t         en;
    uint8_t         width;
} log_header_seg_info_t;

#define __log_header_seg_provider_id(_seg) log_header_provide_##_seg
#define __log_header_seg_provider_dec(_seg)     \
    __opt_paste( __opt_log_header_##_seg, y,    \
        static void __log_header_seg_provider_id(_seg)(log_info_base_t*);)

__log_header_seg_provider_dec(timestamp)
__log_header_seg_provider_dec(log_type )
__log_header_seg_provider_dec(os_info  )
__log_header_seg_provider_dec(subsystem)
__log_header_seg_provider_dec(component)
__log_header_seg_provider_dec(filename )
__log_header_seg_provider_dec(line_num )
__log_header_seg_provider_dec(func_name)

static log_header_seg_info_t
s_log_header_seg_info[__opt_log_header_segments_count] = {
    #define __init_header_seg_info(_seg)                            \
        [__opt_log_header_seg_order_##_seg] = {                     \
            .name = #_seg,                                          \
            __opt_paste(__opt_log_header_##_seg, y,                 \
                .cc = 1,                                            \
                .log_provider = __log_header_seg_provider_id(_seg), \
            )                                                       \
            .en   = 1,                                              \
            .width = __opt_log_disp_w_##_seg                        \
        }

    __init_header_seg_info(timestamp),
    __init_header_seg_info(component),
    __init_header_seg_info(subsystem),
    __init_header_seg_info(filename ),
    __init_header_seg_info(func_name),
    __init_header_seg_info(line_num ),
    __init_header_seg_info(log_type ),
    __init_header_seg_info(os_info  )
    #undef __init_header_seg_info
};

#define __log_header_length  ( 1 +                                             \
    __opt_paste(__opt_log_header_timestamp, y,                                 \
        __opt_paste(__opt_log_header_timestamp_hhh_mm_ss, y, 13)               \
        __opt_paste(__opt_log_header_timestamp_hhh_mm_ss, n,                   \
            __opt_log_disp_w_timestamp) + 1 + )                                \
    __opt_paste(__opt_log_header_filename, y, __opt_log_disp_w_filename + 1 + )\
    __opt_paste(__opt_log_header_line_num, y, __opt_log_disp_w_line_num + 1 + )\
    __opt_paste(__opt_log_header_func_name, y,__opt_log_disp_w_func_name +1+ ) \
    __opt_paste(__opt_log_header_os_info, y,__opt_log_disp_w_os_info +1+ )     \
    __opt_log_disp_w_subsystem + 1 +                                           \
    __opt_log_disp_w_component + 1 +                                           \
    __opt_log_disp_w_log_type + 1 +                                            \
    0 )

uint32_t g_log_header_length = __log_header_length;

static void log_header_filter_list_stats(void)
{
    int i;
    __log_output("==> log header items stats:\n");
    __log_output("\t"__blue__"%-6s%-10s%-10s%s"__default__"\n",
        "order", "item", "compiled", "enabled");
    for(i = 0; i < __opt_log_header_segments_count; ++i) {
        __log_output("\t(%d)   %-13s%-9s%s\n", i,
            s_log_header_seg_info[i].name,
            s_onoff[s_log_header_seg_info[i].cc],
            s_onoff[s_log_header_seg_info[i].en]);
    }
    __log_output("\n");
}

void log_filter_header(const char* name, bool state)
{
    int i;
    for(i = 0; i < __opt_log_header_segments_count; ++i)
    {
        if(strcmp( name, s_log_header_seg_info[i].name ) == 0 )
        {
            if(s_log_header_seg_info[i].cc == 0)
            {
                __log_warn(" == log header item '"__red__"%s"__default__
                    "' is not compiled", name);
            }
            else
            {
                bool prev_state = s_log_header_seg_info[i].en;
                if( state != prev_state )
                {
                    g_log_header_length += (state - prev_state) *
                        (int)s_log_header_seg_info[i].width;
                }
                s_log_header_seg_info[i].en = (state == true);
                __log_info(" == log header item '"__purple__"%s"__default__
                    "' becomes '%s'", name, s_onoff[state]);
            }
            return;
        }
    }
    __log_warn(" == log header does not have the item '"
        __red__"%s"__default__"'", name);
}

void log_filter_header_reorder(const char* name, int new_order)
{
    int i;
    if( new_order >=  __opt_log_header_segments_count)
    {
        __log_warn(" == given order (%d) > max order (%d)",
            new_order, __opt_log_header_segments_count - 1);
        return;
    }

    for(i = 0; i < __opt_log_header_segments_count; ++i)
    {
        if(strcmp( name, s_log_header_seg_info[i].name ) == 0 )
        {
            __log_info(" == curr-order: %d , new-order: (%d)",
                i, new_order);
            if( new_order != i )
            {
                int j;
                log_header_seg_info_t seg = s_log_header_seg_info[i];
                if( new_order < i )
                {
                    for(j = i; j > new_order; --j)
                    {
                        s_log_header_seg_info[j] = s_log_header_seg_info[j-1];
                    }
                }
                else
                {
                    for(j = i; j < new_order; ++j)
                    {
                        s_log_header_seg_info[j] = s_log_header_seg_info[j+1];
                    }
                }
                s_log_header_seg_info[new_order] = seg;
            }
            __log_info(" == header reordered successfully");
            log_header_filter_list_stats();
            return;
        }
    }
    __log_warn(" == log header does not have the item '"
        __red__"%s"__default__"'", name);
}

/* === filter log types operations ========================================== */
static void log_types_filter_list_stats(void)
{
    __log_output("==> log types stats:\n");
    __log_output("\t"__blue__"%-10s%-10s%s"__default__"\n",
        "item", "compiled", "enabled");

    __registry_loop_begin(iter)
        __log_output("\t%-13s%-9s%s\n", iter->type_name,
            s_onoff[ iter->flags & __log_type_flag_cc ? 1 : 0 ],
            s_onoff[ iter->flags & __log_type_flag_en ? 1 : 0 ]
        );
    __registry_loop_end();
    __log_output("\n");
}

void log_filter_type(const char* name, bool state)
{
    __registry_loop_begin(iter)
        if( strcmp( name, iter->type_name ) == 0 ) {
            if( ! ( iter->flags & __log_type_flag_cc ) ) {
                __log_warn(" == log type '"__red__"%s"__default__
                    "' is not compiled", name);
            } else {
                __log_info(" == log type '"__purple__"%s"__default__
                    "' becomes '%s'", name, s_onoff[state]);
                iter->flags &= ~__log_type_flag_en;
                if(state)
                    iter->flags |= __log_type_flag_en;
            }
            return;
        }
    __registry_loop_end();
    __log_output("\n");

    __log_warn(" == non-registered log type '"__red__"%s"__default__"'", name);
}

/* === filter log subsystems and components operations ====================== */
#define __subsys_get_en(id)  __subsys_extr__(s_log_subsystem_info[id], on)
#define __subsys_get_cc(id)  __subsys_extr__(s_log_subsystem_info[id], cc)

#define __comp_get_ss(id)   __comp_extr__(s_log_component_info[id], ss)
#define __comp_get_en(id)   __comp_extr__(s_log_component_info[id], on)
#define __comp_get_cc(id)   __comp_extr__(s_log_component_info[id], cc)
static void log_header_filter_subsystems_stats(void)
{
    __log_output("==> log subsystems/components stats:\n");
    int sys_id;
    int cmp_id;
    const char* cc_str[] = {
        __red__ "not-compiled" __default__,
        __green__ "compiled" __default__};
    const char* en_str[] = {
        __red__ "disabled" __default__,
        __green__ "enabled" __default__};

    __log_output("\t================================\n");
    for(sys_id = 0; sys_id < __log_statistics_sybsystems_count; ++ sys_id) {
        __log_output("\t"__purple__"%-10s"__default__"%-13s" "%-10s\n",
            __subsystem_name(sys_id), cc_str[__subsys_get_cc(sys_id)],
            en_str[__subsys_get_en(sys_id)]);
        __log_output("\t    ----------------------------\n");
        for(cmp_id = 0; cmp_id < __log_statistics_components_count; ++cmp_id) {
            if( sys_id == __comp_get_ss(cmp_id) ) {
                __log_output("\t    "__blue__"%-13s%-10s%s\n",
                    __component_name(cmp_id), s_onoff[__comp_get_cc(cmp_id)],
                    s_onoff[__comp_get_en(cmp_id)]);
            }
        }
        __log_output("\t================================\n");
    }
}

#define __subsys_set_en(id, val) \
    __bitwise_bit_write(8, s_log_subsystem_info[id], __subsys_on_pos, val)
void log_filter_subsystem(const char* subsystem_name, bool state)
{
    int sys_id;
    for(sys_id = 0; sys_id < __log_statistics_sybsystems_count; ++ sys_id) {
        if(strcmp( subsystem_name, __subsystem_name(sys_id) ) == 0) {
            if( ! (__subsys_get_cc(sys_id) ) ) {
                __log_warn(" == non-compiled subsystem '"
                    __purple__"%s"__default__"'", subsystem_name);
            } else {
                __log_info(" == subsystem '"__purple__"%s"__default__
                    "' becomes '%s'", subsystem_name, s_onoff[state]);
                __subsys_set_en(sys_id, state);
            }
            return;
        }
    }
    __log_warn(" == non-existing subsystem '"__purple__"%s"__default__"'",
        subsystem_name);
}

bool log_filter_subsystem_get_state(const char* subsystem_name)
{
    int sys_id;
    bool state = false;
    for(sys_id = 0; sys_id < __log_statistics_sybsystems_count; ++ sys_id) {
        if(strcmp( subsystem_name, __subsystem_name(sys_id) ) == 0) {
            state = __subsys_get_cc(sys_id) && __subsys_get_en(sys_id);
            break;
        }
    }
    return state;
}

#define __comp_set_en(id, val) \
    __bitwise_bit_write(16, s_log_component_info[id], __comp_on_pos, val)
void log_filter_component(const char* subsys_name,
    const char* component_name, bool state)
{
    int sys_id;
    for(sys_id = 0; sys_id < __log_statistics_sybsystems_count; ++ sys_id) {
        if(strcmp( subsys_name, __subsystem_name(sys_id) ) == 0) {
            int cmp_id;
            if( ! (__subsys_get_cc(sys_id) ) ) {
                __log_warn(" == non-compiled subsystem '"
                    __purple__"%s"__default__"'",subsys_name);
            } else {
                for(cmp_id = 0; cmp_id < __log_statistics_components_count;
                    ++cmp_id) {
                    if( sys_id == __comp_get_ss(cmp_id) && 
                        strcmp(component_name, __component_name(cmp_id)) == 0) {
                        __log_info(" == component '"__blue__"%s"__default__
                            "' becomes '%s'", component_name, s_onoff[state]);
                        __comp_set_en(cmp_id, state);
                        return;
                    }
                }
                __log_warn(" == component '"__blue__"%s"__default__
                    "' not exist in subsystem '"__purple__"%s"__default__"'",
                    component_name, subsys_name);
            }
            return;
        }
    }
    __log_warn(" == non-existing subsystem '"__purple__"%s"__default__"'",
        subsys_name);
}
bool log_filter_component_get_state(const char* subsystem_name,
    const char* component_name)
{
    int sys_id;
    bool state = false;
    for(sys_id = 0; sys_id < __log_statistics_sybsystems_count; ++ sys_id) {
        if(strcmp( subsystem_name, __subsystem_name(sys_id) ) == 0) {
            int cmp_id;
            if( ! (__subsys_get_cc(sys_id) ) ) {
                break;
            } else {
                for(cmp_id = 0; cmp_id < __log_statistics_components_count;
                    ++cmp_id) {
                    if( sys_id == __comp_get_ss(cmp_id) && 
                        strcmp(component_name, __component_name(cmp_id)) == 0) {
                        state = __comp_get_en(cmp_id);
                        break;
                    }
                }
            }
            break;
        }
    }
    return state;
}

void log_filter_save_states_and_enable(log_filter_save_state_t* p_filter_state)
{
    p_filter_state->subsystem_save_state =
        log_filter_subsystem_get_state(p_filter_state->subsystem_name);
    p_filter_state->component_save_state =
        log_filter_component_get_state(
            p_filter_state->subsystem_name,
            p_filter_state->component_name);
    log_filter_subsystem(p_filter_state->subsystem_name, true);
    log_filter_component(p_filter_state->subsystem_name,
        p_filter_state->component_name, true);
}

void log_filter_restore_state(log_filter_save_state_t* p_filter_state)
{
    log_filter_subsystem(p_filter_state->subsystem_name,
        p_filter_state->subsystem_save_state);
    log_filter_component(p_filter_state->subsystem_name,
        p_filter_state->component_name,
        p_filter_state->component_save_state);
}

/* === generic filter operations ============================================ */
void log_filter_list_stats(void)
{
    log_header_filter_list_stats();
    log_types_filter_list_stats();
    log_header_filter_subsystems_stats();
}

/** -------------------------------------------------------------------------- *
 * logging intialization routine
 * --------------------------------------------------------------------------- *
 */
static volatile bool s_log_is_init = false;
static void log_engine_init(log_init_params_t* p_init_params);
void log_init(log_init_params_t* p_init_params)
{
    // -- register all basic types
    __log_type_register(output  );
    __log_type_register(printf  );
    __log_type_register(info    );
    __log_type_register(debug   );
    __log_type_register(warn    );
    __log_type_register(error   );
    __log_type_register(assert  );
    __log_type_register(mem_dump);
    __log_type_register(test    );
    __log_type_register(enforce );

    // -- port connection init
    extern void log_buf_mgr_init(log_init_params_t* p_init_params);
    log_buf_mgr_init(p_init_params);

    log_engine_init(p_init_params);

    s_log_is_init = true;
}

/** -------------------------------------------------------------------------- *
 * logging main operations routines
 * --------------------------------------------------------------------------- *
 */
static log_port_get_timestamp_t * p_timestamp_getter;
static log_port_get_current_core_id_t* p_get_core_id;
static log_port_get_current_task_name_t* p_get_task_name;
static uint32_t s_timestamp_counter = 0;
#define __log_get_timestamp() \
    p_timestamp_getter ? p_timestamp_getter() : s_timestamp_counter ++;
#define __log_get_taskname() \
    (p_get_task_name ? p_get_task_name() : "test")
#define __log_get_code_id() \
    (p_get_core_id ? p_get_core_id() : 0)
static void log_engine_init(log_init_params_t* p_init_params)
{
    s_timestamp_counter = 0;
    if(p_init_params) {
        p_timestamp_getter = p_init_params->get_timestamp;
        p_get_task_name = p_init_params->get_task_name;
        p_get_core_id = p_init_params->get_core_id;
    }
}
__log_type_def(NULL, output, 1);
__log_type_def(NULL, printf, 1);
__log_type_def(NULL, info  , 1);
__log_type_def(NULL, debug , 1);
__log_type_def(NULL, warn  , 1);
__log_type_def(NULL, error , 1);
__log_type_def(NULL, assert, 1);

typedef struct {
    uint32_t    started     :1;
    uint32_t    curr_color  :4;
} log_printf_flags_t;

#define __log_printf_flags_get_started() \
    __bitwise_bit_get(32, g_log_type_printf.priv_flags, 0)
#define __log_printf_flags_get_color() \
    __bitwise_bits_read(32, g_log_type_printf.priv_flags, 0xf, 1)

#define __log_printf_flags_set_started(val) \
    __bitwise_bit_write(32, g_log_type_printf.priv_flags, 0, val)
#define __log_printf_flags_set_color(val) \
    __bitwise_bits_write(32, g_log_type_printf.priv_flags, 0xf, 1, val)

/* main log method definition */
void log_stdout(log_info_t* log_info, const char* fmt, ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    log_stdout_fmt(log_info, fmt, arg_ptr);
    va_end(arg_ptr);
}

void log_stdout_fmt(log_info_t* log_info, const char* fmt, va_list arg_ptr)
{
    if(!s_log_is_init) return;
    log_info_base_t base_info = {
        .log_info = log_info
    };

    // -- set buffering info
    base_info.buf = log_buf_fetch(NULL);
    base_info.idx = 0;

    // -- set default color
    #if __opt_test(__opt_global_log_coloring, y)
    base_info.log_color = 0;
    base_info.curr_color = 0;
    base_info.color_len = 0;
    log_provide_color(&base_info, 0);
    #endif

    if(fmt)
    {
        // -- provide a normal formatted string
        log_provide_formatted_string(&base_info, fmt, arg_ptr);
    }
    else
    {
        if(log_info->p_type_info->p_provider)
        {
            log_info->p_basic_info = &base_info;
            log_info->p_type_info->p_provider(log_info);
        }
    }

    log_buf_append_char(&base_info, '\0');
    log_buf_commit(base_info.buf);
}

void log_endl(void)
{
    #if __opt_test(__opt_log_type_printf, y)
    if(__log_printf_flags_get_started()) {
        log_info_base_t base_info = {0};

        base_info.buf = log_buf_fetch(NULL);
        base_info.idx = 0;
        log_buf_append_char(&base_info, '\n');
        log_buf_append_char(&base_info, '\r');
        log_buf_append_char(&base_info, '\0');
        log_buf_commit(base_info.buf);
        __log_printf_flags_set_started(0);
        __log_printf_flags_set_color(0);
    }
    #endif
}

void log_enforce(const char* file, int line, const char* func,
    const char* fmt, ...)
{
    #if __opt_test(__opt_log_type_enforce, y)
    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    log_info_enforce_t log_info = {
        { .p_type_info = &g_log_type_enforce },
        .fmt_str = fmt,
        .arg_ptr = arg_ptr
    };

    log_impl(
        (void*)&log_info,
        __get_comp_id(default, default),
        __opt_paste(__opt_log_header_filename, y, file,)
        __opt_paste(__opt_log_header_line_num, y, line,)
        __opt_paste(__opt_log_header_func_name, y, func,)
        NULL
        );
    va_end(arg_ptr);
    #endif
}

void log_impl(
    log_info_t* log_info,
    int         comp_id,
    __opt_paste(__opt_log_header_filename, y, const char* file,)
    __opt_paste(__opt_log_header_line_num, y, int line,)
    __opt_paste(__opt_log_header_func_name, y, const char* func,)
    const char* fmt,
    ...)
{
    if(!s_log_is_init) return;
    log_info_base_t basic_info = {
        __opt_paste(__opt_log_header_filename, y, .file = file,)
        __opt_paste(__opt_log_header_line_num, y, .line = line,)
        __opt_paste(__opt_log_header_func_name, y, .func = func,)
        .comp_id = comp_id,
        .log_info = log_info
    };
    log_info_base_t * p_basic_info = & basic_info;
    const log_type_info_t *  type_info = log_info->p_type_info;
    log_info->p_basic_info = p_basic_info;

    // -- filter out non-enabled logs types
    if(! ( type_info->flags & __log_type_flag_en) ) {
        return;
    }

    // -- obtain component and subsystem info data
    uint16_t comp_info = s_log_component_info[comp_id];
    uint32_t subsys_id = __comp_extr__(comp_info, ss);
    p_basic_info->subsys_id = subsys_id;
    uint8_t  subsys_info = s_log_subsystem_info[subsys_id];

    // -- check subsystem log enable flag
    if( __subsys_extr__(subsys_info, on) == 0 ) {
        return;
    }

    // -- check component log enable flag
    if( __comp_extr__(comp_info, on) == 0 ) {
        return;
    }

    // -- specify default color based on the subsystem and component color
    char reset_cl = __comp_extr__(comp_info, cl);
    if( reset_cl == 0 ) {
        reset_cl = __subsys_extr__(subsys_info, cl);
    }

    // -- getting a free log buf
    p_basic_info->buf = log_buf_fetch(NULL);
    p_basic_info->idx = 0;

    // -- set default color
    #if __opt_test(__opt_global_log_coloring, y)
    p_basic_info->log_color = reset_cl;
    p_basic_info->curr_color = reset_cl;
    p_basic_info->color_len = 0;
    #endif

    // -- provide header information
    #if __opt_test(__opt_log_type_printf, y)
    {
        bool started = __log_printf_flags_get_started();
        if(type_info == &g_log_type_printf) {
            if( !started ) {
                // -- mark line start and provide header
                __log_printf_flags_set_started(1);
                log_provide_header(p_basic_info, false);
            } else {
                // -- line started before, no header provisioning
                #if __opt_test(__opt_global_log_coloring, y)
                p_basic_info->curr_color = __log_printf_flags_get_color();
                log_provide_color(p_basic_info, p_basic_info->curr_color);
                #endif
            }
        } else {
            if( started ) {
                // -- previous log was printf, so mark it as not started and
                //    provide a '\n' char to start new fresh line
                __log_printf_flags_set_started(0);
                log_provide_char(p_basic_info, '\n');
            } else {
                // -- no previous printf line, no new line is needed
            }
            // -- provide header
            log_provide_header(p_basic_info, false);
        }
    }
    #else
    log_provide_header(p_basic_info, false);
    #endif

    // -- provide the color again based on certain log types
    #if __opt_test(__opt_global_log_coloring, y)
    if(0){}
    __opt_paste( __opt_log_type_warn, y,
        else if( &g_log_type_warn == type_info ) {
            int color = __concat(__log_color_, __opt_log_type_color_warn);
            p_basic_info->log_color = color;
            log_provide_color(p_basic_info, color);
        }
    )
    __opt_paste( __opt_log_type_error, y,
        else if( &g_log_type_error == type_info ) {
            int color = __concat(__log_color_, __opt_log_type_color_error);
            p_basic_info->log_color = color;
            log_provide_color(p_basic_info, color);
        }
    )
    __opt_paste( __opt_log_type_assert, y,
        else if( &g_log_type_assert == type_info ) {
            int color = __concat(__log_color_, __opt_log_type_color_assert);
            p_basic_info->log_color = color;
            log_provide_color(p_basic_info, color);
        }
    )
    #endif

    // -- provide the log message
    if( type_info->p_provider ) {
        type_info->p_provider(log_info);
    }

    if( fmt != NULL ) {
        // -- provide a normal formatted string
        va_list arg_ptr;
        va_start(arg_ptr, fmt);
        log_provide_formatted_string(p_basic_info, fmt, arg_ptr);
        va_end(arg_ptr);
    }

    #if __opt_test(__opt_global_log_coloring, y)
    log_buf_append_char(p_basic_info, 0x1b);
    log_buf_append_char(p_basic_info, '[');
    log_buf_append_char(p_basic_info, 'm');
    #endif

    #if __opt_test(__opt_log_type_printf, y)
    if(type_info != &g_log_type_printf) {
        log_buf_append_char(p_basic_info, '\n');
        log_buf_append_char(p_basic_info, '\r');
    } else {
        #if __opt_test(__opt_global_log_coloring, y)
        __log_printf_flags_set_color(p_basic_info->curr_color);
        #endif
    }
    #else
    log_buf_append_char(p_basic_info, '\n');
    log_buf_append_char(p_basic_info, '\r');
    #endif

    log_buf_append_char(p_basic_info, '\0');
    log_buf_commit(p_basic_info->buf);
}

#if __opt_test(__opt_log_header_timestamp, y)
static void __log_header_seg_provider_id(timestamp)(log_info_base_t* p_info)
{
    uint32_t timestamp = __log_get_timestamp();
    #if __opt_test(__opt_log_header_timestamp_hhh_mm_ss, y)
    uint32_t hours = timestamp / (1000 * 60 * 60);
    uint32_t minutes = timestamp / ( 60 * 1000 ) - (hours * 60);
    uint32_t seconds = timestamp / ( 1000 ) - 
        (hours * 60 * 60) - ( minutes * 60 );
    uint32_t millis = timestamp % 1000;
    log_provide_printf(p_info, "%03d:%02d:%02d-%03d",
        hours, minutes, seconds, millis);
    #else
    log_provide_number( p_info, timestamp, 0, 0,
        __opt_log_disp_w_timestamp );
    #endif
}
#endif

#if __opt_test(__opt_log_header_log_type, y)
static void __log_header_seg_provider_id(log_type )(log_info_base_t* p_info)
{
    __opt_paste(__opt_global_log_coloring, y,
        __opt_paste(__opt_log_type_warn, y,
            if(p_info->log_info->p_type_info == &g_log_type_warn)
                log_provide_color(p_info, 
                    __concat(__log_color_, __opt_log_type_color_warn));
        )
        __opt_paste(__opt_log_type_error, y,
            if(p_info->log_info->p_type_info == &g_log_type_error)
                log_provide_color(p_info, 
                    __concat(__log_color_, __opt_log_type_color_error));
        )
        __opt_paste(__opt_log_type_assert, y,
            if(p_info->log_info->p_type_info ==&g_log_type_assert)
                log_provide_color(p_info, 
                    __concat(__log_color_,__opt_log_type_color_assert));
        )
    )
    log_provide_string(p_info, p_info->log_info->p_type_info->type_name,
        __opt_log_disp_w_log_type, 2);
    #if __opt_test(__opt_global_log_coloring, y) && \
        (__opt_test(__opt_log_type_warn, y) || \
            __opt_test(__opt_log_type_error, y) || \
            __opt_test(__opt_log_type_assert, y))
    log_provide_color(p_info, p_info->log_color);
    #endif
}
#endif

#if __opt_test(__opt_log_header_os_info, y)
static void __log_header_seg_provider_id(os_info  )(log_info_base_t* p_info)
{
    int w = __opt_log_disp_w_os_info;
    if(p_get_core_id) {
        log_provide_number(p_info, p_get_core_id(), 0, 0, 0);
        log_provide_char(p_info, ':');
        w -= 2;
    }
    log_provide_string(p_info, __log_get_taskname(), w, 1);
}
#endif

#if __opt_test(__opt_log_header_subsystem, y)
static void __log_header_seg_provider_id(subsystem)(log_info_base_t* p_info)
{
    log_provide_string(p_info, __subsystem_name(p_info->subsys_id),
        __opt_log_disp_w_subsystem, 2);
}
#endif

#if __opt_test(__opt_log_header_component, y)
static void __log_header_seg_provider_id(component)(log_info_base_t* p_info)
{
    log_provide_string(p_info, __component_name(p_info->comp_id),
        __opt_log_disp_w_component, 2);
}
#endif

#if __opt_test(__opt_log_header_filename, y)
static void __log_header_seg_provider_id(filename )(log_info_base_t* p_info)
{
    log_provide_string(p_info, notdir(p_info->file),
        __opt_log_disp_w_filename, 1);
}
#endif

#if __opt_test(__opt_log_header_line_num, y)
static void __log_header_seg_provider_id(line_num )(log_info_base_t* p_info)
{
    log_provide_number( p_info, p_info->line, 0, 0,
        __opt_log_disp_w_line_num );
}
#endif

#if __opt_test(__opt_log_header_func_name, y)
static void __log_header_seg_provider_id(func_name)(log_info_base_t* p_info)
{
    log_provide_string(p_info, p_info->func,
        __opt_log_disp_w_func_name, 1);
}
#endif

void log_provide_header(
    log_info_base_t* p_basic_info,
    bool        is_empty)
{
    const log_type_info_t * p_type_info = p_basic_info->log_info->p_type_info;
    if( p_type_info == &g_log_type_output ||
        (p_type_info->flags & __log_type_flag_out) ) {
        // no header provisioning for stdout logs
        return;
    }
    log_provide_char(p_basic_info, '\r');
    // -- set default color
    #if __opt_test(__opt_global_log_coloring, y)
    log_provide_color(p_basic_info, p_basic_info->log_color);
    #endif

    log_provide_char(p_basic_info, '|');

    int i;

    for(i = 0; i < __opt_log_header_segments_count; ++i)
    {
        if(s_log_header_seg_info[i].cc && s_log_header_seg_info[i].en)
        {
            if(is_empty)
            {
                log_provide_fill_width(p_basic_info, ' ', 
                    s_log_header_seg_info[i].width);
            }
            else
            {
                s_log_header_seg_info[i].log_provider(p_basic_info);
            }
            log_provide_char(p_basic_info, '|');
        }
    }

    #if __opt_test(__opt_global_log_coloring, y)
    log_provide_color(p_basic_info, p_basic_info->curr_color);
    #endif
}

/* -- end of file ----------------------------------------------------------- */
