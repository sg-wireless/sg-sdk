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
# Desc      This file is responsible for generating the binding micropython
#           suitable files to work with the micropython module registeration
#           system.
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# # synopsis:
#       generate_mpy_cmod_binding_files(
#                     SRCS <file> [<file> ...]
#                   [ GEN_DIR <gen-files-output-dir> ]
#                   [ OUT_VAR <output-variable> ]
#                   [ GEN_LOGS_FILE <generator-script-logs-file> ]
#       )
#
#       SRCS :  the source files that will be investigated to pickup the uPython
#               c-module files that are built using mp-lite-if methodology.
#       GEN_DIR : an optional directory location to specify where the generated
#               files are needed to be stored
#               if not specified, this default location will be used:
#                   ${CMAKE_BINARY_DIR}/cmod_gen_dir
#       OUT_VAR : to specify a variable name to receive the generated files
#       GEN_LOGS_FILE : an optional file to dump the generator output logs
# ---------------------------------------------------------------------------- #
function(generate_mpy_cmod_binding_files)
    set(options)
    set(oneValueArgs
        GEN_DIR
        OUT_VAR
        GEN_LOGS_FILE)
    set(multiValueArgs
        SRCS)
    cmake_parse_arguments( _
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # -- generation directory
    if(NOT DEFINED __GEN_DIR)
        set( __cmod_gen_dir ${CMAKE_BINARY_DIR}/cmod_gen_dir)
    else()
        set( __cmod_gen_dir ${__GEN_DIR} )
    endif()

    # -- temporary generation dir
    set(__temp_dir ${__cmod_gen_dir}/temp)

    # -- source files containing c-modules definitions
    file(GLOB_RECURSE __cmods_srcs ${__SRCS})
    if(NOT DEFINED __SRCS)
        log_msg("-- missed SRCS files while calling uPython module generator"
                warning)
        return()
    endif()
    
    # -- create the gen dir if not exist before calling the generation script
    if(NOT EXISTS ${__cmod_gen_dir})
        file( MAKE_DIRECTORY ${__cmod_gen_dir} )
    endif()

    if(EXISTS ${__temp_dir})
        file(GLOB __old_files "${__temp_dir}/*.c")
        if(__old_files)
            file(REMOVE ${__old_files})
        endif()
    else()
        file( MAKE_DIRECTORY ${__temp_dir} )
    endif()

    set(__python_gen_script ${__dir_src}/libs/mpy-al/gen/gen_mp_cmods.py )

    execute_process(
        COMMAND ${Python3_EXECUTABLE} ${__python_gen_script} generate 
                ${__temp_dir} ${__cmods_srcs}
        OUTPUT_VARIABLE __script_output
        RESULT_VARIABLE __script_result
    )

    if(DEFINED __GEN_LOGS_FILE)
        file(WRITE ${__GEN_LOGS_FILE}
            "[ __script_result ] ${__script_result}\n\n"
            "[ __script_output ]\n\n"
            "${__script_output}")
    endif()

    if(${__script_result} )
        log_msg("${__red__}micropython c-modules generation error:\n"
            "${__script_output}${__default__}" fatal_error)
        return()
    endif()

    file(GLOB __generated_files "${__temp_dir}/*.c")

    set(__final_files)
    foreach(__gen_file ${__generated_files})
        set(__log_msg
            "-- process cmod gen file ${__cyan__}${__gen_file}${__default__}")
        if(__log_show_mpy_cmod_files)
            log_msg(${__log_msg})
        else()
            log_dbg(${__log_msg})
        endif()
        get_filename_component(__filename ${__gen_file} NAME)
        set(__real_file ${__cmod_gen_dir}/${__filename})
        file(COPY_FILE ${__gen_file} ${__real_file} ONLY_IF_DIFFERENT)
        list(APPEND __final_files ${__real_file})
    endforeach()

    if(DEFINED __OUT_VAR)
        set(${__OUT_VAR} ${__final_files} PARENT_SCOPE)
    endif()

endfunction()

# -- end of file ------------------------------------------------------------- #
