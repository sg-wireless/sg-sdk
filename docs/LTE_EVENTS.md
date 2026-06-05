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

Description: LTE Event Handler System Documentation
-->

# LTE Event Handler System

## Overview

The LTE module provides a **unified event handler system** that allows your application to react to various modem events in real-time. Instead of polling status, you can register a single callback function that receives notifications about:

- Network attachment/detachment
- PPP connection/disconnection
- Modem crashes and resets
- Signal quality changes
- Unsolicited response codes (URCs)
- Errors

## Key Concepts

### Event-Driven vs Polling

**Before (Polling)**:
```python
while True:
    if not lte.isconnected():
        print("Connection lost!")
        lte.connect()
    time.sleep(5)
```

**After (Event-Driven)**:
```python
def on_event(event):
    if event['type'] == lte.EVENT_PPP_DISCONNECTED:
        print("Connection lost!")
        lte.connect()

lte.set_event_handler(on_event, lte.EVENT_PPP_DISCONNECTED)
```

### Event Filtering

You can subscribe to:
- **All events**: `lte.EVENT_ALL`
- **Specific events**: List of event types
- **Combined events**: Using bitwise OR

This prevents your callback from being called unnecessarily and improves performance.

## Event Types

| Constant | Value | Description |
|----------|-------|-------------|
| `lte.EVENT_URC` | 0x0001 | Unsolicited response code received from modem |
| `lte.EVENT_NETWORK_ATTACHED` | 0x0002 | Successfully registered on cellular network |
| `lte.EVENT_NETWORK_DETACHED` | 0x0004 | Lost network registration |
| `lte.EVENT_PPP_CONNECTED` | 0x0008 | PPP connection established, data ready |
| `lte.EVENT_PPP_DISCONNECTED` | 0x0010 | PPP connection lost |
| `lte.EVENT_MODEM_CRASH` | 0x0020 | Modem crash detected (UART breaks) |
| `lte.EVENT_MODEM_RESET` | 0x0040 | Modem reset initiated |
| `lte.EVENT_SIGNAL_QUALITY` | 0x0080 | Signal strength changed |
| `lte.EVENT_ERROR` | 0x0100 | Error occurred during operation |
| `lte.EVENT_ALL` | 0xFFFF | Subscribe to all events |

## API Reference

### lte.set_event_handler(callback, events=None)

Register a unified event handler function.

**Parameters**:
- `callback` (function or None): Function to call when events occur. Pass `None` to unregister.
- `events` (int, list, tuple, or None): Event filter
  - `None` or `lte.EVENT_ALL`: Subscribe to all events (default)
  - Single int: Subscribe to one event type
  - List/tuple: Subscribe to multiple specific events
  - Bitmask: Combined events using `|` operator

**Returns**: None

**Raises**: 
- `TypeError`: If callback is not callable
- `OSError`: If registration fails

**Example**:
```python
def my_handler(event):
    print(f"Event: {event['type']}")

# Subscribe to all events
lte.set_event_handler(my_handler)

# Subscribe to specific events
lte.set_event_handler(my_handler, [lte.EVENT_PPP_CONNECTED, lte.EVENT_PPP_DISCONNECTED])

# Subscribe using bitmask
lte.set_event_handler(my_handler, lte.EVENT_PPP_CONNECTED | lte.EVENT_PPP_DISCONNECTED)

# Unregister
lte.set_event_handler(None)
```

### Event Dictionary Structure

Your callback receives a dictionary with event-specific data:

#### All Events
```python
{
    'type': int  # Event type constant (lte.EVENT_xxx)
}
```

#### EVENT_URC
```python
{
    'type': lte.EVENT_URC,
    'data': str  # URC string (e.g., "+CEREG: 5,\"001A\",\"00415C7B\",7")
}
```

#### EVENT_NETWORK_ATTACHED / EVENT_NETWORK_DETACHED
```python
{
    'type': lte.EVENT_NETWORK_ATTACHED,  # or EVENT_NETWORK_DETACHED
    'attached': bool,     # True if attached, False if detached
    'operator': str       # Network operator name (may be absent)
}
```

#### EVENT_PPP_CONNECTED / EVENT_PPP_DISCONNECTED
```python
{
    'type': lte.EVENT_PPP_CONNECTED,  # or EVENT_PPP_DISCONNECTED
    'connected': bool,    # True if connected, False if disconnected
    'ip': str,           # IP address (only when connected)
    'gateway': str       # Gateway IP (only when connected)
}
```

#### EVENT_MODEM_CRASH
```python
{
    'type': lte.EVENT_MODEM_CRASH,
    'break_count': int,        # Number of UART breaks detected
    'recovery_started': bool   # True if automatic recovery initiated
}
```

#### EVENT_MODEM_RESET
```python
{
    'type': lte.EVENT_MODEM_RESET,
    'user_initiated': bool,  # True if reset via lte.reset()
    'reason': str            # Reason for reset (may be absent)
}
```

