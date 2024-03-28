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
 * @brief   This file represents the interface to the buffering management
 *          sub-component component of the logs library.
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_BUF_MGR_H__
#define __LOG_BUF_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- include -------------------------------------------------------------- */

#include "utils_misc.h"
#include "log_config.h"
#include "log_obj.h"

/* --- macros --------------------------------------------------------------- */

/* size of the memory buffer chunk size */
#define __log_buf_size    (128)

/* --- APIs ----------------------------------------------------------------- */
/**
 * @brief   obtaining a new buffer from the pool or obtaining an extended buf
 * @param   buf the current buffer being taken before
 *              NULL -> return a new buffer
 *              not NULL -> return an extended buffer and internally connects
 *                          it to the given \a buf
 * @return  new or extended buffer
 */
char* log_buf_fetch(char* buf);

/**
 * @brief   flushes the given buffer \a buf and its extended buffers
 */
void log_buf_commit(char* buf);

/**
 * @brief   Appends a character \a ch to the buffer.
 * @note    If the buffer is fill, a new extended buffer is fetched and linked
 *          with the given buffer and \a ch is added to the extended buffer.
 */
void log_buf_append_char(log_info_base_t* p_base_info, char ch);

/**
 * @brief   Calculate the length of all previous buffers to the given \a buf
 */
int log_buf_get_prev_len(char* buf);

/* -- end ------------------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_BUF_MGR_H__ */
