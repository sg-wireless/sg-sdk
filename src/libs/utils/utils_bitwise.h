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
 * @brief   This file represents the interface of bit manipulation macros.
 * --------------------------------------------------------------------------- *
 */
#ifndef __UTILS_BITWISE_H__
#define __UTILS_BITWISE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include <stdint.h>
#include "utils_misc.h"

/* --- API Macros ----------------------------------------------------------- */

#define __bitwise_bit_clr(var_bits_size, var, pos)              \
    do{                                                         \
        __tricat(uint,var_bits_size,_t)*p = &(var);             \
        *p &= ~(1u << (pos)); /* clear previous val */          \
    } while(0)

#define __bitwise_bit_set(var_bits_size, var, pos)              \
    do{                                                         \
        __tricat(uint,var_bits_size,_t)*p = &(var);             \
        *p |=  (1u << (pos)); /* write new val */               \
    } while(0)

#define __bitwise_bit_get(var_bits_size, var, pos)              \
        ( ( *(__tricat(uint,var_bits_size,_t)*)&(var) &         \
            (1u << (pos)) ) != 0 )

#define __bitwise_bit_write(var_bits_size, var, pos, val)       \
    do{                                                         \
        __tricat(uint,var_bits_size,_t)*p = &(var);             \
        *p &= ~(1u << (pos));                                   \
        *p |=  ((1u & (val)) << (pos));                         \
    } while(0)

#define __bitwise_bits_write(var_bits_size, var, msk, pos, val) \
    do {                                                        \
        __tricat(uint,var_bits_size,_t)*p = &(var);             \
        *p &= ~((msk) << (pos));                                \
        *p |=  (((val) & (msk)) << (pos));                      \
    } while(0)

#define __bitwise_bits_read(var_bits_size, var, msk, pos)       \
    ( ( * (__tricat(uint,var_bits_size,_t)*) & (var) ) >>       \
        (pos) ) & (msk)

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __UTILS_BITWISE_H__ */
