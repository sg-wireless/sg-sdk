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
# Desc      This file responsible for supporting the generation driving of
#           the log_lib meta information
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __run_logs_defs_generator(
#           GEN_DIR <generation-dir>
#           SOURCES <file> [ <file> ... ]
#       )
#
# description:
#       This function derives the generation of the logs definitions meta
#       structures during the build process.
# ---------------------------------------------------------------------------- #
function(__run_logs_defs_generator)

    set( options )
    set( oneValueArgs GEN_DIR )
    set( multiValueArgs SOURCES )
    cmake_parse_arguments( _
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT DEFINED __GEN_DIR)
        log_msg("GEN_DIR must be specified" fatal_error)
    endif()

    if(NOT DEFINED __SOURCES)
        log_msg("No Specified sources given to look for logs defs" fatal_error)
    endif()

    log_dbg("=== process log_lib generated files .." yellow)

    set(__log_generator_script ${__dir_src}/libs/logs/gen/gen_logs_structs.py)
    if(NOT EXISTS ${__GEN_DIR})
        file( MAKE_DIRECTORY ${__GEN_DIR} )
    endif()

    set(__temp_dir ${__GEN_DIR}/temp)
    if(EXISTS ${__temp_dir})
        file(GLOB __old_files "${__temp_dir}/*")
        if(__old_files)
            file(REMOVE ${__old_files})
        endif()
    else()
        file( MAKE_DIRECTORY ${__temp_dir} )
    endif()

    execute_process(
        COMMAND ${Python3_EXECUTABLE}
            ${__log_generator_script} ${__temp_dir} ${__SOURCES}
        OUTPUT_VARIABLE __script_output
        RESULT_VARIABLE __script_result
    )
    if(${__script_result} )
        log_msg("${__red__}logs meta-files generation error:\n"
            "${__script_output}${__default__}" fatal_error)
        return()
    endif()

    file(GLOB __generated_files "${__temp_dir}/*")
    foreach(__gen_file ${__generated_files})
        set(__log_msg
            "-- process log gen file ${__cyan__}${__gen_file}${__default__}")
        if(__log_show_log_gen_files)
            log_msg(${__log_msg})
        else()
            log_dbg(${__log_msg})
        endif()

        get_filename_component(__filename ${__gen_file} NAME)
        set(__real_file ${__GEN_DIR}/${__filename})
        file(COPY_FILE ${__gen_file} ${__real_file} ONLY_IF_DIFFERENT)
    endforeach()

endfunction()

# --- end of file ------------------------------------------------------------ #
