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
 * @brief   helper function for LTE code translation
 * --------------------------------------------------------------------------- *
 */
#ifndef __LTE_HELPERS_H__
#define __LTE_HELPERS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** -------------------------------------------------------------------------- *
 * Helpers APIs
 * --------------------------------------------------------------------------- *
 */

bool helper_str_search(
    const char* main_str,   // main string to search a string within it
    const char* search_str, // the string to look for it in the main string
    const char* delimiter,  // stop searching is delimiter is encountered
    int         max_size,   // stop searching after reaching this size
                                //  0: no delimiter
    int * found_at          // the founded index in the main str
    );

int helper_str_copy(
    char* dst,
    const char* src,
    int size,
    const char*delimiter
    );

bool helper_str_read_int(const char* str, int* p_int_val);


/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LTE_HELPERS_H__ */
