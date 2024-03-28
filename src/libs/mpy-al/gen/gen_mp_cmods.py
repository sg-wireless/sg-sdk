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
# Desc      [ Pycom Micropython Modules Binding generator ]
#           This file represents the uPython c-module binding files generator
#           for modules that are developed using the mp-lite-if
# ---------------------------------------------------------------------------- #

# --- imports ---------------------------------------------------------------- #

import sys
import re
from pathlib import Path
import os

# --- command lines arguments ------------------------------------------------ #
# syntax:
#   python <script-name>
#       <generate|filenames>
#       <gen-dir>
#       <c-module-filename>+
def user_cmd_check():
    argn = len(sys.argv)
    if argn < 4:
        logl("error in calling script!", 'red')
        logl("   ---> python {} [generate|filenames] <gen-dir> " + \
            "<c-module-filename> +"\
            .format(Path(sys.argv[0]).name), 'green')
        exit(1)
    
    # -- check the command option
    if sys.argv[1] != 'generate' and sys.argv[1] != 'filenames':
        logl("error not valid script command")
        exit(1)

    # -- check generation directory
    gen_dir = sys.argv[2]
    if not os.path.exists(gen_dir):
        logl(" -- creating gen-dir: '{}'".format(gen_dir), 'green')
        os.mkdir(gen_dir)

    # -- checking source files
    cmods_filenames = []
    all_files_exists = True
    for i in range(3, argn):
        file = sys.argv[i]
        if not os.path.isfile(file):
            logl("error: passing non exist cmod file '{}'".format(file), 'red')
            all_files_exists = False
        cmods_filenames.append(file)

    if not all_files_exists:
        exit(1)

def command_is_filenames() -> bool:
    return sys.argv[1] == 'filenames'

def command_is_generate() -> bool:
    return sys.argv[1] == 'generate'

def get_gen_dir():
    return sys.argv[2]

def get_cmods_filenames():
    cmods_filenames = []
    for i in range(3, len(sys.argv)):
        cmods_filenames.append(sys.argv[i])
    return cmods_filenames

# -- user printing
def print_user_args():
    print("=== generation dir: ", get_gen_dir())
    print("=== C modules filenames:")
    for f in get_cmods_filenames():
        print("  > ", f)

# --- log routines ----------------------------------------------------------- #

def log_set_color(color):
    if color == 'black':  print("\033[39m", end=''); return
    if color == 'red':    print("\033[31m", end=''); return
    if color == 'green':  print("\033[32m", end=''); return
    if color == 'yellow': print("\033[33m", end=''); return
    if color == 'blue':   print("\033[34m", end=''); return
    if color == 'purple': print("\033[35m", end=''); return
    if color == 'cyan':   print("\033[36m", end=''); return
    if color == 'white':  print("\033[37m", end=''); return
    if color == 'reset':  print("\033[m"  , end=''); return

def logl(msg, color='reset'):
    if command_is_generate():
        log_set_color(color)
        print(msg, end='')
        log_set_color('reset')
        print('')

def log(msg, color='reset'):
    if command_is_generate():
        log_set_color(color)
        print(msg, end='')
        log_set_color('reset')

# --- regex filteration ------------------------------------------------------ #

# some regex primitives
__W = r"\s*(\w+)\s*"
__Wc = __W + ","
__D = r"\s*(\d+)\s*"
__Dc = __D + ","

# module regex
regex_mod_fun_0 = r"\b__mp_mod_fun_0\s*\(" + __Wc + __W + r"\)"
regex_mod_fun_1 = r"\b__mp_mod_fun_1\s*\(" + __Wc + __W + r"\)"
regex_mod_fun_2 = r"\b__mp_mod_fun_2\s*\(" + __Wc + __W + r"\)"
regex_mod_fun_3 = r"\b__mp_mod_fun_3\s*\(" + __Wc + __W + r"\)"
regex_mod_fun_var = r"\b__mp_mod_fun_var\s*\(" + __Wc + __Wc +__D + r"\)"
regex_mod_fun_var_between = \
    r"\b__mp_mod_fun_var_between\s*\(" + __Wc + __Wc + __Dc + __D + r"\)"
