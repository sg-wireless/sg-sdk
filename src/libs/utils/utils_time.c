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
 * @author  Ahmed Sabry (SGWireless)
 * 
 * @brief   This file reimplemnts the time conversion algorithm from this
 *			webpage - https://www.andrews.edu/~tzs/timeconv/timealgorithm.html
 *			from PHP to C-Language
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include "utils_time.h"

static uint32_t leaps_arr[] = {
	 46828800,   78364801,  109900802,  173059203,  252028804,  315187205,
    346723206,  393984007,  425520008,  457056009,  504489610,  551750411,
    599184012,  820108813,  914803214, 1025136015, 1119744016, 1167264017,
};
#define __leaps_arr_len (sizeof(leaps_arr)/sizeof(leaps_arr[0]))

#if 0
static bool is_leap(uint32_t gps_time) {
	for(int i = 0; i < __leaps_arr_len; i++)
    {
		if( gps_time == leaps_arr[i] )
        {
			return true;
		}
	}
	return false;
}
#endif

enum {
    __CONV_GPS_2_UNIX = 0,
    __CONV_UNIX_2_GPS = 1,
};

static uint32_t count_leaps(uint32_t gps_time, int conv_flag)
{
	uint32_t n = 0;
    uint32_t i;
	if( conv_flag == __CONV_UNIX_2_GPS )
    {
		for( i = 0; i < __leaps_arr_len; i++ )
        {
			if( gps_time >= leaps_arr[i] - i )
            {
				++ n;
			}
		}
	}
    else if( conv_flag == __CONV_GPS_2_UNIX )
    {
		for( i = 0; i < __leaps_arr_len; i++)
        {
			if( gps_time >= leaps_arr[i] )
            {
				++ n;
			}
		}
	}
	return n;
}

/* --- API Macros ----------------------------------------------------------- */

uint32_t time_gps_to_unix(uint32_t gps_time)
{
	uint32_t unix_time = gps_time + 315964800;
	uint32_t n_leaps = count_leaps(gps_time, __CONV_GPS_2_UNIX);
	unix_time -= n_leaps;

	return unix_time;
}

uint32_t time_unix_to_gps(uint32_t unix_time)
{
    int is_leap = 0;


	if( floorf(fmod(unix_time, 1)) != 0 )
    {
		is_leap = 1;
	}

	uint32_t gps_time = unix_time - 315964800;
	uint32_t n_leaps = count_leaps(gps_time, __CONV_UNIX_2_GPS);
	gps_time += n_leaps + is_leap;

	return gps_time;
}

/* -- end of file ----------------------------------------------------------- */
