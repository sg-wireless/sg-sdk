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
# @author   Ahmed Sabry (SG Wireless)
#
# @brief    This file is responsible for building the lora-stack
# ---------------------------------------------------------------------------- #

# --- integrating Semtech LoRaMac building ----------------------------------- #

set(__loramac_dir ${__dir_ext}/LoRaMac-node/src)

set(  __loramac_extra_includes
    ${__dir_src}/libs/logs/inc
    ${__dir_src}/libs/logs/src
    ${__dir_src}/libs/utils
    ${__log_generator_output_dir})

set( __lorastack_includes
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/src/lora_mgr
    ${CMAKE_CURRENT_LIST_DIR}/src/lora_wan
    ${CMAKE_CURRENT_LIST_DIR}/src/lora_raw
    ${CMAKE_CURRENT_LIST_DIR}/src/lora_utils
    ${CMAKE_CURRENT_LIST_DIR}/src/radio_ext
    ${CMAKE_CURRENT_LIST_DIR}/src/stubs
    ${CMAKE_CURRENT_LIST_DIR}/src/stubs/drivers

    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc/drivers
    ${CMAKE_CURRENT_LIST_DIR}/mpy

    ${__loramac_dir}/peripherals/soft-se
    ${__loramac_dir}
    ${__loramac_dir}/radio
    ${__loramac_dir}/mac
    ${__loramac_dir}/mac/region

    ${__loramac_dir}
    ${__loramac_dir}/boards
    ${__loramac_dir}/mac
    ${__loramac_dir}/mac/region
    ${__loramac_dir}/system
    ${__loramac_dir}/radio
    ${__loramac_dir}/radio/sx126x
)

include( ${CMAKE_CURRENT_LIST_DIR}/src/lm/CMakeLists.txt )

include( ${CMAKE_CURRENT_LIST_DIR}/src/lmhandler/CMakeLists.txt )

# get the loramac compile definitions to be added to the lora stack compile
# definition as well
__entity_get_attribute(loramac_node_prjlib__mac DEFINITIONS __loramac_defs)
# get_target_property(__loramac_defs ext_lora_stack_mac COMPILE_DEFINITIONS)
string(REGEX REPLACE "\"" "" __loramac_defs "${__loramac_defs}")
string(REGEX REPLACE
    "REGION_([A-Z][A-Z][1-9][0-9][0-9]);"
    "REGION_\\1=REGION_\\1;"
    __loramac_defs
    "${__loramac_defs}")

# --- unittest options ------------------------------------------------------- #

if( "${TEST}" STREQUAL "lora_unittest" )
    set( __lora_defs __unittest)
    file(GLOB __unittest_srcs "${CMAKE_CURRENT_LIST_DIR}/tst/unittest/*.c")
endif()

# --- prepare lora-stack building -------------------------------------------- #

__sdk_add_component( lora_stack

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/inc/logs_defs.h

    MPY_MODS
        ${__unittest_srcs}
        "${CMAKE_CURRENT_LIST_DIR}/mpy/*.c"
    
    DEFS
        ${__lora_defs}
        ${__loramac_defs}

    SRCS
        ${__unittest_srcs}
        "${CMAKE_CURRENT_LIST_DIR}/src/lora_mgr/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/lora_wan/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/lora_raw/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/lora_utils/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/radio_ext/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/stubs/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/src/stubs/drivers/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/mpy/*.c"

    INCS_PRIV
        ${__lorastack_includes}
        ${__loramac_handler_includes}
        ${__dir_esp_idf}/components/freertos/include/freertos

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}/inc
        ${CMAKE_CURRENT_LIST_DIR}/inc/drivers
    
    REQUIRED_SDK_LIBS
        ${__loramac_fetched_libraries}
        loramac_handler
        adt_lib
        utils_lib
        state_machine_lib
        log_lib
        lora_if
)

# --- end of file ------------------------------------------------------------ #
