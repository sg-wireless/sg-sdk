<!--
Copyright (c) 2023-2024 SG Wireless - All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
copies  of  the  Software,  and  to  permit  persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Description: LTE Module (C Implementation) API Documentation
-->

# lte Module - C Implementation

The `lte` module provides MicroPython bindings for LTE modem functionality using a robust C implementation built on top of the ESP modem library. This is the new implementation that replaces the Python-based `LTE.py` module.

## Usage

```python
import lte

# Initialize modem
lte.init(carrier='standard')

# Attach to network
lte.attach(apn='iot.1nce.net')

# Start data session
lte.connect()

# Check connection
if lte.isconnected():
    print("Connected!")
    print(lte.ifconfig())

# Clean up
lte.disconnect()
lte.deinit()
```

## Methods

### lte.init([carrier='standard'])

Initialize the LTE modem subsystem. This method is idempotent - it's safe to call multiple times.

The modem intelligently handles three possible power states:
1. **Powered off** - Powers on and initializes from scratch
2. **Normal AT mode** (crash recovery) - Resumes from existing state
3. **CMUX mode** (soft reset) - Cleans up and reinitializes

**Parameters:**
- `carrier` (str, optional): Carrier conformance mode for band selection. Options:
  - `'standard'` (default)
  - `'verizon'`
  - `'att'`
  - `'docomo'`
  - `'kddi'`
  - `'telstra'`
  - `'tmo'`
  - `'verizon-no-roaming'`
  - `'3gpp-conformance'`

**Carrier Conformance Mode Behavior:**

The `carrier` parameter controls the modem's carrier conformance mode (AT+SQNCTM), which affects band selection and carrier-specific optimizations:

- **`lte.init()` or `lte.init(carrier=None)`**: Uses whatever conformance mode is currently configured on the modem. Does not change or check the mode.
  
- **`lte.init(carrier='standard')`** or any explicit carrier: Checks the modem's current conformance mode and changes it if different. If the mode needs to change, the modem will automatically reset during initialization.

- **Changing conformance mode after initialization**: If you call `lte.init(carrier='verizon')` after already initializing with a different carrier, the modem will automatically change the conformance mode and reset. No need to call `lte.deinit()` first - the change is handled seamlessly.

**Example:**
```python
# Use default - keeps whatever mode is configured
lte.init()

# Explicitly set to standard mode
lte.init(carrier='standard')

# Switch to Verizon mode - seamlessly changes and resets modem
lte.init(carrier='verizon')

# Switch back to standard - automatic mode change
lte.init(carrier='standard')

# Attach after mode change
lte.attach(apn='iot.1nce.net')
```

**Note:** Changing the conformance mode triggers a modem reset. The initialization process handles this automatically (including the baudrate switch from 115200 to 921600), but it adds a few seconds to the init time.

### lte.attach([apn=None, type='IP', cid=None, band=None, bands=None])

Enable radio and attach to the cellular network. This is a **non-blocking** operation - use `lte.isattached()` to poll for registration status.

> **Note:** NB-IoT initial attachment may take several minutes.

**Parameters:**
- `apn` (str, optional): Access Point Name (e.g., `'iot.1nce.net'`). Defaults to empty string (carrier default)
- `type` (str, optional): PDP context type. Options: `'IP'` (default), `'IPV6'`, `'IPV4V6'`
- `cid` (int, optional): Context Identifier. Default: 1 (Verizon uses 3)
- `band` (int, optional): Single frequency band to use. 0 for auto-select. Valid bands: 1-28, 66, 71
- `bands` (list, optional): List of frequency bands (e.g., `[2, 4, 12]`). Cannot use with `band`

**Example:**
```python
# Simple attach with APN
lte.attach(apn='iot.1nce.net')

# Attach with specific band
lte.attach(apn='iot.1nce.net', band=12)

# Attach with multiple bands
lte.attach(apn='iot.1nce.net', bands=[2, 4, 12])

# Wait for attachment
import time
for i in range(180):  # 3 minute timeout
    if lte.isattached():
        print("Attached!")
        break
    time.sleep(1)
```

### lte.connect([cid=None])

