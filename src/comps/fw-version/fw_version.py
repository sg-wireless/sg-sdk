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
# Desc      Firmware version handling tool
# ---------------------------------------------------------------------------- #

import subprocess
import sys
import os
import re
import datetime
import binascii
from pathlib import Path
script_path = os.path.abspath(os.path.dirname(__file__))
sys.path.append(f'{os.path.dirname(__file__)}/../../../tools/pylibs')
from pylog import *
import pycli

def get_fw_release_version():
    """The release version exists in the file fw_release_version.h"""
    rw_rel_ver_file = f'{script_path}/fw_version.h'

    regex = r"^#define\s+FW_RELEASE_VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)$"
    regex = re.compile(regex)
    ver_dict = {}
    with open(rw_rel_ver_file, 'r') as f:
        for line in f:
            ret = regex.match(line)
            if ret:
                ver_dict[ret.group(1)] = ret.group(2)
    # log_obj(ver_dict)
    error = False
    for ver_comp in ['MAJOR', 'MINOR', 'PATCH']:
        if not ver_comp in ver_dict:
            log(f'error: missing version component {ver_comp}')
            error = True
    if error:
        return None
    ver_str = f'v{ver_dict["MAJOR"]}.{ver_dict["MINOR"]}.{ver_dict["PATCH"]}'
    return {
        'ver-str'   : ver_str,
        'major'     : ver_dict["MAJOR"],
        'minor'     : ver_dict["MINOR"],
        'patch'     : ver_dict["PATCH"],
    }

def get_fw_build_version(custom_str=None):
    def __run_subprocess(cmd_list):
        try:
            ret = subprocess.run(cmd_list, capture_output=True)
        except subprocess.CalledProcessError as e:
            ret = e
        return ret
    resp = __run_subprocess(['git', 'rev-parse', 'HEAD'])
    git_hash_str_full = ''
    if resp.returncode == 0:
        log(resp.stdout)
        # git_hash_full = bytes.fromhex(resp.stdout.decode('utf-8'))
        git_hash_str_full = resp.stdout.decode('utf-8').strip()
    resp = __run_subprocess(['git', 'describe', '--tags', '--dirty',
                             '--match', 'v[0-9]*.[0-9]*.[0-9]*'])
    if resp.returncode == 0:
        resp = re.match(r'''
                        (?P<release>v
                            (?P<major>\d+).
                            (?P<minor>\d+).
                            (?P<patch>\d+)
                        )(?P<extra>-
                        (?P<delta>\d+)-
                        g(?P<hash>[0-9a-fA-F]+)
                        (?:-(?P<dirty>dirty))?
                        )?
                        ''',
                        resp.stdout.decode('utf-8'), re.X)
        if resp:
            resp_dict = resp.groupdict()
            rel_str = resp_dict['release']
            major = resp_dict['major']
            minor = resp_dict['minor']
            patch = resp_dict['patch']
            delta = resp_dict['delta'] if resp_dict['delta'] else 0
            git_hash_str_short = git_hash_str_full[0:8]
            if resp_dict['dirty'] and custom_str == None:
                custom_str = resp_dict['dirty']
            date_obj = datetime.date.today()
            date_str = date_obj.strftime('%Y%m%d')
            date_month = date_obj.month
            date_day   = date_obj.day
            date_year  = date_obj.year
            date = datetime.date.today().strftime('%Y%m%d')
            ver_str = f'{rel_str}-{delta}-{git_hash_str_short}-{date_str}'
            if custom_str:
                custom_str = re.sub(' ', '_', custom_str)
                ver_str += f'-{custom_str}'

            return {
                # -- typical build version
                'build-ver-str' : ver_str,
                'ver-str' : ver_str,

                # -- release version components
                'rel-ver-str': rel_str,
                'rel-major'  : major,
                'rel-minor'  : minor,
                'rel-patch'  : patch,

                # -- git info comps
                'git-delta'      : delta,
                'git-hash-long'  : git_hash_str_full,
                'git-hash-short' : git_hash_str_short,

                # -- date info comps
                'date_str'   : date_str,
                'date_day'   : date_day,
                'date_month' : date_month,
                'date_year'  : date_year,

                # --custom comps
                'custom-str'    : custom_str if custom_str else ''
            }
    else:
        log(f'error: {resp.stderr.decode("utf-8")}')
    return None

