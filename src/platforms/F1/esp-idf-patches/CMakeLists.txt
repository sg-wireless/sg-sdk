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
# Desc      applies the required esp-idf components patches
# ---------------------------------------------------------------------------- #

file(GLOB __sub_dirs "${CMAKE_CURRENT_LIST_DIR}/*")

foreach(__dir ${__sub_dirs})

    if(IS_DIRECTORY ${__dir})
        get_filename_component(__comp ${__dir} NAME)
        if("${__dir}" STREQUAL "${CMAKE_CURRENT_LIST_DIR}/bootloader_main")
            if(NOT BOOTLOADER_BUILD)
                continue()
            endif()
            set(__comp main)
        endif()

        __sdk_add_patch( ESP_IDF
            ENTITY_NAME     ${__comp}
            PATCHES_DIRS    ${__dir}/patches
            FINAL_DIR       ${__dir}/modified_sources
        )
    endif()

endforeach()

unset(__sub_dirs)

# --- end of file ------------------------------------------------------------ #