regex_mod_fun_kw = r"\b__mp_mod_fun_kw\s*\(" + __Wc + __Wc + __D + r"\)"
regex_mod_const = r"\b__mp_mod_const\s*\(" + __Wc + __Wc + __W + r"\)"
regex_mod_name = r"\b__mp_mod_name\s*\(" + __Wc + __W + r"\)"
regex_mod_init = r"\b__mp_mod_init\s*\(" + __W + r"\)"
regex_mod_include = r"\b__mp_mod_include\s*\("+__Wc+r"\s*(\"[\w-]+\.h\")\s*\)"
regex_mod_ifdef = r"\b__mp_mod_ifdef\s*\(" + __Wc + __W + r"\)"
regex_mod_fun_ifdef = r"\b__mp_mod_fun_ifdef\s*\(" + __Wc + __Wc + __W + r"\)"

# class regex
regex_mod_class_method_0 = r"\b__mp_mod_class_method_0\s*\("+__Wc+__Wc+__W+r"\)"
regex_mod_class_method_1 = r"\b__mp_mod_class_method_1\s*\("+__Wc+__Wc+__W+r"\)"
regex_mod_class_method_2 = r"\b__mp_mod_class_method_2\s*\("+__Wc+__Wc+__W+r"\)"
regex_mod_class_method_3 = r"\b__mp_mod_class_method_3\s*\("+__Wc+__Wc+__W+r"\)"
regex_mod_class_method_var = \
    r"\b__mp_mod_class_method_var\s*\(" + __Wc + __Wc + __Wc +__D + r"\)"
regex_mod_class_method_var_between = \
    r"\b__mp_mod_class_method_var_between\s*\(" \
    + __Wc + __Wc + __Wc + __Dc + __D + r"\)"
regex_mod_class_method_kw = r"\b__mp_mod_class_method_kw\s*\(" \
    + __Wc + __Wc + __Wc + __D + r"\)"
regex_mod_class_new = r"\b__mp_mod_class_new\s*\(" + __Wc + __W + r"\)"
regex_mod_class_print = r"\b__mp_mod_class_print\s*\(" + __Wc + __W + r"\)"
regex_mod_class_const = \
    r"\b__mp_mod_class_const\s*\(" + __Wc + __Wc + __Wc + __W + r"\)"
regex_mod_class_ifdef = \
    r"\b__mp_mod_class_ifdef\s*\(" + __Wc + __Wc + __W + r"\)"
regex_mod_class_method_ifdef = \
    r"\b__mp_mod_class_method_ifdef\s*\(" + __Wc + __Wc + __Wc + __W + r"\)"

# lists that carry the filtered strings
list_mod_fun_0 = []
list_mod_fun_1 = []
list_mod_fun_2 = []
list_mod_fun_3 = []
list_mod_fun_var = []
list_mod_fun_var_between = []
list_mod_fun_kw = []
list_mod_const = []
list_mod_name = []
list_mod_init = []
list_mod_include = []
list_mod_ifdef = []
list_mod_fun_ifdef = []
list_mod_class_new = []
list_mod_class_print = []
list_mod_class_method_0 = []
list_mod_class_method_1 = []
list_mod_class_method_2 = []
list_mod_class_method_3 = []
list_mod_class_method_var = []
list_mod_class_method_var_between = []
list_mod_class_method_kw = []
list_mod_class_const = []
list_mod_class_ifdef = []
list_mod_class_method_ifdef = []

