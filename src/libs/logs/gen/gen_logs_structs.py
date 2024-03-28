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
# Details   This script generates the needed meta information for the log-lib
#           to identify the available subsystems and their corresponding
#           components with their desired logging attributes and filteration.
# ---------------------------------------------------------------------------- #

# --- imports ---------------------------------------------------------------- #

import sys
import re
from pathlib import Path
import os

# --- generated files names -------------------------------------------------- #

gen_structs_filename  = "logs_gen_structs.cc"
gen_comps_id_filename = "logs_gen_comp_ids.hh"

# --- command lines arguments ------------------------------------------------ #
# syntax:
#   python <script-name> <gen-dir> <c-filenames>+

def user_cmd_check():
    argn = len(sys.argv)
    if argn < 3:
        logl("error in calling script!", 'red')
        logl("   ---> python {} <gen-dir> <c-filename>+"\
            .format(Path(sys.argv[0]).name), 'green')
        exit(0)

    # -- check generation directory
    gen_dir = sys.argv[1]
    if not os.path.exists(gen_dir):
        logl(" -- creating gen-dir: '{}'".format(gen_dir), 'green')
        os.mkdir(gen_dir)

    # -- checking source files
    filenames = []
    all_files_exists = True
    for i in range(2, argn):
        file = sys.argv[i]
        if not os.path.isfile(file):
            logl("error: passing non exist file '{}'".format(file), 'red')
            all_files_exists = False
        filenames.append(file)

    if not all_files_exists:
        exit(0)

def get_gen_dir():
    return sys.argv[1]

def get_filenames():
    filenames = []
    for i in range(2, len(sys.argv)):
        filenames.append(sys.argv[i])
    return filenames

# --- log routines ----------------------------------------------------------- #
color_black  = "\033[39m"
color_red    = "\033[31m"
color_green  = "\033[32m"
color_yellow = "\033[33m"
color_blue   = "\033[34m"
color_purple = "\033[35m"
color_cyan   = "\033[36m"
color_white  = "\033[37m"
color_reset  = "\033[m"  
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
    log_set_color(color)
    print(msg, end='')
    log_set_color('reset')
    print('')

def log(msg, color='reset'):
    log_set_color(color)
    print(msg, end='')
    log_set_color('reset')

# --- regex filteration ------------------------------------------------------ #

# some regex primitives
__W = r"\s*(\w+)\s*"
__Wc = __W + ","
__D = r"\s*(\d+)\s*"
__Dc = __D + ","

# used regex
regex_subsystem = re.compile( \
    r"(?<!#define )" + \
    r"__log_subsystem_def\s*\(" + __Wc + __Wc + __Wc + __W+r"\)", \
    re.M | re.DOTALL)
regex_component = re.compile( \
    r"(?<!#define )" + \
    r"__log_component_def\s*\(" + __Wc + __Wc + __Wc + __Wc + __W+r"\)", \
    re.M | re.DOTALL)

# lists that carry the filtered strings
list_subsystem = []
list_component = []

used_names = []
def extract_used_name() :
    for t in list_subsystem:
        insert_name = True
        name = t[0]
        for n in used_names:
            if name == n:
                insert_name = False
                break
        if insert_name:
            used_names.append(name)
    for t in list_component:
        insert_name = True
        name = t[1]
        for n in used_names:
            if name == n:
                insert_name = False
                break
        if insert_name:
            used_names.append(name)
    print(used_names)

def add_subsystems_default_comps():
    for t in list_subsystem:
        ss = t[0]
        comp = "default"
        for cc in list_component:
            if cc[0] == ss and cc[1] == comp:
                break
        list_component.extend([(ss, comp, 'default', '1', '1')])

subsystems_files = []
components_files = []

