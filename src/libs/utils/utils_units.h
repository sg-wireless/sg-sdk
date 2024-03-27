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
 * @brief   This file represents the interface of units utilities.
 * --------------------------------------------------------------------------- *
 */
#ifndef __UTILS_UNITS_H__
#define __UTILS_UNITS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "utils_math.h"

/* --- API Macros ----------------------------------------------------------- */

   /*******************************************************//**
    * @def __ns_to_cycles(__ns,__hz)
    * @details  It converts the time in nano seconds to a number
    *           of clock cycles of a given frequency. The number
    *           of cycles is integer, so an implicit ceiling
    *           occurs.
    * @param _ns time in nano-seconds - should be integer
    * @param _hz the frequency in Hz
    * @retval <uint32_t> a ceiled number of cycles
    *           corresponding to the input time
    **********************************************************/
    #define __ns_to_cycles(__ns,__hz)                         \
        (uint32_t)                                            \
        __div_ceiling(                                        \
            (uint64_t)(__ns) * (uint64_t)(__hz)               \
            , 1000000000ULL                                   \
        )

   /*******************************************************//**
    * @def __time2cycles(__time,__freq)
    * @details  It converts the time in seconds to a number
    *           of clock cycles of a given frequency. The number
    *           of cycles is integer, so an implicit ceiling
    *           occurs.
    * @param __time <double> time in seconds
    * @param __freq the frequency in Hz
    * @retval <uint32_t> a ceiled number of cycles
    *           corresponding to the input time and frequency
    **********************************************************/
    #define __time2cycles(__time,__freq)                      \
        (uint32_t)(                                           \
            __ct_ceiling( (__time) * (__freq) )               \
        )

   /*******************************************************//**
    * @def __cycles2time(__cycles,__freq)
    * @details  It converts the input decimal number os cycles
    *           to its corresponding time in seconds.
    * @param __time <double> time in seconds
    * @param __freq the frequency in Hz
    * @retval <double> the corresponding time of input cycles
    **********************************************************/
    #define __cycles2time(__cycles,__freq)                    \
        (double)(                                             \
            (double)(__cycles) / (__freq)                     \
        )

   /*******************************************************//**
    * @def __cycles_to_ns(__fcycles,__hz)
    * @details  It converts the input time in float number of
    *           cycles of a given frequency to its
    *           corresponding ceiled time in nano-seconds.
    * @param __fcycles number of cycles in float
    * @param _hz the frequency in Hz
    * @retval <uint32_t> a ceiled time in nano-seconds
    **********************************************************/
    #define __cycles_to_ns(__fcycles,__hz)                    \
        (uint32_t)                                            \
        __div_ceiling(                                        \
            (uint64_t)((double)(__fcycles) * 1e9) ,           \
            (uint64_t)(__hz)                                  \
        )

   /*******************************************************//**
    * @def __freq_khz(__khz)
    * @details Converts the frequency from KHz to Hz
    * @param __khz input frequency in KHz
    * @retval <uint64_t> the frequency in Hz
    **********************************************************/
    #define __freq_khz(__khz)       ((uint64_t)(__khz) * 1000U)

   /*******************************************************//**
    * @def __freq_mhz(__mhz)
    * @details Converts the frequency from MHz to Hz
    * @param __mhz input frequency in MHz
    * @retval <uint64_t> the frequency in Hz
    **********************************************************/
    #define __freq_mhz(__mhz)    ((uint32_t)(__mhz) * 1000000U)

   /*******************************************************//**
    * @def __to_mhz(__hz)
    * @details Converts the frequency from Hz to MHz
    * @param __hz input frequency in Hz
    * @retval <uint64_t> the frequency in MHz
    **********************************************************/
    #define __to_mhz(__hz)        ((uint32_t)((__hz)/1000000U))
    #define __to_khz(__hz)           ((uint32_t)((__hz)/1000U))

   /*******************************************************//**
    * @def __time_ms(__ms)
    * @details Converts the time from milli-seconds to seconds
    * @param __ms input time in milli-seconds
    * @retval <double> the time in seconds
    **********************************************************/
    #define __time_ms(__ms)             ((double)(__ms) * 1e-3)
    #define __to_ms(__time)          ((uint32_t)(__time * 1e3))

   /*******************************************************//**
    * @def __time_us(__us)
    * @details Converts the time from micro-seconds to seconds
    * @param __us input time in micro-seconds
    * @retval <double> the time in seconds
    **********************************************************/
    #define __time_us(__us)             ((double)(__us) * 1e-6)
    #define __to_us(__time)          ((uint32_t)(__time * 1e6))

   /*******************************************************//**
    * @def __time_ns(__ns)
    * @details Converts the time from nano-seconds to seconds
    * @param __us input time in nano-seconds
    * @retval <double> the time in seconds
    **********************************************************/
    #define __time_ns(__ns)             ((double)(__ns) * 1e-9)
    #define __to_ns(__time)          ((uint32_t)(__time * 1e9))

   /*******************************************************//**
    * @def __msize_kb(__kb)
    * @details Converts the mem size from Kilo Bytes to Bytes
    * @param __kb input tim in Kilo Bytes
    * @retval <uint32_t> the time in Bytes
    **********************************************************/
    #define __msize_kb(__kb)             ((__kb ## ul) << 10ul)

   /*******************************************************//**
    * @def __msize_mb(__mb)
    * @details Converts the mem size from Mega Bytes to Bytes
    * @param __mb input tim in Mega Bytes
    * @retval <uint32_t> the time in Bytes
    **********************************************************/
    #define __msize_mb(__mb)             ((__mb ## ul) << 20ul)

   /*******************************************************//**
    * @def __addr_bits(__msize)
    * @details  It converts the input memory size to the
    *           corresponding number of address bits.
    *           This shall only be used for compile-time calcs
    * @param __bytes input memory size in Bytes
    * @retval <uint32_t> a corresponding number of address bits
    **********************************************************/
    #define __addr_bits(__bytes)                              \
        (                                                     \
            (__bytes) <= (1u <<  0u) ?  0ul                   \
          : (__bytes) <= (1u <<  1u) ?  1ul                   \
          : (__bytes) <= (1u <<  2u) ?  2ul                   \
          : (__bytes) <= (1u <<  3u) ?  3ul                   \
          : (__bytes) <= (1u <<  4u) ?  4ul                   \
          : (__bytes) <= (1u <<  5u) ?  5ul                   \
          : (__bytes) <= (1u <<  6u) ?  6ul                   \
          : (__bytes) <= (1u <<  7u) ?  7ul                   \
          : (__bytes) <= (1u <<  8u) ?  8ul                   \
          : (__bytes) <= (1u <<  9u) ?  9ul                   \
          : (__bytes) <= (1u << 10u) ? 10ul                   \
          : (__bytes) <= (1u << 11u) ? 11ul                   \
          : (__bytes) <= (1u << 12u) ? 12ul                   \
          : (__bytes) <= (1u << 13u) ? 13ul                   \
          : (__bytes) <= (1u << 14u) ? 14ul                   \
          : (__bytes) <= (1u << 15u) ? 15ul                   \
          : (__bytes) <= (1u << 16u) ? 16ul                   \
          : (__bytes) <= (1u << 17u) ? 17ul                   \
          : (__bytes) <= (1u << 18u) ? 18ul                   \
          : (__bytes) <= (1u << 19u) ? 19ul                   \
          : (__bytes) <= (1u << 20u) ? 20ul                   \
          : (__bytes) <= (1u << 21u) ? 21ul                   \
          : (__bytes) <= (1u << 22u) ? 22ul                   \
          : (__bytes) <= (1u << 23u) ? 23ul                   \
          : (__bytes) <= (1u << 24u) ? 24ul                   \
          : (__bytes) <= (1u << 25u) ? 25ul                   \
          : (__bytes) <= (1u << 26u) ? 26ul                   \
          : (__bytes) <= (1u << 27u) ? 27ul                   \
          : (__bytes) <= (1u << 28u) ? 28ul                   \
          : (__bytes) <= (1u << 29u) ? 29ul                   \
          : (__bytes) <= (1u << 30u) ? 30ul                   \
          : (__bytes) <= (1u << 31u) ? 31ul : 32ul            \
        )

        #define __words_to_mb(__words)  ((__words) >> 18U)
        #define __bytes_to_mb(__bytes)  ((__bytes) >> 20U)
        #define __bytes_to_kb(__bytes)  ((__bytes) >> 10U)

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __UTILS_MATH_H__ */
