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
# Desc      This file is responsible for building the Semtech LoRaMac-Handler
# ---------------------------------------------------------------------------- #

set( __loramac_appcommon_dir ${__loramac_dir}/apps/LoRaMac/common)

set( __modified_files_dir ${CMAKE_CURRENT_LIST_DIR}/modified_sources)
set( __patches_files_dir  ${CMAKE_CURRENT_LIST_DIR}/patches )

# collect needed source files from Semtech LoRaMac application handler layer
file(GLOB __loramac_handler_original_sources
    ${__loramac_appcommon_dir}/LmHandler/*.c
    ${__loramac_appcommon_dir}/LmHandler/packages/*.c

    ${__loramac_appcommon_dir}/CayenneLpp.c
    # ${__loramac_appcommon_dir}/NvmDataMgmt.c
)

# --- preparing Pycom FW component ------------------------------------------- #
set(__loramac_handler_includes
    ${__loramac_appcommon_dir}
    ${__loramac_appcommon_dir}/LmHandler
    ${__loramac_appcommon_dir}/LmHandler/packages
)
__sdk_add_component( loramac_handler
    SRCS
        "${__loramac_handler_original_sources}"
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    INCS_PRIV
        ${CMAKE_CURRENT_LIST_DIR}
        ${__loramac_handler_includes}

        ${__lorastack_includes}
    REQUIRED_SDK_LIBS
        utils_lib
        lora_stack
)

__sdk_add_patch(
    ENTITY_NAME     loramac_handler
    FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
    PATCHES_DIRS    ${CMAKE_CURRENT_LIST_DIR}/patches
)


# --- end of file ------------------------------------------------------------ #
