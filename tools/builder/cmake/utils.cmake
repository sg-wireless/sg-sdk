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
# Desc      This file contains some cmake helpful functions.
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_update_file_contents( <file> <new_contents> )
#
# description:
#       This function updates the file with the newer contents if the file does
#       not exist or it is different than the existing contents.
# ---------------------------------------------------------------------------- #
function(__sdk_update_file_contents __file __contents)
    if(EXISTS ${__file})
        file(READ ${__file} __prev_contents)
    endif()

    if(NOT "${__prev_contents}" STREQUAL "${__contents}")
        log_msg("-- update ${__file}" green)
        file(WRITE ${__file} ${__contents})
    else()
        log_dbg("file no change :: ${__green__}${__file}${__default__}")
    endif()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_concatenate_files(
#                           <output_file>
#                           <input-file-1> [ <input-file-2> ... ]
#       )
#
# description:
#       This function concatenates all given input files into a single output
#       file if at least one of the input files is newer than the output file.
# ---------------------------------------------------------------------------- #
function(__sdk_concatenate_files output_file )

    if(NOT ARGN)
        log_msg("error: no given files to concatenate" fatal_error)
    endif()

    set(__do_update OFF)

    foreach(__file ${ARGN})
        if(${__file} IS_NEWER_THAN ${output_file})
            set(__do_update ON)
            break()
        endif()
    endforeach()

    if(__do_update)
        log_msg("-- update ${__cyan__}${output_file}${__default__}")
        if(EXISTS ${output_file})
            file(REMOVE ${output_file})
        endif()
        foreach(__file ${ARGN})
            file(READ ${__file} __contents)
            file(APPEND ${output_file} ${__contents})
        endforeach()
    else()
        log_dbg("-- already updated: ${__green__}${output_file}${__default__}")
    endif()

endfunction()

# --- end of file ------------------------------------------------------------ #
