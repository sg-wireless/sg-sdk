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
# Desc      This file is responsible for updating the patches for the modified
#           source files.
# ---------------------------------------------------------------------------- #

# syntax:
#   update_patches.sh  <original-dir>  <modified-dir>  <patch-dir>
#
__original_dir=$1
__modified_dir=$2
__patch_dir=$3
if [ -z $4 ]; then
  __search_mask="*.c"
else
  __search_mask=$4
fi

if [[ ! -d ${__original_dir} ]];then
    echo " -- error: non-existing original folder '${__original_dir}'"
    exit 1;
fi
if [[ ! -d ${__modified_dir} ]];then
    echo " -- error: non-existing modified folder '${__modified_dir}'"
    exit 1;
fi
if [[ -z ${__patch_dir} ]];then
    echo " -- error: no provided patch dir '${__patch_dir}'"
    exit 1;
fi

echo "__original_dir = ${__original_dir}"
echo "__modified_dir = ${__modified_dir}"
echo "__patch_dir    = ${__patch_dir}"

if [[ ! -d ${__patch_dir} ]];then
    mkdir -p ${__patch_dir}
fi

__modified_files=($(find ${__modified_dir} -name ${__search_mask}))

for i in ${!__modified_files[@]}; do
    __modified_file=${__modified_files[$i]}
    __filename=${__modified_file##*/}
    __original_file=$(find ${__original_dir} -name "${__filename}")
    echo "@@@ original file: ${__original_file}"
    if [[ ! -f ${__original_file} ]]; then
        echo "    -- file not present"
        echo "    -- patching skipped!"
    else
        __patch_file=${__patch_dir}/${__filename}.patch
        echo "    --> patch_file    >>> ${__patch_file}"
        echo "    --> modified_file >>> ${__modified_file}"
        echo "    --> original_file >>> ${__original_file}"
        __diff_output=$(diff -ub ${__original_file} ${__modified_file})
        if [[ ! -z ${__diff_output} ]]; then
            echo "${__diff_output}" > ${__patch_file}
            echo "    --> patching done!"
        else
            echo "    --> files are identical"
            if [[ -f ${__patch_file} ]]; then
                echo "    --> previous patch file exist clearing it"
                echo "" > ${__patch_file}
            else
                echo "    --> patching skipped!"
            fi
        fi
    fi
done

# --- end of file ------------------------------------------------------------ #
