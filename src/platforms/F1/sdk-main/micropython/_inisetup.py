import uos
from flashbdev import bdev

def _has_rgbled():
    try:
        import rgbled
        return True
    except ImportError:
        return False

_HAS_RGBLED = _has_rgbled()

DEFAULT_BOOT_PY_SIMPLE = \
    "# This file is executed on every boot (including wake-boot from deepsleep)"

DEFAULT_BOOT_PY_RGBLED = '''\
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

rgbled.deinit()
rgbled.initialize()
rgbled.color(0x00000000)

print('Clear LED and blink blue')
rgbled.heartbeat(0x000008, 4000, 1)
'''

DEFAULT_MAIN_PY_SIMPLE = \
    "# This file is executed on every boot (including wake-boot from deepsleep)"

DEFAULT_MAIN_PY_RGBLED = '''\
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
'''

def check_bootsec():
    buf = bytearray(bdev.ioctl(5, 0))  # 5 is SEC_SIZE
    bdev.readblocks(0, buf)
    empty = True
    for b in buf:
        if b != 0xFF:
            empty = False
            break
    if empty:
        return True
    fs_corrupted()

def reset_fs():
    print("Formatting flash and creating a new file system")
    sec = bdev.ioctl(5, 0)
    buf = bytearray(sec)
    for i in range(sec):
        buf[i] = 0xFF

    for blk in range(8):
        bdev.writeblocks(blk, buf)

    bdev.ioctl(3, 0)
    setup()
    print("Done")

def _erase_partition(label):
    from esp32 import Partition
    parts = Partition.find(Partition.TYPE_DATA, label=label)
    if not parts:
        print("Partition '{}' not found".format(label))
        return
    part = parts[0]
    blk_size = part.ioctl(5, 0)
    blk_count = part.ioctl(4, 0)
    buf = bytearray(blk_size)
    for i in range(blk_size):
        buf[i] = 0xFF
    print("Erasing partition '{}' ({} blocks of {} bytes)".format(label, blk_count, blk_size))
    for blk in range(blk_count):
        part.writeblocks(blk, buf)
    print("Done")

def reset_nvs():
    _erase_partition("nvs")

def reset_otadata():
    _erase_partition("otadata")

def fs_corrupted():
    import micropython
    import time

    # Re-enable Ctrl+C during this error loop so the user can break out
    # and access the REPL. Keyboard interrupts are disabled by default
    # during frozen _boot.py execution (since MicroPython 1.20+).
    micropython.kbd_intr(3)

    while 1:
        print(
            """\
The filesystem appears to be corrupted. If you had important data there, you
may want to make a flash snapshot to try to recover it. Otherwise, perform
factory reprogramming of MicroPython firmware (completely erase flash, followed
by firmware programming) or press ctrl+c to interrupt and run _inisetup.reset_fs().
"""
        )
        time.sleep(3)


def setup():
    check_bootsec()
    print("Performing initial setup, creating an Lfs2 file system on /")
    uos.VfsLfs2.mkfs(bdev)
    vfs = uos.VfsLfs2(bdev)
    # Mount to /
    uos.mount(vfs, "/")
    # Create the default folders
    vfs.mkdir("cert")
    #vfs.mkdir("sys")
    vfs.mkdir("lib")
    with open("/boot.py", "w") as f:
        f.write(DEFAULT_BOOT_PY_RGBLED if _HAS_RGBLED else DEFAULT_BOOT_PY_SIMPLE)
    with open("/main.py", "w") as f:
        f.write(DEFAULT_MAIN_PY_RGBLED if _HAS_RGBLED else DEFAULT_MAIN_PY_SIMPLE)

    return vfs