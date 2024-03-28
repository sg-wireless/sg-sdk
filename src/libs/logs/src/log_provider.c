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
 * @brief   This file represents the implementation to the logs basic
 *          provisioning methods used to provide basic log patterns into the
 *          logs buffers.
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "utils_misc.h"

#include "log_config.h"
#include "log_obj.h"
#include "log_provider.h"
#include "log_buf_mgr.h"
#include "log_colors_defs.h"
#include "log_lib.h"

/* --- APIs ----------------------------------------------------------------- */

void log_provide_color(
    log_info_base_t* p_basic_info,
    char    color)
{
    #if __opt_test(__opt_global_log_coloring, y)
    log_buf_append_char(p_basic_info, 0x1b);
    log_buf_append_char(p_basic_info, '[');
    if( color ) {
        log_buf_append_char(p_basic_info, '3');
        log_buf_append_char(p_basic_info, '0' + color);
        p_basic_info->color_len += 5;
    } else {
        p_basic_info->color_len += 3;
    }
    log_buf_append_char(p_basic_info, 'm');
    #endif
}

#define __set_num_flag(_var, _flag) _var |= __concat(__disp_num_flag_, _flag)
#define __clr_num_flag(_var, _flag) _var &= ~__concat(__disp_num_flag_, _flag)
#define __get_num_flag(_bool, _flags, _flag) \
    _bool = ( _flags & __concat(__disp_num_flag_, _flag) ) != 0
#define __tst_num_flag(_flags, _flag) \
    ( _flags & __concat(__disp_num_flag_, _flag) )

void log_provide_number(
    log_info_base_t* p_basic_info,
    uint64_t    num,
    uint32_t    flags,
    int         precesion,
    int         w)
{
    char s_digits[20];
    int  len = 0;
    char sign = 0;
    char fil_char;
    bool zero_pad  = __tst_num_flag(flags, zero_pad);
    bool lower_hex = __tst_num_flag(flags, lower_hex);
    bool precise   = __tst_num_flag(flags, precise);
    bool is_signed = __tst_num_flag(flags, signed);

    if( num == 0 ) {
        s_digits[len++] = '0';
        if(is_signed) {
            if(__tst_num_flag(flags, minus_zero)) {
                sign = '-';
            } else if(__tst_num_flag(flags, plus_zero)) {
                sign = '+';
            }
        }
    } else if(lower_hex || __tst_num_flag(flags, upper_hex)) {
        unsigned int n;
        while( num ) {
            n = num & 0x0Ful;
            s_digits[len++] = 
                n < 10
                    ? '0' + n
                    : (lower_hex ? 'a' : 'A') + n - 10;
            num >>= 4;
        }
    } else {
        if(is_signed) {
            if( (int64_t)num < 0 ) {
                sign = '-';
                num = -num;
            } else if(__tst_num_flag(flags, plus)) {
                sign = '+';
            }
        }
        while( num ) {
            s_digits[len++] = '0' + (num % 10);
            num /= 10;
        }
    }
    if(precise && precesion > len) {
        int d = precesion - len;
        while( d-- )
            s_digits[len++] = '0';
    }

    int len_sign = len + (sign ? 1 : 0);
    if( __tst_num_flag(flags, left_align) ) {

        if( sign )
            log_provide_char(p_basic_info, sign);

        while(len --)
            log_provide_char(p_basic_info, s_digits[len]);

        if( len_sign < w ) {
            log_provide_fill_width(p_basic_info, ' ', w - len_sign);
        }

    } else { // -- right align

        if( precise ) {
            fil_char = ' ';
        } else {
            if(zero_pad) {
                fil_char = '0';
                if(sign) {
                    log_provide_char(p_basic_info, sign);
                    sign = 0;
                }
            } else {
                fil_char = ' ';
            }
        }

        if( len_sign < w ) {
            log_provide_fill_width(p_basic_info, fil_char, w - len_sign);
        }

        if( sign )
            log_provide_char(p_basic_info, sign);

        while(len --)
            log_provide_char(p_basic_info, s_digits[len]);
    }
}

