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
# Author    Ahmed Sabry (SGWireless)
#
# Desc      Including the build files of the SGW3501 board dir.
# ---------------------------------------------------------------------------- #

# environment
cmake_minimum_required(VERSION 3.21)
include($ENV{__tree_dir_builder}/cmake/env.cmake)

# target
set(IDF_TARGET esp32s3)

log_var(APP_NAME)
log_var(APP_DIR)
log_var(SDK_PLATFORM)
log_var(SDK_BOARD_OEM_NAME)
log_var(SDK_BOARD_OEM_NUMBER)
log_var(SDK_BOARD_SHIELD)
log_var(VARIANT)
log_var(__partition_table)
log_list(__sdkconfigs_files)

__sdk_menu_config_group_add(MAIN.PLATFORM.F1 "F1 Platform")

# sdk components cmake lists
__sdk_add_comp_dirs(
    ${__dir_platform_comps}/logs-if
    ${__dir_platform_comps}/ioexp-if
    ${__dir_platform_comps}/sys-inspect-if
    ${__dir_platform_comps}/efuse-if
    ${__dir_platform_comps}/sys-info

    ${__dir_platform}/mpy-hooks
    ${__dir_sdk_drivers}/common
    ${__dir_platform}/bootloader_components/boot-if
    ${__dir_platform}/esp-idf-patches

    ${__dir_src}/libs/*

    ${__dir_sdk_comps}/fw-version
)

if( __feature_lora )
    __sdk_add_comp_dirs(
        ${__dir_platform_comps}/lora-if
        ${__dir_sdk_comps}/lora
    )
endif()

if( __feature_lte )
    __sdk_add_comp_dirs(
        ${__dir_platform_comps}/lte-poc
    )
endif()

# platform log_lib platform subsystem define
__sdk_add_logs_defs_files(${CMAKE_CURRENT_LIST_DIR}/logs_defs.h)

# define the main SDK config file to be the sdkconfigs.h
__sdk_add_compile_options("-DMAIN_SDK_CONFIG_FILE=\"sdkconfig.h\""
    SDK_LIBS USR_LIBS)

__sdk_add_compile_options(-include sdkconfig.h SDK_LIBS USR_LIBS)

if(DEFINED __sdkconfigs_files)
    __sdk_add_kconfig_default(${__sdkconfigs_files})
endif()

if(DEFINED __partition_table)
    __set_global_attribute(PARTITION_TABLE ${__partition_table})
endif()

# bind to the esp-idf build system
include (${__dir_cmake}/esp-idf.cmake)
if(OFF)
    # the actual call to the project() will be occur inside esp-idf.cmake
    # this fake call here to get ride of a certain cmake warning
    project(${__prj_name})
endif()

# --- end of file ------------------------------------------------------------ #
