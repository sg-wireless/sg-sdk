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
 * --------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2022, Pycom Limited.
 *
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * @author  Ahmed Sabry (Pycom)
 * 
 * @brief   This file represents the interface of math utilities.
 * --------------------------------------------------------------------------- *
 */
#ifndef __UTILS_MATH_H__
#define __UTILS_MATH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* --- API Macros ----------------------------------------------------------- */

   /********************************************************************//**
    * @def __div_ceiling(__x,__y)
    * @brief   divide __x/__y and ceil the value
    * @param __x dividant
    * @param __y divisor
    * @return  ceil(__x/__y)
    ***********************************************************************/
    #define __div_ceiling(__x,__y)                                         \
        (                                                                  \
            ( (__x) + (__y) - 1U ) /                                       \
            (__y)                                                          \
        )

   /********************************************************************//**
    * @def __ct_ceiling(_x)
    * @brief   compile-time ceiling
    * @param __x input double number to be ceiled
    * @return  ceil(_x)
    ***********************************************************************/
    #define __ct_ceiling(_x)                                               \
        (uint32_t)(                                                        \
            (uint64_t)(_x) +                                               \
            (uint64_t)((double)(_x) > (uint64_t)(_x))                      \
        )

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __UTILS_MATH_H__ */