Start a data session using CMUX multiplexing and PPP protocol. This is a **non-blocking** operation - use `lte.isconnected()` to poll for PPP status.

**Prerequisites:** Network must be attached (`lte.isattached()` returns `True`)

**Parameters:**
- `cid` (int, optional): Context Identifier (typically uses CID from `attach()`)

**Example:**
```python
# Wait for connection
lte.connect()
import time
for i in range(60):
    if lte.isconnected():
        print("Connected!")
        print(lte.ifconfig())
        break
    time.sleep(1)
```

### lte.disconnect()

Disconnect from the data session but keep network attachment.

**Example:**
```python
lte.disconnect()
```

### lte.detach()

Gracefully detach from the cellular network and disable radio functionality.

**Example:**
```python
lte.detach()
```

### lte.deinit([detach=True, power_off=True])

Deinitialize the modem with optional control over network detachment and power off.

This method allows flexible power management strategies:
- **Full shutdown** (default): Detaches from network and powers off modem
- **Low power mode**: Keeps modem powered and attached, using PSM/eDRX for power saving
- **Quick restart**: Keeps modem powered for faster re-initialization

**Parameters:**
- `detach` (bool, optional): If `True`, detach from cellular network. Default: `True`
- `power_off` (bool, optional): If `True`, power off the modem completely. Default: `True`

**Power Saving Strategy:**

When using `detach=False, power_off=False`, the modem stays attached to the network in a low-power state:
- Flow control pins are de-asserted, allowing the modem to enter power saving mode
- Use PSM (Power Saving Mode) or eDRX (extended DRX) for ultra-low power consumption
- Modem can wake the ESP32 via RING signal for incoming messages or mobile-terminated events
- ESP32 can enter deep sleep while modem maintains network connection

**Example:**
```python
# Full shutdown (default)
lte.deinit()

# Keep modem powered for quick restart
lte.deinit(power_off=False)

# Low power mode with network attachment (PSM/eDRX)
# Configure PSM/eDRX first with AT commands
lte.send_at_cmd('AT+CPSMS=1,"","","00000001","00000001"')  # Enable PSM
lte.deinit(detach=False, power_off=False)
# ESP32 can now deep sleep, modem wakes it via RING on incoming data

# Detach but keep powered (for reconfiguration)
lte.deinit(detach=True, power_off=False)
```

**Use Cases:**

1. **Normal shutdown**: `lte.deinit()` - Clean disconnect and power off
2. **Quick restart**: `lte.deinit(power_off=False)` - Faster next `lte.init()`
3. **Ultra-low power with connectivity**: `lte.deinit(detach=False, power_off=False)` + PSM/eDRX
4. **Reconfigure network**: `lte.deinit(detach=True, power_off=False)` - Change APN/bands

### lte.reset()

Perform a hardware reset on the modem using `AT^RESET`. Waits for the modem to shutdown and reboot, then re-detects baudrate.

This function can take up to 10 seconds to complete.

**Example:**
```python
lte.reset()
```

### lte.isattached()

Check if the modem is attached to the cellular network.

**Returns:** `True` if registered (home or roaming), `False` otherwise

**Example:**
```python
if lte.isattached():
    print("Attached to network")
```

### lte.is_attached()

Alias for `lte.isattached()`. Provided for compatibility with legacy scripts.

**Returns:** `True` if attached, `False` otherwise

### lte.isconnected()

Check if PPP data connection is active and IP address has been obtained.

**Returns:** `True` if connected, `False` otherwise

**Example:**
```python
if lte.isconnected():
    ip_info = lte.ifconfig()
    print(f"IP: {ip_info[0]}")
```

### lte.is_connected()

Alias for `lte.isconnected()`. Provided for compatibility with legacy scripts.

**Returns:** `True` if connected, `False` otherwise

### lte.send_at_cmd(cmd='AT', [timeout=-1, wait_ok_error=False, check_error=False, buffer_size=4096])

Send a raw AT command to the modem and return the response.

> **Important:** CMUX allows simultaneous AT commands and data sessions.

**Parameters:**
- `cmd` (str): AT command to send (without `\r\n`). Default: `'AT'`
- `timeout` (int, optional): Timeout in milliseconds. -1 for default (5000ms)
- `wait_ok_error` (bool, optional): Wait for OK/ERROR response. Default: False
- `check_error` (bool, optional): Raise exception on ERROR. Default: False
- `buffer_size` (int, optional): Response buffer size (1024-32768). Default: 4096

