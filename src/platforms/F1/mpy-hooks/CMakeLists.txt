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
# Desc      This plugs the mpy-hook component into the build system
# ---------------------------------------------------------------------------- #

if("${__build_variant}" STREQUAL "micropython")

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/uart.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/uart.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
    EXTRA_INCLUDES  ${CMAKE_CURRENT_LIST_DIR}
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/help.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/help.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/machine_i2c.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/machine_i2c.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/machine_pin.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/machine_pin.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/machine_timer.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/machine_timer.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/shared/runtime/pyexec.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/pyexec.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/machine_sdcard.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/machine_sdcard.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_component( f1_mpy_hooks

    MPY_MODS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}

    MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/mpy_hooks.config
    MENU_PROMPT     "micropython hooks"
    MENU_GROUP      MAIN.PLATFORM.F1
)

endif() # __build_variant == micropython

# --- end of file ------------------------------------------------------------ #