void log_provide_float(
    log_info_base_t* p_basic_info,
    float       f_num,
    uint32_t    flags,
    int         precesion,
    int         w)
{
    bool is_negative = false;
    if(f_num < 0) {
        f_num = -f_num;
        is_negative = true;
    }
    int r_w = precesion ? precesion : 6;
    int l_w = w - (r_w + 1);
    if( l_w < 0 ) l_w = 0;

    uint64_t int_part = (uint64_t)floor(f_num);

    int pad_spaces = 0;
    if(l_w) {
        if(__tst_num_flag(flags, left_align)) {
            uint64_t t = int_part;
            int num_len = 0;
            if(int_part == 0) {
                num_len++;
            } else {
                while(t) {
                    num_len++;
                    t /= 10;
                }
            }
            if(is_negative || __tst_num_flag(flags, plus))
                num_len++;
            if(num_len < l_w)
                pad_spaces = l_w - num_len;
            l_w = 0;
        }
    }

    __clr_num_flag(flags, precise);
    __set_num_flag(flags, signed);
    if(is_negative) {
        int_part = (uint64_t)((int64_t)int_part * -1);
        __set_num_flag(flags, minus_zero);
    } else if(__tst_num_flag(flags, plus)) {
        __set_num_flag(flags, plus_zero);
    }

    log_provide_number(p_basic_info, int_part, flags, 0, l_w);
    log_provide_char(p_basic_info, '.');

    // -- fraction part provisioning
    uint32_t tens = 1;
    for(int h=0; h < r_w; h++) tens *= 10U;
    uint64_t num_frac = (uint64_t)llround( (f_num - int_part) * tens );
    __set_num_flag(flags, precise);
    __clr_num_flag(flags, plus);
    __clr_num_flag(flags, signed);
    __clr_num_flag(flags, minus_zero);
    __clr_num_flag(flags, plus_zero);
    log_provide_number(p_basic_info, num_frac, flags, r_w, r_w);

    if(pad_spaces)
        log_provide_char_fill(p_basic_info, ' ', pad_spaces);
}

static const char* check_and_provide_color_info(
    log_info_base_t* p_basic_info,
    const char* str)
{
    __opt_paste(__opt_global_log_coloring, y, int color = 0;)
    if(str[0] == '%' && str[1] == 'C') {
        switch( str[2] ) {
            #if __opt_test( __opt_global_log_coloring, y )
            case 'd': color = p_basic_info->log_color;break;
            case 'r': color = __log_color_red;      break;
            case 'g': color = __log_color_green;    break;
            case 'b': color = __log_color_blue;     break;
            case 'y': color = __log_color_yellow;   break;
            case 'p': color = __log_color_purple;   break;
            case 'c': color = __log_color_cyan;     break;
            case 'w': color = __log_color_white;    break;
            case 'k': color = __log_color_black;    break;
            #else
            case 'd': case 'r': case 'g': case 'b': case 'y':
            case 'p': case 'c': case 'w': case 'k':
                break;
            #endif
            default:
                return str;
        }
        #if __opt_test( __opt_global_log_coloring, y )
        p_basic_info->curr_color = color;
        log_provide_color(p_basic_info, color);
        #endif
        return str + 3;
    }
    else if ( str[0] == '\033' && str[1] == '[')
    {
        if(str[2] == 'm')
        {
            #if __opt_test( __opt_global_log_coloring, y )
            p_basic_info->curr_color = p_basic_info->log_color;
            log_provide_color(p_basic_info, p_basic_info->log_color);
            #endif
            return str + 3;
        }
        else if(str[2] == '3' && str[3] > '0' && str[3] <= '9' &&
            str[4] == 'm')
        {
            #if __opt_test( __opt_global_log_coloring, y )
            p_basic_info->curr_color = str[3];
            log_provide_color(p_basic_info, str[3] - '0');
            #endif
            return str + 5;
        }
    }
    return str;
}

void log_provide_string(
    log_info_base_t* p_basic_info,
    const char* str,
    int         w,
    int         align)
{
    int len_no_color = log_strlen(str);
    int empty_len = w - len_no_color;
    int l_pad = 0, r_pad = 0;

    if( empty_len > 0 ) {
        if( align == 0 ) { // -- right
            l_pad = empty_len;
        } else if( align == 1 ) { // -- left
            r_pad = empty_len;
        } else if( align == 2) { // -- middle
            l_pad = empty_len >> 1;
            r_pad = empty_len - l_pad;
        }
    }

    if(l_pad) {
        log_provide_fill_width(p_basic_info, ' ', l_pad);
    }

    if(w == 0 || w > len_no_color)
        w = len_no_color;

    while( *str ) {
        str = check_and_provide_color_info(p_basic_info, str);
        if( w -- ) {
            log_provide_char(p_basic_info, *(str++));
        } else
            break;
    }

    if(r_pad) {
        log_provide_fill_width(p_basic_info, ' ', r_pad);
    }
}

