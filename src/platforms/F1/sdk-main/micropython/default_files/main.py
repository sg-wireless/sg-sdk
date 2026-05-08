# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author    SG Wireless
# ---------------------------------------------------------------------------- #

import rgbled
import time
import _thread
from ctrl_debug import print_debug

STATUS_UPDATE_PERIOD = 4000
LED_BLINK_PERIOD = 4000

def led_indicator_thread(status_update_cycle = 4000):
    print_debug(9, 'LED indicator thread started')
    lastCtrlStatus=False
    while True:
        ctrlStatus = ctrl.isconnected()
        if not (ctrlStatus == lastCtrlStatus):
            if ctrlStatus == True:
                print_debug(9, 'Ctrl connected - blink green')
                rgbled.heartbeat(0x000400, LED_BLINK_PERIOD, 1)
            else:
                print_debug(9, 'Ctrl disconnected - blink blue')
                rgbled.heartbeat(0x000008, LED_BLINK_PERIOD, 1)
        lastCtrlStatus = ctrlStatus
        time.sleep_ms(status_update_cycle)  

if ctrl_config.items():
    _thread.stack_size(1024)
    _thread.start_new_thread(led_indicator_thread,(STATUS_UPDATE_PERIOD,))