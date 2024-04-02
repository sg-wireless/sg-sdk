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
# Desc      This file is responsible for preparing for micropython library build
#           and integrating the micropython building ecosystem.
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __micropython_update_generated_modules()
#
# description:
#       this function is responsible for managing the micropython c-modules
#       file generation.
# ---------------------------------------------------------------------------- #
function(__micropython_update_generated_modules)

    include(${__dir_src}/libs/mpy-al/gen/cmod_binding_gen.cmake)
    __entity_find(__libs_with_cmods MPY_CMODS "")

    set(__gen_dir ${CMAKE_BINARY_DIR}/mpy_cmod_gen_dir)
    if(NOT EXISTS ${__gen_dir})
        file( MAKE_DIRECTORY ${__gen_dir} )
    endif()

    foreach(__sdk_lib ${__libs_with_cmods})
        __entity_get_attribute(${__sdk_lib} MPY_CMODS __cmods LOG_OFF)
        set(__gen_files_var ${__sdk_lib}__cmod_generated_files)
        generate_mpy_cmod_binding_files(
            SRCS    ${__cmods}
            OUT_VAR ${__gen_files_var}
            GEN_DIR ${__gen_dir}
            GEN_LOGS_FILE ${__gen_dir}/__sdk_lib_${__sdk_lib}.log
        )
        log_list(${__sdk_lib}__cmod_generated_files)
        if(${__sdk_lib}__cmod_generated_files)
            __entity_set_attribute(${__sdk_lib}
                SOURCES ${${__gen_files_var}} APPEND)

            __entity_set_attribute(${__sdk_lib}
                MPY_GEN_FILES ${${__gen_files_var}} APPEND)
        endif()
    endforeach()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __micropython_update_usermod_list_files()
#
# description:
#       this function is responsible for generating cmake list files for
#       defining the usermod libraries for micropython QSTR generation process
#       purpose only.
# ---------------------------------------------------------------------------- #
function(__micropython_update_usermod_list_files)

    __entity_find(__libs_with_cmods MPY_CMODS "")

    set(__usermod_lists_dir ${CMAKE_BINARY_DIR}/mpy_usermod_lists_dir)
    if(NOT EXISTS ${__usermod_lists_dir})
        file( MAKE_DIRECTORY ${__usermod_lists_dir} )
    endif()

    foreach(__sdk_lib ${__libs_with_cmods})

        set(__lib_name usermod_${__sdk_lib})
        set(__list_file ${__usermod_lists_dir}/${__lib_name}.cmake)

        log_dbg("process updating usermod list file "
            "${__cyan__}${__list_file}${__default__}")

        __entity_get_attribute(${__sdk_lib} MPY_CMODS __cmods_files LOG_OFF)
        __entity_get_attribute(${__sdk_lib} MPY_GEN_FILES __gen_files LOG_OFF)

        # -- generate usermod cmake list file for qstr generation purpose
        set(__contents)
        set(__ts "    ")
        string(APPEND __contents "\n")
        string(APPEND __contents
            "add_library(${__lib_name} INTERFACE IMPORTED)\n")
        string(APPEND __contents "target_sources(${__lib_name} INTERFACE\n")
        foreach(__file ${__cmods_files} ${__gen_files})
            string(APPEND __contents  "${__ts}${__file}\n")
        endforeach()
        string(APPEND __contents  "${__ts})\n")

        __entity_get_attribute(${__sdk_lib} INCS_PRIV __incs_priv LOG_OFF)
        __get_global_attribute(SDK_INCS __incs_sdk)
        set(__incs ${__incs_priv} ${__incs_sdk})
        list(REMOVE_DUPLICATES __incs)
        string(APPEND __contents
            "target_include_directories(${__lib_name} INTERFACE\n")
        foreach(__inc_dir ${__incs})
            string(APPEND __contents  "${__ts}${__inc_dir}\n")
        endforeach()
        string(APPEND __contents  "${__ts})\n")

        string(APPEND __contents
            "target_link_libraries(usermod INTERFACE ${__lib_name})\n")

        __sdk_update_file_contents(${__list_file} ${__contents})
        __entity_set_attribute(${__sdk_lib}
            MPY_USERMOD_LIST_FILE ${__list_file})

    endforeach()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __micropython_update_esp_idf_list_file()
