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
# Desc      § This file represents the test engine for atomated test famework.
#           § Some of the testing attributes such as (regex, skip, py-host, .)
#             are inspired from the original micropython run-tests.py script.
#           § Some new concepts are introduced to help in distributed test
#             across boards and hosts.
#           § Powerful test reports are also introduced.
#           § Easy to integrate with Jenkins
# ---------------------------------------------------------------------------- #

import serial
import fcntl
import argparse
import threading
import time
import os
import subprocess
import re
import textwrap
import json
import sys
sys.path.append(os.path.dirname(__file__) + '/../pylibs') # pylog
import pylog
from pylog import log, log_line, log_tab_dec, log_tab_inc, \
    log_decorated_header, log_underlined, log_list, \
    log_get_color_str, log_field

# ---------------------------------------------------------------------------- #
# test engine configs
# ---------------------------------------------------------------------------- #
g_results_dir = None
g_disp_pass_result = False
g_default_timeout = 30 # seconds

# display metrics
g_width_total       = 150   # total line width
g_width_tag_job     = 7     # the place of displaying 'TEST' or 'JOB:nn'
g_width_tag_tst     = 4     # the place of displaying 'Tnn' for sub tests
g_width_verdict     = 10    # the place holder for verdict display

g_indent_job_details    = g_width_tag_job
g_indent_tst            = g_width_tag_job
g_indent_tst_details    = g_indent_tst + g_width_tag_tst

g_width_name_job        = g_width_total - g_width_tag_job - g_width_verdict
g_width_name_tst        = g_width_name_job - g_width_tag_tst

g_width_results_job     = g_width_total - g_indent_job_details
g_width_results_tst     = g_width_total - g_indent_tst_details

g_color_list_idx        = pylog.GREY
g_color_specs_caption   = pylog.YELLOW

g_color_tag_job         = pylog.CYAN
g_color_tag_tst         = pylog.GREY
g_color_name_job        = pylog.DEFAULT
g_color_name_job_fill   = pylog.GREY
g_color_name_tst        = pylog.DEFAULT
g_color_name_tst_fill   = pylog.GREY

g_color_flag_host_exp   = pylog.BLUE
g_color_flag_timeout    = pylog.RED
g_color_flag_fail       = pylog.RED
g_color_flag_skip_tst   = pylog.CYAN
g_color_flag_skip_usr   = pylog.CYAN
g_color_flag_ignore     = pylog.PURPLE
g_color_flag_regex      = pylog.YELLOW
g_color_flag_target     = pylog.GREY

g_color_verdict_pass    = pylog.GREEN
g_color_verdict_fail    = pylog.RED
g_color_verdict_skip    = pylog.CYAN
g_color_verdict_ignore  = pylog.PURPLE
g_color_verdict_invalid = pylog.RED

g_log_test_name_w = 80
g_log_job_type_field_w = 6

g_two_sided_w = 120

# ---------------------------------------------------------------------------- #
# serial ports management
# ---------------------------------------------------------------------------- #

# an array containing all nodes ports
g_ports_list = []

def port_scan():
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    test_nodes = []
    macos_usb_regex = '^/dev/.*.usbserial-[0-9a-fA-F]+[2cC]$'
    for p in ports:
        if re.match(macos_usb_regex, p[0]):
            test_nodes.append(p[0])
            pass
    ubuntu_usb_regex = '^/dev/ttyUSB([0-9]+)$'
    for p in ports:
        res = re.match(ubuntu_usb_regex, p[0])
        if res:
            if int(res[1]) % 4 == 2:
                test_nodes.append(res[0])
    return test_nodes

