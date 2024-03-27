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
 * @brief   This file represents the interface of bit manipulation macros.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdio.h>

#include "utils_math.h"
#include "utils_units.h"

/* --- test implementation -------------------------------------------------- */

int main(void)
{
    #define TEST_TIME   __time_ns( 2000 )
    #define TEST_FREQ   __freq_mhz(60)
    #define TEST_CYCLES __time2cycles( TEST_TIME, TEST_FREQ )
    #define TEST_CYCLES_2_TIME __cycles2time(TEST_CYCLES , TEST_FREQ)
    printf("-- time = %u ns = %u us = %u ms\n",
        __to_ns(TEST_TIME), __to_us(TEST_TIME), __to_ms(TEST_TIME));
    printf("   freq = %u MHz = %u KHz = %u Hz\n",
        __to_mhz(TEST_FREQ), __to_khz(TEST_FREQ), TEST_FREQ);
    printf("   time of one cycle = %u ns\n",
        __to_ns(__cycles2time(1, TEST_FREQ)));
    printf("   ceiled number of cycles = %u cycle for %u ns\n",
        TEST_CYCLES, __to_ns(TEST_TIME));
    printf("   exact time of %u cycles = %u ns\n",
        TEST_CYCLES, __to_ns(TEST_CYCLES_2_TIME));

    printf("\n");

    printf("\n[ -- Check div ceiling macro to find best freq scalar -- ]\n");
    #define __tc_find_best_scalar(__fin,__fmax)                             \
        do{                                                                 \
            uint32_t scalar = __div_ceiling(__freq_mhz(__fin),              \
                                            __freq_mhz(__fmax));            \
            printf("\t- ");                                                 \
            printf("freq_in = %3u MHz, ",  __fin);                          \
            printf("freq_max = %3u MHz, ", __fmax);                         \
            printf("best scaler = %2u , ", scalar);                         \
            printf("freq_out = %3u MHz", __fin / scalar );                  \
            printf("\n");                                                   \
        } while(0)
    __tc_find_best_scalar(280 , 100);
    __tc_find_best_scalar(280 ,  50);
    __tc_find_best_scalar(280 ,  60);
    __tc_find_best_scalar(280 ,  93);
    __tc_find_best_scalar(280 ,  90);
    __tc_find_best_scalar(140 , 100);
    __tc_find_best_scalar(140 ,  90);
    __tc_find_best_scalar(140 ,  80);
    __tc_find_best_scalar(140 ,  71);
    __tc_find_best_scalar(140 ,  70);
    #undef __tc_find_best_scalar

    printf("\n");

    printf("\n[ -- Check memory size macros -- ]\n");
    #define __tc_mem_size(__mb)                                     \
        printf("\t- memory size = %2u MB, addr bits = %u bits\n",   \
            __bytes_to_mb(__msize_mb( __mb )),                      \
            __addr_bits(__msize_mb( __mb ))                         \
            )
    __tc_mem_size(32);
    __tc_mem_size(31);
    __tc_mem_size(16);
    __tc_mem_size(15);
    __tc_mem_size( 8);
    __tc_mem_size( 9);
    #undef __tc_mem_size

	return 0;
}

/* -- end of file ----------------------------------------------------------- */
