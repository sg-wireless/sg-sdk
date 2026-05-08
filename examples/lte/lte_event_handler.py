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
LTE Event Handler Example

Demonstrates the unified event handler system for LTE module.
Shows how to subscribe to specific events and react to connection
state changes, modem crashes, and unsolicited responses.
"""
import esp
import logs
logs.filter_subsystem('lte', False)
logs.filter_component('lte', 'espmodem', False)
logs.filter_component('lte', 'modlte', False)
import lte
import time
esp.osdebug(0, esp.LOG_NONE)

# Track connection state
connection_state = {
    'registration_stat': None,
    'ppp_connected': False,
    'last_signal': None,
    'crash_count': 0
}

def lte_event_handler(event):
    """
    Unified event handler for all LTE events.
    
    The event parameter is a dictionary with:
    - 'type': Event type constant (lte.EVENT_xxx)
    - Additional keys depending on event type
    """
    
    event_type = event['type']
    print(f"\n=== LTE Event: 0x{event_type:04x} ===")
    
    # Registration status events
    if event_type == lte.EVENT_REGISTRATION_STATUS:
        stat = event['stat']
        stat_names = {
            0: "not registered (not searching)",
            1: "registered (home network)",
            2: "not registered (searching)",
            3: "registration denied",
            4: "unknown",
            5: "registered (roaming)",
            6: "registered (SMS only, home)",
            7: "registered (SMS only, roaming)",
            8: "emergency services only",
            9: "registered (CSFB not preferred, home)",
            10: "registered (CSFB not preferred, roaming)",
            80: "temporary network failure"
        }
        stat_name = stat_names.get(stat, f"unknown ({stat})")
        
        # Show registration status
        if stat in [1, 5]:  # Registered
            print(f"✓ Registered: {stat_name}")
        elif stat == 2:  # Searching
            print(f"⌛ Searching for network...")
        elif stat == 3:  # Denied
            print(f"✗ Registration denied!")
        elif stat == 80:  # Temporary failure
            print(f"⚠️  Temporary network failure")
        else:
            print(f"Status: {stat_name}")
        
        # Show optional fields if present
        if 'tac' in event and event['tac']:
            print(f"  Tracking Area Code: {event['tac']}")
        if 'ci' in event and event['ci']:
            print(f"  Cell ID: {event['ci']}")
        if 'act' in event and event['act'] != -1:
            act_names = {7: "E-UTRAN (LTE-M)", 9: "E-UTRAN (NB-IoT)"}
            act_name = act_names.get(event['act'], f"AcT {event['act']}")
            print(f"  Access Technology: {act_name}")
        if 'cause_type' in event and event['cause_type'] != -1:
            print(f"  Reject Cause Type: {event['cause_type']}")
        if 'reject_cause' in event and event['reject_cause'] != -1:
            print(f"  Reject Cause: {event['reject_cause']}")
        if 'active_time' in event and event['active_time']:
            print(f"  PSM Active Time: {event['active_time']}")
        if 'periodic_tau' in event and event['periodic_tau']:
            print(f"  PSM Periodic TAU: {event['periodic_tau']}")
        
        connection_state['registration_stat'] = stat
        
    # PPP connection events
    elif event_type == lte.EVENT_PPP_CONNECTED:
        print(f"✓ PPP connected!")
        if 'ip' in event:
            print(f"  IP: {event['ip']}")
        if 'netmask' in event:
            print(f"  Netmask: {event['netmask']}")
        if 'gateway' in event:
            print(f"  Gateway: {event['gateway']}")
        if 'dns1' in event:
            print(f"  DNS1: {event['dns1']}")
        if 'dns2' in event and event['dns2']:
            print(f"  DNS2: {event['dns2']}")
        connection_state['ppp_connected'] = True
        
    elif event_type == lte.EVENT_PPP_DISCONNECTED:
        print(f"✗ PPP disconnected!")
        connection_state['ppp_connected'] = False
        # Application can implement reconnection logic here:
        # - Check if still attached to network
        # - Implement exponential backoff
        # - Call lte.connect() when appropriate
        
    # Modem crash/reset events
    elif event_type == lte.EVENT_MODEM_CRASH:
        print(f"⚠️  MODEM CRASH DETECTED!")
        print(f"  Break count: {event['break_count']}")
        # if event['recovery_started']:
        #     print(f"  Recovery initiated automatically")
        connection_state['crash_count'] += 1
        
    elif event_type == lte.EVENT_MODEM_RESET:
        print(f"🔄 Modem reset")
        if event['user_initiated']:
            print(f"  User initiated reset")
        if 'reason' in event:
            print(f"  Reason: {event['reason']}")
            
    # Signal quality updates
    elif event_type == lte.EVENT_SIGNAL_QUALITY:
        rssi = event['rssi']
        rssi_dbm = event['rssi_dbm']
        ber = event['ber']
        print(f"📶 Signal: RSSI={rssi} ({rssi_dbm} dBm), BER={ber}")
        connection_state['last_signal'] = (rssi, rssi_dbm, ber)
        
    # Unsolicited response codes
    elif event_type == lte.EVENT_URC:
        urc_data = event['data']
        print(f"[URC] {urc_data.strip()}")
        
    # Error events
    elif event_type == lte.EVENT_ERROR:
        print(f"❌ Error occurred!")
        print(f"  Error code: {event['error_code']}")
        if 'message' in event:
            print(f"  Message: {event['message']}")
        if 'operation' in event:
            print(f"  Operation: {event['operation']}")
    
    else:
        print(f"Unknown event type: 0x{event_type:04x}")


def example_all_events():
    """Subscribe to all LTE events"""
    
    lte.init()
    
    # Register handler for all events
    lte.set_event_handler(lte_event_handler, lte.EVENT_ALL)
    
    print("Event handler registered - now connecting...")
    
    # This will trigger multiple events:
    # - REGISTRATION_STATUS when network status changes
    # - PPP_CONNECTED when connection established
    # - URC events during the process

    lte.attach(band=20, apn="eapn1.net")
    time.sleep(5)  # Wait for attachment
    
# Run examples
if __name__ == '__main__':
    try:
        # Choose which example to run:
        example_all_events()
    except KeyboardInterrupt:
        print("\nInterrupted by user")
    finally:
        # Cleanup
        pass