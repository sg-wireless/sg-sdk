
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
# Desc      This file plugs the efuse interface component in the build system
# ---------------------------------------------------------------------------- #

__sdk_add_component( f1_efuse_if

    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/src/mod_efuse.c

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/src/efuse_if.c

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}/inc

    REQUIRED_ESP_LIBS
        efuse

    MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/cfg/efuse.config
    MENU_PROMPT     "Board eFuses"
    MENU_GROUP      MAIN.PLATFORM.F1
)

# --- end of file ------------------------------------------------------------ #