def port_open(serial_port):
    try:
        ser = serial.Serial(
            port = serial_port,
            baudrate = 115200,
            timeout = 3,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE)
        try:
            fcntl.flock(ser.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        except IOError:
            print('port is busy')
            return None

        ser.isOpen()
        # print('port "{}" is opened'.format(serial_port))

    except IOError:
        print('port "{}" is incorrect'.format(serial_port))
        return None
    return ser

def port_close(p):
    if p.isOpen():
        p.close()

def get_avail_ports():
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    test_nodes = []
    for p in ports:
        if re.match('^/dev/.*.usbmodemPy.+$', p[0]):
            test_nodes.append(p[0])
            pass
    return test_nodes

def log_test_nodes():
    log_list(g_ports_list, horiz=False, numbering=True,
             numbering_color=pylog.GREY, color=pylog.CYAN)

def port_add(ports):
    for p in ports:
        # log('-- add node port: {}'.format(p))
        port = port_open(p)
        if port:
            port_close(port)
            g_ports_list.append(p)
        else:
            log('port {}{}{} is wrong or not available'.format(
                log_get_color_str(pylog.RED),
                p,
                log_get_color_str(pylog.DEFAULT)
            ))

def port_acquire() -> object:
    global g_ports_list
    port = None
    for p in g_ports_list:
        port = port_open(p)
        if port:
            break
    if port:
        g_ports_list.remove(p)
        # log(' -- port [ {} ] is acquired'.format(p), pylog.GREEN)
        return port
    return None

def port_release(port):
    if not isinstance(port, serial.Serial):
        log('-- release wrong serial object: {}'.format(port))
        return

    p_name = port.port
    port_close(port)
    global g_ports_list
    g_ports_list.append(p_name)
    # log(' -- port [ {} ] is released'.format(p_name), pylog.CYAN)
    pass

def port_read(port, size=1):
    try:
        ret = port.read(size)
    except:
        log('\n------ read exception ------\n', color=pylog.RED)
    return ret

def port_mgr_test():
    cmd_parser = argparse.ArgumentParser(
        formatter_class = argparse.RawDescriptionHelpFormatter,
        description = 'test port management')

    cmd_parser.add_argument('--port', nargs='*', help='node serial port')

    args = cmd_parser.parse_args()

    if args.port:
        port_add(args.port)

    acquired_ports = []
    while True:
        port = port_acquire()
        if port:
            acquired_ports.append(port)
        else:
            break

    for p in acquired_ports:
        port_release(p)

    acquired_ports = []
    while True:
        port = port_acquire()
        if port:
            acquired_ports.append(port)
        else:
            break

    for p in acquired_ports:
        port_release(p)
    pass

# ---------------------------------------------------------------------------- #
# Test Statistics
# ---------------------------------------------------------------------------- #
class TestStats:
    def __init__(self, total_tests=0):
        self.total = total_tests
        self.passed = 0
        self.failed = 0
        self.skipped = 0
        self.ignored = 0
        self.invalid = 0
        self.pass_list = []
        self.fail_list = []
        self.skip_list = []
        self.ignore_list = []
        self.invalid_list = []

    def add_tests(self, count):
        self.total += count

    def test_pass(self, test_name):
        self.passed += 1
        self.pass_list.append(test_name)
    def test_fail(self, test_name):
        self.failed += 1
        self.fail_list.append(test_name)
    def test_skip(self, test_name):
        self.skipped += 1
        self.skip_list.append(test_name)
    def test_ignore(self, test_name):
        self.ignored += 1
        self.ignore_list.append(test_name)
    def test_invalid(self, test_name):
        self.invalid += 1
        self.invalid_list.append(test_name)
    def log(self):
        log('total   # {:4}'.format(self.total))
        log('passed  # {:4}'.format(self.passed), pylog.GREEN)
        log('skipped # {:4}'.format(self.skipped), pylog.CYAN)
        log('failed  # {:4}'.format(self.failed), pylog.RED)
        log('ignored # {:4}'.format(self.ignored), pylog.PURPLE)
        if self.invalid != 0:
            log('*** invalid tests # {}'.format(self.invalid), pylog.RED)

        if self.passed + self.skipped + self.failed + self.ignored + \
            self.invalid != self.total:
            log('*** test script counters error', pylog.RED)
        if self.skipped > 0:
            log_decorated_header('skipped tests', width=g_width_total,
                                 caption_color=pylog.CYAN)
            log_list(self.skip_list, horiz=True, wrap_after=1, numbering=True)
        if self.failed > 0:
            log_decorated_header('failed tests', width=g_width_total,
                                caption_color=pylog.RED)
            log_list(self.fail_list, horiz=True, wrap_after=1, numbering=True)
        if self.ignored > 0:
            log_decorated_header('ignored tests', width=g_width_total,
                                 caption_color=pylog.PURPLE)
            log_list(self.ignore_list, horiz=True, wrap_after=1, numbering=True)
        if self.invalid > 0:
            log_decorated_header('invalid tests', width=g_width_total,
                                 caption_color=pylog.RED)
            log_list(self.invalid_list, horiz=True, wrap_after=1,
                     numbering=True)

g_test_stats = TestStats(0)

# ---------------------------------------------------------------------------- #
# Test Protocol -- Micropython Data Exchange
# ---------------------------------------------------------------------------- #
# Micropython REPL Protocol
# =========================
#
#   +-------+                                                   +-------+
#   |   PC  |                                                   |  SUT  |
#   +---+---+                                                   +---+---+
#       |                                                           |    
#       |      interrupt any running prog (__CTRL_C, __CTRL_C)      |    
#       |---------------------------------------------------------->|    
#       |             switch to raw REPL (__CTRL_A)                 |    
#       |---------------------------------------------------------->|    
#       |            b'raw REPL; CTRL-B to exit\r\n>'               |    
#       |<..........................................................|    
#       |                 soft reset (__CTRL_D)                     |    
#       |---------------------------------------------------------->|    
#       |            b'raw REPL; CTRL-B to exit\r\n>'               |    
#       |<..........................................................|    
#       |                                                           |    
#       |                                                           |    
#       |                   send test script                        |    
#       |---------------------------------------------------------->|    
#       |             end of test script (__CTRL_D)                 |    
#       |---------------------------------------------------------->|    
#       |                           b'OK'                           |    
#       |<..........................................................|    
#       |                                                           |    
#       |                                                           |    
#       |                      test output                          |    
#       |<..........................................................|    
#       |           test finish indication b'\x04\x04>'             |    
#       |<..........................................................|    
#       |                                                           |    

class TestProtocolMicropython:
    __CTRL_A = b'\x01'  # enter raw REPL
    __CTRL_B = b'\x02'  # enter friendly REPL
    __CTRL_C = b'\x03'  # twice: interrupt any running program
    __CTRL_D = b'\x04'  # soft reset
    __CTRL_E = b'\x05'  # paste mode
    __CTRL_F = b'\x06'  # safeboot reset
    REPL_RAW = 0
    REPL_FRIENDLY = 1
    def __init__(self, serial_port, is_stopped, repl_type=REPL_RAW) -> None:
        self.__is_stopped = is_stopped
        self.__port = serial_port
        self.__repl_type = repl_type
        pass

    def __read_until(self, last_bytes, include_last_in_out=False):
        accum = b''
        a = len(last_bytes)
        while True:
            # accum += self.__port.read()
            accum += port_read(self.__port, 1)
            if len(accum) >= a and accum[-a:] == last_bytes:
                break
            if self.__is_stopped():
                # return None
                return accum
        return accum[0 : -a] + (last_bytes if include_last_in_out else b'')

    def __read(self, timeout=2):
        accum = b''
        counter = 0
        self.__port.timeout = timeout
        while True:
            b = self.__port.read(1)
            if len(b) == 0:
                counter += 1
            else:
                accum += b
                counter = 0
            if counter == timeout or self.__is_stopped():
                break
        return accum

    def exec(self, context) -> str:
        if self.__repl_type == TestProtocolMicropython.REPL_RAW:
            return self.__exec_repl_raw(context)
        elif self.__repl_type == TestProtocolMicropython.REPL_FRIENDLY:
            return self.__exec_repl_friendly(context)
        else:
            return 'ERROR -- unsupported test protocol'

    def __exec_repl_raw(self, script):
        # interrupt any running program
        self.__port.write(self.__CTRL_C)
        self.__port.write(self.__CTRL_C)
        # switch to raw REPL
        self.__port.write(self.__CTRL_A)
        self.__read_until(b'raw REPL; CTRL-B to exit\r\n>')
        # do soft reset
        self.__port.write(self.__CTRL_D)
        self.__read_until(b'raw REPL; CTRL-B to exit\r\n>')
        # send the micropython code to be executed
        self.__port.write(script)
        # send end of the micropython code, to let the device start execution
        self.__port.write(self.__CTRL_D)
        # wait device confirmation
        self.__read_until(b'OK')
        # receive the device output until end of execution indication
        out = self.__read_until(b'\x04\x04>')
        return str(out, encoding='utf-8', errors='backslashreplace')

    def __exec_repl_friendly(self, commands_list):
        # interrupt any running program
        self.__port.write(self.__CTRL_C)
        self.__port.write(self.__CTRL_C)
        # switch to friendly REPL
        self.__port.write(self.__CTRL_B)
        out = self.__read_until(b'>>> ', include_last_in_out=True)

        for cmd in commands_list:
            if cmd == "CTRL-A":
                self.__port.write(self.__CTRL_A)
            elif cmd == "CTRL-B":
                self.__port.write(self.__CTRL_B)
            elif cmd == "CTRL-C":
                self.__port.write(self.__CTRL_C)
            elif cmd == "CTRL-D":
                self.__port.write(self.__CTRL_C)
            elif cmd == "CTRL-E":
                self.__port.write(self.__CTRL_E)
            elif cmd == "CTRL-F":
                self.__port.write(self.__CTRL_F)
            else:
                cmd += '\r\n'
                self.__port.write(bytearray(cmd, 'utf-8'))
            out += self.__read()

        return str(out, encoding='utf-8', errors='backslashreplace')

# ---------------------------------------------------------------------------- #
# Test Protocol -- Host shell process
# ---------------------------------------------------------------------------- #
class TestProtocolHostProcess:
    def __init__(self, command, is_stopped=None,
                 timeout=g_default_timeout) -> None:
        self.__is_stopped = is_stopped
        self.__command = command
        self.__timeout = timeout
        pass

    def exec(self, script=None) -> str:
        cmd_list = self.__command.split()
        try:
            ret = subprocess.run(
                cmd_list, timeout=self.__timeout, capture_output=True)
        except subprocess.CalledProcessError as e:
            ret = e
        # print(ret)
        out = ''
        if ret.returncode != 0:
            tag_color = pylog.log_get_color_str(pylog.PURPLE)
            err_color = pylog.log_get_color_str(pylog.RED)
            def_color = pylog.log_get_color_str(pylog.DEFAULT)
            out += '{}[errcode]{} {}\n'.format(
                tag_color, err_color, ret.returncode)
            out += '{}[stderr ]{} {}\n'.format(
                tag_color, err_color, ret.stderr.decode('utf-8'))
            out += '{}[stdout ]{} '.format(tag_color, def_color)
        out += ret.stdout.decode('utf-8')
        return out

# ---------------------------------------------------------------------------- #
# Test Verdict
# ---------------------------------------------------------------------------- #
class TestVerdict:
    PASS = 0
    FAIL = 1
    SKIP = 2
    IGNORE = 3
    NOT_EXEC = 4
    INVALID = 5

    LOG_VERDICT_W = 10

    def __init__(self) -> None:
        self.__v = TestVerdict.NOT_EXEC
        pass

    def get(self) -> int:
        return self.__v

    def compare(self, verdict_value) -> bool:
        return self.__v == verdict_value
    
    def set_pass(self) -> bool:
        self.__v = self.PASS
    def set_fail(self) -> bool:
        self.__v = self.FAIL
    def set_skip(self) -> bool:
        self.__v = self.SKIP
    def set_ignore(self) -> bool:
        self.__v = self.IGNORE
    def set_not_exec(self) -> bool:
        self.__v = self.NOT_EXEC
    def set_invalid(self) -> bool:
        self.__v = self.INVALID

    def is_pass(self) -> bool:
        return self.__v == self.PASS
    def is_fail(self) -> bool:
        return self.__v == self.FAIL
    def is_skip(self) -> bool:
        return self.__v == self.SKIP
    def is_ignore(self) -> bool:
        return self.__v == self.IGNORE
    def is_not_exec(self) -> bool:
        return self.__v == self.NOT_EXEC
    def is_invalid(self) -> bool:
        return self.__v == self.INVALID

    def string(self):
        if self.__v == TestVerdict.PASS:
            return 'PASS'
        elif self.__v == TestVerdict.FAIL:
            return 'FAIL'
        elif self.__v == TestVerdict.SKIP:
            return 'SKIP'
        elif self.__v == TestVerdict.IGNORE:
            return 'IGNORE'
        elif self.__v == TestVerdict.NOT_EXEC:
            return 'NOT_EXEC'
        elif self.__v == TestVerdict.INVALID:
            return 'INVALID'
    
    def logging_color(self):
        if self.__v == TestVerdict.PASS:
            return g_color_verdict_pass
        elif self.__v == TestVerdict.FAIL:
            return g_color_verdict_fail
        elif self.__v == TestVerdict.SKIP:
            return g_color_verdict_skip
        elif self.__v == TestVerdict.IGNORE:
            return g_color_verdict_ignore
        elif self.__v == TestVerdict.NOT_EXEC:
            return g_color_verdict_invalid
        elif self.__v == TestVerdict.INVALID:
            return g_color_verdict_invalid

    def log(self, width=LOG_VERDICT_W, dimmed=False, endl=False):
        log('[', endl=False)
        txt = ''
        if dimmed:
            txt += log_get_color_str(pylog.DIMMED)
        txt += self.string()
        if dimmed:
            txt += log_get_color_str(pylog.HILIGHT)
        log_field(txt, width - 2, color=self.logging_color(),
                  align=pylog.CENTER, endl=False)
        log(']', endl=endl)
        pass

# ---------------------------------------------------------------------------- #
# Test Context
# ---------------------------------------------------------------------------- #
class TestContext:
    """A Class specifying the test related input and output parameters"""
    __IDLE__    = 0
    __READY__   = 1
    __RUNNING__ = 2
    __DONE__    = 3
    state_str = { __IDLE__:'IDLE', __READY__:'READY', __RUNNING__:'RUNNING',
                  __DONE__:'DONE' }

    TARGET_BOARD_MICROPYTHON = 0
    TARGET_HOST = 1

    PROTO_MPY_RAW = 0
    PROTO_MPY_FRIENDLY = 1
    PROTO_HOST_PROCESS = 2

    def __init__(self, name, tst_proc:list,
                 exp_file:str=None, exp_value:str=None,
                 parent_job:object=None, proto=PROTO_MPY_RAW) -> None:
        # --- input parameters
        self.name = name        # name of the test or the main test file
        self.parent_job = parent_job # an optional link to the parent job
        self.tst_proc = tst_proc # the testing scripts in ordered sequence
        self.exp_file = exp_file # file containing an expected value
        self.tst_script = None  # a concatenation of all test files contents
        self.exp = exp_value    # the expected value. it depends on the given
                                # 'exp_file' and the 'pyhost' flag
        self.f_ignore = False   # flag to ignore the test failure
        self.f_pyhost = False   # flag to take the expected value from the host
        self.f_skip = False     # flag to skip launching this test
        self.f_show_out = False # flag to display test output even if it passed
        self.f_hide_fail_details = False # flag to hide fail tests details
        self.f_regex = False    # flag to indicate that expected value has regex
        self.node_port = None   # the test node serial port
        self.node_port_name = None
        self.timeout = g_default_timeout # default test timeout
        # self.target = TestContext.TARGET_BOARD_MICROPYTHON
        self.host_cmd = None
        # self.target_commands = None
        self.results_dir = None

        # --- output parameters
        self.out = None         # the target node test output
        self.f_expired = False  # the test deadline expired
        self.f_skipped = False  # the test is skipped by its output
        self.missing_resource = None # the file that does not exist
        self.verdict = TestVerdict() # the final test verdict
        self.fail_cause = []  # the failure cause message
        self.f_unexpected_output = False
        self.f_pyhost_exp = False

        self.state = self.__IDLE__

        # --- execution parameters
        self.stop_event = threading.Event()
        self.thread_exec = None
        self.thread_timer = None
        self.protocol_obj = None
        self.protocol_type =  proto
        self.__fail_l_line = 0
        self.__fail_r_line = 0

        pass

    def prepare(self) -> bool:

        self.state = self.__DONE__
            # the test object state will be considered done during the
            # preparation because it may be vindicated as failed
            # after all preparation steps passed successfully, the test state
            # will become ready to be launched to the test node

        # -- process test procedure
        if self.protocol_type == self.PROTO_MPY_RAW:
            # self.tst_proc is a sequence of python scripts, that shall be
            # concatenated in a single script
            tst_scripts = self.tst_proc
            if len(tst_scripts) == 0:
                self.verdict.set_fail()
                self.add_fail_cause('no given script files to test')
            x_tst_file = check_files_existence(tst_scripts)
            if x_tst_file:
                # one at least of the test files not exist, abort
                self.verdict.set_fail()
                self.add_fail_cause('file {}{}{} does not exist'.format(
                    log_get_color_str(pylog.DEFAULT),
                    x_tst_file,
                    log_get_color_str(pylog.RED),
                ))
                return False
            else:
                # read all test scripts and append them one by one
                self.tst_script = b''
                for tst in tst_scripts:
                    with open(tst, 'rb') as f:
                        self.tst_script += f.read()
                        self.tst_script += b'\r\n'
            pass
        elif self.protocol_type == self.PROTO_MPY_FRIENDLY:
            pass
        elif self.protocol_type == self.PROTO_HOST_PROCESS:
            pass
        else:
            self.verdict.set_fail()
            self.add_fail_cause('test engine: unsupported protocol: {}'.format(
                self.protocol_type
            ))
            return False

        # -- process expected value
        if self.exp_file:
            # check existence of the given expect file
            x_exp_file = check_files_existence([self.exp_file])
            if x_exp_file:
                # the given expect file does not exist, abort
                self.missing_resource = x_exp_file
                self.verdict.set_fail()
                self.add_fail_cause('file {}{}{} does not exist'.format(
                    log_get_color_str(pylog.DEFAULT),
                    x_exp_file,
                    log_get_color_str(pylog.RED),
                ))
                return False
            # read the contents of the 'exp-file'
            with open(self.exp_file) as f:
                self.exp = filter_test_text(f.read())
        elif self.exp:
            pass # expected value is given, so it is considered with priority
        elif self.f_pyhost and self.protocol_type == TestContext.PROTO_MPY_RAW:
            # this test can run on host for expect value prediction
            # run the test on host and get its output as the target expectation
            pyhost_dummy_script = './__dump_pyhost_script.py'
            with open('./__dump_pyhost_script.py', 'wb') as f:
                f.write(self.tst_script)
            self.exp = TestProtocolHostProcess(
                ' '.join(["python3", pyhost_dummy_script])).exec()
            os.remove(pyhost_dummy_script)
            self.f_pyhost_exp = True

        # -- init expiration event flag
        self.stop_event.clear()

        # -- skipped tests handling
        if self.f_skip:
            self.verdict.set_skip()
            self.state = self.__DONE__
            return True

        # -- acquire node port
        if self.protocol_type == TestContext.PROTO_MPY_RAW or \
            self.protocol_type == TestContext.PROTO_MPY_FRIENDLY:
            node_port = port_acquire()
            if not node_port:
                self.verdict.set_fail()
                self.add_fail_cause('no available test nodes')
                return False
            else:
                self.node_port = node_port
                self.node_port_name = node_port.port

        # -- prepare testing protocol to micropython protocol
        if self.protocol_type == TestContext.PROTO_MPY_RAW:
            self.protocol_obj = TestProtocolMicropython(
                self.node_port, self.is_expired,
                TestProtocolMicropython.REPL_RAW)
        elif self.protocol_type == TestContext.PROTO_MPY_FRIENDLY:
            self.protocol_obj = TestProtocolMicropython(
                self.node_port, self.is_expired,
                TestProtocolMicropython.REPL_FRIENDLY)
        elif self.protocol_type == TestContext.PROTO_HOST_PROCESS:
            self.protocol_obj = TestProtocolHostProcess(
                ' '.join(self.tst_proc), self.is_expired, self.timeout)

        # -- prepare execution parameters
        self.thread_exec = threading.Thread(target=self.run, args=())
        self.thread_timer = threading.Timer(float(self.timeout),
                        function=self.expire, args=())
        
        if not(self.thread_exec and self.thread_timer):
            self.verdict.set_fail()
            self.add_fail_cause('failed to init thread resources')
            return False

        self.state = self.__READY__
        return True
    
    def is_ready(self) -> bool:
        return self.state == self.__READY__
    
    def expire(self):
        self.f_expired = True
        self.force_stop()
    
    def run(self):
        if self.state != self.__RUNNING__:
            self.verdict.set_fail()
            self.add_fail_cause('test-engine: internal state error')
            return

        # execute the prepared test script
        self.out = filter_test_text(self.protocol_obj.exec(
            self.tst_script if self.protocol_type == TestContext.PROTO_MPY_RAW
                else self.tst_proc))

        # stop timer thread
        self.thread_timer.cancel()
        self.thread_timer = None

        # post processing
        if self.exp or self.exp_file:
            (self.__fail_l_line, self.__fail_r_line) = compare_test_results(
                self.out, self.exp, self.f_regex)
            if self.__fail_l_line != 0 or self.__fail_r_line != 0:
                self.f_unexpected_output = True
                self.add_fail_cause('unexpected output')

        if self.out == 'SKIP':
            # the test is skipped by its own output
            self.verdict.set_skip()
            self.f_skipped = True
        elif (self.exp or self.exp_file) and self.__fail_l_line == 0 and \
                self.__fail_r_line == 0:
            # the test succeeded
            self.verdict.set_pass()
        elif self.is_expired():
            # the test timeout expired -> hence it fails
            self.verdict.set_fail()
            self.add_fail_cause('test timeout expired')
        elif self.exp or self.exp_file:
            # the expected test results does not match its output results
            self.verdict.set_fail()
        else:
            # test with no expected value, will be considered pass
            self.verdict.set_pass()

        if self.verdict.is_fail() and self.f_ignore:
            # set verdict to ignore if it is failed and has ignore flag
            self.verdict.set_ignore()

        self.state = self.__DONE__
        pass

    def is_passed(self) -> bool:
        return self.state == self.__DONE__ and self.verdict.is_pass()
    def is_failed(self) -> bool:
        if self.verdict.is_fail() and self.f_ignore:
            self.verdict.set_ignore()
            return False
        return self.state == self.__DONE__ and self.verdict.is_fail()
    def is_skipped(self) -> bool:
        return self.state == self.__DONE__ and self.verdict.is_skip()
    def is_ignored(self) -> bool:
        return self.state == self.__DONE__ and self.verdict.is_ignore()
    def is_not_exec(self) -> bool:
        return self.state == self.__IDLE__ and self.verdict.is_not_exec()

    def launch(self):
        if self.f_skip and self.state == self.__DONE__:
            return
        if self.state != self.__READY__:
            self.verdict.set_fail()
            self.add_fail_cause('test-engine: starting non ready test')
            return
        self.state = self.__RUNNING__
        self.thread_timer.start()
        self.thread_exec.start()
        pass

    def wait(self):
        if self.verdict.is_skip():
            return
        if self.verdict.is_fail():
            return
        self.thread_exec.join()
        self.thread_exec = None
        if self.state != self.__DONE__:
            self.verdict.set_fail()
            self.add_fail_cause('test-engine: test thread finished without' + \
                               ' state update')
        if self.thread_timer:
            self.thread_timer.cancel()
            self.thread_timer = None
        pass

    def free_resources(self):
        if self.thread_exec:
            self.thread_exec = None
        if self.thread_timer:
            self.thread_timer.cancel()
            self.thread_timer = None
        if self.node_port:
            port_release(self.node_port)
            self.node_port = None
        pass

    def is_expired(self) -> bool:
        return self.stop_event.is_set()

    def force_stop(self):
        self.stop_event.set()
        pass

    def log(self, indentation=0, name_width=g_width_name_tst, endl=True):
        log(' ' * indentation, endl=False)

        flags_text = self.flags_text()
        name_len = len(self.name) + 1
        log(self.name + ' ', color=g_color_name_tst, endl=False)
        log_field(flags_text, name_width - name_len, align=pylog.RIGHT,
                  fill='-', fill_color=g_color_name_tst_fill, endl=False)

        self.verdict.log(endl=True, dimmed=True)

    def flags_legends():
        log('{:12}: {}'.format('HOST-EXP',
            'expected value by running the same script on host'),
            g_color_flag_host_exp)
        log('{:12}: {}'.format('REGEX',
            'expected value has a regular expressions'),
            g_color_flag_regex)
        log('{:12}: {}'.format('USR-SKP',
            'Test case execution is skipped by user request'),
            g_color_flag_skip_usr)
        log('{:12}: {}'.format('TST-SKP',
            'Test case verdict is skipped by test execution'),
            g_color_flag_skip_tst)
        log('{:12}: {}'.format('TOUT',
            'Test case timeout expires'),
            g_color_flag_timeout)
        log('{:12}: {}'.format('X-OUT',
            'Test case output does not match expected'),
            g_color_flag_fail)
        log('{:12}: {}'.format('X-PORT',
            'No available test nodes for this test'),
            g_color_flag_fail)
        log('{:12}: {}'.format('IGNORE_FAIL',
            'Ignore failure of the test case if any'),
            g_color_flag_ignore)
        log('{:12}: {}'.format('MPY-RAW',
            'Target node test with micropython RAW protocol'),
            g_color_flag_target)
        log('{:12}: {}'.format('MPY-RPL',
            'Target node test with micropython REPL protocol'),
            g_color_flag_target)
        log('{:12}: {}'.format('HOST',
            'Test will execute on the host machine'),
            g_color_flag_target)

    def flags_text(self) -> str:
        txt_l = []
        state_text = ''
        #    HOST-EXP <USR|TST>-SKIP FAIL_IGNORE REGEX TOUT
        # if self.f_pyhost and not (self.exp_file):
        if self.f_pyhost_exp:
            txt_l.append(log_get_color_str(g_color_flag_host_exp) + 'HOST-EXP')
        if self.f_skip:
            txt_l.append(log_get_color_str(g_color_flag_skip_usr) + 'USR-SKP')
        if self.f_skipped:
            txt_l.append(log_get_color_str(g_color_flag_skip_tst) + 'TST-SKP')
        if self.f_regex:
            txt_l.append(log_get_color_str(g_color_flag_regex) + 'REGEX')
        if self.f_expired:
            txt_l.append(log_get_color_str(g_color_flag_timeout) + 'TOUT')
        if self.f_unexpected_output:
            txt_l.append(log_get_color_str(g_color_flag_fail) + 'X-OUT')
        if (self.protocol_type == TestContext.PROTO_MPY_RAW or \
            self.protocol_type == TestContext.PROTO_MPY_FRIENDLY) and \
            self.node_port_name == None:
            txt_l.append(log_get_color_str(g_color_flag_fail) +'X-PORT')
        if self.f_ignore:
            txt_l.append(log_get_color_str(g_color_flag_ignore) + 'IGNORE_FAIL')
        if self.protocol_type == TestContext.PROTO_MPY_RAW:
            txt_l.append(log_get_color_str(g_color_flag_target) + 'MPY-RAW')
        elif self.protocol_type == TestContext.PROTO_MPY_FRIENDLY:
            txt_l.append(log_get_color_str(g_color_flag_target) + 'MPY-RPL')
        elif self.protocol_type == TestContext.PROTO_HOST_PROCESS:
            txt_l.append(log_get_color_str(g_color_flag_target) + 'HOST   ')

        if self.verdict.is_not_exec():
            state_text = ' [state: {}{}{}]'.format(
                log_get_color_str(pylog.YELLOW),
                self.state_str[self.state],
                log_get_color_str(pylog.DEFAULT))

        if len(txt_l) > 0:
            return log_get_color_str(pylog.DIMMED) + (' '.join(txt_l)) + \
                log_get_color_str(pylog.HILIGHT) + state_text + ' '
            # return log_get_color_str(pylog.GREY) + '[ ' + \
            #     log_get_color_str(pylog.DIMMED) + (' '.join(txt_l)) + \
            #     log_get_color_str(pylog.HILIGHT) + \
            #     log_get_color_str(pylog.GREY) + ' ]' + state_text
        return '' + state_text

    def add_fail_cause(self, cause):
        self.fail_cause.append(cause)

    def log_details(self, indentation=0, width=g_width_results_tst):
        # -- print results to screen
        if not self.f_hide_fail_details:
            def indent():
                log(' ' * indentation, endl=False)

            if self.verdict.is_not_exec():
                return

            if self.verdict.is_fail() or self.verdict.is_ignore():
                attr_w = 20
                if len(self.fail_cause) > 0:
                    indent()
                    log_field('Failure Cause', attr_w, color=pylog.BLUE)
                    log(self.fail_cause[0], color=pylog.RED)
                    cnt = 1
                    while cnt < len(self.fail_cause):
                        indent()
                        log((' ' * attr_w) + self.fail_cause[cnt],
                            color=pylog.RED)
                        cnt += 1

                if self.node_port_name:
                    indent()
                    log_field('Test Node Port', attr_w, color=pylog.BLUE)
                    log(self.node_port_name, color=pylog.RED)

                if self.missing_resource:
                    indent()
                    log_field('File not exist', attr_w, color=pylog.BLUE)
                    log(self.missing_resource, color=pylog.RED)

                if self.out or self.exp:
                    log_two_sided_output(left=self.out, right=self.exp,
                        indentation=indentation, width=width,
                        left_header='OUTPUT', right_header='EXPECTED',
                        mark_l_lines=[self.__fail_l_line],
                        mark_r_lines=[self.__fail_r_line])

            if self.verdict.is_pass() and self.f_show_out and self.out:
                log_one_side_output(self.out, indentation=indentation,
                                    width=width, header='Test Output')

        # -- dump results to files
        if self.results_dir and \
            (   self.verdict.is_fail() or \
                self.verdict.is_ignore() or \
                (self.verdict.is_skip() and self.f_skipped) \
            ) and \
            (self.out or self.exp):

            target_dir = self.results_dir
            if self.parent_job and self.parent_job.n_tests > 1:
                job_dir = self.parent_job.name
                target_dir += '/' + re.sub(r'[\/\.]', '__', job_dir)
            if not os.path.exists(target_dir):
                os.mkdir(target_dir)
            tst_name = re.sub(r'[\/\.]', '__', self.name)

            tst_path = target_dir + '/' + tst_name
            if self.out:
                with open(tst_path + '.out', 'w') as f:
                    f.write(filter_output_results(self.out))
            if self.exp:
                with open(tst_path + '.exp', 'w') as f:
                    f.write(filter_output_results(self.exp))

# ---------------------------------------------------------------------------- #
# Test Job
# ---------------------------------------------------------------------------- #
class TestJob:

    def __init__(self, name) -> None:
        self.name = name    # name for the test job
        self.ctx_list = []  # all contexts of the individual tests of this job
        self.n_tests = 0    # number of individual tests in this job

        self.verdict = TestVerdict()
        pass

    def add_test(self, ctx):
        if isinstance(ctx, TestContext):
            self.ctx_list.append(ctx)
            self.n_tests += 1
            ctx.parent_job = self
        else:
            log('error: adding wrong test context to test job', pylog.RED)
        return self

    def run(self):

        self.log_begin()

        # -- init execution indicators
        fail_ind = False
        skip_ind = False
        ignore_ind = False

        # -- prepare all tests
        for ctx in self.ctx_list:
            if ctx.is_not_exec():
                ctx.prepare()

        # -- launch all test job individuals to the corresponding test nodes
        for ctx in self.ctx_list:
            if ctx.is_ready():
                ctx.launch()

        # -- wait for all to finish or timeout
        for ctx in self.ctx_list:
            # -- wait until the test finish
            ctx.wait()

            # -- update job execution flags
            if ctx.is_failed():
                fail_ind = True
            elif ctx.is_skipped():
                skip_ind = True
            elif ctx.is_ignored():
                ignore_ind = True

            # -- free/release this test acquired resources
            ctx.free_resources()

        # -- deduce the final job verdict
        if fail_ind:
            self.verdict.set_fail()
            g_test_stats.test_fail(self.name)
        elif ignore_ind:
            self.verdict.set_ignore()
            g_test_stats.test_ignore(self.name)
        elif skip_ind:
            self.verdict.set_skip()
            g_test_stats.test_skip(self.name)
        else:
            pass_count = 0
            for ctx in self.ctx_list:
                if ctx.is_passed():
                    pass_count += 1
            if pass_count == self.n_tests and self.n_tests > 0:
                self.verdict.set_pass()
                g_test_stats.test_pass(self.name)
            else:
                self.verdict.set_invalid()
                g_test_stats.test_invalid(self.name)

        self.log_end()

    def log_begin(self):
        tag_text = '{}{}{}'.format(
            log_get_color_str(g_color_tag_job
                              if self.n_tests > 1 else g_color_tag_tst),
            'JOB:{}'.format(self.n_tests) if self.n_tests > 1 else 'TEST',
            log_get_color_str(pylog.DEFAULT),
            )
        log_field(tag_text, g_width_tag_job, endl=False)
        log(self.name, endl=False)
        pylog.log_flush()
        pass

    def log_end(self):

        flags_txt = self.ctx_list[0].flags_text()  if self.n_tests == 1 else ''
        name_len = len(self.name) + 1
        log(' ', endl=False)
        log_field(flags_txt, g_width_name_job - name_len,
                    align=pylog.RIGHT,
                    fill='-', fill_color=g_color_name_job_fill, endl=False)
        self.verdict.log(width=g_width_verdict, endl=True)

        if self.n_tests == 1:
            self.ctx_list[0].log_details(indentation=g_indent_job_details,
                                         width=g_width_results_job)
        else:
            counter = 1
            for ctx in self.ctx_list:
                log(' ' * g_width_tag_job, endl=False)
                t_msg = 'T{}:'.format(counter)
                t_msg_len = pylog.get_colored_str_len(t_msg)
                log_field(t_msg, g_width_tag_tst, color=g_color_tag_tst,
                          endl=False)
                counter += 1
                ctx.log()
                ctx.log_details(indentation=g_indent_tst_details,
                                width=g_width_results_tst)

def job_mgr_test():
    cmd_parser = argparse.ArgumentParser(
        formatter_class = argparse.RawDescriptionHelpFormatter,
        description = 'test job management')

    cmd_parser.add_argument('--port', nargs='*', help='node serial port')

    args = cmd_parser.parse_args()

    if args.port:
        port_add(args.port)
    
    test_file = 'ext/micropython/tests/extmod/ujson_dumps_float.py'
    t1 = TestContext(test_file, [test_file])
    t1.f_pyhost = True

    test_file = 'ext/micropython/tests/extmod/ubinascii_unhexlify.py'
    t2 = TestContext(test_file, [test_file])
    t2.f_pyhost = True
    t2.f_skip = True

    test_file = 'ext/micropython/tests/extmod/machine_timer.py'
    t3 = TestContext(test_file, [test_file])
    # t3.f_pyhost = True

    job = TestJob('test-job-of-two-tests')

    job.add_test(t1)
    job.add_test(t2)
    job.add_test(t3)

    job.log_begin()
    job.log_end()

    job.run()

    test_file = 'ext/micropython/tests/extmod/ubinascii_unhexlify.py'
    job = TestJob(test_file)
    tt = TestContext(test_file, [test_file])
    tt.f_pyhost = True
    job.add_test(tt)

    job.run()

    g_test_stats.log()

# ---------------------------------------------------------------------------- #
# utils
# ---------------------------------------------------------------------------- #
def filter_output_results(results_str):
    # convert all '\r\n' or '\n\r' to '\n'
    results_str = re.sub('[\r\n]+', '\n', results_str)

    # remove all trailing '\n'
    while results_str[-1:] == '\n':
            results_str = results_str[:-1]
    
    # remove any included terminal color information
    results_str = re.sub(r'\x1b\[\d*m', '', results_str)

    return results_str.strip()

def filter_test_text(text):
    # convert all '\r\n' or '\n\r' to '\n'
    text = re.sub('[\r\n]+', '\n', text)

    # remove all trailing '\n'
    while text[-1:] == '\n':
            text = text[:-1]

    return text.strip()

def replace_non_visible_chars(text):
    new_text = ''
    for c in text:
        i = ord(c)
        if (i >= 32 and i <=126) or c == '\n' or c == '\033':
            new_text += c
        else:
            new_text += log_get_color_str(pylog.YELLOW) + \
                '\\x{:02x}'.format(i) + \
                log_get_color_str(pylog.DEFAULT)
    return new_text

def compare_test_results(output, expected, is_regex) -> (int, int):
    exp = filter_output_results(expected)
    out = filter_output_results(output)
    exp_lines = exp.splitlines()
    out_lines = out.splitlines()
    if is_regex:
        # convert into array of lines
        skip_until = False
        zero_or_more = False
        exp_counter = 0
        def get_next_exp_regex():
            nonlocal exp_counter
            nonlocal skip_until
            nonlocal zero_or_more
            if exp_counter >= len(exp_lines):
                return re.compile('')
            if exp_lines[exp_counter] == '########':
                skip_until = True
            elif exp_lines[exp_counter].startswith('########'):
                zero_or_more = True
                regex = re.compile(
                    exp_lines[exp_counter].removeprefix('########'))
                exp_counter += 1
                return regex
            else:
                skip_until = False
            while exp_counter < len(exp_lines) and \
                    exp_lines[exp_counter] == '########':
                exp_counter += 1
            if exp_counter >= len(exp_lines):
                return re.compile('')
            regex = re.compile(exp_lines[exp_counter])
            exp_counter += 1
            return regex
        exp_regex = get_next_exp_regex()
        out_lineno = 0
        for out_line in out_lines:
            out_lineno += 1
            # print("*:{} +:{}  OUT:[ {} ] EXP:[ {} ]".format(
            #     skip_until, zero_or_more, out_line, exp_lines[exp_counter-1]))
            matched = exp_regex.fullmatch(out_line)
            cont = False
            while zero_or_more:
                if matched:
                    cont = True
                    break
                else:
                    zero_or_more = False
                    exp_regex = get_next_exp_regex()
                    matched = exp_regex.fullmatch(out_line)
            if cont:
                continue

            if skip_until:
                if matched:
                    exp_regex = get_next_exp_regex()
                else:
                    continue
            elif not matched:
                return (out_lineno, exp_counter)
            else:
                exp_regex = get_next_exp_regex()
        if exp_counter < len(exp_lines):
            return (out_lineno, exp_counter)
        else:
            return (0, 0)
    else:
        lineno = 0
        out_lines_count = len(out_lines)
        exp_lines_count = len(exp_lines)
        while lineno < out_lines_count and lineno < exp_lines_count:
            if exp_lines[lineno] != out_lines[lineno]:
                break
            lineno += 1
            pass
        if lineno < out_lines_count or lineno < exp_lines_count:
            return (lineno + 1, lineno + 1)
        return (0, 0)

def get_list_file_contents(lists_files) -> list:
    ret_list = []
    for list_file in lists_files:
        if not os.path.exists(list_file):
            log("file '{}' not exist".format(list_file), pylog.RED)
            exit(1)
        with open(list_file, 'r') as reader:
            contents = reader.readlines()
            for file in contents:
                file_name = file.strip()
                if file_name != '':
                    ret_list.append(file.strip())
    return ret_list

def get_dir_files_contents(dirs_list, files_extension, exceptions=None) -> list:
    ret_list = []
    for dir in dirs_list:
        for f in os.listdir(dir):
            if f.endswith(files_extension):
                include = True
                if exceptions:
                    for e in exceptions:
                        if f == os.path.basename(e):
                            include = False
                            break
                if include:
                    ret_list.append(dir + '/' + f)
    return ret_list

def check_files_existence(files) -> object:
    for f in files:
        if not os.path.exists(f):
            return f
    return None

def log_two_sided_output(left, right, indentation=0, width=g_width_total,
                         left_header=None, right_header=None,
                         mark_l_lines=[], mark_r_lines=[]):
    def indent():
        log(' ' * indentation, endl=False)
    page_w = (width - ( 1 if (width%2) != 0 else 2 ) ) // 2

    # -- log header
    def disp_border():
        pylog.log('~' * page_w, color=pylog.GREY, endl=False)
        log('+', endl=False, color=pylog.GREY)
        pylog.log('~' * page_w, color=pylog.GREY)

    if left_header or right_header:
        indent()
        log_field(left_header if left_header else '', page_w,
                  align=pylog.CENTER, color=pylog.BLUE,
                  fill='~', fill_color=pylog.GREY)
        log('+', endl=False, color=pylog.GREY)
        log_field(right_header if right_header else '', page_w,
                  align=pylog.CENTER, color=pylog.BLUE, endl=True,
                  fill='~', fill_color=pylog.GREY)

    l_lines = []
    r_lines = []
    if left:
        left = replace_non_visible_chars(left)
        l_lines = left.split('\n')
    if right:
        right = replace_non_visible_chars(right)
        r_lines = right.splitlines()

    l_rows = len(l_lines)
    r_rows = len(r_lines)
    rows = l_rows
    if rows < r_rows:
        rows = r_rows

    l_color = log_get_color_str(pylog.DEFAULT)
    r_color = l_color
    for row in range(rows):

        l_line_len = pylog.get_colored_str_len(l_lines[row]) \
            if left and row < l_rows else 0
        r_line_len = pylog.get_colored_str_len(r_lines[row]) \
            if right and row < r_rows else 0

        l_line_idx = 0
        r_line_idx = 0

        lineno_w = 5
        line_w = page_w - lineno_w

        disp_lineno = True
        while l_line_idx < l_line_len or r_line_idx < r_line_len or disp_lineno:
            indent()
            hi_l_line = True if row + 1 in mark_l_lines else False
            hi_r_line = True if row + 1 in mark_r_lines else False

            mark_l_char = '>' if hi_l_line else ' '
            mark_r_char = '>' if hi_r_line else ' '

            if hi_l_line:
                pylog.log_bkg_color(pylog.RED)
            # -- left side
            if row < l_rows:
                if disp_lineno:
                    log('{}{}{:{}} {}'.format(
                        mark_l_char,
                        log_get_color_str(pylog.DIMMED),
                        row + 1, lineno_w-2,
                        log_get_color_str(pylog.HILIGHT)),
                        color=pylog.GREY, endl= False)
                else:
                    log(' ' * lineno_w, endl=False)

                if l_line_idx < l_line_len:
                    next_idx = l_line_idx + line_w
                    if next_idx > l_line_len:
                        next_idx = l_line_len
                    disp_part = l_color + l_lines[row][
                        pylog.get_colored_str_pos(l_lines[row], l_line_idx):
                        pylog.get_colored_str_pos(l_lines[row], next_idx)]
                    log_field(disp_part, line_w, endl=False)
                    l_line_idx = next_idx
                    l_color = pylog.get_last_used_color_str(disp_part)
                else:
                    log(' ' * line_w, endl=False)
            else:
                log(' ' * page_w, endl=False)

            # -- separation
            if hi_l_line:
                pylog.log_bkg_color(pylog.DEFAULT)
                log('', endl=False)

            log('|', color=pylog.GREY, endl=False)

            if hi_r_line:
                pylog.log_bkg_color(pylog.GREEN)

            # -- right side
            if row < r_rows:
                if disp_lineno:
                    log('{}{}{:{}} {}'.format(
                        mark_r_char,
                        log_get_color_str(pylog.DIMMED),
                        row + 1, lineno_w-2,
                        log_get_color_str(pylog.HILIGHT)),
                        color=pylog.GREY, endl= False)
                else:
                    log(' ' * lineno_w, endl=False)

                if r_line_idx < r_line_len:
                    next_idx = r_line_idx + line_w
                    if next_idx > r_line_len:
                        next_idx = r_line_len
                    disp_part = r_color + r_lines[row][
                        pylog.get_colored_str_pos(r_lines[row], r_line_idx):
                        pylog.get_colored_str_pos(r_lines[row], next_idx)]
                    log_field(disp_part, line_w, endl=False)
                    r_line_idx = next_idx
                    r_color = pylog.get_last_used_color_str(disp_part)
                else:
                    log('', endl=False)
            else:
                log(' ' * (line_w + lineno_w), endl=False)

            if hi_r_line:
                pylog.log_bkg_color(pylog.DEFAULT)

            log('')

            disp_lineno = False

    indent()
    disp_border()
    pass

def log_one_side_output(text, indentation=0, width=g_width_total, header=None):
    def indent():
        log(' ' * indentation, endl=False)
    page_w = width

    indent()
    if header:
        log_field(header, page_w, align=pylog.CENTER, color=pylog.BLUE,
                  fill='~', fill_color=pylog.GREY, endl=True)
    else:
        pylog.log('~' * page_w, color=pylog.GREY)

    t_lines = []
    if text:
        text = replace_non_visible_chars(text)
        t_lines = text.split('\n')

    rows = len(t_lines)

    t_color = log_get_color_str(pylog.DEFAULT)
    for row in range(rows):
        t_line_len = pylog.get_colored_str_len(t_lines[row]) \
            if text and row < rows else 0

        t_line_idx = 0

        lineno_w = 4
        line_w = page_w - lineno_w

        disp_lineno = True
        while t_line_idx < t_line_len or disp_lineno:
            indent()
            if row < rows:
                if disp_lineno:
                    log('{:{}} '.format(row + 1, lineno_w-1),
                        color=pylog.GREY, endl= False)
                else:
                    log(' ' * lineno_w, endl=False)

                if t_line_idx < t_line_len:
                    next_idx = t_line_idx + line_w
                    if next_idx > t_line_len:
                        next_idx = t_line_len
                    disp_part = t_lines[row][
                        pylog.get_colored_str_pos(t_lines[row], t_line_idx):
                        pylog.get_colored_str_pos(t_lines[row], next_idx)]
                    log(t_color, endl=False)
                    log_field(disp_part, line_w, endl=True)
                    t_line_idx = next_idx
                    t_color = pylog.get_last_used_color_str(disp_part)
                else:
                    log(' ' * line_w)
            else:
                log(' ' * page_w)

            disp_lineno = False

    indent()
    pylog.log('~' * page_w, color=pylog.GREY)
    pass

# ---------------------------------------------------------------------------- #
# Section Helpers
# ---------------------------------------------------------------------------- #
g_color_section_header = pylog.YELLOW
g_color_subsection_header = pylog.GREEN

def log_section_header(header):
    log('')
    log_field(header, g_width_total, color=g_color_section_header,
              align=pylog.CENTER, fill='=', fill_color=pylog.BLUE, endl=True)
def log_subsection_header(header):
    log_decorated_header(header, g_width_total, '-', g_color_subsection_header)

def log_section_separator():
    log_line(length=g_width_total, char='=', color=pylog.BLUE)

def log_test_cases_list(cap, test_cases, remove_prefix=''):
    log_tab_inc()
    width = g_width_total - 2 * 4
    log_field('{}--- [ {}{}{} ]'.format(
        log_get_color_str(pylog.GREY),
        log_get_color_str(pylog.PURPLE),
        cap,
        log_get_color_str(pylog.GREY),
        ), width=width, endl=True, fill='-', fill_color=pylog.GREY)
    log_tab_dec()
    log_list(test_cases, horiz=True, wrap_after=2, numbering=True,
             numbering_color=pylog.GREY, item_width=(width-4) // 2,
             color=pylog.GREY,
             remove_prefix=remove_prefix)

# ---------------------------------------------------------------------------- #
# JSON Helpers
# ---------------------------------------------------------------------------- #
def validate_json_format(json_obj, json_format) -> list:
    error = []
    for elem in json_obj:
        if elem in json_format:
            if type(json_obj[elem]) == json_format[elem]:
                pass
            else:
                error.append(
                    '{}JSON element {}\'{}\'{} type is '.format(
                            log_get_color_str(pylog.RED),
                            log_get_color_str(pylog.DEFAULT),
                            elem,
                            log_get_color_str(pylog.RED)
                        ) + '{}{}{} but found wrong {}{}{}'.format(
                            log_get_color_str(pylog.GREEN),
                            json_format[elem],
                            log_get_color_str(pylog.RED),
                            log_get_color_str(pylog.PURPLE),
                            type(json_obj[elem]),
                            log_get_color_str(pylog.RED)
                        ))
        else:
            error.append('{}JSON element {}\'{}\'{} is not allowed'.format(
                log_get_color_str(pylog.RED),
                log_get_color_str(pylog.DEFAULT),
                elem,
                log_get_color_str(pylog.RED),
            ))
    return error

# ---------------------------------------------------------------------------- #
# Test Engine Class
# ---------------------------------------------------------------------------- #
class TestEngine:

    __initialized = False

    __test_cases:str = []
    __regex_test_cases:str = []
    __ignore_test_cases:str = []
    __skip_test_cases:str = []

    __test_suites:str = []
    __test_suites_objs = []

    __timeout = None
    __results_dir = None
    __disable_colors = False
    __show_pass_results = False
    __hide_fail_results = False
    __enable_host_expect = False

    __ports = []

    def __init__(self) -> None:
        if TestEngine.__initialized:
            return

        # -- command line options specifications -------------------------------
        parser = argparse.ArgumentParser()

        array_opts = {
            # opt           :help
            '--ports' : 'serial ports of the test nodes devices',
            '--test-cases'  :'test cases files (.py or .json)',
            '--test-dirs'   :'directories containing test cases',
            '--regex-test-cases':'test cases containing regex in its expect',
            '--regex-test-cases-lists':'files whose lines are test cases' + \
                                       ' containing regex in its expect',
            '--ignore-test-cases':'test cases to be ignored if failed',
            '--ignore-test-cases-lists':'files whose lines are test cases' + \
                                        ' to be ignored if failed',
            '--skip-test-cases':'test cases to be skipped during the execution',
            '--skip-test-cases-lists':'files whose lines are test cases' + \
                                      ' to be skipped during the execution',
            '--test-suites':'list of test suites config files'
        }

        single_opts = {
            '--timeout' : {
                'default' : 30,
                'help':     'a default timeout for any test case'},
            '--results-dir' : {
                'default' : None,
                'help':     'default dir for failed outputs dumping'},
        }

        flag_opts = {
            '--disable-colors' : 'disables terminal colors',
            '--show-pass-results' : 'enable passed test cases output verbosity',
            '--hide-fail-results' : 'hide failed test cases output verbosity',
            '--enable-host-expect': 'enable expected value prediction using' + \
                ' the host python interpreter for relavent test cases',
            '--show-flags-legends':'show test flags legends'
        }

        for opt in array_opts:
            parser.add_argument(opt, nargs='+', help=array_opts[opt])
        for opt in single_opts:
            parser.add_argument(opt, default=single_opts[opt]['default'],
                                help=single_opts[opt]['help'])
        for opt in flag_opts:
            parser.add_argument(opt, action='store_true', help=flag_opts[opt])

        args = parser.parse_args()

        if args.show_flags_legends:
            TestContext.flags_legends()
            exit(0)

        # -- test nodes initialization -----------------------------------------
        if args.ports:
            self.__ports = args.ports
        if len(self.__ports) == 0:
            self.__ports = port_scan()

        if len(self.__ports) == 0:
            log('no specified or connected device', pylog.RED)
            parser.print_help()
            exit(0)

        port_add(self.__ports)

        # -- extracting CLI provided specs -------------------------------------

        if args.test_cases:
            TestEngine.__test_cases.extend(args.test_cases)
        if args.test_dirs:
            TestEngine.__test_cases.extend(collect_test_cases(args.test_dirs))

        if args.regex_test_cases:
            TestEngine.__regex_test_cases.extend(args.regex_test_cases)
        if args.regex_test_cases_lists:
            TestEngine.__regex_test_cases.extend(
                get_list_file_contents(args.regex_test_cases_lists))

        if args.ignore_test_cases:
            TestEngine.__ignore_test_cases.extend(args.ignore_test_cases)
        if args.ignore_test_cases_lists:
            TestEngine.__ignore_test_cases.extend(
                get_list_file_contents(args.ignore_test_cases_lists))

        if args.skip_test_cases:
            TestEngine.__skip_test_cases.extend(args.skip_test_cases)
        if args.skip_test_cases_lists:
            TestEngine.__skip_test_cases.extend(
                get_list_file_contents(args.skip_test_cases_lists))

        if args.test_suites:
            TestEngine.__test_suites.extend(args.test_suites)

        TestEngine.__timeout = args.timeout
        TestEngine.__results_dir = args.results_dir

        if args.disable_colors:
            TestEngine.__disable_colors = args.disable_colors
            pylog.log_disable_colors()
        if args.show_pass_results:
            TestEngine.__show_pass_results = args.show_pass_results
        if args.hide_fail_results:
            TestEngine.__hide_fail_results = args.hide_fail_results
        if args.enable_host_expect:
            TestEngine.__enable_host_expect = args.enable_host_expect

        g_test_stats.add_tests(len(self.__test_cases))

        self.__init_test_suites()

        TestEngine.__initialized = True
        pass

    def __init_test_suites(self):
        for ts in TestEngine.__test_suites:
            self.__test_suites_objs.append(TestSuite(ts, self))

    def test_cases(self) -> list:
        return self.__test_cases
    def is_regex_exp(self, test_case) -> bool:
        return test_case in self.__regex_test_cases
    def is_ignored(self, test_case) -> bool:
        return test_case in self.__ignore_test_cases
    def is_skipped(self, test_case) -> bool:
        return test_case in self.__skip_test_cases

    def timeout(self) -> int:
        return self.__timeout
    def show_passed_result(self) -> bool:
        return self.__show_pass_results
    def hide_failed_result(self) -> bool:
        return self.__hide_fail_results
    def host_expect_enabled(self) -> bool:
        return self.__enable_host_expect
    def results_dir(self):
        return self.__results_dir

    def log(self):
        log_subsection_header('Test Engine Options')
        pylog.log_tab_inc()
        opt_color_str = log_get_color_str(pylog.PURPLE)
        def_color_str = log_get_color_str(pylog.DEFAULT)
        def log_key_value_pair(key, value):
            log('{}{:25s}{} {}'.format(opt_color_str, key, def_color_str,value))
        log_key_value_pair('show-pass-results', TestEngine.__show_pass_results)
        log_key_value_pair('hide-fail-results', TestEngine.__hide_fail_results)
        log_key_value_pair('disable-colors', TestEngine.__disable_colors)
        log_key_value_pair('enable-host-expect',
                           TestEngine.__enable_host_expect)
        log_key_value_pair('timeout', TestEngine.__timeout)
        log_key_value_pair('results-dir', TestEngine.__results_dir)
        log('{}test nodes {}'.format(opt_color_str,def_color_str))
        log_test_nodes()
        pylog.log_tab_dec()

        log_test_cases_list('CLI test-cases', TestEngine.__test_cases)
        log_test_cases_list('CLI ignore-test-cases',
                            TestEngine.__ignore_test_cases)
        log_test_cases_list('CLI skip-test-cases',
                            TestEngine.__skip_test_cases)
        log_test_cases_list('CLI regex-test-cases',
                            TestEngine.__regex_test_cases)
        log_test_cases_list('Test Suites', TestEngine.__test_suites)

        for ts in self.__test_suites_objs:
            ts.log()

    def exec(self):
        for tc in self.__test_cases:
            TestCase(tc, parent=self)
        for ts in self.__test_suites_objs:
            ts.exec()
            pass
        pass

# ---------------------------------------------------------------------------- #
# Test Suite Class
# ---------------------------------------------------------------------------- #
class TestSuite:
    __json_fmt = {
        'name'                  : str,
        'relative-path'         : str,
        'show-pass-results'     : bool,
        'hide-fail-results'     : bool,
        'enable-host-expect'    : bool,
        'timeout'               : int,
        'results-dir'           : str,
        'test-cases'            : list,
        'test-dirs'             : list,
        'regex-test-cases'      : list,
        'regex-test-cases-lists': list,
        'ignore-test-cases'     : list,
        'ignore-test-cases-lists': list,
        'skip-test-cases'       : list,
        'skip-test-cases-lists' : list
    }
    def __init__(self, json_file, test_engine:TestEngine=None) -> None:
        self.__json_file = json_file
        self.__te = test_engine
        self.__name = None
        self.__relative = ''
        self.__error = []
        self.__test_cases = []
        self.__regex_test_cases = []
        self.__ignore_test_cases = []
        self.__skip_test_cases = []
        self.__show_pass_results = False
        self.__hide_fail_results = False
        self.__enable_host_expect = False
        self.__timeout = 30
        self.__results_dir = None
        if test_engine:
            self.__show_pass_results = test_engine.show_passed_result()
            self.__hide_fail_results = test_engine.hide_failed_result()
            self.__enable_host_expect = test_engine.host_expect_enabled()
            self.__timeout = test_engine.timeout()
            self.__results_dir = test_engine.results_dir()
        self.__parse_test_suite_json(json_file)
        pass

    def __parse_test_suite_json(self, json_file):
        if not os.path.exists(json_file):
            self.__error = ['test suite .json file does not exist']
            return
        with open(json_file, 'r') as f:
            try:
                m = json.loads(f.read())
            except json.JSONDecodeError as e:
                self.__error.extend(['JSON file decode error {}{}: {}{}'.
                    format(log_get_color_str(pylog.DEFAULT), json_file, e,
                    log_get_color_str(pylog.RED)
                    )])
                return
            err = validate_json_format(m, TestSuite.__json_fmt)
            if len(err) > 0:
                self.__error.extend(err)
                return

            if 'relative-path' in m:
                self.__relative = m['relative-path'] + '/'

            if 'name' in m:
                self.__name = m['name']
            if 'show-pass-results' in m:
                self.__show_pass_results = m['show-pass-results']
            if 'hide-fail-results' in m:
                self.__hide_fail_results = m['hide-fail-results']
            if 'enable-host-expect' in m:
                self.__enable_host_expect = m['enable-host-expect']
            if 'timeout' in m:
                self.__timeout = m['timeout']
            if 'results-dir' in m and len(m['results-dir']) > 0:
                self.__results_dir = m['results-dir']

            if 'test-cases' in m:
                l = [self.__relative + tc for tc in m['test-cases']]
                self.__test_cases.extend(l)
            if 'test-dirs' in m:
                l = [self.__relative + dir for dir in m['test-dirs']]
                self.__test_cases.extend(collect_test_cases(l))

            if 'regex-test-cases' in m:
                l = [self.__relative + tc for tc in m['regex-test-cases']]
                self.__regex_test_cases.extend(l)
            if 'regex-test-cases-lists':
                l = [self.__relative + f for f in m['regex-test-cases-lists']]
                self.__regex_test_cases.extend(get_list_file_contents(l))
            if 'ignore-test-cases' in m:
                l = [self.__relative + tc for tc in m['ignore-test-cases']]
                self.__ignore_test_cases.extend(l)
            if 'ignore-test-cases-lists':
                l = [self.__relative + f for f in m['ignore-test-cases-lists']]
                self.__ignore_test_cases.extend(get_list_file_contents(l))
            if 'skip-test-cases' in m:
                l = [self.__relative + tc for tc in m['skip-test-cases']]
                self.__skip_test_cases.extend(l)
            if 'skip-test-cases-lists':
                l = [self.__relative + f for f in m['skip-test-cases-lists']]
                self.__skip_test_cases.extend(get_list_file_contents(l))

            g_test_stats.add_tests(len(self.__test_cases))
            pass

    def test_cases(self) -> list:
        return self.__test_cases
    def is_regex_exp(self, test_case) -> bool:
        return test_case in self.__regex_test_cases or \
            self.__te.is_regex_exp(test_case)
    def is_ignored(self, test_case) -> bool:
        return test_case in self.__ignore_test_cases or \
            self.__te.is_ignored(test_case)
    def is_skipped(self, test_case) -> bool:
        return test_case in self.__skip_test_cases or \
            self.__te.is_skipped(test_case)

    def timeout(self) -> int:
        return self.__timeout
    def show_passed_result(self) -> bool:
        return self.__show_pass_results
    def hide_failed_result(self) -> bool:
        return self.__hide_fail_results
    def host_expect_enabled(self) -> bool:
        return self.__enable_host_expect
    def results_dir(self):
        return self.__results_dir

    def log(self):
        log('\n')
        name = self.__name if self.__name else self.__json_file
        log_subsection_header('Test Suite >> ' + name)
        pylog.log_tab_inc()
        if self.__error:
            for e in self.__error:
                log('error: ' + e, pylog.RED)
            pylog.log_tab_dec()
            return
        opt_color_str = log_get_color_str(pylog.PURPLE)
        def_color_str = log_get_color_str(pylog.DEFAULT)
        def log_key_value_pair(key, value):
            log('{}{:25s}{} {}'.format(opt_color_str, key, def_color_str,value))
        log_key_value_pair('show-pass-results', self.__show_pass_results)
        log_key_value_pair('enable-host-expect', self.__enable_host_expect)
        log_key_value_pair('timeout', self.__timeout)
        log_key_value_pair('results-dir', self.__results_dir)
        log_key_value_pair('relative-base-path', self.__relative)
        pylog.log_tab_dec()

        prefix = 'TS :: ' + name + ' :: '
        remove_prefix = self.__relative
        log_test_cases_list(prefix + 'test-cases',
            self.__test_cases, remove_prefix=remove_prefix)
        log_test_cases_list(prefix + 'ignore-test-cases',
            self.__ignore_test_cases, remove_prefix=remove_prefix)
        log_test_cases_list(prefix + 'skip-test-cases',
            self.__skip_test_cases, remove_prefix=remove_prefix)
        log_test_cases_list(prefix + 'regex-test-cases',
            self.__regex_test_cases, remove_prefix=remove_prefix)

    def exec(self):
        log_section_header('Executing Test Suite: {}{}'.format(
                log_get_color_str(pylog.DEFAULT),
                self.__name if self.__name else self.__json_file))
        for tc in self.__test_cases:
            TestCase(tc, parent=self)
        pass

# ---------------------------------------------------------------------------- #
# Test Case Helpers
# ---------------------------------------------------------------------------- #
def collect_test_cases(dirs_list):
    ret_list = []
    for dir in dirs_list:
        if not (os.path.exists(dir) and os.path.isdir(dir)):
            continue
        for f in sorted(os.listdir(dir)):
            p = dir + '/' + f
            include = False
            if os.path.isdir(p) and os.path.exists(p + '/' + '__job__.json'):
                include = True
            elif f.endswith('.py') or f.endswith('.json'):
                include = True
            if include:
                ret_list.append(p)
    return ret_list

class TestCase:

    __json_tc_fmt = {   # allowed attributes in the single test-case JSON
        'tst-proc'      : list,
        'exp-file'      : str,
        'exp-value'     : list,
        'timeout'       : int,
        'regex'         : bool,
        'skip'          : bool,
        'ignore'        : bool,
        'show-pass'     : bool,
        'hide-fail'     : bool,
    }

    __json_job_fmt = {  # allowed attributes in the job test-case JSON
        'job-tests'     : list,
        'protocol'      : str,
        'name'          : str,
        'tst-proc'      : list,
        'exp-file'      : str,
        'exp-value'     : list,
        'exp-host'      : bool,
        'show-pass'     : bool,
        'hide-fail'     : bool,
        'timeout'       : int,
        'regex'         : bool,
        'skip'          : bool,
        'ignore'        : bool,
    }

    __proto_map = {
        "micropython-repl-raw"      : TestContext.PROTO_MPY_RAW,
        "micropython-repl-friendly" : TestContext.PROTO_MPY_FRIENDLY,
        "host-process"              : TestContext.PROTO_HOST_PROCESS
    }

    def __init__(self, test_case, parent=None) -> None:
        # the test_case can be a single test or a job test
        self.__ref = test_case
        self.__name = test_case
        self.__parent = parent

        # init test case attributes
        self.__tst_proc = []
        self.__exp_file = None
        self.__exp_value = None
        self.__skip = False
        self.__ignore = False
        self.__regex = False
        self.__pyhost = False
        self.__show_out = False
        self.__hide_fail_res = False
        self.__timeout = None
        self.__results_dir = None
        self.__proto = TestContext.PROTO_MPY_RAW

        # inherit initial attributes values from the parent Test Suite
        if parent:
            self.__skip = parent.is_skipped(test_case)
            self.__ignore = parent.is_ignored(test_case)
            self.__regex = parent.is_regex_exp(test_case)
            self.__pyhost = parent.host_expect_enabled()
            self.__show_out = parent.show_passed_result()
            self.__hide_fail_res = parent.hide_failed_result()
            self.__timeout = parent.timeout()
            self.__results_dir = parent.results_dir()

        self.__error = []

        self.__process_test_case()
        pass

    def __process_test_case(self):
        if os.path.isdir(self.__ref):
            self.__process_job_test_case()
        elif self.__ref.endswith('.py'):
            self.__process_single_py_test_case()
        elif self.__ref.endswith('.json'):
            self.__process_single_json_test_case()
        else:
            self.__error = ['unsupported test case file']
            self.__process_test_case_exception()

    def __process_single_py_test_case(self):
        exp_file = self.__ref + '.exp'
        if os.path.exists(exp_file):
            self.__exp_file = exp_file
        self.__tst_proc = [self.__ref]
        TestJob(self.__ref).add_test(self.__create_test_object()).run()
        pass

    def __process_single_json_test_case(self):
        if not os.path.exists(self.__ref):
            self.__error = ['test-case file not exist']
            self.__process_test_case_exception()
            return
        with open(self.__ref, 'r') as f:
            try:
                m = json.loads(f.read())
            except json.JSONDecodeError as e:
                self.__error.extend(['JSON file decode error {}{}: {}{}'.
                    format(log_get_color_str(pylog.DEFAULT), self.__ref, e,
                    log_get_color_str(pylog.RED)
                    )])
                self.__process_test_case_exception()
                return
            err = validate_json_format(m, TestCase.__json_tc_fmt)
            if len(err) > 0:
                self.__error.extend(err)
                self.__process_test_case_exception()
                return
            if not 'tst-proc' in m or len(m['tst-proc']) == 0:
                # test procedure is mandatory, so abort
                self.__error = ['test procedure not provided']
                self.__process_test_case_exception()
                return
            self.__proto = TestContext.PROTO_MPY_FRIENDLY
            self.__tst_proc = m['tst-proc']
            basedir = os.path.dirname(self.__ref)
            if 'exp-file' in m and os.path.exists(basedir+'/' + m['exp-file']):
                self.__exp_file = m['exp-file']
            elif 'exp-value' in m:
                self.__exp_value = '\n'.join(m['exp-value'])
            elif os.path.exists(self.__ref + '.exp'):
                self.__exp_file = self.__ref + '.exp'
            else:
                pass # no expected value for this test case
            if 'timeout' in m:
                self.__timeout = m['timeout']
            if 'regex' in m:
                self.__regex = m['regex']
            if 'skip' in m:
                self.__skip = m['skip']
            if 'ignore' in m:
                self.__ignore = m['ignore']
            if 'show-pass' in m:
                self.__show_out = m['show-pass']
            if 'hide-fail' in m:
                self.__hide_fail_res = m['hide-fail']
        TestJob(self.__ref).add_test(self.__create_test_object()).run()
        pass

    def __process_job_test_case(self):
        with open(self.__ref + '/__job__.json', 'r') as f:
            try:
                m = json.loads(f.read())
            except json.JSONDecodeError as e:
                self.__error.extend(['JSON file decode error {}{}: {}{}'.
                    format(log_get_color_str(pylog.DEFAULT), self.__ref, e,
                    log_get_color_str(pylog.RED)
                    )])
                self.__process_test_case_exception()
                return
            err = validate_json_format(m, TestCase.__json_job_fmt)
            if 'job-tests' in m and type(m['job-tests']) == list:
                cnt = 1
                for t in m['job-tests']:
                    prefix = 'in {}T{}{}, '.format(
                        log_get_color_str(pylog.DEFAULT), cnt,
                        log_get_color_str(pylog.RED))
                    err.extend([ prefix + e for e in
                        validate_json_format(t, TestCase.__json_job_fmt)])
                    for elem in ['protocol', 'tst-proc']:
                        if not elem in t:
                            err.extend([prefix +
                                'element {}\'{}\'{} must be specified'.format(
                                    log_get_color_str(pylog.DEFAULT), elem,
                                    log_get_color_str(pylog.RED)
                                )])
                    if 'protocol' in t and \
                        not t['protocol'] in TestCase.__proto_map.keys():
                        err.extend([prefix +
                            'element {}\'protocol\'{} '.format(
                                log_get_color_str(pylog.DEFAULT),
                                log_get_color_str(pylog.RED)) + \
                            'has wrong specified value ({}{}{})'.format(
                                log_get_color_str(pylog.BLUE), t['protocol'],
                                log_get_color_str(pylog.RED)
                            )])
                    cnt += 1
            if len(err) > 0:
                self.__error.extend(err)
                self.__process_test_case_exception()
                return
            counter = 1
            job = TestJob(self.__ref)
            for t in m['job-tests']:
                self.__load_default_flags()

                self.__proto = TestCase.__proto_map[t['protocol']]

                self.__name = None
                if 'name' in t:
                    self.__name = t['name']
                elif self.__proto == TestContext.PROTO_MPY_RAW and \
                        len(t['tst-proc']) == 1:
                    self.__name = t['tst-proc'][0]
                else:
                    self.__name = 'job-sub-test-{}'.format(counter)
                counter += 1

                tst_proc = t['tst-proc']
                if self.__proto == TestContext.PROTO_MPY_RAW:
                    tst_proc = [ self.__ref + '/' + f for f in t['tst-proc'] ]
                self.__tst_proc = tst_proc

                basedir = self.__ref
                if 'exp-file' in t and \
                        os.path.exists(basedir + '/' + t['exp-file']):
                    self.__exp_file = basedir + '/' + t['exp-file']
                elif 'exp-value' in t:
                    self.__exp_value = '\n'.join(t['exp-value'])
                elif os.path.exists(basedir + '/' + self.__name + '.exp'):
                    self.__exp_file = self.__ref + '.exp'
                else:
                    pass # no expected value for this test case

                if 'exp-host' in t:
                    self.__pyhost = t['exp-host']
                if 'show-pass' in t:
                    self.__show_out = t['show-pass']
                if 'hide-fail' in t:
                    self.__hide_fail_res = t['hide-fail']
                if 'timeout' in t:
                    self.__timeout = t['timeout']
                if 'regex' in t:
                    self.__regex = t['regex']
                if 'skip' in t:
                    self.__skip = t['skip']
                if 'ignore' in t:
                    self.__ignore = t['ignore']
                job.add_test(self.__create_test_object())

            job.run()
        pass

    def __load_default_flags(self):
        self.__tst_proc = []
        self.__exp_file = None
        self.__exp_value = None
        self.__skip = False
        self.__ignore = False
        self.__regex = False
        self.__pyhost = False
        self.__show_out = False
        self.__timeout = None
        self.__results_dir = None
        self.__proto = TestContext.PROTO_MPY_RAW

        # inherit initial attributes values from the parent Test Suite
        if self.__parent:
            parent = self.__parent
            test_case = self.__ref
            self.__skip = parent.is_skipped(test_case)
            self.__ignore = parent.is_ignored(test_case)
            self.__regex = parent.is_regex_exp(test_case)
            self.__pyhost = parent.host_expect_enabled()
            self.__show_out = parent.show_passed_result()
            self.__hide_fail_res = parent.hide_failed_result()
            self.__timeout = parent.timeout()
            self.__results_dir = parent.results_dir()
        pass

    def __create_test_object(self) -> TestContext:
        ctx = TestContext(self.__name, self.__tst_proc,
                          exp_file=self.__exp_file,
                          exp_value=self.__exp_value,
                          proto=self.__proto)
        ctx.f_ignore = self.__ignore
        ctx.f_skip = self.__skip
        ctx.f_regex = self.__regex
        ctx.f_show_out = self.__show_out
        ctx.f_hide_fail_details = self.__hide_fail_res
        ctx.f_pyhost = self.__pyhost
        ctx.results_dir = self.__results_dir
        ctx.timeout = self.__timeout
        return ctx

    def __process_test_case_exception(self):
        if len(self.__error) > 0:
            ctx = TestContext(self.__ref, [self.__ref], None, None)
            job = TestJob(self.__ref)
            job.add_test(ctx)

            ctx.verdict.set_fail()
            for err in self.__error:
                ctx.add_fail_cause(err)
            job.run()

# ---------------------------------------------------------------------------- #
# Main Procedure
# ---------------------------------------------------------------------------- #
def main():

    # -- constructing test engine object
    te = TestEngine()

    log_section_header('Testing Options')
    te.log()
    log_section_separator()

    log_section_header('Test Execution')
    te.exec()
    log_section_separator()
    log_section_header('Test Statistics')
    g_test_stats.log()
    log_section_separator()

    exit(g_test_stats.failed) # return code

if __name__ == "__main__":
    main()

# --- end of file  ----------------------------------------------------------- #
