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
# Desc      This file extracts all micropython modules whish are developped
#           using the methodology descriped in common/utils/mp_lite_if.h file
#           and generate the required binding files for micropython echo
#           system
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# Input Targets:                                                               #
# ==============                                                               #
# This makefile handle the following input targets                             #
# A) gen_mp_mods_binding_files                                                 #
#    to generate a file per each found micropython module in the given source  #
#    files at the given generation path                                        #
#    the user shall provide the following command line variables along with    #
#    this input target GEN_DIR and SRC_FILES and the syntax will be as:        #
#    make -f <path/to/this/makefile>  gen_mp_mods_binding_files                #
#        GEN_DIR=<path/to/generation/dir>                                      #
#        SRC_FILES="<list of source files to search for the modules>"          #
#                                                                              #
# B) gen_reg_file                                                              #
#    to generate a dummy source file through which the micropython script      #
#    makemoduledefs.py can detect the modules and bind them to the micropython #
#    echo system                                                               #
#    the user shall provide the following command line variables along with    #
#    this input target REG_FILE and SRC_FILES and the syntax will be as:       #
#    make -f <path/to/this/makefile>  gen_reg_file                             #
#        REG_FILE=<path/to/registration/filename>                              #
#        SRC_FILES="<list of source files to search for the modules>"          #
# ---------------------------------------------------------------------------- #
__goal__gen_binding_files := gen_mp_mods_binding_files
__goal__gen_registration_file := gen_reg_file

__gen_dir := $(GEN_DIR)
__input_files := $(SRC_FILES)
__reg_file := $(REG_FILE)

# ---------------------------------------------------------------------------- #
# error checking                                                               #
# ---------------------------------------------------------------------------- #
__makefile_name := $(firstword $(MAKEFILE_LIST))

ifeq ($(SRC_FILES),)
	$(error "-- $(__makefile_name) -- no given source files")
endif

ifeq ($(MAKECMDGOALS),$(__goal__gen_binding_files))
    ifeq ($(GEN_DIR),)
        $(error "-- $(__makefile_name) -- no given gen file")
    endif
else
    ifeq ($(MAKECMDGOALS),$(__goal__gen_registration_file))
        ifeq ($(REG_FILE),)
            $(error "-- $(__makefile_name) -- error no specified REG_FILE")
        endif
    else
        $(error "-- $(__makefile_name) -- wrong or not specified make target")
    endif
endif

# ---------------------------------------------------------------------------- #
# module extraction regular expressions                                        #
#   the aim is to extract all of the module information from the provided lite #
#   interface which exist in this file "mp_lite_if.h"                          #
#                                                                              #
#   for module registration, the registration will be done for the modules     #
#   which added this line after the module method definitions                  #
#       #include __mp_mod_binding_file( module_x )                             #
# ---------------------------------------------------------------------------- #
__mod_fun_regex := \
    __mp_mod_fun_([0-3]|var|var_between|kw)\(\s*(\w+)\s*(,\s*(\w+)\s*)+\)
__mod_init_regex := __mp_mod_init\(\s*\w+\s*\)
__mod_name_regex := __mp_mod_name\(\s*\w+\s*,\s*\w*\s*\)
__mod_const_regex := __mp_mod_const\(\s*\w+\s*,\s*\w+\s*,\s*\w+\s*\)

__mod_collections_regex := $(__mod_fun_regex)|$(__mod_init_regex)
__mod_collections_regex := $(__mod_collections_regex)|$(__mod_name_regex)
__mod_collections_regex := $(__mod_collections_regex)|$(__mod_const_regex)

__mod_class_method_regex := \
  __mp_mod_class_method_([0-3]|var|var_between|kw)\(\s*(\w+)\s*(,\s*(\w+)\s*)+\)
__mod_class_print_new_regex := __mp_mod_class_(print|new)\(\s*\w+\s*,\s*\w*\s*\)
__mod_class_regex := $(__mod_class_method_regex)|$(__mod_class_print_new_regex)

__mod_collections_regex := "$(__mod_collections_regex)|$(__mod_class_regex)"

__regex_mp_binding_statment := "^.include\s+__mp_mod_binding_file\(\s*\w+\s*\)"

