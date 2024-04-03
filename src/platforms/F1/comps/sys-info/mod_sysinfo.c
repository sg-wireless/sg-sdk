/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   This file represents a uPython c-module interface to the exported
 *          APIs of system information.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "mp_lite_if.h"
#include "sysinfo.h"

/* --- module functions definitions ----------------------------------------- */

__mp_mod_name(sysinfo, SystemInfo);


__mp_mod_fun_0(sysinfo, board) (void) {

    sysinfo_board();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, flash) (void) {

    sysinfo_flash_stats();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, spiram) (void) {

    sysinfo_spiram_stats();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, version) (void) {

    sysinfo_version();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, efuses) (void) {

    sysinfo_efuses();

    return mp_const_none;
}

__mp_mod_fun_0(sysinfo, all) (void) {

    sysinfo_board();
    sysinfo_version();
    sysinfo_efuses();
    sysinfo_flash_stats();
    sysinfo_spiram_stats();

    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