def filter_new_file_contents(filename):

    log(" -- Parsing Micropython C-Module File @ ", 'blue')
    logl(filename, 'cyan')

    text = ""
    with open(filename, 'r') as reader:
        text = reader.read()
    list_mod_fun_0           .extend(re.findall(regex_mod_fun_0          ,text))
    list_mod_fun_1           .extend(re.findall(regex_mod_fun_1          ,text))
    list_mod_fun_2           .extend(re.findall(regex_mod_fun_2          ,text))
    list_mod_fun_3           .extend(re.findall(regex_mod_fun_3          ,text))
    list_mod_fun_var         .extend(re.findall(regex_mod_fun_var        ,text))
    list_mod_fun_var_between .extend(re.findall(regex_mod_fun_var_between,text))
    list_mod_fun_kw          .extend(re.findall(regex_mod_fun_kw         ,text))
    list_mod_const           .extend(re.findall(regex_mod_const          ,text))
    list_mod_name            .extend(re.findall(regex_mod_name           ,text))
    list_mod_init            .extend(re.findall(regex_mod_init           ,text))
    list_mod_include         .extend(re.findall(regex_mod_include        ,text))
    list_mod_ifdef           .extend(re.findall(regex_mod_ifdef          ,text))
    list_mod_fun_ifdef       .extend(re.findall(regex_mod_fun_ifdef      ,text))
    list_mod_class_new       .extend(re.findall(regex_mod_class_new      ,text))
    list_mod_class_print     .extend(re.findall(regex_mod_class_print    ,text))
    list_mod_class_method_0  .extend(re.findall(regex_mod_class_method_0 ,text))
    list_mod_class_method_1  .extend(re.findall(regex_mod_class_method_1 ,text))
    list_mod_class_method_2  .extend(re.findall(regex_mod_class_method_2 ,text))
    list_mod_class_method_3  .extend(re.findall(regex_mod_class_method_3 ,text))
    list_mod_class_method_var.extend( \
        re.findall(regex_mod_class_method_var, text))
    list_mod_class_method_var_between.extend( \
        re.findall(regex_mod_class_method_var_between, text) )
    list_mod_class_method_kw.extend(re.findall(regex_mod_class_method_kw, text))
    list_mod_class_const.extend(re.findall(regex_mod_class_const, text))
    list_mod_class_ifdef    .extend(re.findall(regex_mod_class_ifdef     ,text))
    list_mod_class_method_ifdef.extend(
        re.findall(regex_mod_class_method_ifdef, text))

def print_lists():
    print("list_mod_fun_0                   ",list_mod_fun_0                   )
    print("list_mod_fun_1                   ",list_mod_fun_1                   )
    print("list_mod_fun_2                   ",list_mod_fun_2                   )
    print("list_mod_fun_3                   ",list_mod_fun_3                   )
    print("list_mod_fun_var                 ",list_mod_fun_var                 )
    print("list_mod_fun_var_between         ",list_mod_fun_var_between         )
    print("list_mod_fun_kw                  ",list_mod_fun_kw                  )
    print("list_mod_name                    ",list_mod_name                    )
    print("list_mod_init                    ",list_mod_init                    )
    print("list_mod_include                 ",list_mod_include                 )
    print("list_mod_ifdef                   ",list_mod_ifdef                   )
    print("list_mod_fun_ifdef               ",list_mod_fun_ifdef               )
    print("list_mod_class_new               ",list_mod_class_new               )
    print("list_mod_class_print             ",list_mod_class_print             )
    print("list_mod_class_method_0          ",list_mod_class_method_0          )
    print("list_mod_class_method_1          ",list_mod_class_method_1          )
    print("list_mod_class_method_2          ",list_mod_class_method_2          )
    print("list_mod_class_method_3          ",list_mod_class_method_3          )
    print("list_mod_class_method_var        ",list_mod_class_method_var        )
    print("list_mod_class_method_var_between",list_mod_class_method_var_between)
    print("list_mod_class_method_kw         ",list_mod_class_method_kw         )
    print("list_mod_class_const             ",list_mod_class_const             )
    print("list_mod_class_ifdef             ",list_mod_class_ifdef             )
    print("list_mod_class_method_ifdef      ",list_mod_class_method_ifdef      )

# --- getters methods -------------------------------------------------------- #

def get_modules():
    mods = []
    for c in    list_mod_name + \
                list_mod_class_new + list_mod_class_method_0 + \
                list_mod_class_method_1 + list_mod_class_method_2 + \
                list_mod_class_method_3 + list_mod_class_method_kw + \
                list_mod_class_method_var + list_mod_class_method_var_between +\
                list_mod_class_print + list_mod_fun_0 + list_mod_fun_1 + \
                list_mod_fun_2 + list_mod_fun_3 + list_mod_fun_kw + \
                list_mod_fun_var + list_mod_fun_var_between + \
                list_mod_class_const:
        mods.append(c[0])
    for c in list_mod_init:
        mods.append(c)
    mods = list(dict.fromkeys(mods))
    return mods

def get_module_classes(mod):
    classes = []
    # scan all lists that can include classes
    for c in    list_mod_class_new + list_mod_class_method_0 + \
                list_mod_class_method_1 + list_mod_class_method_2 + \
                list_mod_class_method_3 + list_mod_class_method_kw + \
                list_mod_class_method_var + list_mod_class_method_var_between +\
                list_mod_class_print + list_mod_class_const:
        if c[0] == mod:
            classes.append(c[1])
    classes = list(dict.fromkeys(classes))
    return classes

