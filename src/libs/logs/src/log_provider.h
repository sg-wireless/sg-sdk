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
 * @brief   This file represents the interface to the logs basic provisioning
 *          methods used to provide basic log patterns into the logs buffers.
 *          such log patterns are like:
 *              number -- string -- color -- formatted string -- printf
 *              character -- newline -- filled width -- printable character
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_PROVIDER_H__
#define __LOG_PROVIDER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- include -------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include "log_obj.h"
#include "log_colors_defs.h"

/** -------------------------------------------------------------------------- *
 * basic providers declarations
 * --------------------------------------------------------------------------- *
 */
/**
 * @brief   provides the terminal color information into the logs buffer
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   color   the color info
 */
void log_provide_color(
    log_info_base_t* p_basic_info,
    char    color);

/**
 * @brief   provides the given number \a num
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   num     the number to be provided
 * @param   flags   number display tweaking flags
 * @param   precesion   min displayed digits. write 0 to the left if needed
 * @param   w       provides the num \a num in width \a w
 */
void log_provide_number(
    log_info_base_t* p_basic_info,
    uint64_t    num,
    uint32_t    flags,
    int         precesion,
    int         w);
    /* -- allowed flags ------------------- */
    #define __disp_num_flag_signed      (1<<0)
    #define __disp_num_flag_long        (1<<1)
    #define __disp_num_flag_longlong    (1<<2)
    #define __disp_num_flag_lower_hex   (1<<3)
    #define __disp_num_flag_upper_hex   (1<<4)
    #define __disp_num_flag_plus        (1<<5)
    #define __disp_num_flag_zero_pad    (1<<6)
    #define __disp_num_flag_left_align  (1<<7)
    #define __disp_num_flag_precise     (1<<8)
    #define __disp_num_flag_minus_zero  (1<<9)
    #define __disp_num_flag_plus_zero   (1<<10)

/**
 * @brief   provides the given number \a num
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   f_num   the floating point number to be provided
 * @param   flags   number display tweaking flags
 * @param   precesion   the fraction part precision
 * @param   w       provides the f_num \a f_num in width \a w
 */
void log_provide_float(
    log_info_base_t* p_basic_info,
    float       f_num,
    uint32_t    flags,
    int         precesion,
    int         w);

/**
 * @brief   provides the given string \a str
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   str     the string
 * @param   w       provides the string \a str in width \a w
 * @param   align   align the given string \a str in the given width \a w
 *                  0:right 1:left 2:middle
 */
void log_provide_string(
    log_info_base_t* p_basic_info,
    const char* str,
    int         w,
    int         align);

/**
 * @brief   provides the given string \a str
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   fmt     the formatted string
 * @param   arg_ptr reference to the variable arguments list
 */
void log_provide_formatted_string(
    log_info_base_t* p_basic_info,
    const char* fmt,
    va_list     arg_ptr);

/**
 * @brief   provides like a printf function
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   fmt     the formatted string
 */
void log_provide_printf(
    log_info_base_t* p_basic_info,
    const char* fmt,
    ...);

/**
 * @brief   provides the prefix header of the log message
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   is_empty     identifies if the header to be filled with info or not
 */
void log_provide_header(
    log_info_base_t* p_basic_info,
    bool        is_empty);

/**
 * @brief   provides a single character.
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   ch     the character to be provided
 */
void log_provide_char(
    log_info_base_t* p_basic_info,
    char ch);

/**
 * @brief   provides a repeated single character.
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   ch     the character to be provided
 * @param   repeat_len repetition length of the character
 */
void log_provide_char_fill(
    log_info_base_t* p_basic_info,
    char ch,
    uint32_t repeat_len);

/**
 * @brief   provides a filled size of a certain char
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   ch     the character to fill with
 * @param   width  the specified width
 */
void log_provide_fill_width(
    log_info_base_t* p_basic_info,
    char ch,
    int  width);

/**
 * @brief   provides an ascii printable character letters, digits and symbols
 * @param   p_basic_info reference to the log instance basic info object.
 * @param   ch     the character to be provided
 * @param   default_ch the character to be provided in case it is non-printable
 * @param   printable_char_color color of the printable character. If not
 *                  printable, the current default color will persist.
 */
void log_provide_printable_char(
    log_info_base_t* p_basic_info,
    char ch,
    char default_ch,
    int  printable_char_color);

/**
 * @brief   provides a new line. it implies flushing the current line and
 *          provisioning of empty header again and a new line is ready for
 *          extra provisioning.
 * @param   p_basic_info reference to the log instance basic info object.
 */
void log_provide_newline(log_info_base_t* p_basic_info);

/* -- end ------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_PROVIDER_H__ */
