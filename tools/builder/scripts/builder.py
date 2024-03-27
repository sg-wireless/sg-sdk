#!/usr/bin/env python3
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
# Desc      The main builder script
# ---------------------------------------------------------------------------- #

import subprocess
import sys
import os
import toml
from pathlib import Path
import serial
import fcntl
sys.path.append(f'{os.path.dirname(__file__)}/../../pylibs')
from pylog import *
import pycli

# ---------------------------------------------------------------------------- #
# Tree Structure
# ---------------------------------------------------------------------------- #
class TreeStruct:
    __root_dir = os.path.abspath(f'{os.path.dirname(sys.argv[0])}/../../..')
    __platforms_dir = f'{__root_dir}/src/platforms'
    __platforms_info = {}
    def __init__(self) -> None:
        first = True
        if not os.path.exists(self.__platforms_dir):
            return
        for p in sorted(os.listdir(self.__platforms_dir)):
            if os.path.isdir(f'{self.__platforms_dir}/{p}'):
                self.__platforms_info[p] = {'boards' : []}
                boards_list = []
                board_dir = f'{self.__platforms_dir}/{p}/boards'
                if os.path.exists(board_dir):
                    for f in sorted(os.listdir(board_dir)):
                        if os.path.isfile(f'{board_dir}/{f}') \
                            and f.endswith('.toml'):
                            boards_list.append(f.removesuffix('.toml'))
                self.__platforms_info[p]['boards'] = boards_list
        pass

    def get_platform_config_file(self, platform) -> str:
        cfg_file = f'{self.__platforms_dir}/{platform}/platform_config.toml'
        if platform in self.__platforms_info and os.path.isfile(cfg_file):
            return cfg_file
        return None

    def get_platforms_names(self) -> list:
        p_names = []
        for p_info in self.__platforms_info:
            p_names.append(p_info)
        return p_names

    def get_platform_dir(self, platform) -> str:
        if platform in self.__platforms_info:
            return f'{self.__platforms_dir}/{platform}'
        return None

    def get_boards_dir(self, platform) -> str:
        if platform in  self.__platforms_info:
            board_dir = f'{self.__platforms_dir}/{platform}/boards'
            if os.path.exists(board_dir):
                return board_dir
        return None

    def get_board_config_file(self, board):
        platform = self.get_platform_name(board)
        if platform:
            return f'{self.__platforms_dir}/{platform}/boards/{board}.toml'
        return None

    def get_boards_names(self) -> list:
        ret_list = []
        for platform in self.__platforms_info:
            ret_list += self.__platforms_info[platform]['boards']
        if len(ret_list) > 0:
            return ret_list
        return None

    def get_platform_name(self, board) -> str:
        for platform in self.__platforms_info:
            if board in self.__platforms_info[platform]['boards']:
                return platform
        return None

    def get_build_dir(self, app, platform, board, variant):
        if app == None:
            app = "sdk-default"
        dir = f'{self.__root_dir}/build/{app}/{platform}/{board}/{variant}'
        if not os.path.exists(dir):
            os.makedirs(dir)
            pass
        return dir

    def get_config_generation_dir(self, app, platform, board, variant):
        dir = self.get_build_dir(app, platform, board, variant) + \
                '/generated-configs'
        if not os.path.exists(dir):
            os.makedirs(dir)
            pass
        return dir

    def get_app_board_config_file(self, app_dir):
        cfg_file = f'{app_dir}/board_config.toml'
        if app_dir and os.path.isdir(app_dir) \
            and os.path.isfile(cfg_file):
            return cfg_file
        return None

    def get_submodule_path(self, submodule):
        return f'{self.__root_dir}/ext/{submodule}'

    def get_ext_path(self):
        return f'{self.__root_dir}/ext'
    
    def export_tree_paths(self, platform):
        os.environ["__tree_dir_root"]     = self.__root_dir    
        os.environ["__tree_dir_ext"]      = f'{self.__root_dir}/ext'    
        os.environ["__tree_dir_src"]      = f'{self.__root_dir}/src'
        os.environ["__tree_dir_platform"] = f'{self.__platforms_dir}/{platform}'
        os.environ["__tree_dir_libs"]     = f'{self.__root_dir}/src/libs'
        os.environ["__tree_dir_drivers"]  = f'{self.__root_dir}/src/drivers'
        os.environ["__tree_dir_tools"]    = f'{self.__root_dir}/tools'
        os.environ["__tree_dir_builder"]  = f'{self.__root_dir}/tools/builder'


