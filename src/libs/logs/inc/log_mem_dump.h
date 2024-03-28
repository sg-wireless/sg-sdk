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
 * @brief   This file contains the extended interface for the log type of
 *          memory dump.
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_MEMDUMP_H__
#define __LOG_MEMDUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include "log_lib.h"

/* --- macros --------------------------------------------------------------- */

#define __log_dump(addr, cbytes, line_bytes, flags, word_len)       \
    __opt_paste(__get_log_type_opt(mem_dump), y,                    \
        do {                                                        \
            log_info_dump_t info = {                                \
                { .p_type_info = &g_log_type_mem_dump },            \
                (void*)addr, cbytes, line_bytes, flags, word_len }; \
            __log(mem_dump, (void*)&info, NULL);                    \
        } while(0)                                                  \
    )

#define __log_output_dump(addr, cbytes, line_bytes, flags, word_len)\
    __opt_paste(__opt_log_type_output, y,                           \
    do {                                                            \
        log_info_dump_t info = {                                    \
            { .p_type_info = &g_log_type_mem_dump_output },         \
            (void*)addr, cbytes, line_bytes, flags, word_len };     \
        log_stdout( (void*)&info, NULL );                           \
    } while(0))

__log_type_dec(mem_dump);

/* memory dumping in std out will be considered as if it is a direct output */
__opt_paste(__opt_log_type_output, y,
    extern log_type_info_t g_log_type_mem_dump_output;
)

/* --- typedefs ------------------------------------------------------------- */

typedef struct {
    log_info_t      log_info_base;
    uint8_t*        addr;
    uint32_t        cbytes;
    uint32_t        bytes_per_line;
    uint32_t        flags;
                    #define __log_dump_flag_disp_char           (1<<0)
                    #define __log_dump_flag_disp_char_on_rhs    (1<<1)
                    #define __log_dump_flag_disp_upper_hex      (1<<2)
                    #define __log_dump_flag_hide_address        (1<<3)
                    #define __log_dump_flag_hide_offset         (1<<4)
                    #define __log_dump_flag_normal_output       (1<<5)
    enum {
        __word_len_8,
        __word_len_16,
        __word_len_32,
    } word_len;
} log_info_dump_t;

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_MEMDUMP_H__ */