def generate_version_header(filename, custom_str=None):
    ver = get_fw_build_version(custom_str)
    # log_obj(ver)
    with open(filename, 'w') as f:
        def fwrite(macro, val):
            s = f'#define FW_BUILD_VERSION_{macro:20s} '
            v = f'{val}'
            f.write(s)
            if len(s) + len(v) > 80:
                f.write('\\\n\t')
            f.write(v + '\n')
            pass

        f.write(f'// auto-generated file\n\n\n')
        if not ver:
            fwrite('ENABLE', 0)
        else:
            fwrite('ENABLE', 1)
            f.write(f'\n\n')
            f.write(f'// -- build version full string \n')
            fwrite('STRING', f"\"{ver['build-ver-str']}\"")
            fwrite('CUSTOM_STR', f"\"{ver['custom-str']}\"")

            f.write(f'\n\n')
            f.write(f'// -- release version info components \n')
            fwrite('RELEASE_STR', f"\"{ver['rel-ver-str']}\"")
            fwrite('RELEASE_MAJOR', f"{ver['rel-major']}")
            fwrite('RELEASE_MINOR', f"{ver['rel-minor']}")
            fwrite('RELEASE_PATCH', f"{ver['rel-patch']}")

            f.write(f'\n\n')
            f.write(f'// -- git info components \n')
            fwrite('GIT_TAG_FULL_STR', f"\"{ver['git-hash-long']}\"")
            fwrite('GIT_TAG_SHORT_STR', f"\"{ver['git-hash-short']}\"")
            fwrite('GIT_TAG_SHORT_U32', '0x' + \
                    binascii.hexlify(
                        bytes.fromhex(ver['git-hash-short'])[::-1]
                    ).decode('utf-8'))
            fwrite('GIT_DELTA', f"{ver['git-delta']}")

            f.write(f'\n\n')
            f.write(f'// -- date info components \n')
            fwrite('DATE_STR', f"\"{ver['date_year']}." + \
                   f"{ver['date_month']:02d}.{ver['date_day']:02d}\"")
            fwrite('DATE_DAY', f"{ver['date_day']}")
            fwrite('DATE_MONTH', f"{ver['date_month']}")
            fwrite('DATE_YEAR', f"{ver['date_year']}")

