# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
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
# Maintainer  Christian Ehlers (SG Wireless)
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

if(__feature_lte)
__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/machine_uart.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/machine_uart.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
    EXTRA_INCLUDES  ${CMAKE_CURRENT_LIST_DIR}
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/network_ppp.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/network_ppp.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)
endif()

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
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/machine_sdcard.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/machine_sdcard.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

__sdk_add_patch(
    ENTITY_NAME     micropython
    ORIGINAL_FILE   ${__dir_micropython}/ports/esp32/mpthreadport.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/mpthreadport.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

# Collect source files
set(MPY_HOOKS_SRCS
    "${CMAKE_CURRENT_LIST_DIR}/i2c_ioexp_hooks.c"
    "${CMAKE_CURRENT_LIST_DIR}/machine_timers_hooks.c"
    "${CMAKE_CURRENT_LIST_DIR}/uart_irq_hooks.c"
)

# Add UART hooks only if LTE feature is enabled
if(__feature_lte)
    list(APPEND MPY_HOOKS_SRCS "${CMAKE_CURRENT_LIST_DIR}/machine_uart_hooks.c")
    set(UART_GUARD_DEP f1_uart_guard)
else()
    set(UART_GUARD_DEP "")
endif()

__sdk_add_component( f1_mpy_hooks

    MPY_MODS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    MPY_EXCLUDE_FILES
        ${__dir_micropython}/ports/esp32/mpthreadport.c

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/machine_timers_hooks.c

    SRCS
        ${MPY_HOOKS_SRCS}

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}

    REQUIRED_SDK_LIBS
        ${UART_GUARD_DEP}

    REQUIRED_ESP_LIBS
        esp_driver_i2s
        esp_wifi
        esp_driver_spi
        esp_driver_gpio

    MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/mpy_hooks.config
    MENU_PROMPT     "micropython hooks"
    MENU_GROUP      MAIN.PLATFORM.F1
)

endif() # __build_variant == micropython

# --- end of file ------------------------------------------------------------ #

