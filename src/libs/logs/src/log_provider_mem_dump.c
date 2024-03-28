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
 * @brief   This file represents the implementations of the log mem-dump type
 *          provider
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include <stdio.h>

#include "log_lib.h"
#include "log_config.h"
#include "log_provider.h"
#include "log_buf_mgr.h"

/* --- type private implementation ------------------------------------------ */

#if __opt_test( __opt_log_type_mem_dump, y ) || \
    __opt_test( __opt_log_type_output, y )
static void log_provide_memory_dump(log_info_t* p_log_info);
#endif

__log_type_def(log_provide_memory_dump, mem_dump, 1);
#if __opt_test(__opt_log_type_output, y)
    log_type_info_t g_log_type_mem_dump_output = {
        .p_provider = log_provide_memory_dump,
        .type_name = "mem_dump_output",
        .flags = __log_type_flag_en | __log_type_flag_cc | __log_type_flag_out,
    };
#endif

#define __dump_color_addr       __log_color_green
#define __dump_color_offset     __log_color_cyan
#define __dump_color_bytes      __log_color_blue
#define __dump_color_print_char __log_color_purple
#define __dump_color_print_def  __dump_color_bytes

#if __opt_test( __opt_log_type_mem_dump, y ) || \
    __opt_test( __opt_log_type_output, y )
static void log_provide_memory_dump(log_info_t* p_log_info)
{
    log_info_dump_t* p_info = (log_info_dump_t*)p_log_info;
    void* p_basic_info = p_info->log_info_base.p_basic_info;

    uint8_t*    addr        = p_info->addr;
    uint32_t    cbytes      = p_info->cbytes;
    uint32_t    line_bytes  = p_info->bytes_per_line;
    uint32_t    flags       = p_info->flags;
    int         word_len    = 1u << (p_info->word_len);

    uint32_t line_words = (line_bytes / word_len) +
                        (((word_len-1) & line_bytes) != 0);
    uint32_t rem = cbytes % word_len;
    cbytes += rem ? word_len - rem : 0;
    int b_disp_char = flags & __log_dump_flag_disp_char;
    int b_disp_on_rhs_only = flags & __log_dump_flag_disp_char_on_rhs;
    bool upper_hex = flags & __log_dump_flag_disp_upper_hex;
    bool hide_address = flags & __log_dump_flag_hide_address;
    bool hide_offset = flags & __log_dump_flag_hide_offset;
    uint32_t offset = 0;
    uint32_t num_flags = __disp_num_flag_precise |
        (upper_hex ? __disp_num_flag_upper_hex : __disp_num_flag_lower_hex);

    uint32_t line_counter = 0;

    while( cbytes )
    {
        if( line_counter == 0 )
        {
            if( ! hide_address ) {
                log_provide_printf(p_basic_info,
                    upper_hex ? "[ "__green__"%08X"__default__" ]"
                        : "[ "__green__"%08x"__default__" ]", addr);
            }
            if( ! hide_offset ) {
                log_provide_printf(p_basic_info,
                    upper_hex ? "[ "__cyan__"%04X"__default__" ]"
                        : "[ "__cyan__"%04x"__default__" ]", offset);
            }
        }
        switch(p_info->word_len)
        {
            case __word_len_8:
                log_provide_number(p_basic_info, (uint64_t)*(uint8_t*)addr,
                    num_flags, 2, 3);
                if(b_disp_char && ! b_disp_on_rhs_only){
                    log_provide_printable_char(p_basic_info, addr[0], '.',
                        __dump_color_print_char);
                }
                break;
            case __word_len_16:
                log_provide_number(p_basic_info, (uint64_t)*(uint16_t*)addr,
                    num_flags, 4, 5);            
                if(b_disp_char && ! b_disp_on_rhs_only){
                    log_provide_printable_char(p_basic_info, addr[0], '.',
                        __dump_color_print_char);
                    log_provide_printable_char(p_basic_info, addr[1], '.',
                        __dump_color_print_char);
                }
                break;
            case __word_len_32:
                log_provide_number(p_basic_info, (uint64_t)*(uint32_t*)addr,
                    num_flags, 8, 9);            
                if(b_disp_char && ! b_disp_on_rhs_only){
                    log_provide_printable_char(p_basic_info, addr[0], '.',
                        __dump_color_print_char);
                    log_provide_printable_char(p_basic_info, addr[1], '.',
                        __dump_color_print_char);
                    log_provide_printable_char(p_basic_info, addr[2], '.',
                        __dump_color_print_char);
                    log_provide_printable_char(p_basic_info, addr[3], '.',
                        __dump_color_print_char);
                }
            default:
                break;
        }
        addr += word_len;
        if(++ line_counter == line_words)
        {
            line_counter = 0;
            if(b_disp_char && b_disp_on_rhs_only)
            {
                uint32_t i;
                uint32_t line_width = line_words * word_len;
                char* ptr = (char*)addr - line_width;
                log_provide_char(p_basic_info, ' ');
                log_provide_char(p_basic_info, ' ');
                for(i = 0; i < line_width; i++)
                {
                    log_provide_printable_char(p_basic_info, ptr[i], '.',
                        __dump_color_print_char);
                }
            }
            if(cbytes - word_len)
                log_provide_newline(p_basic_info);
        }
        cbytes -= word_len;
        offset += word_len;
    }
    if(line_counter)
    {
        int count = (word_len*2+1) * (line_words - line_counter);
        while(count--)
            log_provide_char(p_basic_info, ' ');

        if(b_disp_char && b_disp_on_rhs_only)
        {
            uint32_t i;
            uint32_t line_width = line_counter * word_len;
            char* ptr = (char*)addr - line_width;
            log_provide_char(p_basic_info, ' ');
            log_provide_char(p_basic_info, ' ');
            for(i = 0; i < line_width; i++)
            {
                log_provide_printable_char(p_basic_info, ptr[i], '.',
                        __dump_color_print_char);
            }
        }
        //log_provide_newline(p_basic_info);
    }
}
#endif

/* --- end of file ---------------------------------------------------------- */
