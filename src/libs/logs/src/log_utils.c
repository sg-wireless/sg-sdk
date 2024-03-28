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
 * @brief   This file represents the logging utilities implementation for Pycom
 *          specific Firmware components
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */
#include <stdio.h>
#include "log_lib.h"

/* --- APIs Implementations ------------------------------------------------- */

void log_decorate_list_prepare_next_item(log_decorate_list_t* context)
{
    if(context->meta_started) {
        __log_printf(context->item_sep_str );
    } else {
        context->meta_started = 1;
    }

    if(context->meta_counter == context->items_per_line) {
        context->meta_counter = 0;
        __log_printf("\n");
        __log_printf_fill(context->list_tab_w,' ', 0);
    }

    ++ context->meta_counter;
}

void log_fill(log_util_info_t*p_info, uint32_t len, char fill_char,
    bool endline, bool is_output)
{
    char arr[10] = {'%'};
    int i = 1;
    uint32_t ll = len;

    while(ll)
    {
        arr[ i++ ] = '0' + ll % 10;
        ll /= 10;
    }
    arr[i] = 'c';
    int j = i + 1;
    if( endline )
        arr[j++] = '\n';
    arr[j] = '\0';

    --i;
    j = 1;
    while( i > j )
    {
        char t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
        --i;
        ++j;
    }

    if(is_output)
        __log_output(arr,fill_char);
    else
        __log_util_printf(p_info, arr,fill_char);
}

void log_header(log_util_info_t* p_info, const char* caption, uint32_t width,
    char fill_char, bool is_output)
{
    log_field(p_info, caption, width, fill_char, __center__, true, is_output);
}

int log_strlen(const char* str)
{
    int len = 0;
    while(*str)
    {
        if(str[0] == '\033' && str[1] == '[')
        {
            if(str[2] == 'm')
            {
                str += 3;
                continue;
            }
            else if(str[2] == '3' && str[3] > '0' && str[3] <= '9' &&
                str[4] == 'm')
            {
                str += 5;
                continue;
            }
        }
        else if(str[0] == '%' && str[1] == 'C')
        {
            char ch = str[2];
            if(ch == 'k' || ch == 'r' || ch == 'g' ||
               ch == 'y' || ch == 'b' || ch == 'p' ||
               ch == 'c' || ch == 'w' || ch == 'd')
            {
                str += 3;
                continue;
            }
        }

        if(*str)
        {
            ++len;
        }

        ++str;
    }
    return len;
}

void log_field_old(log_util_info_t* p_info, const char* str, uint32_t width,
    char fill_char, int align, bool endl, bool is_output)
{
    int str_len = log_strlen(str);
    int prefix_w = 0;
    int suffix_w = 0;

    if( align == __left__ )
    {
        str_len += 1;
        if( str_len < width )
        {
            prefix_w = 0;
            suffix_w = width - str_len;
        }
    }
    else if( align == __right__ )
    {
        str_len += 1;
        if( str_len < width )
        {
            suffix_w = 0;
            prefix_w = width - str_len;
        }
    }
    else // -- center
    {
        str_len += 2;
        if( str_len < width )
        {
            suffix_w = width - str_len;
            prefix_w = suffix_w >> 1;
            suffix_w -= prefix_w;
        }
    }

    if( prefix_w )
        log_fill(p_info, prefix_w, fill_char, 0, is_output);

    if(is_output)
        __log_output("%s%s%s", prefix_w ? " " : "",  str, suffix_w ? " " : "");
    else
        __log_util_printf(p_info, "%s%s%s", prefix_w ? " " : "",  str,
            suffix_w ? " " : "");

    if( suffix_w )
        log_fill(p_info, suffix_w, fill_char, endl, is_output);
}

void log_field(log_util_info_t* p_info, const char* str, uint32_t width,
    char fill_char, int align, bool endl, bool is_output)
{
    int str_len = log_strlen(str);
    int prefix_w = 0;
    int suffix_w = 0;

    if( align == __left__ )
    {
        if( str_len < width )
        {
            prefix_w = 0;
            suffix_w = width - str_len;
        }
    }
    else if( align == __right__ )
    {
        if( str_len < width )
        {
            suffix_w = 0;
            prefix_w = width - str_len;
        }
    }
    else if( align == __center__ )
    {
        if( str_len < width )
        {
            suffix_w = width - str_len;
            prefix_w = suffix_w >> 1;
            suffix_w -= prefix_w;
        }
    }

    if( prefix_w )
    {
        log_fill(p_info, prefix_w, fill_char, 0, is_output);
    }

    char fmt[10];
    snprintf(fmt, 10, "%%%ds", str_len);

    if(is_output)
    {
        __log_output(fmt, str);
    }
    else
    {
        __log_util_printf(p_info, fmt, str);
    }

    if( suffix_w )
    {
        log_fill(p_info, suffix_w, fill_char, false, is_output);
    }

    if(endl)
    {
        if(is_output)
        {
            __log_output("\n");
        }
        else
        {
            __log_util_printf(p_info, "\n");
        }
    }
}

void log_hex(log_util_info_t* p_util_info,
    uint8_t* buf, uint32_t len, bool lower, bool is_out)
{
    const char* fmt_1st = lower ? "%02x" : "%02X";
    const char* fmt_nxt = lower ? " %02x" : " %02X";

    if(is_out)
    {
        if( len -- ) {
            __log_output(fmt_1st, *buf++);
        }

        while( len -- ) {
            __log_output(fmt_nxt, *buf++);
        }
    }
    else
    {
        if( len -- ) {
            __log_util_printf(p_util_info, fmt_1st, *buf++);
        }

        while( len -- ) {
            __log_util_printf(p_util_info, fmt_nxt, *buf++);
        }
    }
}

static int s_list_tab_w = 0;
static int s_indent_w = 0;
void log_list_start(int tab_w)
{
    s_list_tab_w = tab_w > 0 ? tab_w : 4;
    s_indent_w = 0;
}
void log_list_end(void)
{
    s_list_tab_w = 0;
    s_indent_w = 0;
}
void log_list_item(const char* fmt, ...)
{
    __log_output_fill(s_indent_w, ' ', false);
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    __log_output_fmt(fmt, arg_ptr);
    va_end(arg_ptr);
    __log_output("\n");
}
void log_list_indent(void)
{
    s_indent_w += s_list_tab_w;
}
void log_list_outdent(void)
{
    if(s_indent_w >= s_list_tab_w)
        s_indent_w -= s_list_tab_w;
}

/* -- general strings ------------------------------------------------------- */

const char* g_on_off[] = {__red__"off"__default__, __green__"on "__default__};
const char* g_yes_no[] = {__red__"no "__default__, __green__"yes"__default__};

const char* g_timer_ctor    = __blue__"ctor()  "__default__" -> ";
const char* g_timer_dtor    = __blue__"~dtor() "__default__" -> ";
const char* g_timer_start   = __blue__"start   "__default__" -> ";
const char* g_timer_stop    = __blue__"stop    "__default__" -> ";
const char* g_timer_expire  = __blue__"expire  "__default__" -> ";

/* -- end of file ----------------------------------------------------------- */