def get_class_methods(mod, cl):
    methods = []
    for c in    list_mod_class_method_0 + \
                list_mod_class_method_1 + list_mod_class_method_2 + \
                list_mod_class_method_3 + list_mod_class_method_kw + \
                list_mod_class_method_var + list_mod_class_method_var_between:
        if c[0] == mod and c[1] == cl:
            methods.append(c[2])
    methods = list(dict.fromkeys(methods))
    return methods

def get_class_consts(mod, cl):
    consts = []
    for c in    list_mod_class_const:
        if c[0] == mod and c[1] == cl:
            consts.append((c[2], c[3]))
    consts = list(dict.fromkeys(consts))
    return consts

def class_new_method_exists(mod, cl):
    for c in list_mod_class_new:
        if c[0] == mod and c[1] == cl:
            return True
    return False

def class_print_method_exists(mod, cl):
    for c in list_mod_class_print:
        if c[0] == mod and c[1] == cl:
            return True
    return False

def mod_init_exists(mod):
    for c in list_mod_init:
        if c == mod:
            return True
    return False

def get_mod_name(mod):
    for c in list_mod_name:
        if c[0] == mod:
            return c[1]
    return None

def get_mod_consts(mod):
    consts = []
    for c in list_mod_const:
        if c[0] == mod:
            consts.append((c[1], c[2]))
    return consts

def get_mod_funcs(mod):
    funcs = []
    for c in list_mod_fun_0 + list_mod_fun_1 + list_mod_fun_2 + list_mod_fun_3\
            + list_mod_fun_kw + list_mod_fun_var + list_mod_fun_var_between:
        if c[0] == mod:
            funcs.append(c[1])
    return funcs

def get_mod_includes(mod):
    includes = []
    for c in list_mod_include:
        if c[0] == mod:
            includes.append(c[1])
    includes = list(dict.fromkeys(includes))
    return includes

def get_mod_ifdefs(mod):
    ifdefs = []
    for c in list_mod_ifdef:
        if c[0] == mod and not c[1] in ifdefs:
            ifdefs.append(c[1])
    ifdefs = ifdefs
    return ifdefs

def get_mod_fun_ifdefs(mod, fun):
    ifdefs = []
    for c in list_mod_fun_ifdef:
        if c[0] == mod and c[1] == fun and not c[2] in ifdefs:
            ifdefs.append(c[2])
    return ifdefs

def get_mod_class_ifdefs(mod, c):
    ifdefs = []
    for i in list_mod_class_ifdef:
        if i[0] == mod and i[1] == c and not i[2] in ifdefs:
            ifdefs.append(i[2])
    ifdefs = ifdefs
    return ifdefs

def get_mod_class_method_ifdefs(mod, c, m):
    ifdefs = []
    for i in list_mod_class_method_ifdef:
        if i[0] == mod and i[1] == c and i[2] == m and not i[3] in ifdefs:
            ifdefs.append(i[3])
    return ifdefs

# --- generator functions ---------------------------------------------------- #

def write_class_methods_declarations(mod,c,f):
    if class_new_method_exists(mod, c):
        f.write("extern __mp_mod_class_new({}, {}) (void);\n".format(mod, c))
    if class_print_method_exists(mod, c):
        f.write("extern __mp_mod_class_print({}, {}) (void);\n".format(mod, c))
    dec = "__mp_declare_mod_class_meth_obj"
    for m in list_mod_class_method_0:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, 0);\n".format(mod, c, m[2]))
    for m in list_mod_class_method_1:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, 1);\n".format(mod, c, m[2]))
    for m in list_mod_class_method_2:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, 2);\n".format(mod, c, m[2]))
    for m in list_mod_class_method_3:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, 3);\n".format(mod, c, m[2]))
    for m in list_mod_class_method_var:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, VAR);\n".format(mod, c, m[2]))
    for m in list_mod_class_method_var_between:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, VAR_BETWEEN);\n".format(mod, c, m[2]))
    for m in list_mod_class_method_kw:
        if m[0] == mod and m[1] == c:
            f.write(dec + "({}, {}, {}, KW);\n".format(mod, c, m[2]))
    f.write("\n")

def write_mod_ifdef_start(f, mod, prefix=''):
    ifdefs = get_mod_ifdefs(mod)
    if len(ifdefs) > 0:
        for ifdef in ifdefs:
            f.write(f'{prefix}#ifdef {ifdef}\n')
        f.write("\n\n")

