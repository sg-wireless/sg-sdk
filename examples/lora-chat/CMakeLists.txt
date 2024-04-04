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
# Desc      It supports example build.
# ---------------------------------------------------------------------------- #

__sdk_add_component( lora_c_example

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    LOGS_DEFS
        "${CMAKE_CURRENT_LIST_DIR}/main.c"

    INCS_PRIV
        ${__dir_ext}/esp-idf/components/log/include
        ${__dir_ext}/esp-idf/components/spi_flash/include
        ${__dir_ext}/esp-idf/components/nvs_flash/include

    REQUIRED_ESP_LIBS
        freertos
        spi_flash
    
    REQUIRED_SDK_LIBS
        log_lib
        f1_logs_if
        lora_stack
        lora_if
)

# --- end of file ------------------------------------------------------------ #