def run_sanity_checker():
    sanity_flag = True
    # subsystems duplications
    subsystems = {}
    idx = 0
    for ss in list_subsystem:
        try:
            subsystems[ ss[0] ][ 'count' ] += 1
            subsystems[ ss[0] ][ 'files' ].append(subsystems_files[idx])
            sanity_flag = False
        except KeyError:
            subsystems[ ss[0] ] = {
                'files': [ subsystems_files[idx] ],
                'count': 1,
                'comps': {} }
        idx += 1
    if not sanity_flag:
        for ss in subsystems:
            if subsystems[ ss ][ 'count' ] > 1:
                logl("=== Error: multiple definitions of log subsystem " + \
                    "'{}{}{}':".format(color_green, ss, color_red), 'red')
                log("  in files: ")
                i = 0
                for f in subsystems[ ss ][ 'files' ]:
                    log(str(i) + ':', 'reset')
                    log(f + " ", 'blue')
                    i += 1
                logl("")
    
    # components duplications and its subsystem definition
    undef_subsys = {}
    idx = 0
    is_multiple_components_def = False
    is_undefined_subsystem = False
    for cc in list_component:
        try:
            subsystems[ cc[0] ]
            try:
                subsystems[ cc[0] ][ 'comps' ][ cc[1] ]
                subsystems[ cc[0] ][ 'comps' ][ cc[1] ][ 'count' ] += 1
                subsystems[ cc[0] ][ 'comps' ][ cc[1] ][ 'files' ].\
                    append(components_files[idx])
                is_multiple_components_def = True
            except KeyError:
                subsystems[ cc[0] ][ 'comps' ][ cc[1] ] = {
                    'count': 1,
                    'files': [ components_files[idx] ]
                }
        except KeyError:
            is_undefined_subsystem = True
            try:
                undef_subsys[ cc[0] ][ cc[1] ][ 'count' ] += 1
                undef_subsys[ cc[0] ][ cc[1] ][ 'files' ].\
                    append(components_files[idx])
            except KeyError:
                undef_subsys[ cc[0] ] = {
                    cc[1] : {
                        'files': [ components_files[idx] ],
                        'count': 1
                    }
                }
        idx += 1
    
    if is_multiple_components_def:
        sanity_flag = False
        for sys in subsystems:
            for comp in subsystems[ sys ][ 'comps' ]:
                if subsystems[ sys ][ 'comps' ][ comp ][ 'count' ] > 1:
                    logl("=== Error: multiple definitions of " + \
                        "log component '{}{}{}':".format(\
                            color_yellow, comp, color_red), 'red')
                    i=0
                    log("  in files: ")
                    for f in subsystems[ sys ][ 'comps' ][ comp ][ 'files' ]:
                        log(str(i) + ':', 'reset')
                        log(f + " ", 'blue')
                        i += 1
                    logl("")

    if is_undefined_subsystem:
        sanity_flag = False
        for sys in undef_subsys:
            logl("=== Error: undefined subsystem '{}{}{}' has comps:". \
                format(color_purple, sys, color_red), 'red')
            for comp in undef_subsys[sys]:
                log("  -> comp:")
                log("'{}'".format(comp), 'blue')
                log(" count: ")
                log(undef_subsys[sys][comp]['count'], 'blue')
                log(" in files: ")
                logl(undef_subsys[sys][comp]['files'], 'blue')


    if not sanity_flag:
        logl(' -- generation script stopped --', 'red')
        exit(1)
    
    for sys in subsystems:
        log(" -- detected subsystem: '{}{}{}' with components: '{}default{}'". \
            format(color_yellow, sys, color_reset, color_blue, color_reset), \
            'reset')
        for comp in subsystems[sys]['comps']:
            if comp != 'default':
                log(" , '{}{}{}'".format(color_blue, comp, color_reset))
        logl("")

def filter_new_file_contents(filename):

    log(" -- Parsing File @ ", 'blue')
    logl(filename, 'cyan')

    text = ""
    with open(filename, 'r') as reader:
        text = reader.read()
    
    # remove comments
    comments_regex = re.compile(r"\/\*.*?\*\/", \
        re.M | re.DOTALL)
    text = comments_regex.sub("", text)
    comments_regex = re.compile(r"\/\/.*?$")
    text = comments_regex.sub("", text)

    found_sybsystems = regex_subsystem.findall(text)
    found_components = regex_component.findall(text)
    counter = 0
    while counter < len(found_sybsystems):
        subsystems_files.append(filename)
        counter += 1
    counter = 0
    while counter < len(found_components):
        components_files.append(filename)
        counter += 1
    list_subsystem.extend(found_sybsystems)
    list_component.extend(found_components)

# --- generation ------------------------------------------------------------- #

