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
# Desc      It represents a pretty command line parser.
# ---------------------------------------------------------------------------- #

import os
import sys
import re
from pylog import *
from pytext import *

class PyCli:

    def __init__(self, caption=None, help=None, help_width=100) -> None:
        self.__args = []
        self.__pos_args = []
        self.__script_name = os.path.basename(sys.argv[0])
        self.__groups = ['default']
        self.__caption = caption
        self.__help = help
        self.__help_width = help_width
        pass

    def __add_help_flag(self):
        if not self.__get_arg('help'):
            self.add_flag('help', group= 'default',
                help='displays this help')

    def __trim_name(self, arg_name):
        return arg_name.replace('--', "", 1)
    
    def __get_arg(self, arg_name):
        for arg in self.__args + self.__pos_args:
            if arg['name'] == arg_name:
                return arg
        return None

    def __check_arg(self, arg_name):
        for arg in self.__args + self.__pos_args:
            if arg['name'] == arg_name:
                log(f'argument "{arg_name}" is duplicated',
                        color=RED)
                exit(1)
    def __group_add(self, group):
        if not group in self.__groups:
            self.__groups.append(group)
    
    def __group_args(self, group) -> list:
        args = []
        for arg in self.__pos_args + self.__args:
            if 'group' in arg and arg['group'] == group:
                args.append(arg)
        return args

    def add_flag(self, name, help=None, group='default'):
        self.__check_arg(name)
        self.__group_add(group)
        self.__args.append({
            'name' : self.__trim_name(name),
            'type' : 'flag',
            'required' : False,
            'default' : False,
            'group' : group,
            'help': help})
        pass

    def add_arg(self, name, default=None, required=False, nargs=1,
                help=None, group='default'):
        self.__check_arg(name)
        self.__group_add(group)
        self.__args.append({
            'name' : self.__trim_name(name),
            'type' : 'arg',
            'default' : default,
            'required' : required,
            'group' : group,
            'nargs' : nargs,
            'help': help
            })
        pass

    def add_opt(self, name, options, default=None, required=False, nargs=1,
                help=None, group='default', help_opts=None):
        self.__check_arg(name)
        self.__group_add(group)
        self.__args.append({
            'name' : self.__trim_name(name),
            'type' : 'opt',
            'opts' : options,
            'default' : default,
            'required' : required,
            'group' : group,
            'nargs' : nargs,
            'help': help,
            'help_opts': help_opts
            })
        pass

    def add_var_int(self, name, default=None, required=False, help=None
                    , group='default'):
        self.__check_arg(name)
        self.__group_add(group)
        self.__args.append({
            'name' : self.__trim_name(name),
            'type' : 'var_int',
            'default' : default,
            'required' : required,
            'group' : group,
            'help': help
            })
        pass

    def add_var_str(self, name, default=None, required=False, help=None
                    , group='default'):
        self.__check_arg(name)
        self.__group_add(group)
        self.__args.append({
            'name' : self.__trim_name(name),
            'type' : 'var_str',
            'default' : default,
            'required' : required,
            'group' : group,
            'help': help
            })
        pass

    def add_var_opt(self, name, opts, default=None, required=False, help=None,
                    group='default', help_opts=None):
        self.__check_arg(name)
        self.__group_add(group)
        self.__args.append({
            'name' : self.__trim_name(name),
            'type' : 'var_opt',
            'opts' : opts,
            'default' : default,
            'required' : required,
            'group' : group,
            'help': help,
            'help_opts': help_opts,
            })
        pass

    def add_pos_arg(self, name, opts=None, default=None, help=None,
                    group='default', help_opts=None):
        self.__check_arg(name)
        self.__group_add(group)
        self.__pos_args.append({
            'name' : self.__trim_name(name),
            'type' : 'pos',
            'opts' : opts,
            'default': default,
            'required': True if not default else False,
            'group' : group,
            'nargs' : 1,
            'help': help,
            'help_opts': help_opts
            })
        pass

    def __get_arg_str(self, arg, opt_color=CYAN, sym_color=GREEN,
                    default_color=GREY):
        lb = ''
        rb = ''
        if arg['type'] == 'pos':
            name_str = ''
        elif arg['type'].startswith('var_'):
            name_str = f'{arg["name"]}='
        else:
            name_str = f'--{arg["name"]}'

        if 'required' in arg and arg['required'] == False:
            lb = '['
            rb = ']'

        if arg['type'] == 'pos':
            rep_str = arg["name"].upper()
        elif 'nargs' in arg:
            arg_cap_name = arg['name'].upper()
            if arg['nargs'] == None or arg['nargs'] == 1:
                rep_str = ' ' + arg_cap_name
            elif type(arg['nargs']) == int:
                rep_str = " {}{}{}{}".format(arg_cap_name,
                                                '...{', arg["nargs"], '}')
            elif arg['nargs'] == '+':
                rep_str = f' {arg_cap_name}...'
            else:
                rep_str = ''
        elif arg['type'] == 'var_int':
            rep_str = f'<integer>'
        elif arg['type'] == 'var_str':
            rep_str = f'<string>'
        elif arg['type'] == 'var_opt':
            rep_str = f'<option>'
        else:
            rep_str = ''

        if opt_color:
            name_str = log_get_color_str(opt_color) + name_str
        if sym_color:
            rep_str = log_get_color_str(sym_color) + rep_str
        color = log_get_color_str(default_color)
        return f'{color}{lb}{name_str}{rep_str}{color}{rb}'

    def show_help(self):
        self.__add_help_flag()


        def get_max_arg_name_width():
            max_len = 0
            for arg in self.__args + self.__pos_args:
                arg_len = get_colored_str_len(self.__get_arg_str(arg))
                if arg_len > max_len:
                    max_len = arg_len
            return max_len
        def get_max_arg_opts_width(arg):
            max_len = 0
            default = arg['default']
            for opt in arg['opts']:
                opt_len = len(opt) + \
                    (len('**') if opt == default else 0)
                if opt_len > max_len:
                    max_len = opt_len
            return max_len
        tot_w = self.__help_width
        log_line(tot_w)
        if self.__caption:
            log_field(self.__caption, width=tot_w, align=CENTER,
                            color=YELLOW, endl=True)
        if self.__help:
            log(wrap(self.__help, tot_w, html_br_only=True))
            log('')
        log_underlined("synopsis:")

        log_tab_inc()

        cmd_name_w = len(self.__script_name) + 4
        cmd_opts_w = tot_w - cmd_name_w - log_tab_size()
        log_field(self.__script_name, width=cmd_name_w, color=YELLOW)
        group_args_str = ''
        group_args_str_len = 0
        for group in self.__groups:
            group_args = self.__group_args(group)
            space_len = 0
            for arg in group_args:
                s = ' ' * space_len + self.__get_arg_str(arg)
                s_len = get_colored_str_len(s) + space_len
                if group_args_str_len + s_len > cmd_opts_w:
                    log(group_args_str)
                    group_args_str = ' ' * cmd_name_w + s
                    group_args_str_len = s_len
                else:
                    group_args_str += s
                    group_args_str_len += s_len
                space_len = 1
            log(group_args_str)
            group_args_str = ' ' * cmd_name_w
            group_args_str_len = 0

        # field_w = get_max_arg_name_width() + 2
        field_w = 15
        help_indent = field_w + log_tab_size()
        help_w = tot_w - help_indent
        for group in self.__groups:
            group_args = self.__group_args(group)
            if len(group_args) == 0:
                continue
            log('')
            log_decorated_header(group, width=tot_w - log_tab_size(),
                caption_color=PURPLE)
            for arg in group_args:
                log('')
                arg_str = self.__get_arg_str(arg)
                if get_colored_str_len(arg_str) + 2 > field_w:
                    log(self.__get_arg_str(arg))
                    log_field('', field_w)
                else:
                    log_field(self.__get_arg_str(arg), field_w)
                if 'help' in arg and arg['help'] != None:
                    log(wrap(arg['help'],
                        width=help_w, next_indent=help_indent,
                        html_br_only=True))
                else:
                    log('-')
                if 'opts' in arg and arg['opts'] != None:
                    new_list = []
                    for opt in arg['opts']:
                        if opt == arg['default']:
                            new_list.append(
                                f'{COLOR_YELLOW}*{COLOR_CYAN}{opt}' +
                                f'{COLOR_YELLOW}*')
                        else:
                            new_list.append(opt)
                    log_field('', field_w)
                    log_decorated_header(arg['name'].upper(),
                        width=help_w, caption_color=GREEN)
                    item_field_w = get_max_arg_opts_width(arg) + 2
                    if 'help_opts' in arg and arg['help_opts']:
                        indent_item = log_tab_size() + field_w + 2
                        help_opt_indent = indent_item + item_field_w
                        help_field_w= tot_w - help_opt_indent
                        idx = 0
                        for opt in arg['opts']:
                            log_field('', indent_item - log_tab_size())
                            log_field(new_list[idx], item_field_w, color=CYAN)
                            idx += 1
                            if opt in arg['help_opts']:
                                log(wrap(arg['help_opts'][opt],
                                    width=help_field_w,
                                    next_indent=help_opt_indent,
                                    html_br_only=True))
                            else:
                                log('-')
                    else:
                        list_tot_w=tot_w-(field_w+log_tab_size()+2)
                        row_items=list_tot_w // item_field_w
                        item_w = list_tot_w // row_items
                        log_list(new_list, horiz=True, indent=field_w+2,
                            wrap_after=row_items, color=CYAN,
                            item_width=item_w)
        log_tab_dec()

        log('')

        log_line(tot_w)
        pass

    def parse(self) -> dict:
        self.__add_help_flag()
        results = {}
        cli_opts = {}
        def init_default_values():
            for arg in self.__pos_args + self.__args:
                if 'default' in arg and arg['default'] != None:
                    if 'nargs' in arg:
                        if arg['nargs'] == 1:
                            results[arg['name']] = arg['default']
                        else:
                            results[arg['name']] = [arg['default']]
                    else:
                        results[arg['name']] = arg['default']
        init_default_values()

        def parse_token(token) -> tuple:
            res = re.match(r'^--(\w[\w\-]*)$', token)
            if res:
                return ('arg', res[1])
            res = re.match(r'^(\w[\w\d]*)=(.+)$', token)
            if res:
                var_val = int(res[2]) if res[2].isdigit() else res[2]
                return ('var', res[1], var_val)
            return ('pos', token)
        def get_arg_desc(arg_name) -> dict:
            for arg in self.__args:
                if arg['name'] == arg_name and \
                    arg['type'] in ['flag', 'arg', 'opt']:
                    return arg
            return None
        def get_var_desc(var_name) -> dict:
            for arg in self.__args:
                if arg['name'] == var_name and \
                    arg['type'] in ['var_int', 'var_str', 'var_opt']:
                    return arg
            return None
        pos_args_counter = 0
        def process_pos_arg(pos_val) -> bool:
            nonlocal pos_args_counter
            counter = 0
            for pos_arg in self.__pos_args:
                if counter < pos_args_counter:
                    counter += 1
                    continue
                if pos_arg['opts'] != None:
                    if pos_val in pos_arg['opts']:
                        results[pos_arg['name']] = pos_val
                        pos_args_counter += 1
                        return True
                    elif pos_arg['default'] != None:
                        results[pos_arg['name']] = pos_arg['default']
                        pos_args_counter += 1
                    else:
                        return False
                else:
                    results[pos_arg['name']] = pos_val
                    pos_args_counter += 1
                    return True
            return False

        errors = []
        def process_error(err_msg):
            nonlocal errors
            errors.append(err_msg)

        def add_to_results(arg, val):
            if arg['name'] in cli_opts:
                process_error(f'argument "{arg["name"]}" is specified before')
            else:
                if arg['type'] in ['arg', 'opt']:
                    if arg['nargs'] == 1:
                        results[arg['name']] = val[0]
                    else:
                        results[arg['name']] = val
                else:
                    results[arg['name']] = val
                cli_opts[arg['name']] = True
            pass

        def process_var_arg(var, val):
            var_arg = get_var_desc(var)
            if not var_arg and \
                not process_pos_arg(f'{var}={val}'):
                process_error(
                    f'unknown argument {var}={val}')
                return
            else:
                arg_type = type(tok_lex[2])
                if var_arg['type'] == 'var_int' and arg_type != int:
                    process_error(f'variable "{var}" of type <int>' +
                        f' but found <{arg_type}> "{val}"')
                    return
                elif var_arg['type'] == 'var_str' and arg_type != str:
                    process_error(f'variable "{var}" of type <str>' +
                        f' but found <{arg_type}> "{val}"')
                    return
                elif var_arg['type'] == 'var_opt' and \
                        not val in var_arg['opts']:
                    process_error(f'variable "{var}" op type <opt>' +
                        f' but given not allowed opt "{val}"')
                    return
                else:
                    add_to_results(var_arg, val)


        __IDLE          = 0
        __WAIT_N_ARGS   = 1
        __WAIT_OPT_ARG  = 2
        __ERROR         = 3

        state = __IDLE

        curr_arg = None
        curr_arg_results = []
        def process_state_machine_end():
            if state != __IDLE and curr_arg:
                if len(curr_arg_results) == 0:
                    process_error(f'missing argument {curr_arg["name"]} value')
                else:
                    add_to_results(curr_arg, curr_arg_results)

        def process_state_machine(tok_lex):
            # tok_lex   = ( 'arg', <arg-name> )
            #           = ( 'var', <var-name>, <var_val> )
            #           = ( 'pos', <pos-val> )
            nonlocal state
            nonlocal curr_arg
            nonlocal curr_arg_results
            if state == __IDLE:
                if tok_lex[0] == 'arg':
                    curr_arg = get_arg_desc(tok_lex[1])
                    if not curr_arg:
                        process_error(f'unknown argument --{tok_lex[1]}')
                        return
                    if curr_arg['type'] == 'arg':
                        curr_arg_results = []
                        state = __WAIT_N_ARGS
                    elif curr_arg['type'] == 'opt':
                        state = __WAIT_OPT_ARG
                        curr_arg_results = []
                    elif curr_arg['type'] == 'flag':
                        add_to_results(curr_arg, True)
                    else:
                        pass

                elif tok_lex[0] == 'var':
                    process_var_arg(tok_lex[1], tok_lex[2])
                    pass
                elif tok_lex[0] == 'pos':
                    if not process_pos_arg(tok_lex[1]):
                        process_error(f'unknown pos arg "{tok_lex[1]}"')
                        return
                    pass
                else:
                    pass
                pass
            elif state == __WAIT_N_ARGS:
                # tok_lex   = ( 'arg', <arg-name> )
                #           = ( 'var', <var-name>, <var_val> )
                #           = ( 'pos', <pos-val> )
                new_arg = get_arg_desc(tok_lex[1])
                new_var = get_var_desc(tok_lex[1])
                if (tok_lex[0] == 'arg' and new_arg) or \
                    (tok_lex[0] == 'var' and new_var):
                    if len(curr_arg_results) > 0:
                        add_to_results(curr_arg, curr_arg_results)
                    else:
                        process_error(f'the argument "{curr_arg["name"]}" ' + 
                                      'has missing value')
                    curr_arg = new_arg
                    curr_arg_results = []
                    if new_var:
                        process_var_arg(tok_lex[1], tok_lex[2])
                        state = __IDLE
                    if curr_arg:
                        if curr_arg['type'] == 'arg':
                            pass
                        elif curr_arg['type'] == 'opt':
                            state = __WAIT_OPT_ARG
                        elif curr_arg['type'] == 'flag':
                            add_to_results(curr_arg, True)
                            state = __IDLE
                        else:
                            pass
                else:
                    if tok_lex[0] == 'arg':
                        tt = f'--{tok_lex[1]}'
                    elif tok_lex[0] == 'var':
                        if len(f'{tok_lex[2]}'.split()) > 1:
                            tt = f'{tok_lex[1]}="{tok_lex[2]}"'
                        else:
                            tt = f'{tok_lex[1]}={tok_lex[2]}'
                    else:
                        tt = tok_lex[1]
                    curr_arg_results.append(tt)
                    if curr_arg['nargs'] != '+' and \
                        curr_arg['nargs'] == len(curr_arg_results):
                        add_to_results(curr_arg, curr_arg_results)
                        curr_arg = None
                        curr_arg_results = []
                        state = __IDLE
                    pass
                pass
            elif state == __WAIT_OPT_ARG:
                # tok_lex   = ( 'arg', <arg-name> )
                #           = ( 'var', <var-name>, <var_val> )
                #           = ( 'pos', <pos-val> )
                new_arg = get_arg_desc(tok_lex[1])
                new_var = get_var_desc(tok_lex[1])
                if (tok_lex[0] == 'arg' and new_arg) or \
                    (tok_lex[0] == 'var' and new_var):
                    if len(curr_arg_results) > 0:
                        add_to_results(curr_arg, curr_arg_results)
                    else:
                        process_error(f'the argument "{curr_arg["name"]}" ' + 
                                      'has missing value')
                    curr_arg = new_arg
                    curr_arg_results = []
                    if new_var:
                        process_var_arg(tok_lex[1], tok_lex[2])
                        state = __IDLE
                    if curr_arg:
                        if curr_arg['type'] == 'arg':
                            state = __WAIT_N_ARGS
                            pass
                        elif curr_arg['type'] == 'opt':
                            pass
                        elif curr_arg['type'] == 'flag':
                            add_to_results(curr_arg, True)
                            state = __IDLE
                        else:
                            pass
                else:
                    if curr_arg['opts'] == None:
                        process_error(f'the argument "{curr_arg["name"]}" ' + 
                            f'has no options: discard {tok_lex[1]}')
                    elif not tok_lex[1] in curr_arg['opts']:
                        process_error(f'the argument "{curr_arg["name"]}" ' + 
                                      f'has wrong option value {tok_lex[1]}')
                        if not process_pos_arg(tok_lex[1]):
                            process_error(f'unknown argument {tok_lex[1]}')
                        state = __IDLE
                    else:
                        curr_arg_results.append(tok_lex[1])
                        if curr_arg['nargs'] != '+' and \
                            curr_arg['nargs'] == len(curr_arg_results):
                            add_to_results(curr_arg, curr_arg_results)
                            curr_arg = None
                            curr_arg_results = []
                            state = __IDLE
                    pass
                pass
            elif state == __ERROR:
                pass

        for token in sys.argv[1:]:
            tok_lex = parse_token(token)
            # log(f' -- cli-word: {token} == {tok_lex}')
            process_state_machine(tok_lex)
        process_state_machine_end()

        for arg in self.__args + self.__pos_args:
            if arg['required'] and not arg['name'] in results:
                process_error('missing required argument ' +
                    self.__get_arg_str(arg))

        if len(errors) > 0:
            log_list(errors, color=RED, numbering=True, numbering_color=BLUE)

        if results['help'] == True:
            self.show_help()
            exit(0)
        
        if len(errors) > 0:
            exit(1)

        return results

# --- end of file ------------------------------------------------------------ #
