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
# Desc      support the building of the fuel-gauge interface component and its
#           corresponding driver BQ27421
# ---------------------------------------------------------------------------- #

__sdk_menu_config_group_add(MAIN.PLATFORM.F1.FUEL_GAUGE "Fuel Gauge")

__sdk_add_component( f1_fuel_gauge_if

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/src/fuel_gauge.c

    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/src/mod_fuel_gauge.c

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/bq27421_stub/*.c"

    INCS_PRIV
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/bq27421_stub

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}/inc

    MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/cfg/fuel_gauge.config
    MENU_PROMPT     "Fuel Gauge interface component"
    MENU_GROUP      MAIN.PLATFORM.F1.FUEL_GAUGE
)

set(__dir_driver_bq27421 ${__dir_ext}/lib-BQ27421)

__sdk_add_component( driver_bq27421

    SRCS
        ${__dir_driver_bq27421}/bq27421.c

    INCS_IF
        ${__dir_driver_bq27421}

    INCS_PRIV
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/src/bq27421_stub
)

__sdk_add_patch(
    ENTITY_NAME     driver_bq27421
    ORIGINAL_FILE   ${__dir_driver_bq27421}/bq27421.c
    PATCH_FILE      ${CMAKE_CURRENT_LIST_DIR}/patches/bq27421.c.patch
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
)

# --- end of file ------------------------------------------------------------ #
