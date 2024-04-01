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
 * @brief   micropython module for fw versioning demonstration
 * --------------------------------------------------------------------------- *
 */

#include "mp_lite_if.h"
#include "log_lib.h"
#include "fw_version.h"

/** -------------------------------------------------------------------------- *
 * module API implementation
 * --------------------------------------------------------------------------- *
 */
__mp_mod_fun_0(fw_version, show)(void)
{
    __log_output("version: "__yellow__"%s"__default__"\n",
        fw_version_get_string());
    __log_output("release: "__yellow__"%s"__default__"\n",
        fw_version_get_release_string());
    __log_output("build:   "__yellow__"%s"__default__"\n",
        fw_version_get_build_string());
    return mp_const_none;
}

__mp_mod_fun_0(fw_version, release)(void)
{
    __log_output("release: "__yellow__"%s"__default__"\n",
        fw_version_get_release_string());
    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
