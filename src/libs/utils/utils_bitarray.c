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
 * @brief  This file represents the interface of generic bitarray.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdbool.h>
#include "utils_bitarray.h"

/* --- API Definitions ------------------------------------------------------ */

void bitarray_write(uint32_t *bitarray, uint32_t idx, bool value)
{
    uint32_t shl = idx & 31u;
    uint32_t msk = 1u << shl;
    uint32_t val = value << shl;
    uint32_t pos = (idx >> 5u) + 1;
    bitarray[ pos ] &= ~msk;
    bitarray[ pos ] |= val;
}

void bitarray_write_all(uint32_t *bitarray,bool value)
{
    uint32_t size = bitarray[0];
    int i;
    uint32_t w = value ? (uint32_t)(-1) : 0u;
    for(i = 1; i < size; ++i)
        bitarray[i] = w;
}

bool bitarray_read(uint32_t *bitarray, uint32_t idx)
{
    uint32_t shl = idx & 31u;
    uint32_t msk = 1u << shl;
    uint32_t pos = idx >> 5u;
    return (bitarray[pos + 1] & msk) != 0;
}

/* -- end of file ----------------------------------------------------------- */
