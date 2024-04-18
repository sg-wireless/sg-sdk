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
 * @brief   for any utils related to firmware versioning
 * --------------------------------------------------------------------------- *
 */

#include "fw_version.h"
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * defining version constants
 * --------------------------------------------------------------------------- *
 */
static const char s_fw_version_string[] = __fw_version_string();
static const char s_fw_version_custom_str[] = __fw_version_custom_str();
static const char s_fw_version_release_str[] = __fw_version_release_str();
static const uint32_t s_fw_version_release_major = __fw_version_release_major();
static const uint32_t s_fw_version_release_minor = __fw_version_release_minor();
static const uint32_t s_fw_version_release_patch = __fw_version_release_patch();
static const char s_fw_version_git_tag_full_str[] =
    __fw_version_git_tag_full_str();
static const char s_fw_version_git_tag_short_str[] =
    __fw_version_git_tag_short_str();
static const uint32_t s_fw_version_git_tag_short_u32 =
    __fw_version_git_tag_short_u32();
static const uint32_t s_fw_version_git_delta  = __fw_version_git_delta();
static const char s_fw_version_date_str[]  = __fw_version_date_str();
static const uint32_t s_fw_version_date_day   = __fw_version_date_day();
static const uint32_t s_fw_version_date_month = __fw_version_date_month();
static const uint32_t s_fw_version_date_year  = __fw_version_date_year();

static const char s_fw_version_time_str[]   = __fw_version_time_str();

/** -------------------------------------------------------------------------- *
 * APIs implementation
 * --------------------------------------------------------------------------- *
 */

const char* fw_version_string(void)
{
    return s_fw_version_string;
}
const char* fw_version_custom_str(void)
{
    return s_fw_version_custom_str;
}
const char* fw_version_release_str(void)
{
    return s_fw_version_release_str;
}
uint32_t fw_version_release_major(void)
{
    return s_fw_version_release_major;
}
uint32_t fw_version_release_minor(void)
{
    return s_fw_version_release_minor;
}
uint32_t fw_version_release_patch(void)
{
    return s_fw_version_release_patch;
}
const char* fw_version_git_tag_full_str(void)
{
    return s_fw_version_git_tag_full_str;
}
const char* fw_version_git_tag_short_str(void)
{
    return s_fw_version_git_tag_short_str;
}
uint32_t fw_version_git_tag_short_u32(void)
{
    return s_fw_version_git_tag_short_u32;
}
uint32_t fw_version_git_delta(void)
{
    return s_fw_version_git_delta;
}
const char* fw_version_date_str(void)
{
    return s_fw_version_date_str;
}
const char* fw_version_time_str(void)
{
    return s_fw_version_time_str;
}
uint32_t fw_version_date_day(void)
{
    return s_fw_version_date_day;
}
uint32_t fw_version_date_month(void)
{
    return s_fw_version_date_month;
}
uint32_t fw_version_date_year(void)
{
    return s_fw_version_date_year;
}

/* --- end of file ---------------------------------------------------------- */
