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

"""
Non-blocking OTA upgrade example (status polling)

This demonstrates the non-blocking OTA API that allows the REPL to remain
responsive during firmware upgrades using status polling.

New API:
    fuota.start_upgrade(url, blocking) - Start OTA (blocking=False for background)
    fuota.status()                     - Get current OTA status
    fuota.abort_upgrade()              - Cancel ongoing OTA
"""

UPGRADE_URL="https://your_server.com/application.bin"
WIFI_SSID='<YOUR_SSID>'
WIFI_PWD='<YOUR_PWD>'

import fuota
import time
import sys

# import logs
# logs.filter_subsystem('F1', True)
# logs.filter_component('F1', 'fuota', True)

def upgrade(url, blocking):
    print("="*70)
    print("modfuota OTA Upgrade")
    print("="*70)
    
    fuota.info()
    
    print(f"\n🚀 Starting OTA from: {url}")
    print("-" * 70)
    
    if blocking:
        # Run in blocking mode
        fuota.upgrade(url)
    else:
        # Start in background
        fuota.start_upgrade(url)
    
def check_upgrade():

    # Poll until complete
    start_time = time.ticks_ms()
    last_bytes = 0
    
    while True:
        status = fuota.status()
        state = status['state']
        bytes_written = status['bytes_written']
        total_size = status.get('total_size', None)
        if total_size is not None:
            percentage = int(bytes_written / total_size * 100)
            tkb = total_size / 1024.0
        
        if state == 'running':
            kb = bytes_written / 1024.0
            elapsed = time.ticks_diff(time.ticks_ms(), start_time) / 1000.0
            print(f"📥 [{elapsed:6.1f}s] Downloaded: {kb:7.1f} KB", end=' ')
            if total_size is not None:
                print(f"/ {tkb:7.1f} KB [ {percentage}% ]", end=' ')
            print('')
            last_bytes = bytes_written
            time.sleep(1)
        elif state == 'success':
            elapsed = time.ticks_diff(time.ticks_ms(), start_time) / 1000.0
            kb = bytes_written / 1024.0
            print("-" * 70)
            print(f"✅ Success! {kb:.1f} KB in {elapsed:.1f}s")
            print("\nNext: Reset to load the new firmware.")
            return True
        else:
            print(f"❌ OTA {state}")
            print(f"❌ status {status}")
            if 'error_code' in status:
                print(f"   Error Code: {status['error_code']}")
            if 'err_msg' in status:
                print(f"   Error: {status['err_msg']}")
            return False

def connect_wifi():
    import network
    wl = network.WLAN()
    wl.active(1)
    wl.connect(WIFI_SSID, WIFI_PWD)
    print('Connecting to ', WIFI_SSID)
    while not wl.isconnected():
        print('o', end='')
        time.sleep(.25)
        if not wl.isconnected():
            print('O', end='')
            time.sleep(.25)
    print('\nCONNECTED!')
    print(wl.ifconfig())

def update_time():
    import ntptime
    success = False
    for ntp_counter in range(5):
        try:
            ntptime.settime()
            success = True
            break
        except:
            pass
    return success


def upgrade_callback(timer):
    print('Starting upgrade using Timer task...')
    upgrade(UPGRADE_URL, True)    

if __name__ == "__main__":
    connect_wifi()

    # This is important for certificate validation!
    if not update_time():
        raise OSError("Unable to set NTP TIME! Required for certificate validation!")

    upgrade(UPGRADE_URL, False)

    import _thread
    _thread.start_new_thread(check_upgrade,())

