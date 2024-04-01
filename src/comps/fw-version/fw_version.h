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
#define FW_RELEASE_VERSION_MAJOR            0
// minor: for Backward compatible new feature
#define FW_RELEASE_VERSION_MINOR            0
// patch: Backward compatible bug fix
#define FW_RELEASE_VERSION_PATCH            0

// FW release version as string
#define FW_RELEASE_VERSION_STRING       "v" \
    __stringify(FW_RELEASE_VERSION_MAJOR)"."\
    __stringify(FW_RELEASE_VERSION_MINOR)"."\
    __stringify(FW_RELEASE_VERSION_PATCH)

#if defined(FW_BUILD_VERSION_ENABLE) && FW_BUILD_VERSION_ENABLE
    #define FW_VERSION_STRING       FW_BUILD_VERSION_STRING
#else
    #define FW_VERSION_STRING       FW_RELEASE_VERSION_STRING
#endif

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
/**
 * @brief   retrieve a pointer to the version string
 */
const char* fw_version_get_string(void);

/**
 * @brief   retrieve a pointer to the release version string
 */
const char* fw_version_get_release_string(void);

/**
 * @brief   retrieve a pointer to the build version string
 */
const char* fw_version_get_build_string(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __FW_VERSION_H_ */
