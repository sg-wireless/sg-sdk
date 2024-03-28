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
# Desc		This file contains the test build for the logs library
# ---------------------------------------------------------------------------- #

# --- targets ---------------------------------------------------------------- #
input_targets   := clean build generate preprocessed help test
default_targets := build test

# --- tweaking variables ----------------------------------------------------- #
build_dir := build
gen_dir   := ${build_dir}/gen

# --- build source files ----------------------------------------------------- #
common_dir := ../..
src_dirs := ../src ./
srcs := $(notdir $(foreach dir,${src_dirs},$(wildcard ${dir}/*.c))) \
		$(notdir ${common_dir}/utils/utils_fs_path.c)              \
		$(notdir ${common_dir}/utils/utils_bitarray.c)
gens := ${gen_dir}/logs_gen_comp_ids.hh \
        ${gen_dir}/logs_gen_structs.cc
gen_srcs := $(foreach dir,${src_dirs},$(wildcard ${dir}/*.c)) ../inc/log_lib.h

# --- build artifacts files -------------------------------------------------- #
objs := $(addprefix ${build_dir}/obj/,$(srcs:.c=.o))
deps := $(objs:.o=.d)
proc := $(objs:.o=.i)
bin  := ${build_dir}/a.out

# --- build flags and search paths ------------------------------------------- #
incs :=                 \
    ../src              \
    ../inc              \
    ${common_dir}/utils \
    ${gen_dir}

cflags := $(addprefix -I,${incs})

vpath %.c ../src ./ ${common_dir}/utils
vpath %.h ../src ../inc

# --- build driving rules ---------------------------------------------------- #
.PHONY: default createdirs ${input_targets}

default: ${default_targets}

clean:
	@echo "-- cleaning ..."
	rm -rf ${build_dir}
build: createdirs ${gens} ${bin}
generate: createdirs ${gens}
preprocessed: createdirs ${proc}
help:
test: build
	./${bin}

createdirs:
	@mkdir -p ${build_dir}/obj
	@mkdir -p ${gen_dir}

${bin}: ${objs}
	gcc -o $@ $^

${build_dir}/obj/%.i: %.c
	gcc -E $< -o ${@:.o=.i} ${cflags}

${build_dir}/obj/%.o: %.c
	gcc -c $< -o $@ -MD ${cflags}

${gens}: ${gen_srcs}
	python3 ../gen/gen_logs_structs.py ${gen_dir} ${gen_srcs}

# --- dependencies inclusion ------------------------------------------------- #
-include ${deps}

# --- end of file ------------------------------------------------------------ #
