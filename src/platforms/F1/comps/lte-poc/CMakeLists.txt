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
# Author    Ahmed Sabry (SG Wireless)
#
# Desc      Specify the LTE.py implementation in both python and C to the build
#           system.
# ---------------------------------------------------------------------------- #

__sdk_add_component( lte_py_in_c

    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/mod_lte.c

    LOGS_DEFS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    SRCS
        ${CMAKE_CURRENT_LIST_DIR}/lte_helpers.c
        ${CMAKE_CURRENT_LIST_DIR}/lte_ppp.c
        ${CMAKE_CURRENT_LIST_DIR}/lte_uart.c
        ${CMAKE_CURRENT_LIST_DIR}/lte.c
        ${CMAKE_CURRENT_LIST_DIR}/mod_lte.c

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}
)

__sdk_add_micropython_frozen_manifest("${CMAKE_CURRENT_LIST_DIR}/manifest.py")

# --- end of file ------------------------------------------------------------ #