# ---------------------------------------------------------------------------- #
# Builder CLI
# ---------------------------------------------------------------------------- #
class BuilderCli:
    def __init__(self, tree:TreeStruct) -> None:

        cli = pycli.PyCli(
            caption="The Firmware SDK Builder CLI",
            help_width=110,
            help=f'''<br>{COLOR_GREEN}
                This is the main build system command line interface for the SDK
                build system.
                <br><br>
                The available options are grouped mainly in three categories:
                </indent><br>
                * {FORMAT_UNDERLINED}hardware options{FORMAT_CLEAR}
                {COLOR_GREEN}</indent>
                the required options for platform and board selection.
                </dndent><br>
                * {FORMAT_UNDERLINED}software options{FORMAT_CLEAR}
                {COLOR_GREEN}</indent>
                the required options to select certain build variants and
                specifying the user project main directory.
                </dndent>
                <br>
                * {FORMAT_UNDERLINED}build system options{FORMAT_CLEAR}
                {COLOR_GREEN}
                the actual build commands such as build and flash
                {COLOR_DEFAULT}
                </dndent>''')

        self.__cli = cli
        self.__tree = tree

        # -- hardware build variants
        group_hardware_options = 'hardware options'

        cli.add_opt( 'board'
            , required  = True
            , options   = tree.get_boards_names()
            , group     = group_hardware_options
            , help      = f''' The target hardware platform board.'''
        )

        # -- hardware build variants
        group_software_options = 'software options'

        cli.add_opt( 'variant'
            , default   = 'micropython'
            , group     = group_software_options
            , required  = False
            , options   = ['micropython', 'native']
            , help      = f'''The software build variant.'''
            , help_opts = {
                  'micropython' : '''build the firmware with micropython
                                    support'''
                , 'native'      : '''build the firmware for C/C++ application
                                    without micropython stuff'''
            }
        )

        cli.add_arg( 'defs'
            , nargs     = '+'
            , group     = group_software_options
            , help      = f'''To direct a special definitions to both the
                            build system and the compiler as well.
                            <br>
                            {COLOR_GREY}Example: {COLOR_BLUE}
                            --defs SOME_DEF_VAR="one two" SOME_COUNTER=1
                            ''')

        # -- hardware build variants
        group_build_options = 'build system options'
        cli.add_pos_arg( 'command'
            , opts      = [
                            'build',
                            'clean',
                            'flash',
                            'erase',
                            'config'
                            ]
            , group     = group_build_options
            , default   = 'build'
            , help      = f'''The main build command target.'''
            , help_opts = {
                  'build'   : '''To only build the firmware image'''
                , 'clean'   : '''To clean the built artifacts'''
                , 'flash'   : f'''To build and flash the connected board(s)
                                '''
                , 'erase'   : f'''To erase the whole flash of the connected
                                board(s)
                                '''
                , 'config'  : f'''To open the configuration menu. It is the same
                                menu that is used in the Linux kernel
                                configuration'''
            }
        )

        cli.add_arg( 'port'
            , nargs     = '+'
            , required  = False
            , group     = group_build_options
            , help      = f'''The Port to be used for flashing.
                            Specifying more than one port is allowed
                            <br>
                            {COLOR_RED}@note{COLOR_DEFAULT} This option must be
                            given along with the {COLOR_CYAN}flash
                            {COLOR_DEFAULT} and {COLOR_CYAN}erase
                            {COLOR_DEFAULT}commands
                            '''
        )

        cli.add_arg( 'project-dir'
            , nargs     = 1
            , required  = False
            , group     = group_build_options
            , help      = f'''To specify the user/example specific project
                            folder'''
        )

        cli.add_arg( 'custom-version-string'
            , nargs     = 1
            , required  = False
            , group     = group_build_options
            , help      = f'''To specify a custom string to be used in firmware
                            build versioning'''
        )

        self.__options = cli.parse()
        self.__check_arguments_sanity()
        # log(self.__options)

    def __check_arguments_sanity(self):
        sanity_failed = False
        project_dir = self.get_project_dir()
        if project_dir != None:
            if not os.path.isdir(project_dir):
                log(f'-- error: dir "{project_dir}" does not exist', RED)
                sanity_failed = True
            else:
                self.__options['project-dir'] = os.path.abspath(project_dir)
        ports = self.get_ports()
        if ports:
            for p in ports:
                try:
                    ser = serial.Serial(p, 115200)
                    try:
                        fcntl.flock(ser.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
                    except IOError:
                        log(f'-- error: port {COLOR_CYAN}{p}{COLOR_RED} ' + 
                            'is busy', RED)
                        sanity_failed = True
                    ser.close()

                except IOError:
                    log(f'-- error: port {COLOR_CYAN}{p}{COLOR_RED}' +
                        ' is incorrect or in use', RED)
                    sanity_failed = True
        command = self.get_build_command()
        if command in ['flash', 'erase'] and ports == None:
            log(f'-- error: port must be specified for {command} command', RED)
            sanity_failed = True
        if sanity_failed:
            exit(1)

    def get_build_command(self):
        return self.__options['command']

    def get_ports(self):
        if 'port' in self.__options:
            return self.__options['port']
        return None

    def get_defs(self):
        if 'defs' in self.__options:
            return self.__options['defs']
        return None

    def get_variant(self):
        if 'variant' in self.__options:
            return self.__options['variant']
        return None

    def get_board(self):
        if 'board' in self.__options:
            return self.__options['board']
        return None

    def get_project_dir(self):
        if 'project-dir' in self.__options:
            return self.__options['project-dir']
        return None

    def get_custom_version_string(self):
        if 'custom-version-string' in self.__options:
            return self.__options['custom-version-string']
        return None

    def __get_max_opt_str_len(self):
        max_len = 0
        for opt in self.__options:
            opt_len = get_colored_str_len(opt)
            if max_len < opt_len:
                max_len = opt_len
        return max_len
    def show_options(self):
        opt_name_w = self.__get_max_opt_str_len() + 2
        for opt in self.__options:
            log_field(opt, width=opt_name_w,
                color = CYAN)
            if type(self.__options[opt]) == list:
                log_list(self.__options[opt], first_indent=0,
                    indent=opt_name_w)
            else:
                log(self.__options[opt])
        pass

# ---------------------------------------------------------------------------- #
# Configurator class
# ---------------------------------------------------------------------------- #
class ConfigsMgr:

    def __init__(self, app_cfg_file, board_cfg_file, platform_cfg_file) -> None:
        log('-- init ConfigsMgr instance')
        log(f'-- app config file:       {COLOR_BLUE}{app_cfg_file}')
        log(f'-- board config file:     {COLOR_BLUE}{board_cfg_file}')
        log(f'-- platform config file:  {COLOR_BLUE}{platform_cfg_file}')
                
        self.__app_configs = None
        self.__board_configs = None
        self.__platform_configs = None
        if app_cfg_file:
            with open(app_cfg_file, 'r') as f:
                self.__app_configs = toml.loads(f.read())
        if board_cfg_file:
            with open(board_cfg_file, 'r') as f:
                self.__board_configs = toml.loads(f.read())
        if platform_cfg_file:
            with open(platform_cfg_file, 'r') as f:
                self.__platform_configs = toml.loads(f.read())
        pass

    def get_config(self, config_key, merge=False):
        """This function return the highest priority config
        the priority is as follows:
            [1]app-config -> [2]board-config -> [3]platform-config
        merge option causes the dict to be merged in priority manar as well
        and for list to be concatenated
        """
        keys = config_key.split('.')
        p1 = self.__app_configs
        p2 = self.__board_configs
        p3 = self.__platform_configs
        for k in keys:
            p1 = p1[k] if p1 and k in p1 else None
            p2 = p2[k] if p2 and k in p2 else None
            p3 = p3[k] if p3 and k in p3 else None
        # log(f'== get_config {COLOR_CYAN}{config_key}{COLOR_DEFAULT} ==')
        # log(f'[{COLOR_GREEN}app{COLOR_DEFAULT}     ] {p1}')
        # log(f'[{COLOR_GREEN}board{COLOR_DEFAULT}   ] {p2}')
        # log(f'[{COLOR_GREEN}platform{COLOR_DEFAULT}] {p3}')

        def __check_all(t) -> bool:
            nonlocal p1, p2, p3
            if p1 and type(p1) != t:
                return False
            if p2 and type(p2) != t:
                return False
            if p3 and type(p3) != t:
                return False
            return True
        if merge:
            if __check_all(dict):
                collect = {}
                all_keys = (list(p1.keys()) if p1 else [])\
                         + (list(p2.keys()) if p2 else [])\
                         + (list(p3.keys()) if p3 else [])
                for k in all_keys:
                    if p1 and k in p1:
                        collect[k] = p1[k]
                    elif p2 and k in p2:
                        collect[k] = p2[k]
                    elif p3 and k in p3:
                        collect[k] = p3[k]
                return collect
            elif __check_all(list):
                collect = []
                if p1:
                    collect = p1
                if p2:
                    for e in p2:
                        if not e in collect:
                            collect.append(e)
                if p3:
                    for e in p3:
                        if not e in collect:
                            collect.append(e)
                return collect
            pass

        if p1 != None:
            return p1
        if p2 != None:
            return p2
        if p3 != None:
            return p3
        return None

class BuilderContext:
    tree: TreeStruct = None
    cli: BuilderCli = None
    cfg: ConfigsMgr = None

    def __init__(self, tree:TreeStruct, cli:BuilderCli, cfg:ConfigsMgr) -> None:
        self.tree = tree
        self.cli = cli
        self.cfg = cfg
        pass

# ---------------------------------------------------------------------------- #
# pre-build system invocation preparation (submodules, tools installations)
# ---------------------------------------------------------------------------- #
def __run_subprocess(cmd_list, capture_output=True):
    try:
        ret = subprocess.run(cmd_list, capture_output=capture_output)
    except subprocess.CalledProcessError as e:
        ret = e
    return ret

def process_git_submodules(ctx: BuilderContext):

    features = ctx.cfg.get_config("features", merge=True)
    submodules = ctx.cfg.get_config("submodules.default", merge=True)
    submodules.extend(ctx.cfg.get_config(
        f"submodules.variant.{ctx.cli.get_variant()}", merge=True))
    for feat in features:
        if features[feat]:
            submodules.extend(ctx.cfg.get_config(
                f"submodules.features.{feat}", merge=True))
    # log_obj(features)
    # log_obj(submodules)

    for submodule in submodules:
        log(f'-- check submodule {COLOR_YELLOW}{submodule}{COLOR_DEFAULT}')
        path = ctx.tree.get_submodule_path(submodule)
        ret = __run_subprocess(['git', 'submodule', 'status', path])

        if ret.returncode != 0:
            log(f'error({ret.returncode}): ' + \
                      f'{COLOR_RED}{ret.stderr.decode("utf-8")}{COLOR_DEFAULT}')
            exit(1)

        if ret.stdout.decode('utf-8').startswith('-'):
            log(f'   submodule {submodule} may be not fetched or deleted')
            log(f'{COLOR_CYAN}update submodule: {submodule}{COLOR_DEFAULT}')
        
        ret = __run_subprocess(['git', 'submodule', 'update', '--init', path])
        if ret.returncode != 0:
            log(f'error({ret.returncode}): ' + \
                      f'{COLOR_RED}{ret.stderr.decode("utf-8")}{COLOR_DEFAULT}')
            exit(1)
    pass

def process_prerequisite_tools(ctx: BuilderContext):
    tools = ctx.cfg.get_config("tools.default", merge=True)

    for tool in tools:
        if tool == 'esp-idf':
            process_esp_idf_installation(ctx.tree)
        else:
            log(f'-- tool install not supported -> {COLOR_CYAN}{tool}', RED)
    pass

def process_esp_idf_installation(tree: TreeStruct):
    def shell_source(script):
        # source    https://stackoverflow.com/questions/7040592
        #           calling-the-source-command-from-subprocess-popen
        """Sometime you want to emulate the action of "source" in bash,
        settings some environment variables. Here is a way to do it."""
        pipe = subprocess.Popen(f'. {script} >/dev/null 2>&1 && env -0'
                , stdout=subprocess.PIPE, shell=True)
        output = pipe.communicate()[0]
        for line in output.decode('utf-8').split('\0'):
            line = line.split('=', 1)
            if len(line) > 1:
                os.environ.update({line[0]:line[1]})
        return pipe.returncode == 0

    def esp_idf_install(tree: TreeStruct):
        esp_idf_path = tree.get_submodule_path('esp-idf')
        install_script = f'{esp_idf_path}/install.sh'
        try:
            ret = subprocess.run([install_script])
        except subprocess.CalledProcessError as e:
            ret = e
        if ret.returncode != 0:
            log(f'error ({ret.returncode}) occurs during esp-idf installation')
            log(f'please check manually this issue')
            exit(1)
        log('esp-idf successfully installed!')
        pass

    log('-- setting esp-idf envirnment')

    esp_idf_path = tree.get_submodule_path('esp-idf')
    log(f'-- set IDF_PATH = {esp_idf_path}')
    os.environ.update({"IDF_PATH": esp_idf_path})

    export_filename = f'{esp_idf_path}/export.sh'
    log(f'-- source {export_filename}')
    if not os.path.exists(export_filename):
        log('-- esp-idf/export.sh file does not exist, please check')
        exit(1)

    if not shell_source(export_filename):
        log('-- esp-idf SDK might not be installed!, try installation', RED)

        esp_idf_install(tree)

        if not shell_source(export_filename):
            log('esp-idf environment set failed after installation!' +
                ' please check manually!', RED)
            exit(1)
        pass
    pass

# ---------------------------------------------------------------------------- #
# main routine
# ---------------------------------------------------------------------------- #

def create_config_mgr_obj(tree: TreeStruct, cli: BuilderCli) ->  object:
    board = cli.get_board()
    platform = tree.get_platform_name(board)

    # config files in prio order ( app(user) -> board -> platform )
    app_cfg_file = tree.get_app_board_config_file(cli.get_project_dir())
    board_cfg_file = tree.get_board_config_file(board)
    platform_cfg_file = tree.get_platform_config_file(platform)

    return ConfigsMgr(app_cfg_file, board_cfg_file, platform_cfg_file)

def main():

    # create and initialize tree, cli-parser and configs-manger objects
    tree = TreeStruct()
    cli = BuilderCli(tree)
    cli.show_options()
    cfg = create_config_mgr_obj(tree, cli)
    ctx = BuilderContext(tree, cli, cfg)

    # pre-build system preparation (submodules, required tools installations)
    board = cli.get_board()
    platform = tree.get_platform_name(board)
    platform_path = tree.get_platform_dir(platform)
    sys.path.append(platform_path)  # set the platform path in sys for import

    tree.export_tree_paths(platform)

    process_git_submodules(ctx)
    process_prerequisite_tools(ctx)

    # invoke the build handler at platform side to continue the platform
    # specific preparation and invoke the build system
    try:
        import build_handler
        build_handler.run(ctx)

    except Exception as ex:
        log('error: could not execute build_handler', RED)
        log(f'Exception: {ex}', RED)
        exit(1)

if __name__ == "__main__":
    main()

# --- end of file ------------------------------------------------------------ #
