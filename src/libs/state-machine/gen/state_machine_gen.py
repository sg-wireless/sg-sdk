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
# Desc      This file contains the generator script for the state-machines   
# ---------------------------------------------------------------------------- #

# --- imports ---------------------------------------------------------------- #

import sys
import re
from pathlib import Path
import os

# --- command lines arguments ------------------------------------------------ #
# syntax:
#   python <script-name> <gen-dir> <c-module-filename>+
def user_cmd_check():
    argn = len(sys.argv)
    if argn < 3:
        logl("error in calling script!", 'red')
        logl("   ---> python {} <gen-dir> " + "<c-module-filename> +"\
            .format(Path(sys.argv[0]).name), 'green')
        exit(1)
    
    # -- check generation directory
    gen_dir = sys.argv[1]
    if not os.path.exists(gen_dir):
        logl(" -- creating gen-dir: '{}'".format(gen_dir), 'green')
        os.mkdir(gen_dir)

    # -- checking source files
    sm_filenames = []
    all_files_exists = True
    for i in range(2, argn):
        file = sys.argv[i]
        if not os.path.isfile(file):
            logl("error: passing non exist sm file '{}'".format(file), 'red')
            all_files_exists = False
        sm_filenames.append(file)

    if not all_files_exists:
        exit(1)

def get_gen_dir():
    return sys.argv[1]

def get_filenames():
    filnames = []
    for i in range(2, len(sys.argv)):
        filnames.append(sys.argv[i])
    return filnames

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

regex_sm_trans  = r"\__sm_trans\s*\(" + __Wc + __Wc + __Wc + __Wc + __W + r"\)"
regex_sm_action = r"\__sm_action\s*\(" + __Wc + __W + r"\)"
regex_sm_default_action = r"\__sm_state_default_action\s*\(" + __Wc +__W + r"\)"
regex_sm_state_enter = r"\__sm_state_enter\s*\(" + __Wc +__W + r"\)"
regex_sm_state_leave = r"\__sm_state_leave\s*\(" + __Wc +__W + r"\)"
regex_sm_ifdef = r"\__sm_ifdef\s*\(" + __Wc +__W + r"\)"

# lists that carry the filtered strings
list_regex_sm_trans = []
list_regex_sm_action = []
list_regex_sm_default_action = []
list_regex_sm_state_enter = []
list_regex_sm_state_leave = []
list_regex_sm_ifdef = []

def filter_new_file_contents(filename):

    log(" -- Parsing State-Machine deifinition File @ ", 'blue')
    logl(filename, 'cyan')

    text = ""
    with open(filename, 'r') as reader:
        text = reader.read()
    list_regex_sm_trans.extend(re.findall(regex_sm_trans, text))
    list_regex_sm_action.extend(re.findall(regex_sm_action, text))
    list_regex_sm_default_action.\
        extend(re.findall(regex_sm_default_action, text))
    list_regex_sm_state_enter.extend(re.findall(regex_sm_state_enter, text))
    list_regex_sm_state_leave.extend(re.findall(regex_sm_state_leave, text))
    list_regex_sm_ifdef.extend(re.findall(regex_sm_ifdef, text))

def print_lists():
    print("list_regex_sm_trans  >> ",list_regex_sm_trans)
    print("list_regex_sm_action >> ",list_regex_sm_action)
    print("list_regex_sm_default_action >> ",list_regex_sm_default_action)
    print("list_regex_sm_state_enter >> ",list_regex_sm_state_enter)
    print("list_regex_sm_state_leave >> ",list_regex_sm_state_leave)
    print("list_regex_sm_ifdef >> ",list_regex_sm_ifdef)

# --- getters methods -------------------------------------------------------- #

def get_state_machines():
    sms = []
    for it in list_regex_sm_trans:
        sms.append(it[0])
    sms = list(dict.fromkeys(sms))
    return sms

def get_sm_inputs(sm):
    inputs = []
    for it in list_regex_sm_trans:
        if it[0] == sm:
            inputs.append(it[2])
    inputs = list(dict.fromkeys(inputs))
    return inputs

