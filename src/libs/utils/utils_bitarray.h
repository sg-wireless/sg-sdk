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
 * @brief   This file represents the interface of generic bitarray.
 * --------------------------------------------------------------------------- *
 */
#ifndef __BITARRAY_H__
#define __BITARRAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include "utils_misc.h"
#include "utils_math.h"

/* --- typedefs ------------------------------------------------------------- */

typedef uint32_t* bitarray_t;

/* --- API Macros ----------------------------------------------------------- */

/**
 * @brief   it qualifies the bitarray object name for easier user experience
 * @param   __name  bitarray name
 */
#define __bitarray_obj(__name) __concat(__bitarray_, __name)

/**
 * @brief   it defines a new bitarray object
 * @param   __name  bitarray name
 * @param   __bits_count length of the bitarray
 */
#define __bitarray_def(__name, __bits_count)        \
    uint32_t __bitarray_obj(__name)                 \
        [ 1 + __div_ceiling(__bits_count, 32) ] = { \
            __div_ceiling(__bits_count, 32) + 1,    \
            0}

/**
 * @brief   write 1 at the index \a __idx
 * @param   __name  bitarray name
 * @param   __idx the index of the required bit in the bitarray
 */
#define __bitarray_set(__name, __idx)               \
    bitarray_write(__bitarray_obj(__name), __idx, true)

/**
 * @brief   write specific bit value \a __val at the index \a __idx
 * @param   __name  bitarray name
 * @param   __idx the index of the required bit in the bitarray
 * @param   __val the value to be written
 */
#define __bitarray_write(__name, __idx, __val)      \
    bitarray_write(__bitarray_obj(__name), __idx, __val)

/**
 * @brief   write 1 at all bits
 * @param   __name  bitarray name
 */
#define __bitarray_set_all(__name)                  \
    bitarray_write_all(__bitarray_obj(__name), true)

/**
 * @brief   write 0 at the index \a __idx
 * @param   __name  bitarray name
 * @param   __idx the index of the required bit in the bitarray
 */
#define __bitarray_clr(__name, __idx)               \
    bitarray_write(__bitarray_obj(__name), __idx, false)

/**
 * @brief   write 0 at all bits
 * @param   __name  bitarray name
 */
#define __bitarray_clr_all(__name)                  \
    bitarray_write_all(__bitarray_obj(__name), false)

/**
 * @brief   returns the bit value at the index \a __idx
 * @param   __name  bitarray name
 * @param   __idx the index of the required bit in the bitarray
 */
#define __bitarray_get(__name, __idx)               \
    bitarray_read(__bitarray_obj(__name), __idx)

/* --- API Functions -------------------------------------------------------- */

/**
 * @brief   sets a specific value \a val at a specific bitarray index \a idx
 * @param   bitarray    bitarray object id
 * @param   idx         index of the required bit
 * @param   val         the value of the bit to be set
 *                          - true  sets 1
 *                          - false sets 0
 */
void bitarray_write(bitarray_t bitarray, uint32_t idx, bool val);

/**
 * @brief   sets a specific value \a val in all bits of the bit array
 * @param   bitarray    bitarray object id
 * @param   val         the value of the bit to be set
 *                          - true  sets 1
 *                          - false sets 0
 */
void bitarray_write_all(bitarray_t bitarray, bool val);

/**
 * @brief   gets the bit value at a specific bitarray index \a idx
 * @param   bitarray    bitarray object id
 * @param   idx         index of the required bit
 * @return  true  if the bit value is 1
 *          false if the bit value is 0
 */
bool bitarray_read(bitarray_t bitarray, uint32_t idx);

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __BITARRAY_H__ */
