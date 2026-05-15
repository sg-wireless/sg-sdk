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
# Desc      This file plugs the ioexp interface component in the build system
# ---------------------------------------------------------------------------- #

if(__feature_lora OR __feature_lte OR __feature_secure_element)
    __sdk_add_compile_options(-DCONFIG_IOEXP_ENABLED)
    set(CONFIG_IOEXP_ENABLED ON)
    __sdk_add_comp_dirs( ${__dir_sdk_drivers}/pcal6408a )
endif()

if(CONFIG_IOEXP_ENABLED)
    __sdk_add_component( ioexp_if

        MPY_MODS
            ${CMAKE_CURRENT_LIST_DIR}/mod_ioexp.c

        LOGS_DEFS
            ${CMAKE_CURRENT_LIST_DIR}/ioexp.c
            ${CMAKE_CURRENT_LIST_DIR}/ioexp_mp.c

        SRCS
            "${CMAKE_CURRENT_LIST_DIR}/ioexp.c"
            "${CMAKE_CURRENT_LIST_DIR}/ioexp_mp.c"
            "${CMAKE_CURRENT_LIST_DIR}/mod_ioexp.c"

        INCS_IF
            ${CMAKE_CURRENT_LIST_DIR}

        REQUIRED_SDK_LIBS
            driver_pcal6408a
            mp_i2c_bridge

        REQUIRED_ESP_LIBS
            driver
            esp_event
            esp_wifi
            esp_timer
            esp_driver_i2s
            esp_driver_spi
            esp_driver_gpio
    )
endif()

# --- end of file ------------------------------------------------------------ #
