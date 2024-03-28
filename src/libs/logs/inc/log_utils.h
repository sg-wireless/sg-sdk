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
 * @brief   This file represents the logging utilities for Pycom specific
 *          Firmware components
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * Text Drawing Helper macros
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    int comp_id;
    __opt_paste(__opt_log_header_filename, y, const char* file;)
    __opt_paste(__opt_log_header_line_num, y, int line;)
    __opt_paste(__opt_log_header_func_name, y, const char* func;)
    log_info_t log_info;
} log_util_info_t;

#define __log_utility(_utility, args...)                                    \
    __opt_paste(__get_log_type_opt(printf), y,                              \
        __opt_paste(__get_curr_subsys_cc(), 1,                              \
            __opt_paste(__get_curr_comp_cc(), 1,                            \
            do {                                                            \
                log_util_info_t util_info = {                               \
                __opt_paste(__opt_log_header_filename, y, .file = __FILE__,)\
                __opt_paste(__opt_log_header_line_num, y, .line = __LINE__,)\
                __opt_paste(__opt_log_header_func_name, y, .func = __func__,)\
                .comp_id = __get_curr_comp_id(),                            \
                .log_info.p_type_info = &g_log_type_printf                  \
                };                                                          \
                _utility(&util_info, args);                                 \
            } while(0)                                                      \
            )                                                               \
        )                                                                   \
    )
#define __log_util_printf(p_util_info, args...)                             \
    log_impl(&p_util_info->log_info, p_util_info->comp_id,                  \
        __opt_paste(__opt_log_header_filename, y, p_util_info->file,)       \
        __opt_paste(__opt_log_header_line_num, y, p_util_info->line,)       \
        __opt_paste(__opt_log_header_func_name, y, p_util_info->func,)       \
        args                                                                \
    )

/** -------------------------------------------------------------------------- *
 * variable list logging:
 * =====================
 * problem description:
 *   to log a list of item in a line breaks form as an example, we want
 *   to log the following array:
 *       struct{int id; void* addr;} arr[10] = {
 *           {3, 0x08801000}, {5, 0x08802000}, {9, 0x08802000},
 *           {4, 0x08801000}, {0, 0x08802000}, {6, 0x08802000},
 *           {7, 0x08801000}, {1, 0x08802000}, {2, 0x08802000},
 *           {8, 0x08801000}
 *       };
 *   as:
 *   |<-tab->|
 *   |       3(0x08801000) , 5(0x08802000) , 9(0x08802000) , 4(0x08801000) , 
 *   |       0(0x08802000) , 6(0x08802000) , 7(0x08801000) , 1(0x08802000) , 
 *   |       2(0x08802000} , 8(0x08801000)
 * 
 *   we shall use following code snippet:
 *   the first ta
 *       // <-tab-> is 7 char
 *       __log_va_list_init( id_addr_list, 4, 7, " , ");
 *       for(int i=0; i < 10; ++i) {
 *           __log_va_list_item(id_addr_list, "%d(0x%08x)",
 *               arr[i].id, arr[i].addr);
 *       }
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    int items_per_line;
    int list_tab_w;
    const char* item_sep_str;
    int meta_counter, meta_started;
} log_decorate_list_t;

#define __log_decorate_list_start(list_name, line_items, tab_w, sep_str)    \
    log_decorate_list_t list_name = {line_items, tab_w, sep_str, 0, 0}
void log_decorate_list_prepare_next_item(log_decorate_list_t* context);

#define __log_decorate_list_printf_item(list_name, item_str...)             \
     log_decorate_list_prepare_next_item(&list_name);                       \
    __log_printf(item_str)

#define __log_printf_fill(__len, __fill_char, __endl) \
    __log_utility(log_fill, __len, __fill_char, __endl, false)
#define __log_output_fill(__len, __fill_char, __endl) \
    log_fill(NULL, __len, __fill_char, __endl, true)

void log_fill(log_util_info_t* p_info, uint32_t len, char fill_char,
    bool endline, bool is_output);

#define __log_printf_header(__caption, __width, __fill_char) \
    __log_utility(log_header, __caption, __width, __fill_char, false)
#define __log_output_header(__caption, __width, __fill_char) \
    log_header(NULL, __caption, __width, __fill_char, true)

void log_header(log_util_info_t* p_info, const char* caption, uint32_t width,
    char fill_char, bool is_output);

#define __left__    (0)
#define __right__   (1)
#define __center__  (2)

#define __log_printf_field(_str, _width, _fil_ch, _align, _endl) \
    __log_utility(log_field, _str, _width, _fil_ch, _align, _endl, false)
#define __log_output_field(_str, _width, _fil_ch, _align, _endl) \
    log_field(NULL, _str, _width, _fil_ch, _align, _endl, true)

void log_field(log_util_info_t* p_info, const char* str, uint32_t width,
    char fill_char, int align, bool endl, bool is_output);

int log_strlen(const char* str);

#define __log_printf_hex_upper(buf, len)    \
    __log_utility(log_hex, (void*)(buf), len, false, false)
#define __log_printf_hex_lower(buf, len)    \
    __log_utility(log_hex, (void*)(buf), len, true, false)
#define __log_output_hex_upper(buf, len)    \
    __log_utility(log_hex, (void*)(buf), len, false, true)
#define __log_output_hex_lower(buf, len)    \
    __log_utility(log_hex, (void*)(buf), len, true, true)

void log_hex(log_util_info_t* p_util_info,
    uint8_t* buf, uint32_t len, bool lower, bool);

void log_list_start(int tab_w);
void log_list_end(void);
void log_list_item(const char* fmt, ...);
void log_list_indent(void);
void log_list_outdent(void);

#define __log_timer_ctor(_name) __log_info("%s%s", g_timer_ctor, _name)
#define __log_timer_dtor(_name) __log_info("%s%s", g_timer_dtor, _name)
#define __log_timer_start(_name, _period) \
    __log_info("%s%s - %d msec", g_timer_start, _name, _period)
#define __log_timer_stop(_name) __log_info("%s%s", g_timer_stop, _name)
#define __log_timer_expire(_name) __log_info("%s%s", g_timer_expire, _name)

/** -------------------------------------------------------------------------- *
 * tabular helper macros
 * --------------------------------------------------------------------------- *
 */
