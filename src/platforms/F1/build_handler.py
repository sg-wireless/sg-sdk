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
# Desc      This file represents the build handling for F1 platform and for all
#           of its derivations
# ---------------------------------------------------------------------------- #

import sys
import os
import re
import subprocess
import tempfile
import shutil
from pylog import *
from builder import BuilderContext

sys.path.append(f'{os.environ["__tree_dir_src"]}/comps/fw-version')
import fw_version

# ---------------------------------------------------------------------------- #
# local vars
# ---------------------------------------------------------------------------- #
__partition_table_file = None
__sdkconfigs_files = None

# ---------------------------------------------------------------------------- #
# F1 configs preparations
# ---------------------------------------------------------------------------- #
def process_config_generation(ctx: BuilderContext) -> tuple:
    log(f'-- process configuration generation')
    app_dir = ctx.cli.get_project_dir()
    if app_dir:
        app_dir = os.path.basename(app_dir)
    platform = ctx.tree.get_platform_name(ctx.cli.get_board())
    gen_dir = ctx.tree.get_config_generation_dir(app_dir, platform,
                ctx.cli.get_board(), ctx.cli.get_variant() )
    log(gen_dir)

    # partition table generation
    partition_table = ctx.cfg.get_config("configs.partition-table")
    if partition_table:
        global __partition_table_file
        __partition_table_file = f'{gen_dir}/partition_table.csv'
        log(f'-- generate partition table file: {__partition_table_file}')
        with open(__partition_table_file, 'w') as w:
            for row in partition_table:
                first = True
                for col in row:
                    if not first:
                        w.write(', ')
                    first = False
                    w.write(f'{col}')
                w.write('\n')

    # esp-idf sdkconfig files
    global __sdkconfigs_files
    sdkconfigs_files = []
    platform_path = ctx.tree.get_platform_dir(platform)
    files = ctx.cfg.get_config("configs.sdkconfig-files", merge=True)
    for f in files:
        sdkconfigs_files += [f'{platform_path}/{f}']

    # log_obj(sdkconfigs_files)

    sdkconfig_lines = ctx.cfg.get_config("configs.sdkconfig", merge=True)
    # log_obj(sdkconfig_lines)
    if len(sdkconfig_lines) > 0:
        gen_file = f'{gen_dir}/sdkconfig.generated'
        with open(gen_file, 'w') as w:
            for config in sdkconfig_lines:
                w.write(f'{config}={sdkconfig_lines[config]}\n')
        sdkconfigs_files += [gen_file]

    sdkconfig_lines = ctx.cfg.get_config(
        f"configs.{ctx.cli.get_variant()}.sdkconfig", merge=True)
    if len(sdkconfig_lines) > 0:
        gen_file = f'{gen_dir}/sdkconfig.{ctx.cli.get_variant()}.generated'
        with open(gen_file, 'w') as w:
            for config in sdkconfig_lines:
                w.write(f'{config}={sdkconfig_lines[config]}\n')
        sdkconfigs_files += [gen_file]

    features = ctx.cfg.get_config('features', True)
    for feat in features:
        # Handle feature-specific sdkconfig files
        feature_files = ctx.cfg.get_config(
            f"configs.feature.{feat}.sdkconfig-files", merge=True)
        if feature_files:
            for f in feature_files:
                sdkconfigs_files += [f'{platform_path}/{f}']
        
        # Handle feature-specific sdkconfig key-value pairs
        sdkconfig_lines = ctx.cfg.get_config(
            f"configs.feature.{feat}.sdkconfig", merge=True)
        if len(sdkconfig_lines) > 0:
            gen_file = f'{gen_dir}/sdkconfig.{feat}.generated'
            with open(gen_file, 'w') as w:
                for config in sdkconfig_lines:
                    w.write(f'{config}={sdkconfig_lines[config]}\n')
            sdkconfigs_files += [gen_file]

    # Handle --secure flag for secure boot and flash encryption
    if ctx.cli.should_use_secure():
        secure_files = ctx.cfg.get_config("configs.secure.sdkconfig-files", merge=True)
        if secure_files:
            for f in secure_files:
                secure_config_file = f'{platform_path}/{f}'
                if os.path.isfile(secure_config_file):
                    sdkconfigs_files += [secure_config_file]
                    log(f'-- including secure config: {secure_config_file}', GREEN)
                else:
                    log(f'-- warning: secure config file not found at {secure_config_file}', YELLOW)
        else:
            secure_config_file = f'{platform_path}/configs/sdkconfig.secure'
            if os.path.isfile(secure_config_file):
                sdkconfigs_files += [secure_config_file]
                log(f'-- including secure config: {secure_config_file}', GREEN)
            else:
                log(f'-- warning: secure config file not found at {secure_config_file}', YELLOW)

    # log_obj(sdkconfigs_files)
    if len(sdkconfigs_files) > 0:
        __sdkconfigs_files = sdkconfigs_files
    
    pass

