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
# Desc      runs all lora integration tests
# ---------------------------------------------------------------------------- #

import os
import sys

test_suite_path = os.path.dirname(__file__)
test_tool_path = os.path.realpath( os.path.abspath( \
                        test_suite_path + '/../../../../../tools/test' ) )
sys.path.append(test_tool_path)

from mpy_tester import run_test

# ---------------------------------------------------------------------------- #
run_test(
    # -- test case name
    'lorawan_msgs_params/lorawan_msg_params_main.py',
    # -- list of simultaneous required tests
    [{
        # -- list of files that will read and fed to the test node as if it is 
        #    a single test script
        'tst_files' : [
            test_suite_path + '/lorawan_msgs_params/lorawan_msg_params_main.py'
            ],
        'exp_file'  : test_suite_path +
            '/lorawan_msgs_params/lorawan_msg_params_main.py.exp'
    }],
    # -- test timeout, default is 30 seconds
    timeout = 120
)
# ---------------------------------------------------------------------------- #
run_test(
    # -- test case name
    'lorawan_msgs_params/lorawan_msg_params_1.py',
    # -- list of simultaneous required tests
    [{
        'tst_files' : [
            test_suite_path + '/lorawan_msgs_params/lorawan_msg_params_1.py',
            test_suite_path + '/lorawan_msgs_params/lorawan_msg_params_main.py'
            ],
        'exp_file'  : test_suite_path +
            '/lorawan_msgs_params/lorawan_msg_params_1.py.exp'
    }],
    # -- test timeout, default is 30 seconds
    timeout = 120
)
# ---------------------------------------------------------------------------- #
run_test(
    # -- test case name
    'loraraw -- lora_pp_868_sf7_bw125_cr45',
    # -- list of simultaneous required tests
    [
        {
        'tst_files' : [test_suite_path + '/loraraw/lora_pp_868_sf7_bw125_cr45_node_1.py'],
        'exp_file'  : test_suite_path + '/loraraw/lora_pp_868_sf7_bw125_cr45_node_1.py.exp'
        },
        {
        'tst_files' : [test_suite_path + '/loraraw/lora_pp_868_sf7_bw125_cr45_node_2.py'],
        'exp_file'  : test_suite_path + '/loraraw/lora_pp_868_sf7_bw125_cr45_node_2.py.exp'
        },
    ],
    # -- test timeout, default is 30 seconds
    timeout = 10
)
# ---------------------------------------------------------------------------- #
run_test(
    # -- test case name
    'loraraw -- lora_pp_868_sf8_bw125_cr45',
    # -- list of simultaneous required tests
    [
        {
        'tst_files' : [test_suite_path + '/loraraw/lora_pp_868_sf8_bw125_cr45_node_1.py'],
        'exp_file'  : test_suite_path + '/loraraw/lora_pp_868_sf8_bw125_cr45_node_1.py.exp'
        },
        {
        'tst_files' : [test_suite_path + '/loraraw/lora_pp_868_sf8_bw125_cr45_node_2.py'],
        'exp_file'  : test_suite_path + '/loraraw/lora_pp_868_sf8_bw125_cr45_node_2.py.exp'
        },
    ],
    # -- test timeout, default is 30 seconds
    timeout = 10
)
# ---------------------------------------------------------------------------- #
run_test(
    # -- test case name
    'loraraw -- lora_pp_433_sf7_bw125_cr45',
    # -- list of simultaneous required tests
    [
        {
        'tst_files' : [test_suite_path + '/loraraw/lora_pp_433_sf7_bw125_cr45_node_1.py'],
        'exp_file'  : test_suite_path + '/loraraw/lora_pp_433_sf7_bw125_cr45_node_1.py.exp'
        },
        {
        'tst_files' : [test_suite_path + '/loraraw/lora_pp_433_sf7_bw125_cr45_node_2.py'],
        'exp_file'  : test_suite_path + '/loraraw/lora_pp_433_sf7_bw125_cr45_node_2.py.exp'
        },
    ],
    # -- test timeout, default is 30 seconds
    timeout = 10
)
# ---------------------------------------------------------------------------- #
