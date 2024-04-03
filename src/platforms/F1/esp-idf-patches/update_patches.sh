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
# Desc      This file is responsible for updating the patches for the esp-idf
# ---------------------------------------------------------------------------- #

__root_dir=../../../..
__main_update_script=${__root_dir}/tools/builder/cmake/update_patches.sh

for d in *; do
    if [ -d "$d" ]; then
        __modified_dir=./$d/modified_sources
        if [ "$d" = "bootloader_main" ]; then
            __original_dir=${__root_dir}/ext/esp-idf/components/bootloader
        else
            __original_dir=${__root_dir}/ext/esp-idf/components/$d
        fi
        __patch_dir=./$d/patches
        ${__main_update_script} \
            ${__original_dir}   \
            ${__modified_dir} \
            ${__patch_dir}
    fi
done

# --- end of file ------------------------------------------------------------ #