# ---------------------------------------------------------------------------- #
# specific tools patching
# ---------------------------------------------------------------------------- #
def process_esptool_patching(ctx: BuilderContext):

    # There is a problem for flashing from macOS and it has been resolved on
    # this specific commit '7e207d821919982df1ac1a1a5cf9f6e701f36ea1':
    #  https://github.com/espressif/esptool/pull/718/commits

    log('-- not patching esptool.py')
    return

    espidf_path = ctx.tree.get_submodule_path('esp-idf')
    tgt_submodule = f'{espidf_path}/components/esptool_py/esptool'
    tgt_file = f'{tgt_submodule}/esptool.py'
    patch_file = ctx.tree.get_platform_dir(
        ctx.tree.get_platform_name(ctx.cli.get_board())
        ) + '/esp-idf-patches/esptool/esptool.py.patch'

    try:
        cmd = f'cd {espidf_path} &&' + \
              f' git submodule update {tgt_submodule} && cd -'
        subprocess.run(cmd, shell=True, check=True)
    except Exception as e:
        log(f'failed to init submodule -> {e}', RED)
        exit(1)

    try:
        cmd = f'patch --ignore-whitespace {tgt_file}' + \
            f' -R -p0 -s -f --dry-run < {patch_file}'
        subprocess.run(cmd, shell=True, check=True)
        log('-- patching esptool.py maybe already done', CYAN)
    except:
        subprocess.run(f'patch {tgt_file} -p0 < {patch_file}',
                        shell=True, check=True)
        log('-- patching esptool.py passed!', GREEN)

    pass

def process_esptool_reverse_patching(ctx: BuilderContext):
    log('-- not reverse patching esptool.py')
    return

    espidf_path = ctx.tree.get_submodule_path('esp-idf')
    tgt_submodule = f'{espidf_path}/components/esptool_py/esptool'
    tgt_file = f'{tgt_submodule}/esptool.py'
    patch_file = ctx.tree.get_platform_dir(
        ctx.tree.get_platform_name(ctx.cli.get_board())
        ) + '/esp-idf-patches/esptool/esptool.py.patch'

    if os.path.isfile(tgt_file):
        try:
            cmd = f'patch --ignore-whitespace {tgt_file}' + \
                f' -R -p0 -s -f --dry-run < {patch_file}'
            subprocess.run(cmd, shell=True, check=True)
            log('-- reverse patching esptool.py', CYAN)
            subprocess.run(f'patch {tgt_file} -R -p0 -s -f < {patch_file}',
                            shell=True, check=True)
        except:
            log('-- reverse patching esptool.py maybe already done!', RED)
    
    pass

# ---------------------------------------------------------------------------- #
# Helper function to read installed component version from idf_component.yml
# ---------------------------------------------------------------------------- #
def get_installed_component_version(comp_dir):
    """Read the version field from a managed component's idf_component.yml.

    Returns the version string (e.g. '2.0.0') or None if not readable.
    """
    yml_path = os.path.join(comp_dir, 'idf_component.yml')
    if not os.path.isfile(yml_path):
        return None
    try:
        with open(yml_path, 'r') as f:
            for line in f:
                m = re.match(r'^version:\s*[\'"]?([^\s\'"]+)', line)
                if m:
                    return m.group(1)
    except Exception:
        pass
    return None