#### EVENT_SIGNAL_QUALITY
```python
{
    'type': lte.EVENT_SIGNAL_QUALITY,
    'rssi': int,      # Raw RSSI (0-31, 99=unknown)
    'rssi_dbm': int,  # RSSI in dBm (-113 to -51)
    'ber': int        # Bit error rate (0-7, 99=unknown)
}
```

#### EVENT_ERROR
```python
{
    'type': lte.EVENT_ERROR,
    'error_code': int,   # Error code from espmodem_err_t
    'message': str,      # Error message (may be absent)
    'operation': str     # Operation that failed (may be absent)
}
```

## Usage Examples

### Example 1: Basic Event Monitoring

```python
import lte

def event_handler(event):
    event_type = event['type']
    
    if event_type == lte.EVENT_PPP_CONNECTED:
        print(f"Connected! IP: {event['ip']}")
    elif event_type == lte.EVENT_PPP_DISCONNECTED:
        print("Disconnected!")
    elif event_type == lte.EVENT_MODEM_CRASH:
        print(f"CRASH! Breaks: {event['break_count']}")

lte.init()
lte.set_event_handler(event_handler, lte.EVENT_ALL)
lte.attach()
lte.connect()
```

### Example 2: Automatic Reconnection

```python
import lte
import time

reconnecting = False

def connection_monitor(event):
    global reconnecting
    
    if event['type'] == lte.EVENT_PPP_DISCONNECTED:
        if not reconnecting:
            print("Connection lost - reconnecting...")
            reconnecting = True
            try:
                lte.connect()
                reconnecting = False
            except Exception as e:
                print(f"Reconnection failed: {e}")
                reconnecting = False
    
    elif event['type'] == lte.EVENT_PPP_CONNECTED:
        print(f"Connected successfully! IP: {event['ip']}")
        reconnecting = False

# Subscribe only to connection events
lte.set_event_handler(connection_monitor, 
                      [lte.EVENT_PPP_CONNECTED, lte.EVENT_PPP_DISCONNECTED])
```

### Example 3: Application State Machine

```python
import lte

class LTEStateMachine:
    STATE_IDLE = 0
    STATE_ATTACHING = 1
    STATE_ATTACHED = 2
    STATE_CONNECTING = 3
    STATE_CONNECTED = 4
    STATE_ERROR = 5
    
    def __init__(self):
        self.state = self.STATE_IDLE
        lte.set_event_handler(self.handle_event, lte.EVENT_ALL)
    
    def handle_event(self, event):
        event_type = event['type']
        
        if event_type == lte.EVENT_NETWORK_ATTACHED:
            self.state = self.STATE_ATTACHED
            print("State: ATTACHED")
            
        elif event_type == lte.EVENT_PPP_CONNECTED:
            self.state = self.STATE_CONNECTED
            print(f"State: CONNECTED - {event['ip']}")
            
        elif event_type == lte.EVENT_PPP_DISCONNECTED:
            self.state = self.STATE_ATTACHED
            print("State: ATTACHED (connection lost)")
            
        elif event_type == lte.EVENT_NETWORK_DETACHED:
            self.state = self.STATE_IDLE
            print("State: IDLE (network lost)")
            
        elif event_type == lte.EVENT_ERROR:
            self.state = self.STATE_ERROR
            print(f"State: ERROR - {event.get('message', 'Unknown error')}")
    
    def is_ready_to_send(self):
        return self.state == self.STATE_CONNECTED
    
    def start(self):
        self.state = self.STATE_ATTACHING
        lte.init()
        lte.attach()
        
        # Wait for attachment
        while self.state == self.STATE_ATTACHING:
            time.sleep(0.1)
        
        if self.state == self.STATE_ATTACHED:
            self.state = self.STATE_CONNECTING
            lte.connect()

# Usage
state_machine = LTEStateMachine()
state_machine.start()

# Now wait for connected state
while not state_machine.is_ready_to_send():
    time.sleep(0.1)

print("Ready to send data!")
```

### Example 4: Crash Recovery Monitoring

```python
import lte
import time

crash_count = 0
crash_timestamps = []

def crash_monitor(event):
    global crash_count, crash_timestamps
    
    if event['type'] == lte.EVENT_MODEM_CRASH:
        crash_count += 1
        crash_timestamps.append(time.time())
        
        print(f"⚠️  MODEM CRASH #{crash_count}")
        print(f"   Break count: {event['break_count']}")
        print(f"   Recovery: {'Yes' if event['recovery_started'] else 'No'}")
        
        # Alert if multiple crashes in short time
        if len(crash_timestamps) >= 3:
            recent = [t for t in crash_timestamps if time.time() - t < 300]
            if len(recent) >= 3:
                print("🚨 ALERT: 3+ crashes in 5 minutes!")
                # Could send alert, log to file, etc.

lte.set_event_handler(crash_monitor, lte.EVENT_MODEM_CRASH)
```