**Returns:** Response string with OK/ERROR lines removed

**Example:**
```python
# Query network registration
response = lte.send_at_cmd('AT+CEREG?')
print(response)

# Check signal strength
response = lte.send_at_cmd('AT+CSQ')
print(response)

# Long-running command
response = lte.send_at_cmd('AT+SQNINS', wait_ok_error=True, timeout=30000)
```

### lte.mode([new_mode=None])

Get or set the modem operating mode (CAT-M1 or NB-IoT).

**Parameters:**
- `new_mode` (int, optional): New mode to set. Use `lte.CATM1` (0) or `lte.NBIOT` (1)

**Returns:** 
- If no parameter: Current mode (0 for CAT-M1, 1 for NB-IoT)
- If parameter provided: None (mode is set)

> **Note:** Setting a new mode causes the modem to reset.

**Example:**
```python
# Get current mode
current = lte.mode()
print(f"Mode: {'CAT-M1' if current == 0 else 'NB-IoT'}")

# Switch to NB-IoT
lte.mode(lte.NBIOT)
```

### lte.imei()

Get the modem's IMEI (International Mobile Equipment Identity) number.

**Returns:** String containing the 15-digit IMEI

**Example:**
```python
imei = lte.imei()
print(f"IMEI: {imei}")
```

### lte.iccid()

Get the SIM card's ICCID (Integrated Circuit Card Identification) number.

This function checks if the SIM is present and ready before reading the ICCID.

**Returns:** String containing the 19-20 digit ICCID

**Raises:** OSError if SIM card is not present or not ready

**Example:**
```python
try:
    iccid = lte.iccid()
    print(f"ICCID: {iccid}")
except OSError:
    print("SIM card not present or not ready")
```

### lte.get_signal_strength()

Get signal strength information from the modem.

**Returns:** Tuple of `(rssi, rssi_dbm, ber)` where:
- `rssi`: Raw RSSI value (0-31, 99=unknown)
- `rssi_dbm`: RSSI in dBm (-113 to -51, -999=unknown)
- `ber`: Bit Error Rate (0-7, 99=unknown)

**Example:**
```python
rssi, rssi_dbm, ber = lte.get_signal_strength()
print(f"Signal: {rssi_dbm} dBm (BER: {ber})")
```

### lte.get_status()

Get comprehensive modem status information.

**Returns:** Dictionary with the following keys:
- `powered` (bool): Modem power state
- `sim_ready` (bool): SIM card present and ready
- `network_attached` (bool): Attached to network
- `ppp_connected` (bool): PPP session active
- `cmux_active` (bool): CMUX multiplexing enabled
- `baudrate` (int): Current UART baudrate
- `rssi` (int): Signal strength (raw RSSI value)
- `ber` (int): Bit error rate

**Example:**
```python
status = lte.get_status()
print(f"Powered: {status['powered']}")
print(f"Network: {status['network_attached']}")
print(f"PPP: {status['ppp_connected']}")
print(f"Signal: {status['rssi']}")
```

### lte.ifconfig()

Get network interface configuration (IP address information).

**Returns:** Tuple of `(ip, netmask, gateway, dns)` or `None` if not connected

**Example:**
```python
if lte.isconnected():
    ip, netmask, gateway, dns = lte.ifconfig()
    print(f"IP: {ip}")
    print(f"Gateway: {gateway}")
    print(f"DNS: {dns}")
```

### lte.power_on([wait_ok=True])

Power on the LTE modem hardware via IO expander.

**Parameters:**
- `wait_ok` (bool, optional): Wait for modem to respond with OK. Default: True

**Example:**
```python
lte.power_on()
# or
lte.power_on(wait_ok=False)  # Don't wait for response
```

### lte.power_off([force=False])

Power off the LTE modem hardware via IO expander.

**Parameters:**
- `force` (bool, optional): Force immediate power off without graceful shutdown. Default: False

**Example:**
```python
lte.power_off()  # Graceful shutdown
# or
lte.power_off(force=True)  # Immediate power off
```