def version_satisfies_caret(installed_str, constraint_str):
    """Check if *installed_str* satisfies a caret (^) version constraint.

    ^X.Y.Z means >=X.Y.Z and <(X+1).0.0 (for X>0).
    If the constraint doesn't start with '^', falls back to exact prefix match.
    Returns True when the installed version is acceptable.
    """
    if not installed_str:
        return False
    constraint = constraint_str.lstrip('^')
    try:
        inst_parts = [int(p) for p in installed_str.split('.')]
        req_parts = [int(p) for p in constraint.split('.')]
    except (ValueError, AttributeError):
        return False
    # Pad to equal length
    while len(inst_parts) < len(req_parts):
        inst_parts.append(0)
    while len(req_parts) < len(inst_parts):
        req_parts.append(0)
    # Major must match, installed must be >= required
    if inst_parts[0] != req_parts[0]:
        return False
    return inst_parts >= req_parts


# ---------------------------------------------------------------------------- #
# Helper function to download managed components using Python API
# ---------------------------------------------------------------------------- #
def download_managed_component(component_name, component_version, target_dir):
    """
    Download a managed component directly using the IDF Component Manager Python API.
    
    Args:
        component_name: Component name (e.g., "espressif/esp_modem")
        component_version: Version constraint (e.g., "^2.0.0")
        target_dir: Directory where the component should be copied to
    
    Returns:
        True if successful, False otherwise
    """
    try:
        # Create a temporary minimal ESP-IDF project
        with tempfile.TemporaryDirectory() as temp_dir:
            # Create project structure
            main_dir = os.path.join(temp_dir, 'main')
            os.makedirs(main_dir, exist_ok=True)
            
            # Create minimal CMakeLists.txt
            with open(os.path.join(temp_dir, 'CMakeLists.txt'), 'w') as f:
                f.write('''cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(temp_download)
''')
            
            # Create minimal main component
            with open(os.path.join(main_dir, 'CMakeLists.txt'), 'w') as f:
                f.write('idf_component_register(SRCS "main.c" INCLUDE_DIRS ".")')
            
            with open(os.path.join(main_dir, 'main.c'), 'w') as f:
                f.write('#include "freertos/FreeRTOS.h"\nvoid app_main(void) {}')
            
            # Create idf_component.yml with the requested component
            with open(os.path.join(main_dir, 'idf_component.yml'), 'w') as f:
                f.write(f'''dependencies:
  idf:
    version: '>=5.0.0'
  {component_name}: {component_version}
''')
            
            # Run idf.py commands to download the component
            # We still need to shell out for this part as the Python API requires proper project setup
            build_dir = os.path.join(temp_dir, 'build')
            os.makedirs(build_dir, exist_ok=True)
            
            # Set target and update dependencies
            subprocess.run(f'cd {temp_dir} && idf.py set-target esp32s3', 
                         shell=True, check=True, capture_output=True)
            subprocess.run(f'cd {temp_dir} && idf.py update-dependencies', 
                         shell=True, check=True, capture_output=True)
            
            # Copy the downloaded component to target directory
            managed_comp_dir = os.path.join(temp_dir, 'managed_components')
            if os.path.exists(managed_comp_dir):
                # Find the component directory (it will have __ in the name)
                comp_simple_name = component_name.split('/')[-1]
                comp_dir_name = f"{component_name.replace('/', '__')}"
                src_comp = os.path.join(managed_comp_dir, comp_dir_name)
                
                if os.path.exists(src_comp):
                    dst_comp = os.path.join(target_dir, comp_dir_name)
                    if os.path.exists(dst_comp):
                        shutil.rmtree(dst_comp)
                    shutil.copytree(src_comp, dst_comp)
                    return True
        
        return False
    except Exception as e:
        log(f'Error downloading component {component_name}: {e}', RED)
        return False

