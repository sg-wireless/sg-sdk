#!/usr/bin/env python3
# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
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
# Maintainer  Christian Ehlers (SG Wireless)
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
    
    def get_root_dir(self):
        return self.__root_dir
    
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

        # Load defaults from defaults.sdk if it exists
        defaults = self.__load_defaults_sdk(tree)

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
            , required  = (defaults.get('board') is None)
            , default   = defaults.get('board')
            , options   = tree.get_boards_names()
            , group     = group_hardware_options
            , help      = f''' The target hardware platform board.'''
        )

        # -- hardware build variants
        group_software_options = 'software options'

        cli.add_opt( 'variant'
            , default   = defaults.get('variant', 'micropython')
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
                            'config',
                            'env'
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
                , 'env'  : f'''To open the environment for idf.py and cmake to run. It is
                                used for compatibility with the VS Code plugin 
                                from Espressif'''
                }
        )

        cli.add_arg( 'port'
            , nargs     = '+'
            , required  = False
            , default   = defaults.get('port')
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
            , default   = defaults.get('project-dir')
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

        # -- combination flags for chaining operations
        cli.add_flag( 'clean'
            , group     = group_build_options
            , help      = f'''Clean before building the firmware'''
        )

        cli.add_flag( 'erase'
            , group     = group_build_options
            , help      = f'''Erase the device flash after building'''
        )

        cli.add_flag( 'flash'
            , group     = group_build_options
            , help      = f'''Flash the firmware after building'''
        )
        cli.add_arg( 'install-target'
            , nargs     = '+'
            , required  = False
            , group     = group_build_options
            , help      = f'''Target(s) for ESP-IDF toolchain installation.
                            Limits which ESP32 targets are installed when ESP-IDF toolchains
                            are installed (e.g., during first build).
                            <br>
                            {COLOR_GREY}Example: {COLOR_BLUE}
                            --install-target esp32s3
                            {COLOR_GREY}or{COLOR_BLUE}
                            --install-target esp32s3 esp32c3
                            {COLOR_DEFAULT}
                            <br>
                            If not specified, all ESP32 targets will be installed.'''
        )

        cli.add_flag( 'quiet-install'
            , group     = group_build_options
            , help      = f'''Suppress verbose output from ESP-IDF installation.
                            Only errors will be displayed.'''
        )

        cli.add_flag( 'secure'
            , group     = group_build_options
            , help      = f'''Enable secure boot and flash encryption.
                            This will use the full-flash command instead of flash
                            to include the bootloader when flashing.'''
        )

        self.__options = cli.parse()
        self.__check_arguments_sanity()
        # log(self.__options)

    def __load_defaults_sdk(self, tree: TreeStruct) -> dict:
        """Load default values from defaults.sdk file if it exists"""
        defaults = {}
        defaults_path = os.path.join(tree.get_root_dir(), 'defaults.sdk')
        
        if os.path.exists(defaults_path):
            try:
                import toml
                with open(defaults_path, 'r') as f:
                    config = toml.load(f)
                
                # Extract relevant defaults
                if 'board' in config:
                    defaults['board'] = config['board']
                if 'port' in config:
                    # Port can be a string or list - pycli will handle wrapping for nargs
                    defaults['port'] = config['port']
                if 'variant' in config:
                    defaults['variant'] = config['variant']
                if 'project-dir' in config:
                    # Project-dir expects a single value that pycli will wrap
                    defaults['project-dir'] = config['project-dir']
                
                log(f'-- Loaded defaults from {defaults_path}', COLOR_GREEN)
                for key, value in defaults.items():
                    log(f'   {key}: {value}')
            except ImportError:
                log(f'-- Warning: toml module not found, cannot load defaults.sdk', RED)
            except Exception as e:
                log(f'-- Warning: Failed to load defaults.sdk: {e}', RED)
        
        return defaults

    def __check_arguments_sanity(self):
        sanity_failed = False
        command = self.get_build_command()
        
        project_dir = self.get_project_dir()
        if project_dir != None:
            if not os.path.isdir(project_dir):
                log(f'-- error: dir "{project_dir}" does not exist', RED)
                sanity_failed = True
            else:
                self.__options['project-dir'] = os.path.abspath(project_dir)
        
        ports = self.get_ports()
        
        # Only check port validity if command requires it
        if command in ['flash', 'erase'] or self.should_flash() or self.should_erase():
            if ports is None:
                log(f'-- error: port must be specified for flash/erase operations', RED)
                sanity_failed = True
            else:
                # Check port availability
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

    def should_clean(self):
        """Check if --clean flag was specified"""
        return self.__options.get('clean', False)

    def should_erase(self):
        """Check if --erase flag was specified"""
        return self.__options.get('erase', False)

    def should_flash(self):
        """Check if --flash flag was specified"""
        return self.__options.get('flash', False)

    def should_use_secure(self):
        """Check if --secure flag was specified for secure boot and flash encryption"""
        return self.__options.get('secure', False)

    def get_install_targets(self):
        if 'install-target' in self.__options:
            return self.__options['install-target']
        return None

    def should_quiet_install(self):
        """Check if --quiet-install flag was specified"""
        return self.__options.get('quiet-install', False)

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
            process_esp_idf_installation(ctx.tree, ctx.cli)
        else:
            log(f'-- tool install not supported -> {COLOR_CYAN}{tool}', RED)
    pass

def process_esp_idf_installation(tree: TreeStruct, cli: BuilderCli):
    def shell_source(script):
        """Improved shell sourcing function compatible with ESP-IDF v5.4+"""
        log(f'-- sourcing script: {script}')
        
        # Check if debug mode is enabled
        debug_mode = os.environ.get('ESP_IDF_EXPORT_DEBUG', '0') == '1'
        redirect = '' if debug_mode else '>/dev/null 2>&1'
        
        # Use a more robust approach that works with ESP-IDF v5.4
        # Instead of env -0, we use a Python script approach through the shell
        script_cmd = f'''
        set -e
        . "{script}" {redirect}
        python3 -c "import os; [print(f'{{k}}={{v}}') for k, v in os.environ.items() if k.startswith(('IDF_', 'ESP_', 'PATH'))]"
        '''
        
        try:
            result = subprocess.run(['bash', '-c', script_cmd], 
                                  capture_output=True, text=True, check=False)
            
            if result.returncode != 0:
                log(f'-- shell sourcing failed with return code {result.returncode}', RED)
                if result.stderr:
                    log(f'-- stderr: {result.stderr.strip()}', RED)
                return False
            
            # Parse environment variables
            for line in result.stdout.strip().split('\n'):
                if '=' in line and line.strip():
                    key, value = line.split('=', 1)
                    if key.strip() and value.strip():
                        os.environ[key] = value
                        log(f'-- exported: {key}={value[:50]}{"..." if len(value) > 50 else ""}')
            
            return True
            
        except Exception as e:
            log(f'-- shell sourcing exception: {e}', RED)
            return False

    def esp_idf_install(tree: TreeStruct, targets=None, quiet=False):
        esp_idf_path = tree.get_submodule_path('esp-idf')
        install_script = f'{esp_idf_path}/install.sh'
        
        # Build install command with optional targets
        if targets:
            target_args = ' '.join(targets)
            cmd = f'{install_script} {target_args}'
            log(f'-- running ESP-IDF installation for targets: {COLOR_CYAN}{target_args}{COLOR_DEFAULT}')
        else:
            cmd = install_script
            log(f'-- running ESP-IDF installation: {install_script}')
        
        # Change to ESP-IDF directory for installation
        original_cwd = os.getcwd()
        try:
            os.chdir(esp_idf_path)
            
            # Run install script with proper environment
            env = os.environ.copy()
            env['IDF_PATH'] = esp_idf_path
            
            # Suppress output if quiet mode is enabled
            if quiet:
                ret = subprocess.run(cmd, shell=True, env=env, cwd=esp_idf_path,
                                   stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
            else:
                ret = subprocess.run(cmd, shell=True, env=env, cwd=esp_idf_path)
            
            if ret.returncode != 0:
                log(f'-- error ({ret.returncode}) during esp-idf installation', RED)
                # Show stderr in quiet mode if there was an error
                if quiet and ret.stderr:
                    log(f'-- stderr: {ret.stderr.strip()}', RED)
                log(f'-- please check ESP-IDF installation manually', RED)
                exit(1)
                
            log('-- esp-idf successfully installed!')
            
        except Exception as e:
            log(f'-- installation exception: {e}', RED)
            exit(1)
        finally:
            os.chdir(original_cwd)

    def check_python_compatibility(esp_idf_path):
        """Check if Python version meets ESP-IDF v5.4+ requirements (Python 3.8+)"""
        detect_script = f'{esp_idf_path}/tools/detect_python.sh'
        if os.path.exists(detect_script):
            try:
                result = subprocess.run(['bash', detect_script], 
                                      capture_output=True, text=True, check=False)
                if result.returncode != 0:
                    log('-- Python version check failed', RED)
                    log(f'-- ESP-IDF v5.4+ requires Python 3.8 or higher', RED)
                    if result.stderr:
                        log(f'-- error: {result.stderr.strip()}', RED)
                    return False
                    
                log('-- Python version compatibility check passed')
                return True
            except Exception as e:
                log(f'-- Python check exception: {e}', RED)
                return False
        else:
            log('-- Python detection script not found, assuming compatibility')
            return True

    log('-- setting esp-idf environment')

    esp_idf_path = tree.get_submodule_path('esp-idf')
    log(f'-- ESP-IDF path: {esp_idf_path}')
    
    # Verify ESP-IDF directory exists
    if not os.path.exists(esp_idf_path):
        log(f'-- ESP-IDF directory not found: {esp_idf_path}', RED)
        exit(1)
    
    # Set IDF_PATH environment variable
    os.environ["IDF_PATH"] = esp_idf_path
    log(f'-- set IDF_PATH = {esp_idf_path}')

    # Check Python compatibility first
    if not check_python_compatibility(esp_idf_path):
        log('-- Python compatibility check failed, aborting', RED)
        exit(1)

    export_filename = f'{esp_idf_path}/export.sh'
    log(f'-- checking export script: {export_filename}')
    
    if not os.path.exists(export_filename):
        log('-- esp-idf/export.sh file does not exist, please check', RED)
        exit(1)

    # Try to source the export script
    if not shell_source(export_filename):
        log('-- ESP-IDF environment setup failed, attempting installation', RED)

        # Get install targets and quiet mode from CLI if specified
        install_targets = cli.get_install_targets()
        quiet_install = cli.should_quiet_install()
        esp_idf_install(tree, install_targets, quiet_install)

        # Try sourcing again after installation
        if not shell_source(export_filename):
            log('-- ESP-IDF environment setup failed after installation!', RED)
            log('-- Please check the installation manually', RED)
            exit(1)
    
    log('-- ESP-IDF environment successfully configured')
    
    # Verify critical environment variables are set
    required_vars = ['IDF_PATH', 'PATH']
    for var in required_vars:
        if var not in os.environ or not os.environ[var]:
            log(f'-- Warning: {var} not properly set after ESP-IDF setup', RED)
    pass

# ---------------------------------------------------------------------------- #
# secure boot signing key auto-generation
# ---------------------------------------------------------------------------- #
def ensure_secure_boot_signing_key(ctx: BuilderContext):
    """If --secure is used, ensure the signing key PEM exists. If not, generate it.

    This is safe to call only after process_prerequisite_tools() has run, because
    that step sets up the ESP-IDF environment and places espsecure.py on PATH.
    """
    if not ctx.cli.should_use_secure():
        return

    board = ctx.cli.get_board()
    platform = ctx.tree.get_platform_name(board)
    platform_dir = ctx.tree.get_platform_dir(platform)

    # Derive the absolute key path from the sdkconfig.secure entry.
    sdkconfig_secure = f'{platform_dir}/configs/sdkconfig.secure'
    key_path = None
    if os.path.exists(sdkconfig_secure):
        with open(sdkconfig_secure, 'r') as f:
            for line in f:
                if line.startswith('CONFIG_SECURE_BOOT_SIGNING_KEY='):
                    relative = line.split('=', 1)[1].strip().strip('"')
                    key_path = os.path.normpath(
                        os.path.join(platform_dir, relative))
                    break

    if key_path is None:
        log(f'-- warning: could not determine signing key path from '
            f'{sdkconfig_secure}; skipping auto-generation', RED)
        return

    if os.path.exists(key_path):
        log(f'-- secure boot signing key found: {COLOR_CYAN}{key_path}{COLOR_DEFAULT}')
        return

    log(f'-- secure boot signing key not found at: {COLOR_CYAN}{key_path}{COLOR_DEFAULT}')
    log(f'-- generating a new secure boot signing key ...')

    ret = subprocess.run(
        ['espsecure.py', 'generate_signing_key', '--version', '2', key_path])
    if ret.returncode != 0:
        log(f'-- error: failed to generate secure boot signing key', RED)
        exit(1)

    log(f'-- secure boot signing key generated: {COLOR_CYAN}{key_path}{COLOR_DEFAULT}')
    log(f'{COLOR_RED}-- IMPORTANT: Store this private key securely '
        f'and do not commit it to version control!{COLOR_DEFAULT}')


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
    ensure_secure_boot_signing_key(ctx)

    # Handle combination flags (--clean, --erase, --flash)
    commands_to_run = []
    
    # If --clean flag is set, clean first
    if cli.should_clean():
        commands_to_run.append('clean')
    
    # Get the main command
    main_command = cli.get_build_command()
    
    # Build is implicit unless command is 'clean' or 'config'
    if main_command not in ['clean', 'config']:
        commands_to_run.append('build')
    elif main_command == 'config':
        commands_to_run.append('config')
    elif main_command == 'clean' and not cli.should_clean():
        # Only clean command, no flags
        commands_to_run.append('clean')
    
    # If --erase flag is set, erase after build
    if cli.should_erase():
        commands_to_run.append('erase')
    
    # If --flash flag is set or main command is flash, flash after build
    if cli.should_flash() or main_command == 'flash':
        commands_to_run.append('flash')
    elif main_command == 'erase' and not cli.should_erase():
        # Only erase command, no flags
        commands_to_run.append('erase')

    # Execute commands in sequence
    for command in commands_to_run:
        log(f'\n-- Executing command: {command}', COLOR_CYAN)
        
        # Temporarily override the command in context
        original_command = cli.get_build_command()
        cli._BuilderCli__options['command'] = command
        
        try:
            import build_handler
            build_handler.run(ctx)
        except Exception as ex:
            log(f'error: could not execute build_handler for {command}', RED)
            log(f'Exception: {ex}', RED)
            exit(1)
        
        # Restore original command
        cli._BuilderCli__options['command'] = original_command

if __name__ == "__main__":
    main()

# --- end of file ------------------------------------------------------------ #
