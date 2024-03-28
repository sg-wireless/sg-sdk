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
 * @brief   This file represents the implementation of the buffering management
 *          sub-component of the logs library.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils_misc.h"
#include "log_config.h"
#include "log_buf_mgr.h"
#include "log_obj.h"
#include "log_lib.h"

/* --- access guarding and initialization ----------------------------------- */

static log_port_mutex_lock_t * p_access_lock = NULL;
static log_port_mutex_unlock_t * p_access_unlock = NULL;
#define __log_buf_access_lock()     if(p_access_lock)p_access_lock()
#define __log_buf_access_unlock()   if(p_access_unlock)p_access_unlock()

static void log_bufs_init(log_init_params_t* p_init_params);

void log_buf_mgr_init(log_init_params_t* p_init_params)
{
    if(p_init_params) {
        p_access_lock = p_init_params->mutex_lock;
        p_access_unlock = p_init_params->mutex_unlock;
    }

    log_bufs_init(p_init_params);
}

/* --- buffer definition ---------------------------------------------------- */

#if __opt_logs_buffer_memory < __log_buf_size
    #error "error: the logs buffer memory pool must be greater than buf size"
#endif
#define __log_bufs_count  (__opt_logs_buffer_memory / __log_buf_size)
static char s_log_bufs[__log_bufs_count][__log_buf_size];

/* --- typedefs ------------------------------------------------------------- */

static struct log_buf_info_t {
    char*   buf;    // -- reference to buffer
    struct log_buf_info_t* next, *prev;
}   s_log_buf_info[ __log_bufs_count ], *s_log_buf_free_list;

/* --- API definitions ------------------------------------------------------ */

static log_port_serial_output_t * p_serial_output = NULL;

static void log_bufs_init(log_init_params_t* p_init_params)
{
    s_log_buf_free_list = s_log_buf_info;
    int i;
    for(i = 0; i < __log_bufs_count - 1; ++i) {
        s_log_buf_info[i].buf = s_log_bufs[i];
        s_log_buf_info[i].next = &s_log_buf_info[i + 1];
        s_log_buf_info[i].prev = NULL;
    }
    s_log_buf_info[i].buf = s_log_bufs[i];
    s_log_buf_info[i].next = NULL;
    s_log_buf_info[i].prev = NULL;

    if(p_init_params) {
        p_serial_output = p_init_params->serial_out;
    }
}

static void log_buf_serial_out(char* buf, uint32_t size)
{
    if( p_serial_output ) {
        p_serial_output((uint8_t*)buf, size);
    } else {
        int i;
        for(i = 0; i < size ; ++i)
            printf("%c", buf[i]);
    }
}

char* log_buf_fetch(char* buf)
{
    struct log_buf_info_t* p_prev = NULL;
    if( buf )
        p_prev = s_log_buf_info + ((char(*)[__log_buf_size])buf - s_log_bufs);

    struct log_buf_info_t* p_info = NULL;
    do {
        // -- wait for free buf
        while(s_log_buf_free_list == NULL) {
        }
        // -- acquire free buf
        __log_buf_access_lock();
        if(s_log_buf_free_list) {
            p_info = s_log_buf_free_list;
            s_log_buf_free_list = s_log_buf_free_list->next;
        }
        __log_buf_access_unlock();
    } while( p_info == NULL );

    if( buf ) {
        if(p_prev)
            p_prev->next = p_info;
        p_info->prev = p_prev;
        p_info->next = NULL;
    } else {
        p_info->next = p_info->prev = NULL;
    }

    return p_info->buf;
}

void log_buf_commit(char* buf)
{
    struct log_buf_info_t* p_info = 
        s_log_buf_info + ((char(*)[__log_buf_size])buf - s_log_bufs);

    // -- obtain the base buffer
    while(p_info->prev)
        p_info = p_info->prev;

    // -- do flushing
    struct log_buf_info_t* p_iter = p_info;
    struct log_buf_info_t* p_last = NULL;
    __log_buf_access_lock();
    while(p_iter) {
        buf = p_iter->buf;
        uint32_t len = (p_iter->next != NULL) ? __log_buf_size : strlen(buf);
        log_buf_serial_out( buf, len);
        p_last = p_iter;
        p_iter = p_iter->next;
    }

    // -- insert in free list
    p_last->next = s_log_buf_free_list;
    s_log_buf_free_list = p_info;
    p_info->prev = NULL;
    __log_buf_access_unlock();
}

int log_buf_get_prev_len(char* buf)
{
    int ret = 0;
    struct log_buf_info_t* p_info = 
        s_log_buf_info + ((char(*)[__log_buf_size])buf - s_log_bufs);
    
    // -- obtain the base buffer
    while(p_info->prev) {
        p_info = p_info->prev;
        ret += __log_buf_size;
    }
    return ret;
}

void log_buf_append_char(log_info_base_t* p_basic_info, char ch)
{
    char* buf = p_basic_info->buf;
    int   idx = p_basic_info->idx;
    if(idx >= __log_buf_size) {
        buf = log_buf_fetch(buf);
        idx = 0;
    }
    buf[idx++] = ch;
    p_basic_info->buf = buf;
    p_basic_info->idx = idx;
}

/* -- end of file ----------------------------------------------------------- */