void log_provide_formatted_string(
    log_info_base_t* p_basic_info,
    const char* fmt,
    va_list     arg_ptr)
{
    char ch;

    #define __get_char() if( ( ch = *(fmt++) ) == '\0') break
    const char* save;
    while( 1 ) {
        _start_:
        __get_char();
        if( ch != '%' ) {
            if( ch == '\n' ) {
                log_provide_newline(p_basic_info);
            } else if( ch == '\t' ) {
                log_provide_char(p_basic_info, ' ');
                log_provide_char(p_basic_info, ' ');
                log_provide_char(p_basic_info, ' ');
                log_provide_char(p_basic_info, ' ');
            } else {
                log_provide_char(p_basic_info,  ch);
            }
        } else {
            save = fmt - 1;
            uint32_t flags = 0;
            uint64_t num = 0;
            bool is_long    = false;
            bool is_longlong= false;
            bool is_dot     = false;
            bool is_left    = false;
            bool is_right   = false;
            // bool is_float   = false;
            bool is_signed  = false;
            int  precesion  = 0;
            int  w          = 0;
            repeat:
            __get_char();
            if( ch == '-' ) {
                // -- left align
                __set_num_flag(flags, left_align);
                is_left = true;
                goto repeat;
            } else if( ch == '+' ) {
                // -- display sign
                __set_num_flag(flags, plus);
                is_right = true;
                goto repeat;
            } else if( ch == '0' ) {
                // -- fill with '0's
                if(is_dot) goto collect_num;
                __set_num_flag(flags, zero_pad);
                goto repeat;
            } else if( ch > '0' && ch <= '9' ) {
                int num;
                collect_num:
                // -- specified width
                num = ch - '0';
                while(*fmt >= '0' && *fmt <= '9' ) {
                    num = num * 10 + *fmt - '0';
                    fmt++;
                }
                if(is_dot) precesion = num;
                else w = num;
                goto repeat;
            } else if( ch == 'd' ) {
                // -- provide integer
                __set_num_flag(flags, signed);
                is_signed = true;
                goto _provide_num_;
            } else if( ch == 'u' ) {
                // -- provide unsigned integer
                goto _provide_num_;
            } else if( ch == 'l' ) {
                // -- provide long integer
                if( is_longlong ) goto _end_;
                if( is_long ) {
                    is_longlong = true;
                    __set_num_flag(flags, longlong);
                } else {
                    is_long = true;
                    __set_num_flag(flags, long);
                }
                goto repeat;
            } else if( ch == 'x' || ch == 'p' ) {
                // -- provide hex
                __set_num_flag(flags, lower_hex);
                goto _provide_num_;
            } else if( ch == 'X' ) {
                // -- provide hex
                __set_num_flag(flags, upper_hex);
                goto _provide_num_;
            } else if( ch == 'c' ) {
                // -- provide character
                char cc = va_arg(arg_ptr, int);
                if(w)
                    log_provide_char_fill(p_basic_info, cc, w);
                else
                    log_provide_char(p_basic_info, cc);
            } else if( ch == 's' ) {
                // -- provide string
                const char* str = va_arg(arg_ptr, char*);
                int align;
                if(is_left && is_right) align = 2;
                else if(is_left) align = 1;
                else align = 0;
                log_provide_string(p_basic_info, str, w, align);
            } else if( ch == '%' && *save == '%' ) {
                // -- provide '%'
                log_provide_char(p_basic_info, '%');
            } else if( ch == '.' ) {
                // -- precision determination
                is_dot = true;
                __set_num_flag(flags, precise);
                goto repeat;
            } else if( ch == 'f' ) {
                // -- provide floating point num
                log_provide_float(p_basic_info, va_arg(arg_ptr, double),
                    flags, precesion, w);
                goto _start_;
            } else if( ch == 'C' ){
                // -- provide color
                __opt_paste(__opt_global_log_coloring, y, int color = 0;)

                switch(*fmt) {
                    #if __opt_test(__opt_global_log_coloring, y)
                    case 'd': color = p_basic_info->log_color;break;
                    case 'r': color = __log_color_red;      break;
                    case 'g': color = __log_color_green;    break;
                    case 'b': color = __log_color_blue;     break;
                    case 'y': color = __log_color_yellow;   break;
                    case 'p': color = __log_color_purple;   break;
                    case 'c': color = __log_color_cyan;     break;
                    case 'w': color = __log_color_white;    break;
                    case 'k': color = __log_color_black;    break;
                    #else
                    case 'd': case 'r': case 'g': case 'b': case 'y': 
                    case 'p': case 'c': case 'w': case 'k': 
                    #endif
                    default: goto _end_;
                }
                __get_char();
                __opt_paste(__opt_global_log_coloring, y ,
                    p_basic_info->curr_color = color;
                    log_provide_color(p_basic_info, color);)
            } else {
                _end_:
                while( save < fmt ) {
                    log_provide_char(p_basic_info, *save);
                    save++;
                }
                goto _start_;
                _provide_num_:
                if( is_signed ) {
                    if( is_longlong )
                        num = (uint64_t)va_arg(arg_ptr, long long);
                    else if( is_long )
                        num = (uint64_t)va_arg(arg_ptr, long);
                    else
                        num = (uint64_t)va_arg(arg_ptr, int);
                } else {
                    if( is_longlong )
                        num = (uint64_t)va_arg(arg_ptr, long long);
                    else if( is_long )
                        num = (uint64_t)va_arg(arg_ptr, unsigned long);
                    else
                        num = (uint64_t)va_arg(arg_ptr, unsigned int);
                }
                log_provide_number(p_basic_info, num, flags, precesion, w);
            }
        }
    }
}

