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
# Desc      This file is responsible for defining global variables used in the
#           Firmware building such as:
#               - directories locations variables
#               - inclusion of different builder facilities
# ---------------------------------------------------------------------------- #

# --- firmware main folders -------------------------------------------------- #

get_filename_component( __dir_root  "${CMAKE_CURRENT_LIST_DIR}/../../.."
    ABSOLUTE)

set( __dir_src          ${__dir_root}/src )

set( __dir_platform     ${__dir_src}/platforms/${SDK_PLATFORM} )
set( __dir_ext          ${__dir_root}/ext )
set( __dir_esp_idf      ${__dir_ext}/esp-idf )

set( __dir_sdk_comps        ${__dir_src}/comps )
set( __dir_platform_comps   ${__dir_platform}/comps )

set( __dir_micropython  ${__dir_ext}/micropython )

set( __dir_tools        ${__dir_root}/tools )
set( __dir_builder      ${__dir_tools}/builder )
set( __dir_cmake        ${__dir_builder}/cmake )

# --- other global entities definitions -------------------------------------- #

include( ${__dir_cmake}/log.cmake )     # for logging
log_msg_init()

include( ${__dir_cmake}/utils.cmake )

include( ${__dir_cmake}/entity.cmake )  # to add a firmware component
if (NOT CMAKE_SCRIPT_MODE_FILE )
    __entity_init(main_entity GLOBAL)
endif()
include( ${__dir_cmake}/sdk.cmake )    # to add a firmware component
include( ${__dir_cmake}/menu_config.cmake )
if (NOT CMAKE_SCRIPT_MODE_FILE )
    __sdk_menu_config_init()
endif()

__sdk_process_cli_variables()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

# --- end of file ------------------------------------------------------------ #
