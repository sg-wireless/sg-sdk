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
# Desc      This file responsible for supporting the generation of build
#           version header file
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __run_build_version_generator(
#           GEN_DIR         <generation-dir>
#           GEN_FILE        <generated-file>
#           CUSTOM_STRING   <custom-string>
#       )
#
# description:
#       This function derives the generation of the build version header.
# ---------------------------------------------------------------------------- #
function(__run_build_version_generator)

    set( options )
    set( oneValueArgs GEN_DIR GEN_FILE CUSTOM_STRING )
    set( multiValueArgs )
    cmake_parse_arguments( _
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    if(NOT DEFINED __GEN_DIR)
        log_msg("GEN_DIR must be specified" fatal_error)
    endif()

    if(NOT DEFINED __GEN_FILE)
        log_msg("GEN_FILE must be specified" fatal_error)
    endif()

    log_dbg("=== process build version header generation .." yellow)

    set(__log_generator_script ${__dir_sdk_comps}/fw-version/fw_version.py)
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

    if(DEFINED __CUSTOM_STRING)
        set(__custom_str_opt "--custom-string \"${__CUSTOM_STRING}\"")
    endif()

    set(__temp_file ${__temp_dir}/${__GEN_FILE})

    execute_process(
        COMMAND ${Python3_EXECUTABLE}
            ${__log_generator_script} generate-header
            --header-file ${__temp_file}
            ${__custom_str_opt}
        OUTPUT_VARIABLE __script_output
        RESULT_VARIABLE __script_result
    )
    if(${__script_result} )
        log_msg("${__red__}build version header generation error:\n"
            "${__script_output}${__default__}" fatal_error)
        return()
    endif()

    set(__gen_file ${__GEN_DIR}/${__GEN_FILE})
    set(__log_msg
        "-- process version gen file ${__cyan__}${__gen_file}${__default__}")
    if(__log_show_ver_gen_files)
        log_msg(${__log_msg})
    else()
        log_dbg(${__log_msg})
    endif()

    file(COPY_FILE ${__temp_file} ${__gen_file} ONLY_IF_DIFFERENT)

endfunction()

# --- end of file ------------------------------------------------------------ #