#define __col_w(_i, _c, _w, _s)                 \
    ( (_i) == (1) ? (_w) / (_c) - (_s) / 2      \
        : (_i) == (_c) ?                        \
            (_w) - ((_w) / (_c)) * ((_c) - 1) - \
                ((_s) - (_s) / 2)               \
        : (_w) / (_c) - (_s)                    \
    )

/**
 * to use the following macros:
 * 
 * to be able to use the following macros, follow the following example:
 * 
 *      // each column shall have a defined width in the format
 *      //  __w_<column-header-string>
 *      #define __w_label_col    8  // don't surround the number with
 *                                  // parenthesis. It shall be a bare number
 *      #define __w_desc_col    17
 *      #define __w_value_col   15
 * 
 *      {
 *          __log_col_header_c(label_col);  // centered header
 *          __log_col_header_l(desc_col);   // left aligned header
 *          __log_col_header_r(value_col);  // right aligned header
 *          __log_output("\n");
 * 
 *          for(int i = 0; i < 10; i ++)
 *          {
 *              __log_col_str_val_c(label_col, "string");
 *              __log_col_str_val_color_c(desc_col, __green__, "desc str");
 *              __log_col_int_val(value_col, 4, i);
 *              __log_output("\n");
 *          }
 *      }
 */

/* ---[ columns header macros ]--- */
#define __log_col_header_c(_col) \
    __log_output(__green__"%-+"__stringify(__concat(__w_, _col))"s", #_col)
#define __log_col_header_l(_col) \
    __log_output(__green__"%-"__stringify(__concat(__w_, _col))"s", #_col)
#define __log_col_header_r(_col) \
    __log_output(__green__"%"__stringify(__concat(__w_, _col))"s", #_col)

/* ---[ column string value ]--- */
#define __log_col_str_val_c(_col, _str) \
    __log_output("%-+"__stringify(__concat(__w_, _col))"s", _str)
#define __log_col_str_val_l(_col, _str) \
    __log_output("%-"__stringify(__concat(__w_, _col))"s", _str)
#define __log_col_str_val_r(_col, _str) \
    __log_output("%"__stringify(__concat(__w_, _col))"s", _str)

/* ---[ column colored string value ]--- */
#define __log_col_str_val_color_c(_col, _color, _str) \
    __log_output(_color"%-+"__stringify(__concat(__w_, _col))"s", _str)
#define __log_col_str_val_color_l(_col, _color, _str) \
    __log_output(_color"%-"__stringify(__concat(__w_, _col))"s", _str)
#define __log_col_str_val_color_r(_col, _color, _str) \
    __log_output(_color"%"__stringify(__concat(__w_, _col))"s", _str)

/* ---[ column integer value ]--- */
#define __log_col_int_val(_col, _w, _val) \
    __log_output("%"__stringify(_w)"d", _val); \
    __log_output_fill(__concat(__w_, _col) - _w, ' ',  false)

/* ---[ column float value ]--- */
#define __log_col_float_val(_col, _num_w, _frac_w, _val) \
    __log_output("%"__stringify(_num_w)"."__stringify(_frac_w)"f", _val); \
    __log_output_fill(__concat(__w_, _col) - _num_w, ' ',  false)

/* ---[ column hex 32 bits value ]--- */
#define __log_col_hex8_val(_col, _val) \
    __log_output("%08x", _val); \
    __log_output_fill(__concat(__w_, _col) - 8, ' ',  false)

/* -- general strings ------------------------------------------------------- */

extern const char* g_on_off[];
extern const char* g_yes_no[];

extern const char* g_timer_ctor;
extern const char* g_timer_dtor;
extern const char* g_timer_start;
extern const char* g_timer_stop;
extern const char* g_timer_expire;

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_UTILS_H__ */