def write_mod_ifdef_end(f, mod, prefix=''):
    ifdefs = get_mod_ifdefs(mod)
    if len(ifdefs) > 0:
        f.write("\n\n")
        for ifdef in reversed(ifdefs):
            f.write(f'{prefix}#endif /* {ifdef} */\n')

def write_mod_fun_ifdef_start(f, mod, fun, prefix=''):
    ifdefs = get_mod_fun_ifdefs(mod, fun)
    if len(ifdefs) > 0:
        for ifdef in ifdefs:
            f.write(f'{prefix}#ifdef {ifdef}\n')

def write_mod_fun_ifdef_end(f, mod, fun, prefix=''):
    ifdefs = get_mod_fun_ifdefs(mod, fun)
    if len(ifdefs) > 0:
        for ifdef in reversed(ifdefs):
            f.write(f'{prefix}#endif /* {ifdef} */\n')

def write_mod_class_ifdef_start(f, mod, clas, prefix=''):
    ifdefs = get_mod_class_ifdefs(mod, clas)
    if len(ifdefs) > 0:
        for ifdef in ifdefs:
            f.write(f'{prefix}#ifdef {ifdef}\n')

def write_mod_class_ifdef_end(f, mod, clas, prefix=''):
    ifdefs = get_mod_class_ifdefs(mod, clas)
    if len(ifdefs) > 0:
        for ifdef in reversed(ifdefs):
            f.write(f'{prefix}#endif /* {ifdef} */\n')

def write_mod_class_method_ifdef_start(f, mod, clas, method, prefix=''):
    ifdefs = get_mod_class_method_ifdefs(mod, clas, method)
    if len(ifdefs) > 0:
        for ifdef in ifdefs:
            f.write(f'{prefix}#ifdef {ifdef}\n')

def write_mod_class_method_ifdef_end(f, mod, clas, method, prefix=''):
    ifdefs = get_mod_class_method_ifdefs(mod, clas, method)
    if len(ifdefs) > 0:
        for ifdef in reversed(ifdefs):
            f.write(f'{prefix}#endif /* {ifdef} */\n')

def write_mod_funcs_declarations(mod, f):
    dec = "__mp_declare_mod_fun_obj"
    if mod_init_exists(mod):
        f.write(dec + "({}, init, 0);\n".format(mod))
    for m in list_mod_fun_0:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, 0);\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    for m in list_mod_fun_1:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, 1);\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    for m in list_mod_fun_2:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, 2);\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    for m in list_mod_fun_3:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, 3) ;\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    for m in list_mod_fun_var:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, VAR);\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    for m in list_mod_fun_var_between:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, VAR_BETWEEN);\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    for m in list_mod_fun_kw:
        if m[0] == mod:
            write_mod_fun_ifdef_start(f, mod, m[1])
            f.write(dec + "({}, {}, KW);\n".format(mod, m[1]))
            write_mod_fun_ifdef_end(f, mod, m[1])
    f.write("\n")

