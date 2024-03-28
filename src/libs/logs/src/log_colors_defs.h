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
 * @brief   This file contains terminal colors definitions
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_COLORS_DEFS_H__
#define __LOG_COLORS_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * defining logs coloring
 * --------------------------------------------------------------------------- *
 */
#define __log_color_black           9
#define __log_color_red             1
#define __log_color_green           2
#define __log_color_yellow          3
#define __log_color_blue            4
#define __log_color_purple          5
#define __log_color_cyan            6
#define __log_color_white           7
#define __log_color_default         0

#define __log_color_str_black       "\033[39m"
#define __log_color_str_red         "\033[31m"
#define __log_color_str_green       "\033[32m"
#define __log_color_str_yellow      "\033[33m"
#define __log_color_str_blue        "\033[34m"
#define __log_color_str_purple      "\033[35m"
#define __log_color_str_cyan        "\033[36m"
#define __log_color_str_white       "\033[37m"
#define __log_color_str_reset       "\033[m"  

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_COLORS_DEFS_H__ */