#
# description:
#       This function generates an ESP-IDF component cmake list file to build
#       the micropython project as a component library.
# ---------------------------------------------------------------------------- #
function(__micropython_update_esp_idf_list_file __list_file)
    log_dbg("-- start process micropython preparation --" yellow)

    # -- write micropython integration cmake list file
    set(__contents)
    set(__ts "    ")

    string(APPEND __contents "\n")
    string(APPEND __contents "# --- auto-generated cmake list file\n\n")
    string(APPEND __contents "include(${__dir_cmake}/log.cmake)\n\n")
    string(APPEND __contents
        "set(PROJECT_DIR ${__dir_micropython}/ports/esp32)\n\n")

    string(APPEND __contents
        "set(MICROPY_PORT_DIR    ${__dir_micropython}/ports/esp32 )\n")
    string(APPEND __contents
        "set(MICROPY_BOARD_DIR   ${__dir_platform} )\n")

    # USER_C_MODULES
    __entity_find(__usermod_libs MPY_USERMOD_LIST_FILE "")
    string(APPEND __contents "set(USER_C_MODULES\n")
    foreach(__lib ${__usermod_libs})
        __entity_get_attribute(${__lib} MPY_USERMOD_LIST_FILE __file)
        string(APPEND __contents "${__ts}${__file}\n")
    endforeach()
    string(APPEND __contents "${__ts})\n\n")

    # MICROPY_FROZEN_MANIFEST
    __sdk_get_micropython_frozen_manifest(__manifest_files)
    if(__manifest_files)
        string(APPEND __contents "set(MICROPY_FROZEN_MANIFEST\n")
        foreach(__file ${__manifest_files})
            string(APPEND __contents "${__ts}${__file}\n")
        endforeach()
        string(APPEND __contents "${__ts})\n\n")
    else()
        __sdk_add_compile_options("-DMICROPY_MODULE_FROZEN=0")
    endif()

    # MICROPY_CPP_FLAGS_EXTRA
    __get_global_attribute(COMPILE_FLAGS __cflags)
    __get_global_attribute(COMPILE_FLAGS_SDK __sdk_cc)
    string(APPEND __contents "set(MICROPY_CPP_FLAGS_EXTRA\n")
    foreach(__flag ${__cflags} ${__sdk_cc})
        string(APPEND __contents "${__ts}${__flag}\n")
    endforeach()
    string(APPEND __contents "${__ts})\n\n")

    string(APPEND __contents
        "include(${__dir_micropython}/ports/esp32/main/CMakeLists.txt)\n\n")

    # MICROPY_FROZEN_CONTENT dependencies
    __micropython_get_manifest_python_files(__manifest_python_files)
    string(APPEND __contents "if(NOT CMAKE_SCRIPT_MODE_FILE)\n")
    if(__manifest_python_files)

        string(APPEND __contents "add_custom_command(\n")
        string(APPEND __contents "${__ts}OUTPUT \${MICROPY_FROZEN_CONTENT}\n")
        string(APPEND __contents "${__ts}DEPENDS\n")
        foreach(__py_file ${__manifest_python_files})
            string(APPEND __contents "${__ts}${__ts}${__py_file}\n")
        endforeach()
        string(APPEND __contents "${__ts}APPEND\n${__ts})\n")
    endif()

    __entity_find(__libs_with_cmods MPY_CMODS "")
    set(__mpy_gen_files)
    string(APPEND __contents "    set(__cmod_gen_files\n")
    foreach(__lib ${__libs_with_cmods})
        __entity_get_attribute(${__lib} MPY_GEN_FILES __gen_files LOG_OFF)
        if(__gen_files)
            list(APPEND __mpy_gen_files ${__gen_files})
            foreach(__file ${__gen_files})
                string(APPEND __contents "        ${__file}\n")
            endforeach()
        endif()
    endforeach()
    string(APPEND __contents "    )\n")
    log_list(__mpy_gen_files)

    string(APPEND __contents
        "    set(__sdkconfig_file \${CMAKE_BINARY_DIR}/config/sdkconfig.h)\n"
        "    file(GLOB __module_files \n"
        "        \"\${CMAKE_BINARY_DIR}/genhdr/module/*\"\)\n"
        "    foreach(__file\n"
        "        \${MICROPY_QSTRDEFS_LAST}\n"
        "        \${CMAKE_BINARY_DIR}/frozen_content.c\n"
        "        \${__module_files}\n"
        "        )\n"
        "        if(EXISTS \${__file})\n"
        "            foreach(__updated_file\n"
        "                \${__sdkconfig_file} \${__cmod_gen_files})\n"
        "                if(\${__updated_file} IS_NEWER_THAN \${__file})\n"
        "                    file(REMOVE \${__file})\n"
        "                    break()\n"
        "                endif()\n"
        "            endforeach()\n"
        "        endif()\n"
        "    endforeach()\n"
    )

    string(APPEND __contents "endif()\n\n")

    string(APPEND __contents "log_var(MICROPY_TARGET)\n")
    string(APPEND __contents "log_var(MICROPY_MPVERSION)\n")
    string(APPEND __contents "log_var(MICROPY_QSTRDEFS_GENERATED)\n")
    string(APPEND __contents "log_var(MICROPY_MODULEDEFS)\n")
    string(APPEND __contents "log_var(MICROPY_ROOT_POINTERS)\n")
    string(APPEND __contents "log_var(MICROPY_FROZEN_CONTENT)\n")

    __sdk_update_file_contents(${__list_file} ${__contents})

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __micropython_get_manifest_python_files()
#
# description:
#       This function obtains the list of all contributing python files in the
#       frozen manifests.
#       It is needed to set a dependency level between frozen generated code
#       and its python files.
# ---------------------------------------------------------------------------- #
function(__micropython_get_manifest_python_files __output_list)
    __sdk_get_micropython_frozen_manifest( __manifest_files)

    set(__python_files)
    log_dbg("-- fetching manifest python files:")
    foreach(__manifest_file ${__manifest_files})
        log_dbg("> manifest ${__green__}${__manifest_file}${__default__}")
        execute_process(
            COMMAND ${Python3_EXECUTABLE}
                ${__dir_micropython}/tools/manifestfile.py
                --port  ${__dir_micropython}/ports/esp32
                --board ${__dir_platform}
                --top   ${__dir_micropython}
                --lib   ${__dir_micropython}/lib/micropython-lib
                --freeze
                ${__manifest_file}
            OUTPUT_VARIABLE __process_output
            RESULT_VARIABLE __process_result
            )
        if(${__process_result})
            log_msg("${__red__}error in fetching manifest python files:\n"
                "${__process_output}${__default__}" fatal_error)
        endif()

        string(REPLACE "\n" ";" __files "${__process_output}")

        foreach(item ${__files})
            string(REGEX REPLACE "^ManifestOutput[^']+'([^']*)'.*$" "\\1"
                __item ${item})
            list(APPEND __python_files ${__item})
        endforeach()

    endforeach()

    list(REMOVE_DUPLICATES __python_files)

    # log_list(__python_files)

    set(${__output_list} ${__python_files} PARENT_SCOPE)

endfunction()

# --- end of file ------------------------------------------------------------ #
