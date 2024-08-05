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
 * @brief   This file control the firmware versioning
 * --------------------------------------------------------------------------- *
 */
#ifndef __FW_VERSION_H_
#define __FW_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include "utils_misc.h"

/** -------------------------------------------------------------------------- *
 * Release Versioning definitions
 * --------------------------------------------------------------------------- *
 */
// the build system is responsible for build version header generation and
// should be forwarded to the compiled code in the following definition
#ifdef FW_GENERATED_BUILD_VERSION_HEADER
    #include FW_GENERATED_BUILD_VERSION_HEADER
#endif

// major: for Incompatible Changes - New features
#define FW_RELEASE_VERSION_MAJOR            1
// minor: for Backward compatible new feature
#define FW_RELEASE_VERSION_MINOR            0
// patch: Backward compatible bug fix
#define FW_RELEASE_VERSION_PATCH            0

// FW release version as string
#define FW_RELEASE_VERSION_STRING       "v" \
    __stringify(FW_RELEASE_VERSION_MAJOR)"."\
    __stringify(FW_RELEASE_VERSION_MINOR)"."\
    __stringify(FW_RELEASE_VERSION_PATCH)

/**
 * [-- IMPORTANT NOTES --]
 * 
 * § the following check to make sure that the hard-coded release version above
 *   is compatible with the generated build version base release
 * 
 * § when updating the release it shall be done hard-coded here in this file
 *   above and in the git by taging the commit.
 */
#if defined(FW_BUILD_VERSION_ENABLE) && FW_BUILD_VERSION_ENABLE
    #if FW_BUILD_VERSION_RELEASE_MAJOR != FW_RELEASE_VERSION_MAJOR \
        || FW_BUILD_VERSION_RELEASE_MINOR != FW_RELEASE_VERSION_MINOR \
        || FW_BUILD_VERSION_RELEASE_PATCH != FW_RELEASE_VERSION_PATCH
        #error "Build version base release is not compatible with" \
                " the hard-coded release in fw_version.h"
    #endif

    // the following version premitive components shall be defined
    // in the build version geerated file
    #ifndef FW_BUILD_VERSION_STRING
        #error "FW_BUILD_VERSION_STRING is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_CUSTOM_STR
        #error "FW_BUILD_VERSION_CUSTOM_STR is not defined"
    #endif
    // -- release version info components
    #ifndef FW_BUILD_VERSION_RELEASE_STR
        #error "FW_BUILD_VERSION_RELEASE_STR is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_RELEASE_MAJOR
        #error "FW_BUILD_VERSION_RELEASE_MAJOR is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_RELEASE_MINOR
        #error "FW_BUILD_VERSION_RELEASE_MINOR is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_RELEASE_PATCH
        #error "FW_BUILD_VERSION_RELEASE_PATCH is not defined"
    #endif
    // -- git info components 
    #ifndef FW_BUILD_VERSION_GIT_TAG_FULL_STR
        #error "FW_BUILD_VERSION_GIT_TAG_FULL_STR is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_GIT_TAG_SHORT_STR
        #error "FW_BUILD_VERSION_GIT_TAG_SHORT_STR is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_GIT_TAG_SHORT_U32
        #error "FW_BUILD_VERSION_GIT_TAG_SHORT_U32 is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_GIT_DELTA
        #error "FW_BUILD_VERSION_GIT_DELTA is not defined"
    #endif
    // -- date info components
    #ifndef FW_BUILD_VERSION_DATE_STR
        #error "FW_BUILD_VERSION_DATE_STR is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_DATE_DAY
        #error "FW_BUILD_VERSION_DATE_DAY is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_DATE_MONTH
        #error "FW_BUILD_VERSION_DATE_MONTH is not defined"
    #endif
    #ifndef FW_BUILD_VERSION_DATE_YEAR
        #error "FW_BUILD_VERSION_DATE_YEAR is not defined"
    #endif

    #define __fw_version_string()            FW_BUILD_VERSION_STRING
    #define __fw_version_custom_str()        FW_BUILD_VERSION_CUSTOM_STR
    #define __fw_version_release_str()       FW_BUILD_VERSION_RELEASE_STR
    #define __fw_version_release_major()     FW_BUILD_VERSION_RELEASE_MAJOR
    #define __fw_version_release_minor()     FW_BUILD_VERSION_RELEASE_MINOR
    #define __fw_version_release_patch()     FW_BUILD_VERSION_RELEASE_PATCH
    #define __fw_version_git_tag_full_str()  FW_BUILD_VERSION_GIT_TAG_FULL_STR
    #define __fw_version_git_tag_short_str() FW_BUILD_VERSION_GIT_TAG_SHORT_STR
    #define __fw_version_git_tag_short_u32() FW_BUILD_VERSION_GIT_TAG_SHORT_U32
    #define __fw_version_git_delta()         FW_BUILD_VERSION_GIT_DELTA
    #define __fw_version_date_str()          FW_BUILD_VERSION_DATE_STR
    #define __fw_version_date_day()          FW_BUILD_VERSION_DATE_DAY
    #define __fw_version_date_month()        FW_BUILD_VERSION_DATE_MONTH
    #define __fw_version_date_year()         FW_BUILD_VERSION_DATE_YEAR