### lte.is_powered()

Check if the modem is currently powered on.

**Returns:** `True` if powered, `False` otherwise

**Example:**
```python
if lte.is_powered():
    print("Modem is powered on")
```

### lte.check_sim_present()

Check if a SIM card is present and ready.

This function retries up to 5 times with delays, and ensures the radio is enabled to read the SIM.

**Returns:** `True` if SIM is ready, `False` otherwise

**Example:**
```python
if lte.check_sim_present():
    iccid = lte.iccid()
    print(f"SIM ready: {iccid}")
else:
    print("No SIM card detected")
```

### lte.set_event_handler(handler, event_mask=EVENT_ALL, lock=False)

Register a unified event handler for LTE modem events, providing structured event data with automatic parsing.

**Parameters:**
- `handler` (function): Event handler function that receives a dictionary, or `None` to unregister
- `event_mask` (int, optional): Bitmask of events to subscribe to. Default: `lte.EVENT_ALL`
- `lock` (bool, optional): If `True`, lock the event handler so it cannot be overridden until `lte.deinit()` is called. Attempting to set or unregister the handler while locked raises `OSError`. Default: `False`

**Event Handler Signature:**
```python
def handler(event: dict) -> None:
    """
    Args:
        event: Dictionary containing:
            - 'type': Event type constant (lte.EVENT_xxx)
            - Additional keys depending on event type
    """
    pass
```

**Event Types:**

1. **`lte.EVENT_REGISTRATION_STATUS` (0x0001)** - Network registration changes
   - `stat` (int): Registration status (0-10, 80)
     - 0: Not registered (not searching)
     - 1: Registered (home network)
     - 2: Not registered (searching)
     - 3: Registration denied
     - 5: Registered (roaming)
   - `tac` (str, optional): Tracking Area Code
   - `ci` (str, optional): Cell ID
   - `act` (int, optional): Access Technology (-1=unknown, 7=LTE-M, 9=NB-IoT)
   - `cause_type` (int, optional): Reject cause type
   - `reject_cause` (int, optional): Reject cause code
   - `active_time` (str, optional): PSM active time
   - `periodic_tau` (str, optional): PSM periodic TAU

2. **`lte.EVENT_PPP_CONNECTED` (0x0002)** - PPP connection established
   - `ip` (str): Assigned IP address
   - `netmask` (str): Network mask
   - `gateway` (str): Gateway address
   - `dns1` (str): Primary DNS server
   - `dns2` (str, optional): Secondary DNS server

3. **`lte.EVENT_PPP_DISCONNECTED` (0x0004)** - PPP connection lost

4. **`lte.EVENT_MODEM_CRASH` (0x0008)** - Modem crash detected
   - `break_count` (int): Number of break signals received

5. **`lte.EVENT_MODEM_RESET` (0x0010)** - Modem was reset
   - `user_initiated` (bool): Whether reset was user-initiated
   - `reason` (str, optional): Reset reason

6. **`lte.EVENT_SIGNAL_QUALITY` (0x0020)** - Signal strength update
   - `rssi` (int): Raw RSSI value (0-31, 99=unknown)
   - `rssi_dbm` (int): RSSI in dBm (-113 to -51, -999=unknown)
   - `ber` (int): Bit Error Rate (0-7, 99=unknown)

7. **`lte.EVENT_URC` (0x0040)** - Unsolicited response code (raw AT response)
   - `data` (str): The raw URC string

8. **`lte.EVENT_ERROR` (0x0080)** - Error occurred
   - `error_code` (int): Error code
   - `message` (str, optional): Error message
   - `operation` (str, optional): Operation that failed

**Event Mask Combinations:**

You can combine event types using bitwise OR to subscribe to specific events:

```python
# Subscribe to connection events only
lte.set_event_handler(handler, lte.EVENT_PPP_CONNECTED | lte.EVENT_PPP_DISCONNECTED)

# Subscribe to registration and signal quality
lte.set_event_handler(handler, lte.EVENT_REGISTRATION_STATUS | lte.EVENT_SIGNAL_QUALITY)

# Subscribe to all events (default)
lte.set_event_handler(handler, lte.EVENT_ALL)

# Lock the handler so it cannot be overridden (released on lte.deinit())
lte.set_event_handler(handler, lte.EVENT_ALL, lock=True)
```

