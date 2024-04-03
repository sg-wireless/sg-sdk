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
# Desc      firmware/bootloader interface component amalgement between firmware
#           and bootloader builds
# ---------------------------------------------------------------------------- #

if(BOOTLOADER_BUILD)

    include($ENV{__tree_dir_builder}/cmake/log.cmake)
    include($ENV{__tree_dir_builder}/cmake/entity.cmake)
    include($ENV{__tree_dir_builder}/cmake/sdk.cmake)
    include($ENV{__tree_dir_builder}/cmake/utils.cmake)

    __sdk_concatenate_files(${CMAKE_BINARY_DIR}/bootif_kconfig
        ${CMAKE_CURRENT_LIST_DIR}/cfg/safeboot_basics.config
        ${CMAKE_CURRENT_LIST_DIR}/cfg/safeboot_switch.config
    )
    idf_component_register(
        SRCS        "boot_if.c"
        REQUIRES    bootloader_support
        KCONFIG_PROJBUILD ${CMAKE_BINARY_DIR}/bootif_kconfig
    )

    idf_build_set_property(
        INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}"
        APPEND
    )

    include($ENV{__tree_dir_platform}/esp-idf-patches/CMakeLists.txt)

    __entity_find(__patched_libs ENTITY_TYPE patched)

    log_list(__patched_libs)
    foreach(__lib ${__patched_libs})
        if(TARGET __idf_${__lib})
            __sdk_patch_target(${__lib} __idf_${__lib})
        endif()
    endforeach()

else()

    __sdk_add_component( boot_if

        LOGS_DEFS
            "${CMAKE_CURRENT_LIST_DIR}/boot_if.c"

        SRCS
            "${CMAKE_CURRENT_LIST_DIR}/*.c"

        INCS_IF
            ${CMAKE_CURRENT_LIST_DIR}
        
        INCS_PRIV
            ${__dir_esp_idf}/components/bootloader_support/include
    )

    __sdk_menu_config_add_component_menu( safeboot_feature
        ${CMAKE_CURRENT_LIST_DIR}/cfg/safeboot_basics.config
        MENU_PROMPT     "Safeboot Feature"
        MENU_GROUP      MAIN.PLATFORM.F1
    )

    __sdk_menu_config_add_component_menu( safeboot_feature
        ${CMAKE_CURRENT_LIST_DIR}/cfg/safeboot_switch.config
        MENU_GROUP      MAIN.PLATFORM.F1
    )

    if(__feature_safeboot_switch)
        __sdk_add_kconfig_default(
            ${CMAKE_CURRENT_LIST_DIR}/cfg/safeboot_switch.default)
    endif()

endif()

# --- end of file ------------------------------------------------------------ #
