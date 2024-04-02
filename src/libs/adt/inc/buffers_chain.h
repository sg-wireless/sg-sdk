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
 * @brief   Introduce the chained buffers interface.
 * --------------------------------------------------------------------------- *
 */
#ifndef __BUFFERS_CHAIN_H__
#define __BUFFERS_CHAIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include "utils_bitarray.h"
#include "utils_misc.h"
#include "adt_list.h"

/** -------------------------------------------------------------------------- *
 * Description
 * ===========
 * § This module introduces a special memory management technique to handle
 *   blocks of data in variable sizes and keeps the ordering of the data in
 *   FIFO ordering.
 * 
 * § The blocks of data that are written or read are considered integrated
 *   and should not be segmented when it is read.
 *   So the writter writes a series of data blocks of different sizes, and the
 *   reader will read them one by one in the same order and data size of each
 *   block.
 *   in other words, each block of data can be seen as a mail message, and the
 *   writter is sending a mail, and the reader is reading a mail.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * Macros APIs
 * --------------------------------------------------------------------------- *
 */
#define __buf_chain_mgr_id( _name ) __concat(_name, _buf_mgr)
#define __buf_chain_mem_def(_name, _mem_size, _min_buf_size,                \
        _lock, _unlock )                                                    \
    static uint8_t __concat(_name, _mem_space)[_mem_size];                  \
    static __bitarray_def(__concat(_name, _alloc_units),                    \
        _mem_size/_min_buf_size);                                           \
    static buf_header_t __concat(_name, _headers)[_mem_size/_min_buf_size]; \
    static buf_chain_mem_t __concat(_name, _buf_mgr) = {                    \
        .p_mem_space = __concat(_name, _mem_space),                         \
        .mem_space_size = _mem_size,                                        \
        .headers = __concat(_name, _headers),                               \
        .min_buf_size = _min_buf_size,                                      \
        .units_count = _mem_size/_min_buf_size,                             \
        .allocated_units_bitarray =                                         \
            __bitarray_obj(__concat(_name, _alloc_units)),                  \
        .access_lock = _lock,                                               \
        .access_unlock = _unlock,                                           \
        .name = #_name                                                      \
    }

#define __buf_chain_def( _chain_name, _sync_obj, _wait, _signal )           \
    static buf_chain_t __concat(_chain_name, _buf_chain) = {                \
        .name = #_chain_name,                                               \
        .sync_obj = _sync_obj,                                              \
        .sync_wait = _wait,                                                 \
        .sync_signal = _signal,                                             \
    }

#define __buf_chain_init(_p_buf_chain, _name, _sync_obj, _wait, _signal)    \
    do {                                                                    \
        (_p_buf_chain)->name = _name;                                       \
        (_p_buf_chain)->sync_obj = _sync_obj;                               \
        (_p_buf_chain)->sync_wait = _wait;                                  \
        (_p_buf_chain)->sync_signal = _signal;                              \
    } while(0)

#define __buf_chain_connect( _mem_name, _chain_name )                       \
    buf_mem_chain_connect( & __concat(_mem_name, _buf_mgr),                 \
         & __concat(_chain_name, _buf_chain))

#define __buf_chain_disconnect( _mem_name, _chain_name )                    \
    buf_mem_chain_disconnect( & __concat(_mem_name, _buf_mgr),              \
         & __concat(_chain_name, _buf_chain))

#define __buf_chain_read(_chain_name, _buf, _p_len, _is_blocking)           \
    buf_mem_chain_read( & __concat(_chain_name, _buf_chain),                \
        _buf, _p_len, _is_blocking )

#define __buf_chain_write(_chain_name, _buf, _len, _is_blocking)            \
    buf_mem_chain_write( & __concat(_chain_name, _buf_chain),               \
        _buf, _len, _is_blocking )

/** -------------------------------------------------------------------------- *
 * Typedefs
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    adt_list_t  list_links; // -- used by adt_list.c only
    uint8_t*    buf;        // -- start address of the buffer in the mem space
    uint16_t    len;        // -- length of the available data in the buffer
    uint8_t     alloc_units;// -- number of consecutive mem units of this buf
} buf_header_t;

typedef struct _buf_chain_mem_s buf_chain_mem_t;
typedef struct {
    adt_list_t          list_links; // -- used by adt_list.c only
    buf_chain_mem_t*    p_mgr;      // -- ref to the parent memory space mgr
    const char*         name;       // -- name for debugging
    buf_header_t*       list;       // -- list of owned buffers
    bool                r_wait;     // -- reader waiting indicator
    bool                w_wait;     // -- writer waiting indicator
    void*               sync_obj;   // -- sync object
    void(*sync_wait)(void*);        // -- semaphore wait method
    void(*sync_signal)(void*);      // -- semaphore signal method
} buf_chain_t;

struct _buf_chain_mem_s {
    const char*     name;           // -- debug name
    uint8_t*        p_mem_space;    // -- reference to the memory space
    uint32_t        mem_space_size; // -- memory space size
    buf_header_t*   headers;        // -- reference to all headers resources
    uint32_t        min_buf_size;   // -- minimum size of the allocated buffer
    uint32_t        units_count;    // -- number of allocation units
    bitarray_t      allocated_units_bitarray; // -- allocated units indicator
    buf_chain_t*    connected_chains;   // -- connected buffer chains
    void(*access_lock)(void);       // -- critical section access lock method
    void(*access_unlock)(void);     // -- critical section access unlock method
};

typedef enum {
    __BUF_CHAIN_OK,             // -- successful
    __BUF_CHAIN_NO_SPACE,       // -- no available mem space for write method
    __BUF_CHAIN_NO_DATA,        // -- no available data for read method
    __BUF_CHAIN_NOT_CONNECTED,  // -- the buffer chain not connected to mem mgr
} buf_chain_error_t;

/** -------------------------------------------------------------------------- *
 * Macros APIs
 * --------------------------------------------------------------------------- *
 */
void buf_mem_chain_connect(
    buf_chain_mem_t*    p_mgr,
    buf_chain_t*        p_chain
    );

void buf_mem_chain_disconnect(
    buf_chain_mem_t*    p_mgr,
    buf_chain_t*        p_chain
    );

buf_chain_error_t buf_mem_chain_read(
    buf_chain_t*        p_chain,
    uint8_t*            buf,
    uint32_t*           p_len,
    bool                blocking);

buf_chain_error_t buf_mem_chain_read_2(
    buf_chain_t*        p_chain,
    uint8_t*            buf_1,
    uint32_t            buf_1_max,
    uint8_t*            buf_2,
    uint32_t*           p_len,
    bool                blocking);

buf_chain_error_t buf_mem_chain_write(
    buf_chain_t*        p_chain,
    uint8_t*            buf,
    uint32_t            len,
    bool                blocking);

buf_chain_error_t buf_mem_chain_write_2(
    buf_chain_t*        p_chain,
    uint8_t*            buf,
    uint32_t            len,
    uint8_t*            buf_2,
    uint32_t            len_2,
    bool                blocking);

/**
 * This function is not thread safe should be called only in a critical section
 */
buf_chain_error_t buf_mem_chain_clear_buf(
    buf_chain_t*        p_chain,
    buf_header_t*       p_header);

void buf_mem_chain_unblock_reader(buf_chain_t* p_chain);

void buf_mem_chain_unblock_writer(buf_chain_t* p_chain);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __BUFFERS_CHAIN_H__ */