# ---------------------------------------------------------------------------- #
# target command processing
# ---------------------------------------------------------------------------- #
def process_command(ctx: BuilderContext):
    command = ctx.cli.get_build_command()
    board = ctx.cli.get_board()
    
    # Convert flash to full-flash when --secure flag is set
    if ctx.cli.should_use_secure() and command == 'flash':
        log(f'-- secure boot enabled: converting flash to full-flash', CYAN)
        command = 'full-flash'

    platform = ctx.tree.get_platform_name(board)
    platform_dir = ctx.tree.get_platform_dir(platform)
    defs = ctx.cli.get_defs()
    app_dir = ctx.cli.get_project_dir()
    app_name = os.path.basename(app_dir) if app_dir != None else "sdk-default"
    variant = ctx.cli.get_variant()
    build_dir = ctx.tree.get_build_dir(app_name, platform, board, variant)
    features = ctx.cfg.get_config('features', True)
    ports = ctx.cli.get_ports()
    board_ids = ctx.cfg.get_config('id')
    custom_version_str = ctx.cli.get_custom_version_string()
    if board_ids == None:
        log(f'board config file missed the board ids (OEM; name, number)')
        exit(1)

    def __log_opt(opt, val):
        log_field(opt, 15)
        log(val, CYAN)
    __log_opt('command', command)
    __log_opt('board', board)
    __log_opt('platform', platform)
    __log_opt('defs', defs)
    __log_opt('app_dir', app_dir)
    __log_opt('app_name', app_name)
    __log_opt('variant', variant)

    opts = f'-D APP_NAME={app_name}'
    opts += f' -D SDK_PLATFORM={platform}'

    board_name = board_ids["board_name"] if 'board_name' in board_ids else 'NA'
    board_number = board_ids["board_number"] \
        if 'board_number' in board_ids else 'NA'
    board_shield = board_ids["shield"] if 'shield' in board_ids else 'NA'

    sdk_board = f'{board_number}-{board_name}-{board_shield}'

    opts += f' -D SDK_BOARD_NAME={board_name}'
    opts += f' -D SDK_BOARD_NUMBER={board_number}'
    opts += f' -D SDK_BOARD_SHIELD={board_shield}'
    opts += f' -D SDK_BOARD={sdk_board}'

    if custom_version_str:
        opts += f' -D SDK_FW_CUSTOM_VERSION_STRING=\"{custom_version_str}\"'

    if app_dir != None:
        opts += f' -D APP_DIR={app_dir}'
    opts += f' -D__build_variant={variant}'
    for feat in features:
        switch = 'ON' if features[feat] == True else 'OFF'
        feat = feat.replace('-', '_')
        opts +=  f' -D__feature_{feat}:BOOL={switch}'
    
    if defs != None:
        for d in defs:
            if re.match(r'^CMAKE_DEBUG=(ON|OFF)$', d):
                opts +=  f' -D{d}'
            opts +=  f' -D__user_def_{d}'

    if __partition_table_file != None:
        opts +=  f' -D__partition_table={__partition_table_file}'
    
    if __sdkconfigs_files != None:
        opts +=  f' -D__sdkconfigs_files="{";".join(__sdkconfigs_files)}"'

    log(opts)

    cmd_seq = []
    mpy_path = ctx.tree.get_submodule_path('micropython')
    try:
        ver_str = fw_version.get_fw_build_version(custom_version_str)
        if not ver_str:
            ver_str = fw_version.get_fw_release_version()
        pass
    except Exception as ex:
        log('error: getting version string')
        pass

    pkg = f'{build_dir}/{sdk_board}-{ver_str["ver-str"]}.tar'

    if command == 'build' or command == 'flash' or command == 'full-flash' or command == 'env':
        if variant == 'micropython':
            # First, initialize MicroPython submodules (must be done before managed components)
            cmd_seq.append(
                f'make -C {mpy_path}/ports/esp32' +
                f' -f {mpy_path}/py/mkrules.mk GIT_SUBMODULES=' +
                '"lib/berkeley-db-1.xx lib/micropython-lib" submodules')
            
            # Then, download ESP-IDF managed components if missing
            managed_comp_dir = f'{mpy_path}/ports/esp32/managed_components'
            required_components = ['espressif__esp_tinyusb', 'espressif__mdns', 'espressif__tinyusb']
            
            # Check for platform-specific components  {name: version_constraint}
            platform_managed_comp_dir = f'{platform_dir}/managed_components'
            platform_required_components = {}
            if features.get('lte', False):
                platform_required_components['espressif__esp_modem'] = '^2.0.0'
                
            missing_components = []
            platform_missing_components = []
            
            for comp in required_components:
                if not os.path.exists(f'{managed_comp_dir}/{comp}'):
                    missing_components.append(comp)
            
            for comp, required_ver in platform_required_components.items():
                comp_path = f'{platform_managed_comp_dir}/{comp}'
                #log(f'--- Looking for component in: {comp_path}', YELLOW)
                if not os.path.exists(comp_path):
                    #log(f'--- Adding missing component: {comp}', YELLOW)
                    platform_missing_components.append(comp)
                else:
                    installed_ver = get_installed_component_version(comp_path)
                    log(f'component in {comp_path} has version {installed_ver}')
                    if not version_satisfies_caret(installed_ver, required_ver):
                        log(f'-- Component {comp} version mismatch: '
                            f'installed={installed_ver}, required={required_ver}', YELLOW)
                        platform_missing_components.append(comp)
            
            if missing_components:
                log(f'-- Missing MicroPython managed components: {missing_components}', YELLOW)
                for comp in missing_components:
                    log(f'-- Downloading MicroPython managed component: {comp}', CYAN)
                
                # Set target to esp32s3 for F1 platform and update dependencies
                target = 'esp32s3'  # F1 platform uses ESP32-S3
                cmd_seq.extend([
                    f'cd {mpy_path}/ports/esp32 && idf.py set-target {target}',
                    f'cd {mpy_path}/ports/esp32 && idf.py update-dependencies'
                ])
            
            # Download platform-specific components
            # Note: We can't use ComponentManager Python API directly because it downloads
            # to a shared cache location. We need components in the platform's managed_components
            # directory for the build system to find them. So we download via a minimal project
            # and copy the results.
            # Download LTE modem component if needed and missing
            if platform_missing_components and features.get('lte', False):
                log(f'-- Missing platform managed components: {platform_missing_components}', YELLOW)
                os.makedirs(platform_managed_comp_dir, exist_ok=True)
                for comp in platform_missing_components:
                    ver = platform_required_components[comp]
                    comp_name = comp.replace('__', '/')
                    log(f'-- Downloading platform managed component: {comp_name} {ver}', CYAN)
                    if not download_managed_component(comp_name, ver, platform_managed_comp_dir):
                        log(f'-- Failed to download {comp_name} component', RED)
                        cmd_seq.append(f'echo "Warning: Failed to download {comp_name} component via Python API"')
        
        # For env command, execute preparation steps and return early
        if command == 'env':
            # Execute the submodule initialization and managed component downloads
            try:
                counter = 1
                for cmd in cmd_seq:
                    log('STEP(' + str(counter) + ')>> ' + cmd, CYAN)
                    counter += 1
                    subprocess.run(cmd, shell=True, check=True)
            except:
                log(command + ' failed!', RED)
                exit(1)
            
            # Export environment cache after successful preparation
            try:
                env_cache_file = f'{ctx.tree._TreeStruct__root_dir}/.vscode_env_cache'
                with open(env_cache_file, 'w') as f:
                    for key, value in os.environ.items():
                        f.write(f'{key}={value}\n')
                print('Environment prepared successfully.')
                print(f'Environment exported to: {env_cache_file}')
            except Exception as e:
                print(f'Warning: Could not export environment cache: {e}')
                print('Environment prepared successfully.')
            return
        
        # For build/flash commands, add the actual build steps
        cmd_seq.extend([
            # -- to call esp-idf build tool
            f'idf.py -C {platform_dir} -B {build_dir} {opts} build',
            # -- to call micropythom image creation tool
            f'python3 {mpy_path}/ports/esp32/makeimg.py ' +
                f'{build_dir}/sdkconfig ' +
                f'{build_dir}/bootloader/bootloader.bin ' +
                f'{build_dir}/partition_table/partition-table.bin ' +
                f'{build_dir}/application.bin ' +
                f'{build_dir}/firmware.bin ' +
                f'{build_dir}/firmware.uf2',
            # -- to create gziped package file
            f'rm -f {pkg} && tar cf {pkg} -C {build_dir} application.bin &&' +
            f' tar rf {pkg} -C {build_dir} ota_data_initial.bin &&' +
            f' tar rf {pkg} -C {build_dir} flash_args &&' +
            f' tar rf {pkg} -C {build_dir}/bootloader bootloader.bin &&' +
            f' tar rf {pkg} -C {build_dir}/partition_table partition-table.bin'+
            f' && gzip -f {pkg}'
        ])
    elif command == 'config':
        cmd_seq.extend([
            f'idf.py -C {platform_dir} -B {build_dir} {opts} menuconfig'
        ])
    elif command == 'erase':
        process_esptool_patching(ctx)
        for p in ports:
            cmd_seq.extend([f'esptool.py -p {p} erase_flash'])
    elif command == 'clean':
        # Remove build directory directly instead of using 'idf.py fullclean'
        # because the idf-component-manager hooks into fullclean and deletes
        # managed_components, forcing a slow re-download of esp_modem etc.
        cmd_seq.extend([
            f'rm -rf {build_dir}'
        ])

    try:
        counter = 1
        for cmd in cmd_seq:
            log('STEP(' + str(counter) + ')>> ' + cmd, CYAN)
            counter += 1
            subprocess.run(cmd, shell=True, check=True)
    except:
        if command == 'erase':
            process_esptool_reverse_patching(ctx)
        log(command + ' failed!', RED)
        exit(1)

    if command == 'flash':

        process_esptool_patching(ctx)

        flash_cmd_args = '-b 460800 --before=default_reset ' + \
            '--after=hard_reset write_flash'
        with open(f'{build_dir}/flash_args', 'r') as f:
            for line in f:
                pair = re.match(r'^(0x[0-9a-fA-F]+)\s\b(.+)$', line)
                if pair:
                    flash_cmd_args += \
                        f' {pair.group(1)} {build_dir}/{pair.group(2)}'
                else:
                    flash_cmd_args += ' ' + line.strip()
            pass
        cmd_seq = []
        for p in ports:
            flash_cmd = f'esptool.py -p {p} {flash_cmd_args}'
            cmd_seq.extend([flash_cmd])

        try:
            for cmd in cmd_seq:
                log('STEP(' + str(counter) + ')>> ' + cmd, CYAN)
                counter += 1
                subprocess.run(cmd, shell=True, check=True)
        except:
            log(command + ' failed!', RED)
            process_esptool_reverse_patching(ctx)
            exit(1)


    elif command == 'full-flash':

        process_esptool_patching(ctx)

        flash_cmd_args = '-b 460800 --before=default_reset ' + \
            '--after=hard_reset write_flash'

        write_flash_opts = []
        flash_pairs = []

        # Parse flash_args into write_flash options and address/file pairs
        with open(f'{build_dir}/flash_args', 'r') as f:
            for line in f:
                line = line.strip()
                if not line:
                    continue
                pair = re.match(r'^(0x[0-9a-fA-F]+)\s\b(.+)$', line)
                if pair:
                    flash_pairs.append(
                        f'{pair.group(1)} {build_dir}/{pair.group(2)}')
                else:
                    write_flash_opts.append(line)

        if write_flash_opts:
            flash_cmd_args += ' ' + ' '.join(write_flash_opts)

        # Include bootloader at 0x0 first, then regular flash_args pairs
        flash_cmd_args += f' 0x0 {build_dir}/bootloader/bootloader.bin'
        for pair in flash_pairs:
            flash_cmd_args += f' {pair}'

        cmd_seq = []
        for p in ports:
            flash_cmd = f'esptool.py -p {p} {flash_cmd_args}'
            cmd_seq.extend([flash_cmd])

        try:
            for cmd in cmd_seq:
                log('STEP(' + str(counter) + ')>> ' + cmd, CYAN)
                counter += 1
                subprocess.run(cmd, shell=True, check=True)
        except:
            log(command + ' failed!', RED)
            process_esptool_reverse_patching(ctx)
            exit(1)

    if command in ['erase', 'flash', 'full-flash']:
        process_esptool_reverse_patching(ctx)

    pass

# ---------------------------------------------------------------------------- #
# main routine
# ---------------------------------------------------------------------------- #
def run(ctx: BuilderContext) -> None:
    log('-- F1 platform build handler started!')

    process_config_generation(ctx)

    process_command(ctx)
    pass

# --- end of file ------------------------------------------------------------ #
