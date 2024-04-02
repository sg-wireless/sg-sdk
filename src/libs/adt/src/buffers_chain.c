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
 * @brief   Implements the chained buffers memory managment.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <string.h>
#include <stdint.h>

#define __log_subsystem     libs
#define __log_component     buf_chain
#include "log_lib.h"
__log_component_def(libs, buf_chain, green, 1, 0);
#include "utils_bitarray.h"
#include "utils_misc.h"
#include "buffers_chain.h"

/** -------------------------------------------------------------------------- *
 * APIs Implementation
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_ADT_BUFFERS_CHAIN_DEBUG_ENABLE
    #define __debug_logging     (1)
#else
    #define __debug_logging     (0)
#endif
static bool buf_mem_chain_is_connect(
    buf_chain_mem_t*    p_mgr,
    buf_chain_t*        p_chain)
{
    bool is_connected = false;
    buf_chain_t* it;
    __adt_list_foreach(p_mgr->connected_chains, it) {
        if(it == p_chain) {
            is_connected = true;
            break;
        }
    }
    return is_connected;
}

void buf_mem_chain_connect(
    buf_chain_mem_t*    p_mgr,
    buf_chain_t*        p_chain
    )
{
    bool is_connected = buf_mem_chain_is_connect(p_mgr, p_chain);

    if( ! is_connected ) {
        __adt_list_push(p_mgr->connected_chains, p_chain);
        p_chain->p_mgr = p_mgr;
    }
}

void buf_mem_chain_disconnect(
    buf_chain_mem_t*    p_mgr,
    buf_chain_t*        p_chain
    )
{
    bool is_connected = buf_mem_chain_is_connect(p_mgr, p_chain);

    if( is_connected ) {
        p_mgr->access_lock();

        if(p_chain->r_wait || p_chain->w_wait)
        {
            p_chain->r_wait = false;
            p_chain->w_wait = false;
            p_chain->sync_signal(p_chain->sync_obj);
        }

        buf_header_t * p_buf_header;

        do{
            p_buf_header = __adt_list_unshift(p_chain->list);
            if(p_buf_header)
            {
                memset(p_buf_header->buf, 0, p_buf_header->len);
                int idx = (p_buf_header->buf - p_mgr->p_mem_space) /
                    p_mgr->min_buf_size;
                int units = p_buf_header->alloc_units;
                while( units-- ) {
                    bitarray_write(p_mgr->allocated_units_bitarray, idx++,
                        false);
                }
            }
        } while(p_buf_header != NULL);

        __adt_list_del(p_mgr->connected_chains, p_chain);
        p_chain->p_mgr = NULL;

        p_mgr->access_unlock();
    }
}

buf_chain_error_t buf_mem_chain_write(
    buf_chain_t*        p_chain,
    uint8_t*            buf,
    uint32_t            len,
    bool                blocking)
{
    return buf_mem_chain_write_2(p_chain, buf, len, NULL, 0, blocking);
}

buf_chain_error_t buf_mem_chain_write_2(
    buf_chain_t*        p_chain,
    uint8_t*            buf,
    uint32_t            len,
    uint8_t*            buf_2,
    uint32_t            len_2,
    bool                blocking)
{
    buf_chain_mem_t*    p_mgr = p_chain->p_mgr;

    int tot_len = 0;
    bool has_buf_1 = (buf && len);
    bool has_buf_2 = (buf_2 && len_2);
    if(has_buf_1) tot_len += len;
    if(has_buf_2) tot_len += len_2;

    /* number of required adjacent units */
    int req_units = __div_ceiling(tot_len, p_mgr->min_buf_size);

    /* look for available adjacent units */
    int tot_units = p_mgr->units_count;

    restart_write_proc:

    p_mgr->access_lock();

    p_chain->w_wait = false;

    if( ! buf_mem_chain_is_connect(p_mgr, p_chain) )
    {  
        p_mgr->access_unlock();
        return __BUF_CHAIN_NOT_CONNECTED;
    }

    int adjacent_units = 0;
    int i;
    bool found_space = false;
    for( i = 0; i < tot_units; ++i ) {
        if( bitarray_read(p_mgr->allocated_units_bitarray, i) == false )
            ++ adjacent_units;
        else
            adjacent_units = 0;
        if(adjacent_units == req_units) {
            found_space = true;
            while(adjacent_units --) {
                bitarray_write(p_mgr->allocated_units_bitarray, i--, true);
            }
            break;
        }
    }

    if( found_space ) {
        ++i;
        buf_header_t * p_header = & p_mgr->headers[i];
        p_header->alloc_units = req_units;
        p_header->buf = p_mgr->p_mem_space + i * p_mgr->min_buf_size;
        p_header->len = tot_len;
        if(has_buf_1)
            memcpy(p_header->buf, buf, len);
        if(has_buf_2)
            memcpy(p_header->buf + len, buf_2, len_2);
        __adt_list_push(p_chain->list, p_header);

        if(p_chain->r_wait)
            p_chain->sync_signal(p_chain->sync_obj);
        p_mgr->access_unlock();
    } else {
        if( blocking ) {
            p_chain->w_wait = true;
            p_mgr->access_unlock();
            p_chain->sync_wait(p_chain->sync_obj);
            goto restart_write_proc;
        } else {
            p_mgr->access_unlock();
            return __BUF_CHAIN_NO_SPACE;
        }
    }

    #if __debug_logging
    log_filter_save_state_t log_state = {
        .subsystem_name = "libs",
        .component_name = "buf_chain",
    };
    log_filter_save_state(&log_state, true);
    __log_printf_header(p_mgr->name, 80, '-');

    __log_dump(p_mgr->p_mem_space, p_mgr->mem_space_size, 16,
        __log_dump_flag_disp_char_on_rhs|__log_dump_flag_disp_char|
        __log_dump_flag_hide_address, __word_len_8);

    __log_dump((void*)p_mgr->allocated_units_bitarray,
        __div_ceiling(p_mgr->units_count, 32) * 4 + 4, 4,
        __log_dump_flag_hide_address, __word_len_32);

    __log_printf_fill(80, '-', true);
    __log_endl();
    log_filter_restore_state(&log_state);
    #endif

    return __BUF_CHAIN_OK;
}