#else
    #define __fw_version_custom_str()        ""
    #define __fw_version_release_str()       FW_RELEASE_VERSION_STRING
    #define __fw_version_release_major()     FW_RELEASE_VERSION_MAJOR
    #define __fw_version_release_minor()     FW_RELEASE_VERSION_MINOR
    #define __fw_version_release_patch()     FW_RELEASE_VERSION_PATCH
    #define __fw_version_git_tag_full_str()  "00000000000000000000" \
                                             "00000000000000000000"
    #define __fw_version_git_tag_short_str() "00000000"
    #define __fw_version_git_tag_short_u32() 0
    #define __fw_version_git_delta()         0
    /**
     * __DATE__     Mmm dd yyyy
     * __TIME__     hh:mm:ss
     */
    #define __fw_version_date_day_digit0()  \
        ( __DATE__[4] != ' ' ? __DATE__[4] - '0' : 0 )
    #define __fw_version_date_day_digit1()  (__DATE__[5] - '0')
    #define __fw_version_date_day()         \
        (__fw_version_date_day_digit0()*10 + __fw_version_date_day_digit1())

    /* Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec
     * 01   02   03   04   05   06   07   08   09   10   11   12
     * 
     *  A   Apr Aug
     *  D   Dec
     *  F   Feb
     *  J   Jan Jun Jul
     *  M   Mar May
     *  N   Nov
     *  O   Oct
     *  S   Sep
     */
    #define __fw_version_date_month_digit0()    \
        (__DATE__[0]=='O'||__DATE__[0]=='N'||__DATE__[0]=='D' ? 1 : 0)
    #define __fw_version_date_month_digit1()    (                       \
        __DATE__[0]=='O' ? 0 :                                          \
        (__DATE__[0]=='J'&&__DATE__[1]=='a')||__DATE__[0]=='N' ? 1 :    \
        __DATE__[0]=='F'||__DATE__[0]=='D' ? 2 :                        \
        __DATE__[0]=='M'&&__DATE__[2]=='r' ? 3 :                        \
        __DATE__[0]=='A'&&__DATE__[1]=='p' ? 4 :                        \
        __DATE__[0]=='M'&&__DATE__[2]=='y' ? 5 :                        \
        __DATE__[0]=='J'&&__DATE__[2]=='n' ? 6 :                        \
        __DATE__[0]=='J'&&__DATE__[2]=='l' ? 7 :                        \
        __DATE__[0]=='A'&&__DATE__[1]=='u' ? 8 :                        \
        __DATE__[0]=='S' ? 9 : 0 )
    #define __fw_version_date_month()           \
        (__fw_version_date_month_digit0()*10 +  \
        __fw_version_date_month_digit1())

    #define __fw_version_date_year_digit0() (__DATE__[7]-'0')
    #define __fw_version_date_year_digit1() (__DATE__[8]-'0')
    #define __fw_version_date_year_digit2() (__DATE__[9]-'0')
    #define __fw_version_date_year_digit3() (__DATE__[10]-'0')

    #define __fw_version_date_year() (          \
        __fw_version_date_year_digit0() * 1000+ \
        __fw_version_date_year_digit1() * 100 + \
        __fw_version_date_year_digit2() * 10  + \
        __fw_version_date_year_digit3()         )

    #define __fw_version_date_str()                 \
        {   '0' + __fw_version_date_year_digit0(),  \
            '0' + __fw_version_date_year_digit1(),  \
            '0' + __fw_version_date_year_digit2(),  \
            '0' + __fw_version_date_year_digit3(),  \
            '.',                                    \
            '0' + __fw_version_date_month_digit0(), \
            '0' + __fw_version_date_month_digit1(), \
            '.',                                    \
            '0' + __fw_version_date_day_digit0(),   \
            '0' + __fw_version_date_day_digit1(), 0 }

    #if FW_RELEASE_VERSION_MAJOR < 10
        #define __fw_release_version_major_init_list    \
            '0' + FW_RELEASE_VERSION_MAJOR
    #elif FW_RELEASE_VERSION_MAJOR < 100
        #define __fw_release_version_major_init_list    \
            '0' + FW_RELEASE_VERSION_MAJOR/10,          \
            '0' + FW_RELEASE_VERSION_MAJOR%10
    #elif FW_RELEASE_VERSION_MAJOR < 1000
        #define __fw_release_version_major_init_list    \
            '0' + FW_RELEASE_VERSION_MAJOR/100 ,        \
            '0' + FW_RELEASE_VERSION_MAJOR/10%10,       \
            '0' + FW_RELEASE_VERSION_MAJOR%10
    #else
        #error "version major >= 1000 is not supported"
    #endif

    #if FW_RELEASE_VERSION_MINOR < 10
        #define __fw_release_version_minor_init_list    \
            '0' + FW_RELEASE_VERSION_MINOR
    #elif FW_RELEASE_VERSION_MINOR < 100
        #define __fw_release_version_minor_init_list    \
            '0' + FW_RELEASE_VERSION_MINOR/10,          \
            '0' + FW_RELEASE_VERSION_MINOR%10
    #elif FW_RELEASE_VERSION_MINOR < 1000
        #define __fw_release_version_minor_init_list    \
            '0' + FW_RELEASE_VERSION_MINOR/100 ,        \
            '0' + FW_RELEASE_VERSION_MINOR/10%10,       \
            '0' + FW_RELEASE_VERSION_MINOR%10
    #else
        #error "version minor >= 1000 is not supported"
    #endif

    #if FW_RELEASE_VERSION_PATCH < 10
        #define __fw_release_version_patch_init_list    \
            '0' + FW_RELEASE_VERSION_PATCH
    #elif FW_RELEASE_VERSION_PATCH < 100
        #define __fw_release_version_patch_init_list    \
            '0' + FW_RELEASE_VERSION_PATCH/10,          \
            '0' + FW_RELEASE_VERSION_PATCH%10
    #elif FW_RELEASE_VERSION_PATCH < 1000
        #define __fw_release_version_patch_init_list    \
            '0' + FW_RELEASE_VERSION_PATCH/100 ,        \
            '0' + FW_RELEASE_VERSION_PATCH/10%10,       \
            '0' + FW_RELEASE_VERSION_PATCH%10
    #else
        #error "version minor >= 1000 is not supported"
    #endif

    #define __fw_version_string()  {                    \
            'v', __fw_release_version_major_init_list,  \
            '.', __fw_release_version_minor_init_list,  \
            '.', __fw_release_version_patch_init_list,  \
            '-',                                        \
            '0' + __fw_version_date_year_digit0(),      \
            '0' + __fw_version_date_year_digit1(),      \
            '0' + __fw_version_date_year_digit2(),      \
            '0' + __fw_version_date_year_digit3(),      \
            '0' + __fw_version_date_month_digit0(),     \
            '0' + __fw_version_date_month_digit1(),     \
            '0' + __fw_version_date_day_digit0(),       \
            '0' + __fw_version_date_day_digit1(), 0     }