def write_header(f, str, fill_char, width=80):
    str_len = len(str)
    length = width - 6 - 3
    f.write( "/* ")
    f.write(fill_char + fill_char + fill_char)
    if str_len > 0:
        length -= 2 + str_len
        f.write(" {} ".format(str))
    write_fill( f, fill_char, length )
    f.write(" */\n")

def write_justify(f, left_str, right_str, width):
    l_len = len(left_str)
    r_len = len(right_str)
    fill_len = width - l_len - r_len
    f.write(left_str)
    write_fill( f, " ", fill_len )
    f.write(right_str)

def write_left_align(f, left_str, width):
    l_len = len(left_str)
    fill_len = width - l_len
    f.write(left_str)
    write_fill( f, " ", fill_len )

def write_fill(f, fill_char, width):
    while width > 0:
        width -= 1
        f.write(fill_char)

def write_macro_define(f, macro_name, macro_value_str, width, tab_w, \
        def_str="#define "):
    def_len = len( def_str )
    n_len = len(macro_name)
    v_len = len(macro_value_str)
    two_lines = False
    mid_len = 0
    if(n_len + v_len + def_len >= width):
        two_lines = True
        mid_len = width - def_len - n_len - 1
    else:
        mid_len = width - def_len - n_len - v_len
    write_fill( f, " ", tab_w )
    f.write( def_str + macro_name )
    write_fill( f, " ", mid_len )
    if two_lines:
        f.write("\\\n")
        write_fill( f, " ", tab_w + width - v_len )
    f.write( macro_value_str )