buf_chain_error_t buf_mem_chain_read(
    buf_chain_t*        p_chain,
    uint8_t*            buf,
    uint32_t*           p_len,
    bool                blocking)
{
    return buf_mem_chain_read_2(p_chain, NULL, 0, buf, p_len, blocking);
}

buf_chain_error_t buf_mem_chain_read_2(
    buf_chain_t*        p_chain,
    uint8_t*            buf_1,
    uint32_t            buf_1_max,
    uint8_t*            buf_2,
    uint32_t*           p_len,
    bool                blocking)
{
    buf_chain_mem_t*    p_mgr = p_chain->p_mgr;

    restart_read_proc:

    p_mgr->access_lock();
    p_chain->r_wait = false;

    if( ! buf_mem_chain_is_connect(p_mgr, p_chain) )
    {  
        p_mgr->access_unlock();
        return __BUF_CHAIN_NOT_CONNECTED;
    }

    buf_header_t* p_header = __adt_list_unshift(p_chain->list);

    if(p_header) {
        uint8_t* p_src = p_header->buf;
        uint8_t src_len = p_header->len;
        if(buf_1 && buf_1_max)
        {
            uint8_t len = src_len <= buf_1_max ? src_len : buf_1_max;
            memcpy(buf_1, p_src, len);
            src_len -= len;
            p_src += len;
        }

        if(buf_2 && src_len > 0)
        {
            memcpy(buf_2, p_src, src_len);
        }

        memset(p_header->buf, 0, p_header->len);
        *p_len = p_header->len;

        int idx = (p_header->buf - p_mgr->p_mem_space) / p_mgr->min_buf_size;
        int units = p_header->alloc_units;
        while( units-- ) {
            bitarray_write(p_mgr->allocated_units_bitarray, idx++, false);
        }

        if( p_chain->w_wait )
            p_chain->sync_signal(p_chain->sync_obj);
        p_mgr->access_unlock();
    } else {
        if(blocking) {
            p_chain->r_wait = true;
            p_mgr->access_unlock();
            p_chain->sync_wait(p_chain->sync_obj);
            goto restart_read_proc;
        } else {
            p_mgr->access_unlock();
            return __BUF_CHAIN_NO_DATA;
        }
    }

    #if __debug_logging
    log_filter_save_state_t log_state = {
        .subsystem_name = "libs",
        .component_name = "buf_chain",
    };
    log_filter_save_state(&log_state, true);
    __log_printf_header(p_mgr->name, 80, '-');

    __log_dump(p_mgr->p_mem_space, p_mgr->mem_space_size, 16,
        __log_dump_flag_disp_char_on_rhs|__log_dump_flag_disp_char|
        __log_dump_flag_hide_address, __word_len_8);

    __log_dump((void*)p_mgr->allocated_units_bitarray,
        __div_ceiling(p_mgr->units_count, 32) * 4 + 4, 4,
        __log_dump_flag_hide_address, __word_len_32);

    __log_printf_fill(80, '-', true);
    __log_endl();
    log_filter_restore_state(&log_state);
    #endif

    return __BUF_CHAIN_OK;
}

