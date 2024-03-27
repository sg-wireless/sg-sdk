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
# Desc      This file is responsible for cmake logging functions used in build
#           files.
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# debug switches
# ---------------------------------------------------------------------------- #
set(__sdk_cmake_log_file ${CMAKE_BINARY_DIR}/sdk_cmake_log_msg.txt)

if(CMAKE_DEBUG)
    set(__log_debug_micropython ON)
    set(__log_debug_esp_idf     ON)
    set(__log_debug_entity      ON)
    set(__log_debug_utils       ON)
    set(__log_debug_sdk         ON)
    set(__log_debug_global      ON)
else()
    set(__log_debug_micropython OFF)
    set(__log_debug_esp_idf     OFF)
    set(__log_debug_entity      OFF)
    set(__log_debug_utils       OFF)
    set(__log_debug_sdk         OFF)
    set(__log_debug_global      OFF)
endif()

unset(CMAKE_DEBUG CACHE)

set(__log_show_patched_files    OFF)
set(__log_show_mpy_cmod_files   OFF)
set(__log_show_log_gen_files    OFF)
set(__log_show_ver_gen_files    OFF)

# ---------------------------------------------------------------------------- #
# coloring variables
# ---------------------------------------------------------------------------- #
string(ASCII 27 __Esc__)
set(__default__ "${__Esc__}[m")
set(__bold__    "${__Esc__}[1m")
set(__red__     "${__Esc__}[31m")
set(__green__   "${__Esc__}[32m")
set(__yellow__  "${__Esc__}[33m")
set(__blue__    "${__Esc__}[34m")
set(__magenta__ "${__Esc__}[35m")
set(__purple__  "${__Esc__}[35m")
set(__cyan__    "${__Esc__}[36m")
set(__white__   "${__Esc__}[37m")

# ---------------------------------------------------------------------------- #
# file logging initialization
# ---------------------------------------------------------------------------- #
function(log_msg_init)
    file(WRITE ${__sdk_cmake_log_file} "log_msg -- init()\n")
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     log_msg( <msg> [<mode>] [<color>] )
# 
# description:  display a cmake log message
# 
# arguments:
#   <msg>       the message to be logged
# 
#   [<mode>]    optional message mode; modes are
#                   fatal_error:    the cmake will stop after it
#                   warning:        the cmake will continue after it
#
#   [<color>]   optional message color; supported colors are:
#                   red, green, yellow, blue, purple, cyan, white
# 
# ---------------------------------------------------------------------------- #
function( log_msg )

    set( options fatal_error warning
        red green yellow blue purple cyan white
        file_only
        )

    set(__arg_idx 0)
    foreach(__opt ${options})
        set(__opt_${__opt} OFF)
    endforeach()
    set(__msg "")
    while(${__arg_idx} LESS ${ARGC})
        set(__arg ${ARGV${__arg_idx}})
        set(__is_opt OFF)
        foreach(__opt ${options})
            if("${__arg}" STREQUAL "${__opt}")
                set(__opt_${__opt} ON)
                set(__is_opt ON)
                break()
            endif()
        endforeach()

        if(NOT ${__is_opt})
            string(APPEND __msg "${__arg}")
        endif()

        math(EXPR __arg_idx "${__arg_idx} + 1")
    endwhile()

    string(ASCII 27 __esc)
    string( APPEND __color "${__esc}")
    set(__msg_mode)
    
    if( __opt_fatal_error )
        set( __color "${__esc}[31m")
        set(__msg_mode FATAL_ERROR)
    elseif( __opt_warning )
        set( __color "${__esc}[33m")
        set(__msg_mode WARNING)
    else()
        string(APPEND __color "[m")
    endif()

    if( __opt_red )
        set( __color "${__esc}[31m")
    elseif( __opt_green )
        set( __color "${__esc}[32m")
    elseif( __opt_yellow )
        set( __color "${__esc}[33m")
    elseif( __opt_blue )
        set( __color "${__esc}[34m")
    elseif( __opt_purple )
        set( __color "${__esc}[35m")
    elseif( __opt_cyan )
        set( __color "${__esc}[36m")
    elseif( __opt_white )
        set( __color "${__esc}[37m")
    endif()

    string(REGEX REPLACE "${__esc}\\[[0-9]*m" "" __m_file ${__msg})
    file(APPEND ${__sdk_cmake_log_file} "${__m_file}\n")
    if(${__opt_file_only})
        return()
    endif()

    message( ${__msg_mode} "${__color}${__msg}${__esc}[m" )

endfunction()

macro(log_list __list_var)
    block()
        log_dbg("list:: ${__yellow__}${__list_var}${__default__}")
        set(__idx 1)
        foreach(__list_item ${${__list_var}})
            log_dbg("    [${__idx}] ${__list_item}")
            math(EXPR __idx "${__idx} + 1")
        endforeach()
    endblock()
endmacro()

macro(log_var __var_name)
    log_dbg("var:: ${__yellow__}${__var_name}${__default__} "
        "value:: ${__cyan__}${${__var_name}}${__default__}")
endmacro()

function(log_stop)
    log_msg("-- enforced build stop --" fatal_error)
endfunction()

macro(log_all_vars)
    get_cmake_property(__all_vars VARIABLES)
    log_list(__all_vars)
endmacro()

macro(log_dbg)
    block()
    if(CMAKE_CURRENT_FUNCTION_LIST_FILE)
        set(__filename ${CMAKE_CURRENT_FUNCTION_LIST_FILE})
    else()
        set(__filename ${CMAKE_CURRENT_LIST_FILE})
    endif()
    get_filename_component(fun_file ${__filename} NAME)
    string(REPLACE ".cmake" "" __file ${fun_file})
    string(REPLACE "\-" "_" __file ${__file})
    set(__dbg_switch __log_debug_${__file})

    if( NOT (
        (DEFINED ${__dbg_switch} AND ${${__dbg_switch}}) OR
        ((NOT ${__dbg_switch}) AND ${__log_debug_global}) ))
        set(__file_only file_only)
    endif()

    set(__log_tag "[${__file}::${CMAKE_CURRENT_FUNCTION}] ")

    set(__log_list "[${__file}::${CMAKE_CURRENT_FUNCTION}] " ${__file_only})

    if(${ARGC} GREATER 10)
        log_msg(${__file_only} ${__log_tag} ${ARGVN})
    elseif(${ARGC} EQUAL 10)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6}
            ${ARGV7} ${ARGV8} ${ARGV9})
    elseif(${ARGC} EQUAL 9)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6}
            ${ARGV7} ${ARGV8})
    elseif(${ARGC} EQUAL 8)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6}
            ${ARGV7})
    elseif(${ARGC} EQUAL 7)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6})
    elseif(${ARGC} EQUAL 6)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5})
    elseif(${ARGC} EQUAL 5)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3} ${ARGV4})
    elseif(${ARGC} EQUAL 4)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2} ${ARGV3})
    elseif(${ARGC} EQUAL 3)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1} ${ARGV2})
    elseif(${ARGC} EQUAL 2)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0} ${ARGV1})
    elseif(${ARGC} EQUAL 1)
        log_msg(${__file_only} ${__log_tag}
            ${ARGV0})
    else()
        log_msg(${__file_only} ${__log_tag})
    endif()
    endblock()
endmacro()

# --- end of file ------------------------------------------------------------ #
