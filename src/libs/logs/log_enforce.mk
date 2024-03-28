# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# ---------------------------------------------------------------------------- #
# Copyright (c) 2022, Pycom Limited.
#
# This software is licensed under the GNU GPL version 3 or any
# later version, with permitted additional terms. For more information
# see the Pycom Licence v1.0 document supplied with this file, or
# available at https://www.pycom.io/opensource/licensing
#
# Author    Ahmed Sabry (Pycom)
#
# Desc      This file introduces some debugging capabilities to be used any
#            where in the code without any previous declaration or inclusion
# ---------------------------------------------------------------------------- #

ifneq ($(TEST),)
    __call_printf := log_printf_impl
    __declare_log_printf := void log_printf_impl(const char* fmt, ...)
else
    __call_printf := printf
    __declare_log_printf := int printf(const char* fmt, ...)
endif

CFLAGS +=                                                   \
    -D__log_enforce\(__args...\)="                          \
        __log_enforce_min_2_args__(__args,\"\")             \
        "                                                   \
    -D__log_enforce_min_2_args__\(__fmt,__args...\)="       \
        do {                                                \
            $(__declare_log_printf);                        \
            const char* notdir(const char* path);           \
            $(__call_printf)(                               \
                \"[%20s :%4d : %20s] \" __fmt \"%s\\n\",    \
                notdir(__FILE__),                           \
                __LINE__,                                   \
                __func__,                                   \
                __args);                                    \
        } while(0)                                          \
        "

# --- end of file ------------------------------------------------------------ #