# ---------------------------------------------------------------------------- #
# Extraction and generation rules                                              #
# ---------------------------------------------------------------------------- #
.PHONY: \
    $(__goal__gen_binding_files) \
    $(__goal__gen_registration_file)

ifeq ($(MAKECMDGOALS),$(__goal__gen_binding_files))

__str_comma  := ,
__str_esc_lp := \(
__str_esc_rp := \)
__str_lp     := (
__str_rp     := )

__main_list :=                                                              \
    $(shell grep -ohE $(__mod_collections_regex) $(__input_files)           \
        | sed 's/ //g')

__mod_list :=                                                               \
    $(sort $(shell  echo "$(__main_list)"                                   \
        | grep -oh -E "\(\s*[_a-zA-Z0-9]+\s*?[,\)]"                         \
        | sed -e 's/[,()]//g'))

$(info -- $(__mod_list))

__mod_fun_pair_list :=                                                      \
    $(shell echo "$(__main_list)"                                           \
        | grep -ohE "$(__mod_fun_regex)"                                    \
        | sed   -e 's/^[_a-zA-Z0-9]*//' -e 's/[\(\)]//g'                    \
                -e 's/^[_a-zA-Z0-9]*,[_a-zA-Z0-9]*/&--/'                    \
                -e 's/--.*$$//' -e 's/,/-/')

__get_mod_funs = $(patsubst $(1)-%,%,$(filter $(1)-%, $(__mod_fun_pair_list)))

__is_exist_mod_init = \
    $(filter __mp_mod_init$(__str_lp)$(1)$(__str_rp),$(__main_list))

__get_mod_name =                                                            \
    $(patsubst __mp_mod_name$(__str_lp)$(1)$(__str_comma)%$(__str_rp),%,    \
    $(filter __mp_mod_name$(__str_lp)$(1)$(__str_comma)%,$(__main_list)))

__mod_const_val_list :=                                                     \
    $(shell echo "$(__main_list)"                                           \
    | grep -ohE "__mp_mod_const\(.+?\)"                                     \
    | sed -e 's/^[_a-zA-Z0-9]*//' -e 's/[\(\)]//g' -e 's/,/-/g')

__mod_const_list :=                                                         \
    $(shell echo "$(__mod_const_val_list)"                                  \
        | grep -ohE "[_a-zA-Z0-9]+\-[_a-zA-Z0-9]+")

__get_mod_consts = $(patsubst $(1)-%,%,$(filter $(1)-%,$(__mod_const_list)))

__get_mod_const_val =                                                       \
    $(patsubst $(1)-$(2)-%,%,$(filter $(1)-$(2)-%,$(__mod_const_val_list)))

__mod_class_list :=                                                         \
    $(sort $(shell echo "$(__main_list)"                                    \
    | grep -ohE "__mp_mod_class_\w+$(__str_esc_lp)[_a-zA-Z0-9]+,[_a-zA-Z0-9]+"\
    | sed -e 's/^[_a-zA-Z0-9]*$(__str_lp)//' -e 's/,/-/'))

__get_mod_classes = $(patsubst $(1)-%,%,$(filter $(1)-%,$(__mod_class_list)))

__mod_class_method_list :=                                                  \
    $(shell echo "$(__main_list)"                                           \
        | grep -ohE "$(__mod_class_method_regex)"                           \
        | sed   -e 's/^[_a-zA-Z0-9]*//' -e 's/[\(\)]//g'                    \
                -e 's/^[_a-zA-Z0-9]*,[_a-zA-Z0-9]*,[_a-zA-Z0-9]*/&--/'      \
                -e 's/--.*$$//' -e 's/,/-/g')

__get_mod_class_methods =                                                   \
    $(patsubst $(1)-$(2)-%,%,$(filter $(1)-$(2)-%,$(__mod_class_method_list)))

__is_exist_class_new = $(filter __mp_mod_class_new($(1),$(2)), $(__main_list))

__is_exist_class_print =                                                    \
    $(filter __mp_mod_class_print($(1),$(2)), $(__main_list))