#endif

#define __fw_version_time_hours_digit0()    (__TIME__[0]-'0')
#define __fw_version_time_hours_digit1()    (__TIME__[1]-'0')
#define __fw_version_time_minutes_digit0()  (__TIME__[3]-'0')
#define __fw_version_time_minutes_digit1()  (__TIME__[4]-'0')
#define __fw_version_time_str()                {\
    '0' + __fw_version_time_hours_digit0(),     \
    '0' + __fw_version_time_hours_digit1(),     \
    ':',                                        \
    '0' + __fw_version_time_minutes_digit0(),   \
    '0' + __fw_version_time_minutes_digit1(), 0 }

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
const char* fw_version_string(void);
const char* fw_version_custom_str(void);
const char* fw_version_release_str(void);
uint32_t fw_version_release_major(void);
uint32_t fw_version_release_minor(void);
uint32_t fw_version_release_patch(void);
const char* fw_version_git_tag_full_str(void);
const char* fw_version_git_tag_short_str(void);
uint32_t fw_version_git_tag_short_u32(void);
uint32_t fw_version_git_delta(void);
const char* fw_version_date_str(void);
const char* fw_version_time_str(void);
uint32_t fw_version_date_day(void);
uint32_t fw_version_date_month(void);
uint32_t fw_version_date_year(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __FW_VERSION_H_ */
