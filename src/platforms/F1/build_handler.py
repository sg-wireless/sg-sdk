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
# Desc      This file represents the build handling for F1 platform and for all
#           of its derivations
# ---------------------------------------------------------------------------- #

import sys
import subprocess
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

    log('-- patching esptool.py')

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
        log('-- patching esptool.py maybe done already', CYAN)
    except:
        subprocess.run(f'patch {tgt_file} -p0 < {patch_file}',
                        shell=True, check=True)
        log('-- patching esptool.py passed!', GREEN)

    pass

# ---------------------------------------------------------------------------- #
# target command processing
# ---------------------------------------------------------------------------- #
def process_command(ctx: BuilderContext):
    command = ctx.cli.get_build_command()
    board = ctx.cli.get_board()
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

    # log(opts)

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

    if command == 'build' or command == 'flash':
        if variant == 'micropython':
            cmd_seq.append(
                f'make -C {mpy_path}/ports/esp32' +
                f' -f {mpy_path}/py/mkrules.mk GIT_SUBMODULES=' +
                '"lib/berkeley-db-1.xx lib/micropython-lib" submodules')
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
        for p in ports:
            cmd_seq.extend([f'esptool.py -p {p} erase_flash'])
    elif command == 'clean':
        cmd_seq.extend([
            f'idf.py -C {platform_dir} -B {build_dir} {opts} fullclean'
        ])

    try:
        counter = 1
        for cmd in cmd_seq:
            log('STEP(' + str(counter) + ')>> ' + cmd, CYAN)
            counter += 1
            subprocess.run(cmd, shell=True, check=True)
    except:
        log(command + ' failed!', RED)
        exit(1)

    if command == 'flash':
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
            exit(1)
    pass

# ---------------------------------------------------------------------------- #
# main routine
# ---------------------------------------------------------------------------- #
def run(ctx: BuilderContext) -> None:
    log('-- F1 platform build handler started!')

    process_esptool_patching(ctx)

    process_config_generation(ctx)

    process_command(ctx)
    pass

# --- end of file ------------------------------------------------------------ #