buf_chain_error_t buf_mem_chain_clear_buf(
    buf_chain_t*        p_chain,
    buf_header_t*       p_header)
{
    buf_chain_mem_t*    p_mgr = p_chain->p_mgr;

    // -- look for this buf header again
    buf_header_t* it;
    bool found = false;
    __adt_list_foreach(p_chain->list, it)
    {
        if(it == p_header)
        {
            found = true;
            break;
        }
    }

    if(found)
    {
        memset(p_header->buf, 0, p_header->len);

        int idx = (p_header->buf - p_mgr->p_mem_space) / p_mgr->min_buf_size;
        int units = p_header->alloc_units;
        while( units-- ) {
            bitarray_write(p_mgr->allocated_units_bitarray, idx++, false);
        }

        __adt_list_del(p_chain->list, p_header);

        if( p_chain->w_wait )
            p_chain->sync_signal(p_chain->sync_obj);
    }

    #if __debug_logging
    log_filter_save_state_t log_state = {
        .subsystem_name = "libs",
        .component_name = "buf_chain",
    };
    log_filter_save_state(&log_state, true);
    __log_printf_header(p_mgr->name, 80, '-');

    __log_dump(p_mgr->p_mem_space, p_mgr->mem_space_size, 16,
        __log_dump_flag_disp_char_on_rhs|__log_dump_flag_disp_char|
        __log_dump_flag_hide_address, __word_len_8);

    __log_dump((void*)p_mgr->allocated_units_bitarray,
        __div_ceiling(p_mgr->units_count, 32) * 4 + 4, 4,
        __log_dump_flag_hide_address, __word_len_32);

    __log_printf_fill(80, '-', true);
    __log_endl();
    log_filter_restore_state(&log_state);
    #endif

    return __BUF_CHAIN_OK;
}


void buf_mem_chain_unblock_reader(buf_chain_t* p_chain)
{
    buf_chain_mem_t*    p_mgr = p_chain->p_mgr;

    p_mgr->access_lock();

    if(p_chain->r_wait)
        p_chain->sync_signal(p_chain->sync_obj);

    p_chain->r_wait = false;

    p_mgr->access_unlock();
}

void buf_mem_chain_unblock_writer(buf_chain_t* p_chain)
{
    buf_chain_mem_t*    p_mgr = p_chain->p_mgr;

    p_mgr->access_lock();

    if(p_chain->w_wait)
        p_chain->sync_signal(p_chain->sync_obj);
    
    p_chain->w_wait = false;

    p_mgr->access_unlock();
}

/* --- end of file ---------------------------------------------------------- */
