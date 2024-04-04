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
# Desc      This file is responsible to manage the building the external Semtech
#           LoRaMac-node project
# ---------------------------------------------------------------------------- #

# --- pycom-lora-stack to semtech-lora-mac-node options ---------------------- #

set( __loramac_chip sx126x CACHE STRING "default semtech lora chip is sx126x")
if(NOT DEFINED __loramac_extra_includes)
    set( __loramac_extra_includes )
endif()

# --- LoRaMac-node build configurations -------------------------------------- #

set(MBED_RADIO_SHIELD   ""      CACHE STRING "" FORCE)
set(SECURE_ELEMENT      SOFT_SE CACHE STRING "" FORCE)
set(APPLICATION         ""      CACHE STRING "" FORCE)
set(USE_USB_CDC         OFF     CACHE STRING "" FORCE)
set(USE_DEBUGGER        OFF     CACHE STRING "" FORCE)
set(CLASSB_ENABLED      OFF     CACHE STRING "" FORCE)
set(RADIO               ${__loramac_chip} CACHE STRING "" FORCE)
set(BOARD               system  CACHE STRING "" FORCE) # workaround

set(REGION_EU868        ON      CACHE STRING "Region EU868" FORCE)
set(REGION_US915        ON      CACHE STRING "Region US915" FORCE)
set(REGION_CN779        ON      CACHE STRING "Region CN779" FORCE)
set(REGION_EU433        ON      CACHE STRING "Region EU433" FORCE)
set(REGION_AU915        ON      CACHE STRING "Region AU915" FORCE)
set(REGION_AS923        ON      CACHE STRING "Region AS923" FORCE)
set(REGION_CN470        ON      CACHE STRING "Region CN470" FORCE)
set(REGION_KR920        ON      CACHE STRING "Region KR920" FORCE)
set(REGION_IN865        ON      CACHE STRING "Region IN865" FORCE)
set(REGION_RU864        ON      CACHE STRING "Region RU864" FORCE)

# --- LoRaMac-node build procedure ------------------------------------------- #

set( __loramac_bin_dir ${CMAKE_BINARY_DIR}/LoRaMac-node )
set( __loramac_src_dir ${__dir_ext}/LoRaMac-node )

function(fetch_loramac_libraries __fetched_libraries_variable)

    include(FetchContent)

    FetchContent_Declare( __loramac_node_prj
        SOURCE_DIR      ${__loramac_src_dir}
        BINARY_DIR      ${__loramac_bin_dir}
        EXCLUDE_FROM_ALL
        )

    FetchContent_MakeAvailable( __loramac_node_prj )

    set(__fetched_libs)
    foreach(__lib mac radio peripherals)
        get_target_property(__srcs ${__lib} SOURCES)
        get_target_property(__old_incs ${__lib} INTERFACE_INCLUDE_DIRECTORIES)
        set(__incs )
        foreach(__inc_dir ${__old_incs})
            if(${__inc_dir} MATCHES "^.<TARGET_PROPERTY:([a-zA-Z0-9_]+),.*")
                if("${CMAKE_MATCH_1}" STREQUAL "board")
                    set(CMAKE_MATCH_1 boards)
                endif()
                list(APPEND __incs ${__loramac_src_dir}/src/${CMAKE_MATCH_1})
            elseif(EXISTS ${__inc_dir})
                list(APPEND __incs ${__inc_dir})
            endif()
        endforeach()
        get_target_property(__defs ${__lib} COMPILE_DEFINITIONS)

        set(__sdk_lib_name loramac_node_prjlib__${__lib} )
        list(APPEND __fetched_libs ${__sdk_lib_name})
        
        __sdk_add_component(${__sdk_lib_name}
            SRCS    ${__srcs}
            INCS    ${__incs} ${__loramac_extra_includes}
            DEFS    ${__defs} aes_encrypt=__aes_encrypt
        )
        __sdk_add_patch(
            ENTITY_NAME     ${__sdk_lib_name}
            FINAL_DIR       ${CMAKE_CURRENT_LIST_DIR}/modified_sources
            PATCHES_DIRS    ${CMAKE_CURRENT_LIST_DIR}/patches
        )
    endforeach()

    set(${__fetched_libraries_variable} ${__fetched_libs} PARENT_SCOPE)
endfunction()

fetch_loramac_libraries( __loramac_fetched_libraries )

# --- end of file ------------------------------------------------------------ #
