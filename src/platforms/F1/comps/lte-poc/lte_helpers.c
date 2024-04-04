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

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/** -------------------------------------------------------------------------- *
 * helpers implementations
 * --------------------------------------------------------------------------- *
 */

bool helper_str_search(
    const char* main_str,   // main string to search a string within it
    const char* search_str, // the string to look for it in the main string
    const char* delimiter,  // stop searching is delimiter is encountered
    int         max_size,   // stop searching after reaching this size
                                //  0: no delimiter
    int * found_at          // the founded index in the main str
    )
{

    if(!search_str || !main_str) {
        return false;
    }

    int d_start = 0;
    int d_curr = 0;

    int curr_idx = 0;

    while(main_str[curr_idx])
    {
        int i = curr_idx;
        int j = 0;

        while(main_str[i])
        {
            if(delimiter)
            {
                if(delimiter[d_curr] == 0)
                {
                    return false;
                }
                else if(delimiter[d_curr] == main_str[i])
                {
                    ++ d_curr;
                }
                else
                {
                    ++ d_start;
                    while(d_start <= curr_idx)
                    {
                        d_curr = 0;
                        bool found = true;
                        while(d_start + d_curr <= curr_idx)
                        {
                            if(delimiter[d_curr] != main_str[d_start + d_curr])
                            {
                                found = false;
                                break;
                            }
                            ++ d_curr;
                        }
                        if(found)
                        {
                            break;
                        }
                        ++ d_start;
                    }
                }
            }

            if(search_str[j])
            {
                if(main_str[i] != search_str[j])
                {
                    if(max_size && i == max_size) {
                        return false;
                    }
                    break;
                }
            }
            else
            {
                break;
            }

            if(max_size && i == max_size) {
                return false;
            }

            ++i;
            ++j;
        }

        if(!search_str[j])
        {
            if(found_at)
                *found_at = curr_idx;
            return true;
        }

        ++ curr_idx;
    }
    return false;
}

int helper_str_copy(char* dst, const char* src, int size, const char*delimiter)
{
    int curr_idx = 0;
    int d_start = 0;
    int d_curr = 0;

    while(src[curr_idx])
    {
        if(delimiter)
        {
            if(delimiter[d_curr] == 0)
            {
                dst[d_start] = 0;
                return d_start;
            }
            else if(delimiter[d_curr] == src[curr_idx])
            {
                ++ d_curr;
            }
            else
            {
                ++ d_start;
                while(d_start <= curr_idx)
                {
                    d_curr = 0;
                    bool found = true;
                    while(d_start + d_curr <= curr_idx)
                    {
                        if(delimiter[d_curr] != src[d_start + d_curr])
                        {
                            found = false;
                            break;
                        }
                        ++ d_curr;
                    }
                    if(found)
                    {
                        break;
                    }
                    ++ d_start;
                }
            }
        }

        if( ! --size ) {
            break;
        }

        dst[curr_idx] = src[curr_idx];

        ++ curr_idx;
    }

    dst[curr_idx] = 0;
    return curr_idx;
}

bool helper_str_read_int(const char* str, int* p_int_val)
{
    int i = 0;
    bool found_int = false;
    while(*str) {
        char c = *str;
        if(c == ' ') {
            ++ str;
            continue;
        } else if(c >= '0' && c <= '9') {
            found_int = true;
            i = i * 10 + (c - '0');
            ++ str;
            continue;
        }
        break;
    }
    if(p_int_val && found_int) {
        *p_int_val = i;
    }
    return found_int;
}

/* --- end of file ---------------------------------------------------------- */
