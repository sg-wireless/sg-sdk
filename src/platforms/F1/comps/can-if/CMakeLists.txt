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
# ---------------------------------------------------------------------------- #

__sdk_menu_config_group_add(MAIN.PLATFORM.F1.CAN "CAN")

__sdk_add_component( f1_can_if

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/can.c

    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/mod_can.c

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}

    REQUIRED_ESP_LIBS
        driver
        esp_wifi

    MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/can.config
    MENU_PROMPT     "can interface component"
    MENU_GROUP      MAIN.PLATFORM.F1.CAN
)



__sdk_add_kconfig_default(${CMAKE_CURRENT_LIST_DIR}/defaults.config)