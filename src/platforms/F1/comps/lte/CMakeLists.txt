# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
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
# Desc      New robust LTE stack with ESP modem library integration and CMUX support
#           Provides both C API and MicroPython interface with "import lte" compatibility
# ---------------------------------------------------------------------------- #

if(__feature_lte)

    __sdk_add_component( modlte

        MPY_MODS
            ${CMAKE_CURRENT_LIST_DIR}/modlte.c

        LOGS_DEFS
            "${CMAKE_CURRENT_LIST_DIR}/logs_defs.h"

        SRCS
            ${CMAKE_CURRENT_LIST_DIR}/espmodem.c
            ${CMAKE_CURRENT_LIST_DIR}/modlte.c

        INCS_IF
            ${CMAKE_CURRENT_LIST_DIR}

        INCS_PRIV
            ${CMAKE_CURRENT_LIST_DIR}
            # Add explicit paths to esp_modem v2.0.0 managed component headers
            # Types and configs in include/, pre-generated AT command declarations in command/include/
            ${CMAKE_CURRENT_LIST_DIR}/../../managed_components/espressif__esp_modem/include
            ${CMAKE_CURRENT_LIST_DIR}/../../managed_components/espressif__esp_modem/command/include

        REQUIRED_ESP_LIBS
            esp_driver_gpio
            esp_driver_uart
            esp_driver_i2c
            esp_timer
            esp_wifi
            esp_netif
            esp_event
            esp_common
            lwip

        REQUIRES
            espressif__esp_modem
            ioexp_if
            f1_logs_if
            log_lib
            f1_uart_guard
    )

    __sdk_add_micropython_frozen_manifest("${CMAKE_CURRENT_LIST_DIR}/manifest.py")

endif()

# --- end of file ------------------------------------------------------------ #