def run_generator():
    f0 = open(get_gen_dir() + "/" + gen_structs_filename, "w")
    f1 = open(get_gen_dir() + "/" + gen_comps_id_filename, "w")

    # -- headers
    write_header(f0, "", "-")
    write_header(f0, "this is an auto generated file for logs system", " ")
    write_header(f0, "", "-")
    f0.write("\n")
    f0.write("#ifndef __LOGS_GEN_CC_STRUCTS_FILE__\n")
    f0.write("    #define __LOGS_GEN_CC_STRUCTS_FILE__\n")
    f0.write("#else\n")
    err_msg = "error: the file 'logs_gen_comp_ids' must be included once"
    f0.write("    #error \"{}\"\n".format(err_msg))
    f0.write("#endif\n\n")

    write_header(f1, "", "-")
    write_header(f1, "this is an auto generated file for logs system", " ")
    write_header(f1, "", "-")
    f1.write("\n")
    f1.write("#ifndef __LOGS_GEN_HH_COMP_ID_FILE__\n")
    f1.write("#define __LOGS_GEN_HH_COMP_ID_FILE__\n\n")

    # -- includes
    f0.write("#include \"{}\"\n\n".format(gen_comps_id_filename))

    # -- used names
    write_header(f0, "components/subsystems used names", "-")
    f0.write("\n")
    f0.write("static const char* s_used_names [] = {\n")
    first = True
    idx_counter = 0
    for n in used_names:
        if not first: f0.write(",\n")
        write_macro_define(f0, "__used_name_idx_" + n, str(idx_counter), 72, 4)
        f0.write("\n    \"{}\"".format(n))
        idx_counter += 1
        first = False
    f0.write("\n};\n\n")

    # -- subsystems info struct
    write_header(f0, "subsystems info array", "-")
    f0.write("\n")
    write_header(f1, "subsystems log compile enable flags", "-")
    f1.write("\n")
    f0.write("static uint8_t s_log_subsystem_info [] = {\n\n")
    first = True
    subsys_id_counter = 0
    for ss in list_subsystem:
        if not first: f0.write(",\n\n")
        f0.write("    ")
        write_header(f0, "subsystem '{}'".format(ss[0]), "-", 72)
        write_macro_define(f0, "__log_subsystem_{}_id".format(ss[0]),\
            str(subsys_id_counter), 72, 4)
        f0.write("\n")
        write_macro_define(f0, "__log_subsystem_{}_on".format(ss[0]),\
            str(ss[3]), 72, 4)
        f0.write("\n")
        write_macro_define(f0, "__log_subsystem_{}_cl".format(ss[0]),\
            "__log_color_" + ss[1], 72, 4)
        f0.write("\n")
        write_macro_define(f1, "__log_subsystem_{}_cc".format(ss[0]),\
            str(ss[2]), 72, 4)
        f1.write("\n")
        write_macro_define(f0, "__log_subsystem_{}_cc".format(ss[0]),\
            str(ss[2]), 72, 4, def_str="//      ")
        f0.write("\n")
        f0.write("    [__log_subsystem_{}_id] = __subsys_info_init({})".\
            format(ss[0],ss[0]))
        first = False
        subsys_id_counter += 1
    f0.write("\n};\n\n")
    f1.write("\n")

    # -- subsystems names indexing array
    write_header(f0, "subsystems names indexing array", "-")
    f0.write("\n")
    f0.write("static uint8_t s_log_subsystem_names_indices [] = {\n")
    first = True
    for ss in list_subsystem:
        if not first: f0.write(",\n")
        f0.write("    __used_name_idx_" + ss[0])
        first = False
    f0.write("\n};\n\n")

    # -- components info struct and macros definitions
    write_header(f0, "components info array", "-")
    f0.write("\n")
    write_header(f1, "log components id's and compile enable flags", "-")
    f1.write("\n")
    f0.write("static uint16_t s_log_component_info [] = {\n\n")
    first = True
    comp_id_counter = 0
    for cc in list_component:
        if not first: f0.write(",\n\n")
        f0.write("    ")

        write_header(f0, "subsystem '{}' -- component '{}'".\
            format(cc[0], cc[1]), "-", 72)

        write_macro_define(f0, "__log_component_{}_{}_on".\
            format(cc[0], cc[1]), cc[4], 72, 4)
        f0.write("\n")

        write_macro_define(f0, "__log_component_{}_{}_cl".\
            format(cc[0], cc[1]), "__log_color_" + cc[2], 72, 4)
        f0.write("\n")

        write_macro_define(f0, "__log_component_{}_{}_id".\
            format(cc[0], cc[1]), str(comp_id_counter), 72, 4, "//      ")
        f0.write("\n")
        write_macro_define(f1, "__log_component_{}_{}_id".\
            format(cc[0], cc[1]), str(comp_id_counter), 72, 4)
        f1.write("\n")

        write_macro_define(f0, "__log_component_{}_{}_cc".\
            format(cc[0], cc[1]), cc[3], 72, 4, "//      ")
        f0.write("\n")
        write_macro_define(f1, "__log_component_{}_{}_cc".\
            format(cc[0], cc[1]), cc[3], 72, 4)
        f1.write("\n")

        f0.write("    [__log_component_{}_{}_id] = __comp_info_init({}, {})".\
            format(cc[0], cc[1], cc[0], cc[1]))

        comp_id_counter += 1
        first = False
    f0.write("\n};\n\n")
    f1.write("\n")

    # -- statistics definitions
    write_header(f1, "log subsystems and components statistics", "-")
    f1.write("\n")
    write_macro_define(f1, "__log_statistics_sybsystems_count",
        str(subsys_id_counter), 72, 4)
    f1.write("\n")
    write_macro_define(f1, "__log_statistics_components_count",
        str(comp_id_counter), 72, 4)
    f1.write("\n\n")

    # -- components names indexing array
    write_header(f0, "components names indexing array", "-")
    f0.write("\n")
    f0.write("static uint8_t s_log_component_names_indices [] = {\n")
    first = True
    for cc in list_component:
        if not first: f0.write(",\n")
        f0.write("    __used_name_idx_" + cc[1])
        first = False
    f0.write("\n};\n\n")

    # -- files tails
    write_header(f0, "end of file", "-")
    write_header(f1, "end of file", "-")
    f1.write("#endif /* __LOGS_GEN_HH_COMP_ID_FILE__ */\n")

    # -- closing files handles
    f0.close()
    f1.close()

# --- main subroutine -------------------------------------------------------- #

def main():
    logl(" -- executing logs meta info defs generator", 'green')
    user_cmd_check()

    for file in get_filenames():
        filter_new_file_contents(file)

    extract_used_name()

    run_sanity_checker()

    add_subsystems_default_comps()

    # print(list_subsystem)
    # print(list_component)
    # print(subsystems_files)
    # print(components_files)

    run_generator()

main()

# --- end of file ------------------------------------------------------------ #
