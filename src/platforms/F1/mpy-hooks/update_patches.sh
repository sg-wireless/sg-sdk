#!/bin/bash
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
# Desc      This file is responsible for updating the patches for the uPython
# ---------------------------------------------------------------------------- #

__root_dir=../../../..
__modified_dir=./modified_sources
__patch_dir=./patches
__main_update_script=${__root_dir}/tools/builder/cmake/update_patches.sh

__original_dir=${__root_dir}/ext/micropython/ports/esp32
__search_path="uart.c"
${__main_update_script} ${__original_dir} ${__modified_dir} ${__patch_dir} \
    ${__search_path}

__original_dir=${__root_dir}/ext/micropython/ports/esp32
__search_path="help.c"
${__main_update_script} ${__original_dir} ${__modified_dir} ${__patch_dir} \
    ${__search_path}

__original_dir=${__root_dir}/ext/micropython/ports/esp32
__search_path="machine_i2c.c"
${__main_update_script} ${__original_dir} ${__modified_dir} ${__patch_dir} \
    ${__search_path}

# --- end of file ------------------------------------------------------------ #