def main():
    cli = pycli.PyCli('SDK Firmware Version Tool')
    major_str = f'{COLOR_PURPLE}MAJOR{COLOR_DEFAULT}'
    minor_str = f'{COLOR_PURPLE}MINOR{COLOR_DEFAULT}'
    patch_str = f'{COLOR_PURPLE}PATCH{COLOR_DEFAULT}'
    cli.add_pos_arg(
        name='command',
        opts=['release-version', 'build-version', 'generate-header'],
        help='''the required versioning request command''',
        help_opts={
            'release-version':
                f'''the release version as specified in the file
                {COLOR_GREEN}fw_release_version.h{COLOR_DEFAULT}
                by the versioning components: {major_str}, {minor_str},
                {patch_str} and the final release version string will be in this
                format:<br></tab>
                {COLOR_BLUE}v{COLOR_GREY}<{COLOR_PURPLE}MAJOR{COLOR_GREY}>
                </no-space>{COLOR_BLUE}.{COLOR_GREY}<{COLOR_PURPLE}MINOR
                </no-space>{COLOR_GREY}>{COLOR_BLUE}.{COLOR_GREY}<
                </no-space>{COLOR_PURPLE}PATCH{COLOR_GREY}><br>
                </tab>Example: {COLOR_GREEN}v1.0.0
                <br>''',
            'build-version':
                f'''the firmware build version, it has the following format:
                <br></tab>
                {COLOR_GREY}<{COLOR_PURPLE}git-tag{COLOR_GREY}>-
                </no-space><{COLOR_PURPLE}delta-count{COLOR_GREY}>-
                </no-space><{COLOR_PURPLE}git-hash{COLOR_GREY}>-
                </no-space><{COLOR_PURPLE}date{COLOR_GREY}>
                </no-space>[-<{COLOR_PURPLE}custom-str{COLOR_GREY}>] <br>
                where,</indent><br>
                {COLOR_PURPLE}git-tag{COLOR_DEFAULT}</indent>
                    the latest release git tag prior or equals to the current
                    build git tag. and its format is similar to the
                    release-version.</dndent><br>
                {COLOR_PURPLE}delta-count{COLOR_DEFAULT}</indent>
                    the number of git commits since the
                    {COLOR_PURPLE}git-tag{COLOR_DEFAULT} till the current build
                    {COLOR_PURPLE}git-hash{COLOR_DEFAULT}.</dndent><br>
                {COLOR_PURPLE}git-hash{COLOR_DEFAULT}</indent>
                    the current abbreviated git hash value.</dndent><br>
                {COLOR_PURPLE}date{COLOR_DEFAULT}</indent>
                    the current date in this format
                    {COLOR_GREEN}YYYY{COLOR_CYAN}MM{COLOR_RED}DD{COLOR_DEFAULT}.
                    </dndent><br>
                {COLOR_PURPLE}custom-str{COLOR_DEFAULT}</indent>
                    a provided custom string to be appended to the build
                    version. If the current firmware git hash has modification,
                    and no provided custom string, the word {COLOR_RED}dirty
                    {COLOR_DEFAULT} will be used instead.
                    {COLOR_RED}@note:{COLOR_DEFAULT} The custom string can be
                    specified using the option
                    {COLOR_CYAN}--custom-string{COLOR_DEFAULT}.
                    </dndent>
                </dndent><br>
                {COLOR_GREY}Examples:</indent><br>{COLOR_GREEN}
                    v1.0.1-23-d33c144-20240326-hotfix<br>
                    v1.12.1-89-2f36de5-20240209<br>
                    v1.3.2-6-c212d9e-20240313-dirty<br>
                ''',
            'generate-header':
                f'''to generate all versioning components in a specific file.
                <br>{COLOR_RED}@note:{COLOR_DEFAULT} the option
                {COLOR_CYAN}--header-file{COLOR_DEFAULT} must be
                specified along with this command.
                '''
        })

    cli.add_arg('header-file', help='''The header file path and name to
                be used for versioning components generation.''')

    cli.add_arg('custom-string', help='''a user provided string to be appended
                to the requested build version.''')
    
    opts = cli.parse()

    # checking inputs sanity
    sanity_fail = False
    if opts['command'] == 'generate-header':
        if not 'header-file' in opts:
            log(f'{COLOR_RED}error: command {COLOR_CYAN}generate-header' +
                f'{COLOR_RED} needs option {COLOR_CYAN}--header-file' +
                f'{COLOR_RED} to be specified as well')
            sanity_fail = True
        elif not os.path.exists(os.path.dirname(opts['header-file'])):
            p = os.path.dirname(opts["header-file"])
            log(f'{COLOR_RED}error: header-file directory path ' + 
                f'{COLOR_DEFAULT}{p}{COLOR_RED} does not exist', RED)
            sanity_fail = True
    if sanity_fail:
        exit(1)
    
    if opts['command'] == 'release-version':
        ver = get_fw_release_version()
        if ver:
            log(ver['ver-str'])
        else:
            exit(1)
    elif opts['command'] == 'build-version':
        custom_str = opts['custom-string'] if 'custom-string' in opts else None
        ver = get_fw_build_version(custom_str)
        if ver:
            log(ver['ver-str'])
        else:
            exit(1)
    elif opts['command'] == 'generate-header':
        custom_str = opts['custom-string'] if 'custom-string' in opts else None
        generate_version_header(opts['header-file'], custom_str)
        pass
    pass

if __name__ == "__main__":
    main()

# --- end of file ------------------------------------------------------------ #