def get_sm_states(sm):
    states = []
    for it in list_regex_sm_trans:
        if it[0] == sm:
            states.append(it[1])
            states.append(it[4])
    states = list(dict.fromkeys(states))
    return states

def has_default_action(sm, st):
    for it in list_regex_sm_default_action:
        if it[0] == sm and it[1] == st:
            return True
    return False

def has_state_enter(sm, st):
    for it in list_regex_sm_state_enter:
        if it[0] == sm and it[1] == st:
            return True
    return False

def has_state_leave(sm, st):
    for it in list_regex_sm_state_leave:
        if it[0] == sm and it[1] == st:
            return True
    return False

def get_sm_actions(sm):
    actions = []
    for it in list_regex_sm_trans:
        if it[0] == sm:
            actions.append(it[3])
    actions = list(dict.fromkeys(actions))
    return actions

def get_sm_defined_actions(sm):
    actions = []
    for it in list_regex_sm_action:
        if it[0] == sm:
            actions.append(it[1])
    actions = list(dict.fromkeys(actions))
    return actions

def get_sm_state_trans(sm, st):
    trans = []
    for it in list_regex_sm_trans:
        if it[0] == sm and it[1] == st:
            trans.append(( it[2], it[3], it[4] ))
    return trans

def get_sm_ifdefs(sm):
    ifdefs = []
    for it in list_regex_sm_ifdef:
        if it[0] == sm and not it[1] in ifdefs:
            ifdefs.append(it[1])
    return ifdefs

def check_sm_state_trans(sm):
    states = get_sm_states(sm)
    state_trans = []
    error = False
    for st in states:
        for it in list_regex_sm_trans:
            if it[0] == sm and it[1] == st:
                for tr in state_trans:
                    if tr[1] == st and tr[2] == it[2]:
                        logl('error: two different state transition for same'+\
                             ' input', 'red')
                        logl('--> {}'.format(tr))
                        logl('--> {}'.format(it))
                        error = True
                state_trans.append( it )
    if error:
        exit(0)

# --- generation helper functions -------------------------------------------- #

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

def write_ifdefs_start(f, sm, prefix=''):
    ifdefs = get_sm_ifdefs(sm)
    if len(ifdefs) > 0:
        for ifdef in ifdefs:
            f.write(f'{prefix}#ifdef {ifdef}\n')
        f.write("\n")

def write_ifdefs_end(f, sm, prefix=''):
    ifdefs = get_sm_ifdefs(sm)
    if len(ifdefs) > 0:
        f.write("\n")
        for ifdef in reversed(ifdefs):
            f.write(f'{prefix}#endif /* {ifdef} */\n')

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


def write_inputs_declarations(sm, fc, fh):
    write_header(fc, 'INPUTS', '-')
    write_header(fh, 'INPUTS', '-')
    inputs = get_sm_inputs(sm)
    fh.write('enum {\n')
    for it in inputs:
        fh.write('    __sm_input_id({}, {}),\n'.format(sm, it))
    fh.write('};\n')
    fh.write('\n')
    fc.write('static input_table_t sm_{}_inputs_table[] = '.format(sm))
    fc.write('{\n')
    for it in inputs:
        fc.write('    [__sm_input_id({}, {})] = {} "'.format(sm, it, '{'))
        fc.write('{}"'.format(it))
        fc.write(' },\n')
    fc.write('};\n')
    fc.write('#define sm_{}_inputs_table_size \\\n'.format(sm))
    fc.write('    (sizeof(sm_{}_inputs_table)/sizeof(input_table_t))\n'\
            .format(sm))