**Locking:**

When `lock=True` is passed, the handler is protected from being overridden or unregistered. Any subsequent call to `lte.set_event_handler()` (including `lte.set_event_handler(None)`) will raise `OSError` until `lte.deinit()` is called, which releases the lock.

This is useful for system-level code (e.g., the CTRL client) that needs to guarantee its event handler remains active while managing the modem connection:

```python
# System code locks the handler
lte.init()
lte.set_event_handler(system_handler, lte.EVENT_ALL, lock=True)

# User code cannot override it
try:
    lte.set_event_handler(my_handler)  # Raises OSError
except OSError as e:
    print(e)  # "event handler is locked, call lte.deinit() first"

# Lock is released on deinit
lte.deinit()
lte.init()
lte.set_event_handler(my_handler)  # Works now
```

**Basic Example:**
```python
def lte_event_handler(event):
    event_type = event['type']
    
    if event_type == lte.EVENT_REGISTRATION_STATUS:
        stat = event['stat']
        if stat == 1:
            print("✓ Registered (home network)")
        elif stat == 5:
            print("✓ Registered (roaming)")
        elif stat == 2:
            print("⌛ Searching...")
            
    elif event_type == lte.EVENT_PPP_CONNECTED:
        print(f"✓ Connected! IP: {event['ip']}")
        
    elif event_type == lte.EVENT_PPP_DISCONNECTED:
        print("✗ Disconnected")

# Register handler for all events
lte.init()
lte.set_event_handler(lte_event_handler, lte.EVENT_ALL)
lte.attach(apn='iot.1nce.net')
lte.connect()

# Unregister when done
lte.set_event_handler(None)
```

**Complete Example:**
See `/home/ehlers/sg-sdk/examples/lte/lte_event_handler.py` for a comprehensive example showing all event types and how to handle them.

## Constants

### Mode Constants

- `lte.CATM1` (0): CAT-M1 mode constant
- `lte.NBIOT` (1): NB-IoT mode constant

**Example:**
```python
# Set mode to CAT-M1
lte.mode(lte.CATM1)

# Set mode to NB-IoT
lte.mode(lte.NBIOT)
```

### Event Type Constants

Event type constants for use with `lte.set_event_handler()`:

- `lte.EVENT_REGISTRATION_STATUS` (0x0001): Network registration status changes
- `lte.EVENT_PPP_CONNECTED` (0x0002): PPP connection established
- `lte.EVENT_PPP_DISCONNECTED` (0x0004): PPP connection lost
- `lte.EVENT_MODEM_CRASH` (0x0008): Modem crash detected
- `lte.EVENT_MODEM_RESET` (0x0010): Modem reset occurred
- `lte.EVENT_SIGNAL_QUALITY` (0x0020): Signal strength update
- `lte.EVENT_URC` (0x0040): Raw unsolicited response code
- `lte.EVENT_ERROR` (0x0080): Error occurred
- `lte.EVENT_ALL` (0xFFFF): Subscribe to all events

**Example:**
```python
# Subscribe to specific events
lte.set_event_handler(my_handler, lte.EVENT_PPP_CONNECTED | lte.EVENT_PPP_DISCONNECTED)

# Subscribe to all events
lte.set_event_handler(my_handler, lte.EVENT_ALL)
```

## Error Handling

The module raises MicroPython exceptions for errors:

- `OSError` - General modem errors (timeout, not responding, etc.)
- `ValueError` - Invalid parameters
- `MemoryError` - Memory allocation failure
- `TypeError` - Wrong type for callback

**Example:**
```python
try:
    lte.init()
    lte.attach(apn='iot.1nce.net')
except OSError as e:
    print(f"LTE error: {e}")
except ValueError as e:
    print(f"Invalid parameter: {e}")
```

## Implementation Notes

### Differences from LTE.py

This C implementation provides several improvements over the Python `LTE.py` module:

1. **Better Resource Management:**
   - Proper UART reservation/release
   - Memory-efficient buffers
   - WiFi/LTE coexistence handled automatically

