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
# Desc      CMake list file for micropython main component build description.
# ---------------------------------------------------------------------------- #

__sdk_add_component( micropython_main

    MPY_EXCLUDE_FILES
        ${__dir_micropython}/ports/esp32/main.c

    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/main.c

    SRCS
        ${CMAKE_CURRENT_LIST_DIR}/main.c

    INCS
        ${CMAKE_CURRENT_LIST_DIR}
        ${__dir_ext}/esp-idf/components/log/include
        ${__dir_ext}/esp-idf/components/spi_flash/include
        ${__dir_ext}/esp-idf/components/nvs_flash/include

    INCS_IF
        ${__dir_micropython}
        ${__dir_micropython}/ports/esp32
        ${__dir_platform}
)

# __sdk_add_kconfig_default(${__dir_board}/sdk-config/sdkconfig.micropython)

__sdk_add_compile_options(
    "-DMICROPY_VFS_LFS2=1"
    "-DBASE=0"
    "-DMP_CONFIGFILE=\"${CMAKE_CURRENT_LIST_DIR}/mpconfigport.h\""
    "-g"
    "-Os"
)

__sdk_add_micropython_frozen_manifest(
    ${CMAKE_CURRENT_LIST_DIR}/main_manifest.py)

# --- end of file ------------------------------------------------------------ #