void log_provide_printf(
    log_info_base_t* p_basic_info,
    const char* fmt,
    ...)
{
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    log_provide_formatted_string(p_basic_info, fmt, arg_ptr);
    va_end(arg_ptr);
}

void log_provide_newline(log_info_base_t* p_basic_info)
{
    log_buf_append_char(p_basic_info, '\r');
    log_buf_append_char(p_basic_info, '\n');
    log_buf_append_char(p_basic_info, '\0');

    log_buf_commit(p_basic_info->buf);

    p_basic_info->buf = log_buf_fetch(NULL);
    p_basic_info->idx = 0;
    __opt_paste(__opt_global_log_coloring, y, p_basic_info->color_len = 0;)

    log_provide_header(p_basic_info, true);
}

void log_provide_fill_width(
    log_info_base_t* p_basic_info,
    char ch,
    int  width)
{
    while( width -- )
        log_provide_char(p_basic_info, ch);
}

void log_provide_char(
    log_info_base_t* p_basic_info,
    char ch)
{
    __opt_paste(__opt_log_enforce_msg_len, y,
        if(p_basic_info->log_info->p_type_info != &g_log_type_printf ) {

            int tot_len = p_basic_info->idx + 
                log_buf_get_prev_len(p_basic_info->buf);

            extern uint32_t g_log_header_length;
            if( tot_len
                __opt_paste(__opt_global_log_coloring, y,
                - p_basic_info->color_len)
                    >= (int)g_log_header_length + __opt_log_disp_w_msg )
            {
                log_buf_append_char(p_basic_info, '\\');
                log_provide_newline(p_basic_info);
            }
        }
    )
    log_buf_append_char(p_basic_info, ch);
}

void log_provide_char_fill(
    log_info_base_t* p_basic_info,
    char ch,
    uint32_t fill_len)
{

    #if __opt_test(__opt_log_enforce_msg_len, y)
        int rem_line_len = 0;
        bool enable_line_cut = false;

        if(p_basic_info->log_info->p_type_info != &g_log_type_printf ) {
            extern uint32_t g_log_header_length;

            enable_line_cut = true;

            int curr_line_len = 
                p_basic_info->idx + log_buf_get_prev_len(p_basic_info->buf)
                __opt_paste(__opt_global_log_coloring, y,
                    - p_basic_info->color_len);
            
            int full_line_len = (int)g_log_header_length + __opt_log_disp_w_msg;

            rem_line_len = full_line_len - curr_line_len;
        }
        if( enable_line_cut )
        {
            while ( fill_len-- )
            {
                if( rem_line_len == 0 )
                {
                    log_buf_append_char(p_basic_info, '\\');
                    log_provide_newline(p_basic_info);
                    rem_line_len = __opt_log_disp_w_msg;
                }
                -- rem_line_len;
                log_buf_append_char(p_basic_info, ch);
            }
        }
        else
        {
            while ( fill_len-- )
            {
                log_buf_append_char(p_basic_info, ch);
            }
        }
    #else
        while ( fill_len-- )
        {
            log_buf_append_char(p_basic_info, ch);
        }
    #endif
}

void log_provide_printable_char(
    log_info_base_t* p_basic_info,
    char ch,
    char default_ch,
    int  printable_char_color)
{
    #define __is_printable(__ch)    ((__ch) >= 32 && (__ch) <= 126)
    if(__is_printable( ch )) {
        #if __opt_test(__opt_global_log_coloring, y)
        p_basic_info->curr_color = printable_char_color;
        log_provide_color(p_basic_info, printable_char_color);
        log_provide_char(p_basic_info, ch);
        p_basic_info->curr_color = p_basic_info->log_color;
        log_provide_color(p_basic_info, p_basic_info->log_color);
        #else
        log_provide_char(p_basic_info, ch);
        #endif
    }
    else
        log_provide_char(p_basic_info, default_ch);
}

/* -- end of file ----------------------------------------------------------- */