2. **CMUX Support:**
   - Simultaneous AT commands and data sessions
   - No need for `pause_ppp()`/`resume_ppp()`
   - More reliable operation

3. **Event-Driven Architecture:**
   - Unified event handler with structured data
   - No need to manually poll for status changes
   - Real-time event notification with automatic parsing

4. **Robust Initialization:**
   - Handles modem power states intelligently
   - Automatic crash recovery
   - Idempotent initialization

5. **Performance:**
   - Native C implementation
   - Faster command execution
   - Lower memory footprint

### Compatibility Notes

Most methods are compatible with `LTE.py`, but note:

- `pause_ppp()` and `resume_ppp()` are **not needed** (CMUX handles this)
- `read_rsp()` is replaced by `set_event_handler()`
- Initialization is simpler (no explicit baudrate management)
- `check_power()` renamed to `is_powered()`
- Automatic WiFi conflict detection/resolution

## Advanced Usage

### Monitoring Network Registration with Event Handler

```python
import lte
import time

def lte_event_handler(event):
    """Monitor network registration and connection events"""
    event_type = event['type']
    
    if event_type == lte.EVENT_REGISTRATION_STATUS:
        stat = event['stat']
        states = {
            0: "Not registered (not searching)",
            1: "Registered (home network)",
            2: "Not registered (searching)",
            3: "Registration denied",
            5: "Registered (roaming)"
        }
        print(f"Network: {states.get(stat, f'Unknown ({stat})')}")
        
        # Show location info if available
        if 'tac' in event and event['tac']:
            print(f"  Tracking Area: {event['tac']}")
        if 'ci' in event and event['ci']:
            print(f"  Cell ID: {event['ci']}")
        if 'act' in event and event['act'] != -1:
            act_names = {7: "LTE-M", 9: "NB-IoT"}
            print(f"  Technology: {act_names.get(event['act'], 'Unknown')}")
    
    elif event_type == lte.EVENT_SIGNAL_QUALITY:
        rssi_dbm = event['rssi_dbm']
        print(f"Signal: {rssi_dbm} dBm")
    
    elif event_type == lte.EVENT_PPP_CONNECTED:
        print(f"Connected! IP: {event['ip']}")
    
    elif event_type == lte.EVENT_PPP_DISCONNECTED:
        print("Disconnected - implementing reconnection...")
        # Application reconnection logic here

lte.init()
lte.set_event_handler(lte_event_handler, lte.EVENT_ALL)
lte.attach(apn='iot.1nce.net')

# Wait and monitor via events
for i in range(180):
    if lte.isattached():
        print("Attached! Connecting...")
        lte.connect()
        break
    time.sleep(1)

# Events continue to be monitored automatically
time.sleep(60)

lte.set_event_handler(None)
lte.disconnect()
lte.deinit()
```

### Connection Events Only

Subscribe only to connection-related events for simple connection monitoring:

```python
import lte

def connection_handler(event):
    """Handle only connection events"""
    if event['type'] == lte.EVENT_PPP_CONNECTED:
        print(f"✓ Connected: {event['ip']}")
    elif event['type'] == lte.EVENT_PPP_DISCONNECTED:
        print("✗ Disconnected - reconnecting...")
        lte.connect()  # Auto-reconnect

lte.init()
# Subscribe only to PPP events
lte.set_event_handler(
    connection_handler,
    lte.EVENT_PPP_CONNECTED | lte.EVENT_PPP_DISCONNECTED
)
lte.attach(apn='iot.1nce.net')
lte.connect()
```

### Complete Connection Example