def gen_module_file(mod):

    gen_filename = get_gen_dir() + "/__mp_mod_{}.c".format(mod)
    log(" -- GEN Micropython Binding File @ ", 'green')
    logl(gen_filename, 'yellow')

    with open(gen_filename, "w") as f:
        f.write("/////// This is an auto generated file for module " + 
                "'{}'\n\n".format(mod))

        f.write('#include "py/obj.h"\n')
        f.write('#include "mp_lite_if.h"\n\n')
        f.write('\n')
        includes = get_mod_includes(mod)
        if len(includes) > 0:
            for inc in includes:
                f.write("#include {}\n".format(inc))
            f.write('\n\n')

        write_mod_ifdef_start(f, mod)

        f.write("// list of module classes\n")
        classes = get_module_classes(mod)
        for c in classes:
            write_mod_class_ifdef_start(f, mod, c)
            f.write("// -- class '{}'\n".format(c))
            write_class_methods_declarations(mod, c, f)
            # write class map table
            f.write("__mp_mod_class_dict_table({}, {}) ".format(mod, c)+'{\n')
            f.write("    __mp_dict_table_entry_name({}, {}),\n".format(mod,c))
            f.write("    // -- [ class '{}' constants ] --\n".format(c))
            for con in get_class_consts(mod, c):
                f.write("    __mp_mod_class_dict_table_entry_const_int" + \
                        "({}, {}, {}, {}),\n".format(mod, c, con[0], con[1]))
            methods = get_class_methods(mod, c)
            for meth in methods:
                write_mod_class_method_ifdef_start(f, mod, c, meth, '    ')
                f.write("    __mp_mod_class_dict_table_entry_method" + \
                        "({}, {}, {}),\n".format(mod, c, meth))
                write_mod_class_method_ifdef_end(f, mod, c, meth, '    ')
            f.write('};\n\n')
            # write class object definition
            f.write("static MP_DEFINE_CONST_DICT(\n")
            f.write("    ___mp_mod_class_dict_obj_id({}, {}),\n".format(mod, c))
            f.write("    ___mp_mod_class_dict_table_id" + \
                    "({}, {}) );\n\n".format(mod, c))
            # write class object definition
            f.write("MP_DEFINE_CONST_OBJ_TYPE (\n")
            f.write("    ___mp_class_obj_id({}, {}),\n".format(mod, c))
            f.write("    MP_QSTR_{},\n".format(c))
            f.write("    MP_TYPE_FLAG_NONE,\n")
            if class_new_method_exists(mod, c):
                f.write("    make_new, ___mp_mod_class_fun_id" + \
                        "({}, {}, _new),\n".format(mod, c))
            if class_print_method_exists(mod, c):
                f.write("    print, ___mp_mod_class_fun_id" + \
                        "({}, {}, _print),\n".format(mod, c))
            f.write("    locals_dict, " + \
                    "(const void *)&___mp_mod_class_dict_obj_id" + \
                    "({}, {})\n".format(mod, c))
            f.write("    );\n")
            write_mod_class_ifdef_end(f, mod, c)
            f.write("\n")
        
        # module dict table
        f.write("/// -- module dict table\n")
        write_mod_funcs_declarations(mod,f)
        f.write("__mp_mod_dict_table({})".format(mod) + ' {\n')
        f.write("    // -- [ module name ] --\n")
        mod_name = get_mod_name(mod)
        if mod_name != None:
            f.write("    __mp_dict_table_entry_name" + \
                    "({}, {}),\n".format(mod, mod_name))
        f.write("    // -- [ module init ] --\n")
        if mod_init_exists(mod):
            f.write("    __mp_dict_table_entry_modinit({}),\n".format(mod))
        f.write("    // -- [ module classes ] --\n")
        for c in classes:
            write_mod_class_ifdef_start(f, mod, c, '    ')
            f.write("    __mp_dict_table_entry_class({}, {}),\n".format(mod, c))
            write_mod_class_ifdef_end(f, mod, c, '    ')
        f.write("    // -- [ module constants ] --\n")
        for c in get_mod_consts(mod):
            f.write("    __mp_dict_table_entry_const_int" + \
                    "({}, {}, {}),\n".format(mod, c[0], c[1]))
        f.write("    // -- [ module global functions ] --\n")
        for c in get_mod_funcs(mod):
            write_mod_fun_ifdef_start(f, mod, c, '    ')
            f.write("    __mp_dict_table_entry_method" + \
                    "({}, {}),\n".format(mod, c))
            write_mod_fun_ifdef_end(f, mod, c, '    ')

        f.write('};\n\n')
        f.write("/// module binding and registeration\n")
        f.write('__mp_mod_bind({});\n\n'.format(mod))
        f.write("MP_REGISTER_MODULE(MP_QSTR_{},{}_mod_obj);\n".format(mod,mod))

        write_mod_ifdef_end(f, mod)


def gen_mp_registry_file(filename):
    logl(" -- GEN Micropython Modules Registration File @ "+filename,'purple')
    with open(filename, "w") as f:
        f.write("// automatically generated file\n\n")
        f.write("#if 0\n")
        for mod in get_modules():
            f.write("MP_REGISTER_MODULE" + \
                    "(MP_QSTR_{},{}_mod_obj);\n".format(mod, mod))
        f.write("#endif\n")

# --- main subroutine -------------------------------------------------------- #

def main():
    logl(" -- executing the micropython c-modules binding files generator", \
        'green')
    user_cmd_check()

    for file in get_cmods_filenames():
        filter_new_file_contents(file)

    # print_lists()

    for mod in get_modules():
        print(get_gen_dir() + "/__mp_mod_{}.c".format(mod))
        if command_is_generate():
            gen_module_file(mod)

    # gen_mp_registry_file(get_gen_dir() + "/moduledefs_reg_file.c")

main()

# --- end of file ------------------------------------------------------------ #