def write_actions_declarations(sm, fc, fh):
    write_header(fc, 'ACTIONS', '-')
    write_header(fh, 'ACTIONS', '-')
    actions = get_sm_actions(sm)
    def_actions = get_sm_defined_actions(sm)
    fh.write('enum {\n')
    for it in actions:
        fh.write('    __sm_action_id({}, {}),\n'.format(sm, it))
    fh.write('};\n')
    fh.write('\n')

    fc.write('static action_table_t sm_{}_actions_table[] = '.format(sm))
    fc.write('{\n')
    for it in actions:
        found = False
        fc.write('    [__sm_action_id({}, {})] = {}\n'.format(sm, it, '{'))
        fc.write('        "{}",\n'.format(it))
        for it2 in def_actions:
            if it == it2:
                 fc.write('        __sm_action_fun({}, {}){},\n'\
                          .format(sm, it, '}'))
                 found = True
                 break
        if not found:
            fc.write('        0},\n')
    fc.write('};\n')
    fc.write('#define sm_{}_actions_table_size \\\n'.format(sm))
    fc.write('    (sizeof(sm_{}_actions_table)/sizeof(action_table_t))\n'\
            .format(sm))

def write_methods_declarations(sm, fc, fh):
    write_header(fh, 'METHODS', '-')
    write_header(fc, 'METHODS', '-')
    actions = get_sm_actions(sm)
    def_actions = get_sm_defined_actions(sm)
    fh.write('\n')
    for it in actions:
        for it2 in def_actions:
            if it == it2:
                 fh.write('void __sm_action_fun({}, {})(void* data);\n'\
                          .format(sm, it))
                 break
    fh.write('\n')

    for st in get_sm_states(sm):
        if has_default_action(sm, st):
            fh.write('void __sm_action_fun({}, {}_default)(void* data);\n'\
                     .format(sm, st))
        if has_state_enter(sm, st):
            fc.write('void __sm_action_fun({}, {}_enter)(void* data);\n'\
                     .format(sm, st))
        if has_state_leave(sm, st):
            fc.write('void __sm_action_fun({}, {}_leave)(void* data);\n'\
                     .format(sm, st))
    fh.write('\n')
    fc.write('\n')

def write_states_declarations(sm, fc, fh):
    write_header(fc, 'STATES', '-')
    write_header(fh, 'STATES', '-')
    states = get_sm_states(sm)
    fh.write('enum {\n')
    for it in states:
        fh.write('    __sm_state_id({}, {}),\n'.format(sm, it))
    fh.write('};\n')
    fh.write('\n')
    
    for st in states:
        write_header(fc, 'state -> {}'.format(st), '-')
        trans = get_sm_state_trans(sm, st)

        fc.write('static state_trans_table_t sm_{}_{}_trans_table [] = {}\n'\
                 .format(sm,st,'{'))
        for tr in trans:
            fc.write('    {\n')
            fc.write('        .input_id = __sm_input_id({}, {}),\n'\
                     .format(sm,tr[0]))
            fc.write('        .action_id = __sm_action_id({}, {}),\n'\
                     .format(sm,tr[1]))
            fc.write('        .next_state_id = __sm_state_id({}, {}),\n'\
                     .format(sm,tr[2]))
            fc.write('    },\n')
        fc.write('};\n')
        fc.write('#define sm_{}_{}_trans_table_size \\\n'.format(sm, st))
        fc.write('    (sizeof(sm_{}_{}_trans_table)/'.format(sm, st) + \
                 'sizeof(state_trans_table_t))\n')
        fc.write('\n')
    
    write_header(fc, 'states-table', '-')
    
    fc.write('static state_table_t sm_{}_states_table [] = {}\n'\
             .format(sm, '{'))
    for st in states:
        fc.write('    [__sm_state_id({}, {})] = {}\n'.format(sm, st, '{'))
        fc.write('        .name = "{}",\n'.format(st))
        fc.write('        .trans_table = sm_{}_{}_trans_table,\n'\
                 .format(sm, st))
        fc.write('        .trans_table_size = sm_{}_{}_trans_table_size,\n'\
                 .format(sm, st))
        if has_default_action(sm, st):
            fc.write('        .default_action = ' + \
                     '__sm_action_fun({}, {}_default),\n'.format(sm, st))
        if has_state_enter(sm, st):
            fc.write('        .enter = ' + \
                     '__sm_action_fun({}, {}_enter),\n'.format(sm, st))
        if has_state_leave(sm, st):
            fc.write('        .leave = ' + \
                     '__sm_action_fun({}, {}_leave),\n'.format(sm, st))
        fc.write('    },\n')
    fc.write('};\n')
    fc.write('#define sm_{}_states_table_size \\\n'.format(sm))
    fc.write('    (sizeof(sm_{}_states_table)/sizeof(state_table_t))\n'\
            .format(sm))
    
    fc.write('\n')