```python
import lte
import time

def connect_lte(apn, timeout=180):
    """Connect to LTE with timeout"""
    print("Initializing...")
    lte.init()
    
    # Check SIM
    if not lte.check_sim_present():
        raise RuntimeError("No SIM card")
    
    print(f"ICCID: {lte.iccid()}")
    print(f"IMEI: {lte.imei()}")
    
    # Attach
    print("Attaching...")
    lte.attach(apn=apn)
    
    start = time.time()
    while time.time() - start < timeout:
        if lte.isattached():
            rssi, rssi_dbm, ber = lte.get_signal_strength()
            print(f"Attached! Signal: {rssi_dbm} dBm")
            break
        time.sleep(1)
    else:
        raise TimeoutError("Attachment timeout")
    
    # Connect
    print("Connecting...")
    lte.connect()
    
    start = time.time()
    while time.time() - start < 60:
        if lte.isconnected():
            ip_info = lte.ifconfig()
            print(f"Connected! IP: {ip_info[0]}")
            return
        time.sleep(1)
    else:
        raise TimeoutError("Connection timeout")

# Use it
try:
    connect_lte('iot.1nce.net')
    
    # Now you can use sockets!
    import socket
    s = socket.socket()
    s.connect(('example.com', 80))
    # ...
    
finally:
    lte.disconnect()
    lte.deinit()
```

## Logging and Debugging

The `lte` module uses the SG-SDK structured logging system with two components:

- **`espmodem`** - Low-level ESP modem library operations (UART, AT commands, internal state)
- **`modlte`** - High-level MicroPython bindings (function calls, operations, results)

### Enabling Logging

Enable logging at the start of your script before importing `lte`:

```python
import logs

# Enable the entire 'lte' subsystem
logs.filter_subsystem('lte', True)

# Enable specific components for granular control
logs.filter_component('lte', 'espmodem', True)  # Low-level modem operations
logs.filter_component('lte', 'modlte', True)    # High-level Python bindings

import lte
```

### Logging Levels

Each component logs at different levels:
- **INFO** - High-level operations (function calls, status changes)
- **DEBUG** - Detailed operation flow (AT commands, responses, state transitions)
- **WARN** - Recoverable issues (retries, dropped events)
- **ERROR** - Failures requiring attention

### What Gets Logged

**`modlte` component logs:**
- Function calls with parameters: `lte.init(carrier='standard')`
- AT command execution: `AT: AT+CEREG=2`
- AT command responses: `AT response: +CEREG: 2,0`
- Operation results: `lte.isattached() -> True`
- Callback registration and URC handling
- High-level operation flow (attach, connect, disconnect)

**`espmodem` component logs:**
- Modem initialization and power state detection
- UART communication details
- ESP modem library events
- CMUX multiplexing operations
- PPP session management
- Low-level error handling and retries

### Example Output

```python
import logs
logs.filter_component('lte', 'modlte', True)
import lte

lte.init()
lte.attach(apn='iot.1nce.net')
response = lte.send_at_cmd('AT+CEREG?')
```

**Output:**
```
|000:00:01-234|  info  |1:mp_task |lte.init       |   lte   |   modlte    | lte.init(carrier='standard')
|000:00:01-345|  info  |1:mp_task |lte.init       |   lte   |   modlte    | LTE initialization complete
|000:00:02-456|  info  |1:mp_task |lte.attach     |   lte   |   modlte    | lte.attach(apn='iot.1nce.net', type='IP', cid=1)
|000:00:02-567|  info  |1:mp_task |send_at_cmd    |   lte   |   modlte    | AT: AT+CEREG?
|000:00:02-678|  info  |1:mp_task |send_at_cmd    |   lte   |   modlte    | AT response: +CEREG: 2,5,"001E","059B2A79",7
```

### Debugging Tips

**For general troubleshooting:**
```python
# Enable only high-level operations
logs.filter_component('lte', 'modlte', True)
```

**For deep debugging:**
```python
# Enable both components for full visibility
logs.filter_component('lte', 'espmodem', True)
logs.filter_component('lte', 'modlte', True)
```

**For production:**
```python
# Disable logging or enable only errors
logs.filter_subsystem('lte', False)
```

### Troubleshooting Event Handler Issues

When debugging event handlers, enable `modlte` logging to see:
- Handler registration: `"Event handler registered successfully"`
- Event reception: `"Event handler called from UART task context"`
- Event scheduling: `"Event successfully scheduled for processing"`
- Handler execution: `"Handler is valid and callable, invoking Python function"`

```python
import logs
logs.filter_component('lte', 'modlte', True)
import lte

def event_handler(event):
    print(f"[Event] {event['type']}: {event}")

lte.init()
lte.set_event_handler(event_handler)  # Watch logs for registration confirmation
lte.attach(apn='iot.1nce.net')
```

<!--- end of file --->