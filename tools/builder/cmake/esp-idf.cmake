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
# Desc      The main build driving file for fusion of the SDK, ESP-IDF and
#           micropython project.
# ---------------------------------------------------------------------------- #

include(${__dir_cmake}/micropython.cmake)
include(${__dir_src}/libs/logs/gen/logs_gen.cmake)

# ---------------------------------------------------------------------------- #
# synopsis:
#       __esp_idf_env_vars_checker()
#
# description:
#       checks for the pre-requisits environment variables needed before the
#       build processing.
# ---------------------------------------------------------------------------- #
function(__esp_idf_env_vars_checker)

    # Check SDKBOARD variable. It must be specified in the main build command
    set(fatal_error OFF)
    if(NOT DEFINED SDK_PLATFORM)
        set("${__yellow__}SDK_PLATFORM${__red__} is not specified")
        set(fatal_error ON)
    endif()
    if(NOT DEFINED SDK_BOARD)
        set("${__yellow__}SDK_BOARD${__red__} is not specified")
        set(fatal_error ON)
    endif()

    if(NOT DEFINED IDF_TARGET)
        log_msg("${__yellow__}IDF_TARGET${__red__} is not specified")
            set(fatal_error ON)
    endif()

    if(fatal_error)
        log_msg("fatal error, can not proceed" fatal_error)
    endif()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __esp_idf_get_main_component(
#               [ APP_DIR_VAR   <app-dir-output-variable>  ]
#               [ APP_NAME_VAR  <app-name-output-variable> ]
#       )
#
# description:
#       § It deduces the final output image application name and write it to the
#         caller given variable attached with the keyword APP_NAME_VAR.
#       § It returns the SDK built-in main application component. If the user
#         provided a user project directory and the build variant is native, the
#         returned variable 'APP_DIR_VAR' will be empty.
#       § The caller can request either or both of the app name and app dir
#         but not neither of them.
#       § The identification process depends on the user provided variables on
#         the command line. Those variables are:
#           * VARIANT   the build variant is either "micropython" or "native"
#           * APP_NAME  the desired user application name
#           * APP_DIR   the dedicated user application main directory path
# ---------------------------------------------------------------------------- #
function(__esp_idf_get_main_component)

    cmake_parse_arguments( _ "" "APP_DIR_VAR;APP_NAME_VAR" "" ${ARGN})

    # caller to this function shall pass two inputs variables names to receive
    # the user application name and directory
    if(NOT DEFINED __APP_DIR_VAR AND NOT DEFINED __APP_NAME_VAR)
        log_msg("no specified app name or dir vars" fatal_error)
    endif()

    set(__app_name ${APP_NAME}) # intended dummy setting
    set(__app_name application)
    set(__prj_dir ${__dir_platform})

    if("${__build_variant}" STREQUAL "micropython")
        set(__app_dir ${__prj_dir}/sdk-main/micropython)
    elseif("${__build_variant}" STREQUAL "native")
        if(NOT DEFINED APP_DIR)
            set(__app_dir ${__prj_dir}/sdk-main/native-default)
        endif()
    elseif("${__build_variant}" STREQUAL "")
        log_msg("missing build VARIANT type" fatal_error)
    else()
        log_msg("wrong VARIANT '${__yellow__}${VARIANT}${__default__}'"
            fatal_error)
    endif()
    if(DEFINED __APP_DIR_VAR)
        log_dbg("request APP_DIR: ${__green__}${__app_dir}${__default__}")
        set(${__APP_DIR_VAR} ${__app_dir} PARENT_SCOPE)
    endif()
    if(DEFINED __APP_NAME_VAR)
        log_dbg("request APP_NAME: ${__green__}${__app_name}${__default__}")
        set(${__APP_NAME_VAR} ${__app_name} PARENT_SCOPE)
    endif()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __esp_idf_update_component_list_file( <lib> <cmake-list-file> )
#
# description:
#       This function generates/updates the SDK component cmake list file for
#       the ESP-IDF ecosystem.
# ---------------------------------------------------------------------------- #
function(__esp_idf_update_component_list_file __sdk_lib __cmake_list_file)
    log_dbg("-- prepare esp-idf component-dir for sdk lib "
        "${__green__}${__sdk_lib}${__default__}")

    # -- pick up sdk-lib attributes
    __entity_get_attribute(${__sdk_lib} SOURCES __sources)
    __entity_get_attribute(${__sdk_lib} INCS_IF __incs_if)
    __entity_get_attribute(${__sdk_lib} INCS_PRIV __inc_priv)
    __entity_get_attribute(${__sdk_lib} REQUIRED_ESP_LIBS __required_esp_libs)
    __entity_get_attribute(${__sdk_lib} REQUIRED_SDK_LIBS __required_sdk_libs)
    __get_global_attribute(SDK_INCS __sdk_incs)
    set(__incs ${__inc_priv} ${__incs_if} ${__sdk_incs})
    list(REMOVE_DUPLICATES __incs)

    # -- create intermediate component dir
    set(__contents)

    # -- write compatible ESP-IDF component CMakeLists.txt file
    set(__ts "    ")
    string(APPEND __contents "# --- auto-generated cmake list file\n\n")
    string(APPEND __contents "include(${__dir_cmake}/log.cmake)\n\n")

    string(APPEND __contents "idf_component_register(\n")


    string(APPEND __contents "${__ts}SRCS\n")
    foreach(__item ${__sources})
        string(APPEND __contents "${__ts}${__ts}${__item}\n")
    endforeach()

    string(APPEND __contents "${__ts}INCLUDE_DIRS\n")
    foreach(__item ${__incs})
        string(APPEND __contents "${__ts}${__ts}${__item}\n")
    endforeach()

    string(APPEND __contents "${__ts}REQUIRES\n")
    foreach(__lib ${__required_esp_libs})
        string(APPEND __contents "${__ts}${__ts}${__lib}\n")
    endforeach()
    foreach(__lib ${__required_sdk_libs})
        string(APPEND __contents "${__ts}${__ts}${__lib}\n")
    endforeach()

    string(APPEND __contents ")\n\n")

    if("${__build_variant}" STREQUAL "micropython")
        __entity_get_attribute(${__sdk_lib} MPY_CMODS __cmods)
        if(__cmods)
            log_dbg("lib ${__green__}${__sdk_lib}${__default__} has cmods")
            string(APPEND __contents
                "add_dependencies(__idf_${__sdk_lib} __idf_micropython)\n\n")
        endif()
    endif()

    __sdk_update_file_contents(${__cmake_list_file} ${__contents})
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __esp_idf_update_kconfig_default(
#           <output-file>
#           <input-file> [ <input-file> ... ]
#       )
#
# description:
#       This function updates default configurations of ESP-IDF kconfigs.
# ---------------------------------------------------------------------------- #
function(__esp_idf_update_kconfig_default __out_file __input_file)
    set(__in_files ${__input_file} ${ARGN})
    log_dbg("OUTPUT >> ${__yellow__}${__out_file}${__default__}")
    set(__final_lines)
    foreach(__in ${__in_files})
        log_dbg("INPUT >> ${__cyan__}${__in}${__default__}")
        file(STRINGS ${__in} __lines)
        foreach(__line ${__lines})
            if(NOT __line MATCHES "^#.*")
                log_dbg(" -- line ${__green__}${__line}${__default__}")
                string(REGEX MATCH "^([a-zA-Z0-9_]+)=(.*)$" __out ${__line})
                set(__key ${CMAKE_MATCH_1})
                set(__val ${CMAKE_MATCH_2})
                log_dbg("    key ${__green__}${__key}${__default__}")
                log_dbg("    val ${__yellow__}${__val}${__default__}")
                foreach(__prev_line ${__final_lines})
                    if(__prev_line MATCHES "^${__key}=.*$")
                        log_dbg("    overwrites ${__red__}"
                            "${__prev_line}${__default__}")
                        list(REMOVE_ITEM __final_lines ${__prev_line})
                        break()
                    endif()
                endforeach()
                list(APPEND __final_lines ${__line})
            else()
                log_dbg(" -- line commented "
                    "${__purple__}${__line}${__default__}")
            endif()
        endforeach()
    endforeach()
    set(__contents)
    foreach(__line ${__final_lines})
        string(APPEND __contents "${__line}\n")
    endforeach()
    __sdk_update_file_contents(${__out_file} ${__contents})
endfunction()
# ---------------------------------------------------------------------------- #
# synopsis:
#       __esp_idf_process_preparation()
#
# description:
#       This function apply the required preparation prior the invokation of the
#       ESP-IDF build system.
# ---------------------------------------------------------------------------- #
function(__esp_idf_process_preparation)
    log_dbg("== start esp-idf process preparation" yellow)

    # -- obtain components list set by main board cmake list file
    __sdk_get_comp_list_files(__comp_cmake_list_files)

    # -- add the main application component to the components lists dirs
    log_dbg("STEP >> obtain the application main component dir" cyan)
    __esp_idf_get_main_component(APP_DIR_VAR __sdk_main_dir)
    if(__sdk_main_dir)
        list(APPEND __comp_cmake_list_files ${__sdk_main_dir}/CMakeLists.txt)
    endif()
    
    # -- include all project cmake lists
    log_dbg("STEP >> include all SDK components cmake list files" cyan)
    list(REMOVE_DUPLICATES __comp_cmake_list_files)
    foreach(__list_file ${__comp_cmake_list_files})
        log_dbg("include ${__cyan__}${__list_file}${__default__}")
        include(${__list_file})
    endforeach()

    # -- add the user project main cmake list file
    __sdk_get_user_project_dir(__user_prj_dir)
    if(__user_prj_dir)
        __sdk_enable_user_comps()
        include(${__user_prj_dir}/CMakeLists.txt)
        __sdk_disable_user_comps()
    endif()

    # -- generate the logs definitions
    log_dbg("STEP >> generate the log lib headers" cyan)
    set(__gen_dir ${CMAKE_BINARY_DIR}/sdk_log_gen_dir)
    __sdk_get_logs_defs_files(__files_list)
    if(__files_list)
        __run_logs_defs_generator(GEN_DIR ${__gen_dir} SOURCES ${__files_list})
        __set_global_attribute(SDK_INCS ${__gen_dir} APPEND)
    endif()

    # -- obtain all contributing SDK libraries
    log_dbg("STEP >> obtain all SDK components" cyan)
    __entity_find(__sdk_libraries ENTITY_TYPE library)
    # __entity_list(ATTRIBUTE ENTITY_TYPE VALUES library DISP ENTITY_TYPE)

    set(__sdk_cmake_lists_dir ${CMAKE_BINARY_DIR}/sdk-components)

    set(__esp_idf_comps)

    # --- process micropython project binding
    if("${__build_variant}" STREQUAL "micropython")
        log_dbg("STEP >> start micropython preparation" cyan)
        __micropython_update_generated_modules()
        __micropython_update_usermod_list_files()

        if(NOT EXISTS ${__sdk_cmake_lists_dir}/micropython)
            make_directory(${__sdk_cmake_lists_dir}/micropython)
        endif()
        __micropython_update_esp_idf_list_file(
            ${__sdk_cmake_lists_dir}/micropython/CMakeLists.txt)
        list(APPEND __esp_idf_comps ${__sdk_cmake_lists_dir}/micropython)

        __entity_find(__user_libs ENTITY_TYPE userlib)
        foreach(__userlib ${__user_libs})
            __entity_set_attribute(${__userlib} DEFINITIONS
                "app_main=__unused_app_main__")
        endforeach()
        
    endif()

    # -- set global compile options
    log_dbg("STEP >> set global compile options" cyan)
    __get_global_attribute(COMPILE_FLAGS __global_compile_options)
    log_list(__global_compile_options)
    add_compile_options("${__global_compile_options}")

    # --- prepare sdkconfig combined file
    log_dbg("STEP >> generate the SDKCONFIG_DEFAULTS file" cyan)
    __get_global_attribute(PARTITION_TABLE __part_table)
    set(__part_table_conf_file ${CMAKE_BINARY_DIR}/sdkconfig.partition_table)
    set(__contents
        "\nCONFIG_PARTITION_TABLE_CUSTOM_FILENAME=\"${__part_table}\"\n\n")
    __sdk_update_file_contents(${__part_table_conf_file} ${__contents})
    __sdk_get_kconfig_default(__sdkconfig_files)
    __esp_idf_update_kconfig_default(${SDKCONFIG_DEFAULTS}
        ${__sdkconfig_files} ${__part_table_conf_file})
    # __sdk_concatenate_files(${SDKCONFIG_DEFAULTS}
    #     ${__sdkconfig_files} ${__part_table_conf_file})
    
    # -- create components directories for the ESP-IDF build system
    log_dbg("STEP >> create ESP-IDF SDK components dirs" cyan)
    foreach(__sdk_lib ${__sdk_libraries})

        set(__lib_comp_dir ${__sdk_cmake_lists_dir}/${__sdk_lib})
        if(NOT EXISTS ${__lib_comp_dir})
            make_directory(${__lib_comp_dir})
        endif()

        __esp_idf_update_component_list_file(
            ${__sdk_lib}
            ${__lib_comp_dir}/CMakeLists.txt)

        list(APPEND __esp_idf_comps ${__lib_comp_dir})
    endforeach()

    # -- create configuration-only component for main menu config items
    log_dbg("STEP >> create ESP-IDF SDK configuration component" cyan)
    set(__lib_comp_dir ${__sdk_cmake_lists_dir}/sdk_main_menu_config)
    make_directory(${__lib_comp_dir})

    __sdk_menu_config_generate(${__lib_comp_dir}/Kconfig.projbuild)

    set(__contents "idf_component_register()\n")
    __sdk_update_file_contents(${__lib_comp_dir}/CMakeLists.txt ${__contents})
    list(APPEND __esp_idf_comps ${__lib_comp_dir})

    set(EXTRA_COMPONENT_DIRS ${__esp_idf_comps} PARENT_SCOPE)
endfunction()

function(__esp_idf_process_finalization)
    log_dbg("== start ESP-IDF process finalization" yellow)

    # -- generate required patches
    log_dbg("STEP >> process all required patching" cyan)
    __entity_find(__patched_libs ENTITY_TYPE patched)
    # __entity_list(ATTRIBUTE ENTITY_TYPE VALUES patched DISP ENTITY_TYPE)

    foreach(__lib ${__patched_libs})
        if(TARGET __idf_${__lib})
            __sdk_patch_target(${__lib} __idf_${__lib})
        endif()
    endforeach()

    # -- filter-out excluded micropython files if necessary
    if("${__build_variant}" STREQUAL "micropython")
        log_dbg("STEP >> exclude truncated micropython files" cyan)
        __get_global_attribute(MPY_EXCLUDE_FILES __mpy_excluded_files)
        set(__mpy_target __idf_micropython)
        get_target_property(__mpy_srcs ${__mpy_target} SOURCES)
        list(REMOVE_ITEM __mpy_srcs ${__mpy_excluded_files})
        set_target_properties(${__mpy_target} PROPERTIES
            SOURCES "${__mpy_srcs}")
    endif()

    # -- add private compile flags and definitions to every library
    log_dbg("STEP >> apply private flags to each library" cyan)
    __entity_find(__libs ENTITY_TYPE library)
    __entity_find(__sdk_libs ENTITY_TYPE sdklib)
    __entity_find(__usr_libs ENTITY_TYPE userlib)
    log_list(__sdk_libs)
    log_list(__usr_libs)
    __sdk_get_compile_options(__sdk_libs_cc_flags SDK_LIBS)
    __sdk_get_compile_options(__usr_libs_cc_flags USR_LIBS)
    log_list(__sdk_libs_cc_flags)
    log_list(__usr_libs_cc_flags)
    foreach(__lib ${__libs})
        __entity_get_attribute(${__lib} DEFINITIONS __defs)
        target_compile_definitions(__idf_${__lib} PRIVATE ${__defs})
        set(__flags)
        __entity_get_attribute(${__lib} CFLAGS __flags)
        if(${__lib} IN_LIST __sdk_libs)
            list(APPEND __flags ${__sdk_libs_cc_flags})
        endif()
        if(${__lib} IN_LIST __usr_libs)
            list(APPEND __flags ${__usr_libs_cc_flags})
        endif()

        log_var(__lib)
        get_target_property(__lib_type __idf_${__lib} TYPE)
        log_var(__lib_type)
        log_list(__flags)

        target_compile_options(__idf_${__lib} PRIVATE ${__flags})
    endforeach()

endfunction()

# ---------------------------------------------------------------------------- #
# main esp-idf build system binding procedure
# ---------------------------------------------------------------------------- #

# === environment related variables
__set_global_attribute(SDK_INCS ${CMAKE_BINARY_DIR} APPEND)
__esp_idf_env_vars_checker()

set(CMAKE_TOOLCHAIN_FILE
    $ENV{IDF_PATH}/tools/cmake/toolchain-${IDF_TARGET}.cmake)
if(NOT EXISTS ${CMAKE_TOOLCHAIN_FILE})
    log_msg("abort the toolchain file does not exists "
        "${__yellow__}${CMAKE_TOOLCHAIN_FILE}${__default__}"
        fatal_error)
endif()

# === ESP-IDF specific variables

# Directories to search for components.
#   Defaults to IDF_PATH/components, PROJECT_DIR/components,
#   and EXTRA_COMPONENT_DIRS.
# set(COMPONENT_DIRS)

# Optional list of additional directories to search for components
# set( EXTRA_COMPONENT_DIRS )


# A list of component names to build into the project.
#  § Defaults to all components found in the COMPONENT_DIRS directories.
#  § Use this variable to "trim down" the project for faster build times.
#  § Note that any component which "requires" another component via the REQUIRES
#    or PRIV_REQUIRES arguments on component registration will automatically 
#    have it added to this list, so the COMPONENTS list can be very short.
# set(COMPONENTS)


# A list of components, placed in bootloader_components/, that should be ignored
# by the bootloader compilation.
# set(BOOTLOADER_IGNORE_EXTRA_COMPONENT)

# specify the used and the default sdkconfig files
set(SDKCONFIG ${CMAKE_BINARY_DIR}/sdkconfig)
set(SDKCONFIG_DEFAULTS ${CMAKE_BINARY_DIR}/sdkconfig.combined )

# === set the main project name
__esp_idf_get_main_component(APP_NAME_VAR __prj_name)

# === start preparation processing
__esp_idf_process_preparation()

# === bind to the ESP-IDF build system to tackle the building process configs
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${__prj_name})

# === apply any extra finialization on top of the ESP-IDF preparation.
__esp_idf_process_finalization()

# --- end of file ------------------------------------------------------------ #