$(__goal__gen_binding_files):
	@echo "generating modules definition files"
	$(foreach mod, $(__mod_list),$(shell echo                                  \
        "/* auto generated file */\n\n"                                        \
        "/* module classes list */\n"                                          \
        $(foreach class,$(call __get_mod_classes,$(mod)),                      \
            "\n/* class '$(class)' definitions */\n"                           \
            "__mp_mod_class_dict_table($(mod), $(class)) {\n"                  \
                "  __mp_dict_table_entry_name($(mod),$(class)),\n"             \
            $(foreach method, $(call __get_mod_class_methods,$(mod),$(class)), \
                "  __mp_mod_class_dict_table_entry_method"                     \
                        "($(mod),$(class),$(method)),\n"                       \
            )                                                                  \
            "};\n"                                                             \
            "static MP_DEFINE_CONST_DICT$(__str_lp)\n"                         \
            "    ___mp_mod_class_dict_obj_id($(mod),$(class)),\n"              \
            "    ___mp_mod_class_dict_table_id($(mod),$(class))$(__str_rp);\n" \
            "const mp_obj_type_t ___mp_class_obj_id($(mod),$(class)) = {\n"    \
            "  { &mp_type_type },\n"                                           \
            "  .name = MP_QSTR_$(class),\n"                                    \
            $(if $(call __is_exist_class_print,$(mod),$(class)),               \
            "  .print = ___mp_mod_class_fun_id"                                \
                            "($(mod),$(class),_print)$(__str_comma)\n",)       \
            $(if $(call __is_exist_class_new,$(mod),$(class)),                 \
            "  .make_new = ___mp_mod_class_fun_id"                             \
                            "($(mod),$(class),_new)$(__str_comma)\n",)         \
            "  .locals_dict = (mp_obj_dict_t*)"                                \
                        "&___mp_mod_class_dict_obj_id($(mod),$(class))\n"      \
            "};\n\n"                                                           \
        )                                                                      \
        "/* module dict table */\n"                                            \
        "__mp_mod_dict_table($(mod)) {\n"                                      \
            "  /* -- [ Module Name ] -- */\n"                                  \
            $(if $(call __get_mod_name,$(mod)),                                \
            "  __mp_dict_table_entry_name"                                     \
                "($(mod), $(call __get_mod_name,$(mod)))$(__str_comma)\n",)    \
            "  /* -- [ Module Init ] -- */\n"                                  \
            $(if $(call __is_exist_mod_init,$(mod)),                           \
            "  __mp_dict_table_entry_modinit($(mod))$(__str_comma)\n",)        \
            "  /* -- [ Module Classes ] -- */\n"                               \
            $(foreach class,$(call __get_mod_classes,$(mod)),                  \
            "  __mp_dict_table_entry_class($(mod),$(class)),\n")               \
            "  /* -- [ Module Constants ] -- */\n"                             \
            $(foreach const,$(call __get_mod_consts,$(mod)),                   \
            "  __mp_dict_table_entry_const_int$(__str_lp)$(mod), $(const),     \
                $(call __get_mod_const_val,$(mod),$(const))$(__str_rp),\n")    \
            "  /* -- [ Module functions ] -- */\n"                             \
            $(foreach fun,$(call __get_mod_funs,$(mod)),                       \
            "  __mp_dict_table_entry_method($(mod), $(fun)),\n")               \
        "};\n\n"                                                               \
        "/* module registration */\n"                                          \
        "__mp_mod_register($(mod));\n"                                         \
    > $(__gen_dir)/__mp_mod_$(mod).h))

endif

ifeq ($(MAKECMDGOALS),$(__goal__gen_registration_file))
$(__goal__gen_registration_file):
	@echo 'GEN $(__reg_file)'
	@echo "// automatically generated file" > $(__reg_file)
	@echo "#if 0" >> $(__reg_file)
	@grep -oh -E $(__regex_mp_binding_statment)                             \
        $(SRC_FILES)                                                        \
        | sed -e 's/ //g'                                                   \
              -e 's/.*(\([_a-zA-Z0-9]*\))/\1/'                              \
              -e 's/\(.*\)/MP_REGISTER_MODULE(MP_QSTR_\1,\1_mod_obj,1);/'   \
        >> $(__reg_file)
	@echo "#endif" >> $(__reg_file)
endif

# ---------------------------------------------------------------------------- #
#                               END OF FILE                                    #
# ---------------------------------------------------------------------------- #