### Example 5: Signal Quality Monitoring

```python
import lte

signal_history = []

def signal_monitor(event):
    if event['type'] == lte.EVENT_SIGNAL_QUALITY:
        rssi_dbm = event['rssi_dbm']
        signal_history.append(rssi_dbm)
        
        # Keep only last 10 readings
        if len(signal_history) > 10:
            signal_history.pop(0)
        
        # Calculate average
        avg_signal = sum(signal_history) / len(signal_history)
        
        # Warn if signal is poor
        if rssi_dbm < -100:
            print(f"⚠️  Poor signal: {rssi_dbm} dBm")
        elif rssi_dbm > -80:
            print(f"✓ Good signal: {rssi_dbm} dBm")
        
        print(f"Average (last {len(signal_history)}): {avg_signal:.1f} dBm")

lte.set_event_handler(signal_monitor, lte.EVENT_SIGNAL_QUALITY)
```

## Best Practices

### 1. Keep Callbacks Fast

Event handlers run in the MicroPython main task. Long-running operations will block other events.

**Bad**:
```python
def slow_handler(event):
    time.sleep(10)  # DON'T DO THIS!
    process_event(event)
```

**Good**:
```python
event_queue = []

def fast_handler(event):
    event_queue.append(event)  # Queue for later processing

# Process queue in main loop
def main():
    while True:
        if event_queue:
            event = event_queue.pop(0)
            process_event(event)
        time.sleep(0.1)
```

### 2. Subscribe to Only What You Need

Don't subscribe to `EVENT_ALL` if you only need connection events.

**Less efficient**:
```python
def handler(event):
    if event['type'] == lte.EVENT_PPP_CONNECTED:
        do_something()
    # Ignoring all other events

lte.set_event_handler(handler, lte.EVENT_ALL)
```

**More efficient**:
```python
def handler(event):
    do_something()

lte.set_event_handler(handler, lte.EVENT_PPP_CONNECTED)
```

### 3. Handle Exceptions

Exceptions in event handlers will be logged but won't crash your application.

```python
def safe_handler(event):
    try:
        process_event(event)
    except Exception as e:
        print(f"Error in event handler: {e}")
```

### 4. Unregister When Done

Always unregister event handlers when you're done with them.

```python
try:
    lte.set_event_handler(my_handler, lte.EVENT_ALL)
    # Do work...
finally:
    lte.set_event_handler(None)  # Unregister
```

## Migration from Legacy API

The old `set_unsolicited_callback()` function is deprecated but still supported for backward compatibility.

### Old Code (Deprecated)
```python
def urc_handler(urc_string):
    print(f"URC: {urc_string}")

lte.set_unsolicited_callback(urc_handler)
```

### New Code (Recommended)
```python
def event_handler(event):
    if event['type'] == lte.EVENT_URC:
        print(f"URC: {event['data']}")

lte.set_event_handler(event_handler, lte.EVENT_URC)
```

**Benefits of new API**:
- Single callback for all event types
- Event filtering (performance)
- Access to structured event data
- Future-proof

## Troubleshooting

### Events Not Being Received

1. **Check registration**:
   ```python
   # Make sure callback is registered AFTER lte.init()
   lte.init()
   lte.set_event_handler(my_handler, lte.EVENT_ALL)
   ```

2. **Check event mask**:
   ```python
   # Make sure you're subscribed to the event
   lte.set_event_handler(my_handler, lte.EVENT_PPP_CONNECTED | lte.EVENT_PPP_DISCONNECTED)
   ```

3. **Check callback signature**:
   ```python
   # Callback must accept one parameter
   def my_handler(event):  # ✓ Correct
       pass
   
   def my_handler():  # ✗ Wrong - missing parameter
       pass
   ```

### Scheduler Queue Full

If you see warnings about "Scheduler queue full", your callback is too slow or called too frequently.

**Solutions**:
1. Make callback faster
2. Use event filtering to reduce callback frequency
3. Queue events for processing in main loop

### Memory Issues

Events allocate memory for event data. If you have memory constraints:
1. Subscribe to fewer events
2. Don't store large event history in global variables
3. Process events immediately rather than queuing

## Performance Considerations

- **Event filtering** reduces unnecessary callbacks and memory allocation
- **Fast callbacks** prevent blocking other system tasks
- **Selective subscription** improves battery life (fewer interrupts)

## Thread Safety

- Event callbacks run in the **MicroPython main task**
- Safe to call most MicroPython functions from callbacks
- Avoid calling blocking functions (large `time.sleep()`, long network operations)
- Event data is copied before callback (safe to store references)

## See Also

- [LTE Module API Reference](/src/platforms/F1/comps/lte/modlte.md)
- [Example Scripts](../examples/)

<!--- end of file --->