def write_machine_declaration(sm, fc, fh):
    write_header(fh, 'MACHINE', '-')
    fh.write('extern state_machine_t __sm_machine_id({});\n'.format(sm))
    fh.write('\n')
    write_header(fc, 'MACHINE', '-')
    fc.write('\n')
    fc.write('state_machine_t __sm_machine_id({}) = {}\n'.format(sm, '{'))
    fc.write('    .name = "{}",\n'.format(sm))
    fc.write('    .inputs_table = sm_{}_inputs_table,\n'.format(sm))
    fc.write('    .inputs_table_size = sm_{}_inputs_table_size,\n'.format(sm))
    fc.write('    .actions_table = sm_{}_actions_table,\n'.format(sm))
    fc.write('    .actions_table_size = sm_{}_actions_table_size,\n'.format(sm))
    fc.write('    .state_table = sm_{}_states_table,\n'.format(sm))
    fc.write('    .state_table_size = sm_{}_states_table_size,\n'.format(sm))
    fc.write('};\n')

def gen_sm_file(sm):
    h_filename = get_gen_dir() + "/{}_state_machine.h".format(sm)
    c_filename = get_gen_dir() + "/{}_state_machine.c".format(sm)
    log(" -- GEN State Machine Files @ ", 'green')
    log(c_filename, 'yellow')
    log(' , ')
    logl(h_filename, 'yellow')

    fc = open(get_gen_dir() + "/" + c_filename, "w")
    fh = open(get_gen_dir() + "/" + h_filename, "w")

    write_header(fc, '', '-')
    write_header(fc, 'auto-generated state machine file', ' ')
    write_header(fc, '', '-')

    write_ifdefs_start(fc, sm)
    
    fc.write('#include "state_machine.h"\n')
    fc.write('#include "{}_state_machine.h"\n'.format(sm))
    fc.write('\n')

    write_header(fh, '', '-')
    write_header(fh, 'auto-generated state machine file', ' ')
    write_header(fh, '', '-')


    fh.write('#ifndef {}'.format('__{}_state_machine_h__\n'.format(sm).upper()))
    fh.write('#define {}'.format('__{}_state_machine_h__\n'.format(sm).upper()))
    fh.write('\n')

    write_ifdefs_start(fh, sm)

    write_header(fh, 'includes', '-')
    fh.write('#include "state_machine.h"\n')
    fh.write('\n')

    write_methods_declarations(sm, fc, fh)
    fc.write('\n')
    write_inputs_declarations(sm, fc, fh)
    fc.write('\n')
    write_actions_declarations(sm, fc, fh)
    fc.write('\n')
    write_states_declarations(sm, fc, fh)
    fc.write('\n')
    write_machine_declaration(sm, fc, fh)
    fc.write('\n')
    write_header(fc, 'end of file', '-')
    write_header(fh, 'end of file', '-')

    write_ifdefs_end(fc, sm)
    write_ifdefs_end(fh, sm)

    fh.write('#endif /* {} */\n'.\
             format('__{}_state_machine_h__'.format(sm).upper()))
    fc.close()
    fh.close()

# --- main subroutine -------------------------------------------------------- #

def main():

    logl(" -- executing the state machine file generator", 'green')
    user_cmd_check()

    for file in get_filenames():
        filter_new_file_contents(file)
    
    sms = get_state_machines()
    for sm in sms:
        check_sm_state_trans(sm)

    for sm in sms:
        gen_sm_file(sm)

main()

# --- end of file ------------------------------------------------------------ #
