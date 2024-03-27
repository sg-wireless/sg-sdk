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
# ---------------------------------------------------------------------------- #
# Copyright (c) 2022, Pycom Limited.
#
# This software is licensed under the GNU GPL version 3 or any
# later version, with permitted additional terms. For more information
# see the Pycom Licence v1.0 document supplied with this file, or
# available at https://www.pycom.io/opensource/licensing
#
# Author    Ahmed Sabry (Pycom)
#
# Desc      build the unit test files of the utilities lib
# ---------------------------------------------------------------------------- #

.PHONY: all createdir clean

build_dir := build-unittest

exec := $(build_dir)/unittest
x86_as := $(build_dir)/assembly_x86.s
arm_as := $(build_dir)/assembly_arm.s
src := unittest_main.c
inc := -I../
arm_cc := /Applications/ARM/bin/arm-none-eabi-gcc

obj := $(patsubst %.c,$(build_dir)/%.o,$(src))
dep := $(patsubst %.c,$(build_dir)/%.d,$(src))

vpath *.c .

all: createdir $(exec) $(x86_as) $(arm_as)

createdir:
	@mkdir -p $(build_dir) 

$(exec): $(obj)
	gcc -o $@ $^

$(build_dir)/%.o: %.c
	gcc -MD -o $@ -c $< $(inc)

$(x86_as) : $(src) $(exec)
	gcc $< -S $(inc) -o $@

$(arm_as) : $(src) $(exec)
	$(arm_cc) $< -S -o $@ $(inc)

clean:
	rm -rf $(build_dir)

-include $(dep)

# --- end of file ------------------------------------------------------------ #
