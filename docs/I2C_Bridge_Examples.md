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

Author:     Ahmed Sabry (SG Wireless)
Maintainer: Christian Ehlers (SG Wireless)

Description: I2C Bridge Integration Examples and Documentation
-->

# I2C Bridge Integration Examples

## Contents

* [Overview](#overview)
* [Component Architecture](#component-architecture)
* [Basic Usage Examples](#basic-usage-examples)
* [Advanced Examples](#advanced-examples)
* [Debugging and Troubleshooting](#debugging-and-troubleshooting)
* [Performance Considerations](#performance-considerations)
* [Integration with Other sg-sdk Features](#integration-with-other-sg-sdk-features)

This document provides practical examples of using the MicroPython I2C Bridge architecture
with fuel gauge and IO expander components on SGW3501-F1-StarterKit boards.

## Overview

The I2C bridge allows C/C++ device drivers to communicate through MicroPython's I2C 
implementation, providing:

- ✅ **Unified I2C Management**: Single I2C driver instance prevents conflicts
- ✅ **ESP-IDF v5.4+ Compatibility**: Uses modern MicroPython I2C implementation
- ✅ **Future-Proof**: Automatic ESP-IDF driver updates via MicroPython
- ✅ **Clean Architecture**: Hardware drivers separated from I2C transport

## Component Architecture

```
┌─────────────────────┐    ┌──────────────────┐    ┌─────────────────────┐
│   MicroPython App   │───▶│  Python Modules  │───▶│   MicroPython I2C   │
│                     │    │ fuel_gauge       │    │   (machine.I2C)     │
│                     │    │ ioexp            │    │                     │
└─────────────────────┘    └──────────────────┘    └─────────────────────┘
                                     ▲                        ▲
┌─────────────────────┐    ┌──────────────────┐               │
│   C/C++ Drivers     │───▶│  I2C Bridge      │──────────────▶│
│ fuel_gauge_mp.c     │    │ mp_i2c_bridge    │               │
│ ioexp_mp.c          │    │                  │               │
└─────────────────────┘    └──────────────────┘               │
                                     ▲                        │
┌─────────────────────┐              │                        │
│   Hardware Drivers  │──────────────┘                        │
│ BQ27421 (fuel)      │                                       │
│ PCAL6408A (ioexp)   │                                       │
└─────────────────────┘                                       │
                                                               │
┌─────────────────────┐                                       │
│   I2C Hardware      │──────────────────────────────────────▶│
│ SCL=20, SDA=21      │                                       │
│ 100kHz              │                                       │
└─────────────────────┘
```

## Basic Usage Examples

### 1. Fuel Gauge Basic Monitoring

```python
import fuel_gauge
import time

# Initialize the fuel gauge with default settings
fuel_gauge.init()

# Read battery information
info = fuel_gauge.info()
print(f"Battery voltage: {info.voltage_mV} mV")
print(f"Battery charge: {info.charge_percent}%")
print(f"Battery current: {info.current_mA} mA")
print(f"Battery temperature: {info.temp_degC:.1f}°C")

# Print detailed information
fuel_gauge.print()

# Deinitialize when done
fuel_gauge.deinit()
```

### 2. IO Expander Basic Control

```python
import ioexp

# Initialize the IO expander
ioexp.init()

# Configure pins 0-3 as outputs for LEDs
for pin in range(4):
    ioexp.set_direction(pin, 0)  # 0 = output

# Configure pins 4-7 as inputs with pull-ups for buttons
for pin in range(4, 8):
    ioexp.set_direction(pin, 1)  # 1 = input
    ioexp.set_pullup(pin, 1)     # Enable pull-up

# Control LEDs
ioexp.write_pin(0, 1)  # Turn on LED on pin 0
ioexp.write_pin(1, 0)  # Turn off LED on pin 1

# Read button states
button_state = ioexp.read_pin(4)
print(f"Button on pin 4: {'pressed' if button_state == 0 else 'released'}")
```

## Advanced Examples

### 3. Battery Monitor with LED Status Indicator

```python
import fuel_gauge
import ioexp
import time

def setup_hardware():
    """Initialize both fuel gauge and IO expander"""
    fuel_gauge.init(designCapacity_mAh=1500)
    ioexp.init()
    
    # Configure LEDs: Red=0, Yellow=1, Green=2
    for led_pin in range(3):
        ioexp.set_direction(led_pin, 0)  # Output
        ioexp.write_pin(led_pin, 0)      # Start off

def update_battery_leds():
    """Update LED status based on battery level"""
    info = fuel_gauge.info()
    charge = info.charge_percent
    
    # Clear all LEDs
    for led_pin in range(3):
        ioexp.write_pin(led_pin, 0)
    
    # Battery level indication
    if charge > 60:
        ioexp.write_pin(2, 1)    # Green LED - Good
    elif charge > 30:
        ioexp.write_pin(1, 1)    # Yellow LED - Medium  
    else:
        ioexp.write_pin(0, 1)    # Red LED - Low
    
    # Additional indicators
    if info.isCharging:
        # Blink appropriate LED when charging
        time.sleep(0.1)
        if charge > 60:
            ioexp.write_pin(2, 0)
        elif charge > 30:
            ioexp.write_pin(1, 0)
        else:
            ioexp.write_pin(0, 0)
        time.sleep(0.1)
    
    if info.isCritical:
        # Flash red LED rapidly for critical battery
        for _ in range(3):
            ioexp.write_pin(0, 1)
            time.sleep(0.1)
            ioexp.write_pin(0, 0)
            time.sleep(0.1)

def main():
    """Main battery monitoring loop"""
    setup_hardware()
    
    print("Battery Monitor Started")
    print("LED Status: Green=Good, Yellow=Medium, Red=Low")
    
    try:
        while True:
            # Update battery status display
            update_battery_leds()
            
            # Print status every 10 seconds
            info = fuel_gauge.info()
            print(f"Battery: {info.charge_percent}% "
                  f"({info.voltage_mV}mV, {info.current_mA}mA, "
                  f"{info.temp_degC:.1f}°C)")
            
            if info.isCharging:
                print("  Status: Charging")
            elif info.isDischarging:
                print("  Status: Discharging")
            
            if info.isCritical:
                print("  WARNING: Critical battery level!")
            
            time.sleep(10)
            
    except KeyboardInterrupt:
        print("\nStopping battery monitor...")
    finally:
        # Clean shutdown
        for led_pin in range(3):
            ioexp.write_pin(led_pin, 0)
        fuel_gauge.deinit()

if __name__ == "__main__":
    main()
```

### 4. Interactive System Control Panel

```python
import fuel_gauge
import ioexp
import time
import machine

def setup_control_panel():
    """Setup fuel gauge and IO expander for control panel"""
    fuel_gauge.init()
    ioexp.init()
    
    # Configure outputs: Status LEDs (pins 0-2), Relay control (pin 3)
    for pin in range(4):
        ioexp.set_direction(pin, 0)  # Output
        ioexp.write_pin(pin, 0)      # Start off
    
    # Configure inputs: Buttons (pins 4-7) with pull-ups
    for pin in range(4, 8):
        ioexp.set_direction(pin, 1)  # Input
        ioexp.set_pullup(pin, 1)     # Pull-up enabled
    
    print("Control Panel Setup Complete")
    print("Outputs: LED0=Status, LED1=Activity, LED2=Error, Pin3=Relay")
    print("Inputs: Pin4=Mode, Pin5=Test, Pin6=Reset, Pin7=Emergency")

def read_buttons():
    """Read all button states (returns dict of button states)"""
    buttons = {}
    button_names = ['mode', 'test', 'reset', 'emergency']
    
    for i, name in enumerate(button_names):
        # Read pin (4-7), invert because pull-up means pressed=0
        buttons[name] = not ioexp.read_pin(4 + i)
    
    return buttons

def control_panel_loop():
    """Main control panel logic"""
    setup_control_panel()
    
    system_mode = 0  # 0=Normal, 1=Test, 2=Maintenance
    relay_state = False
    last_button_check = 0
    
    try:
        while True:
            current_time = time.ticks_ms()
            
            # Check buttons every 100ms for responsiveness
            if time.ticks_diff(current_time, last_button_check) > 100:
                buttons = read_buttons()
                last_button_check = current_time
                
                # Button handling
                if buttons['mode']:
                    system_mode = (system_mode + 1) % 3
                    print(f"Mode changed to: {['Normal', 'Test', 'Maintenance'][system_mode]}")
                    time.sleep(0.3)  # Debounce
                
                if buttons['test'] and system_mode == 1:
                    print("Running test sequence...")
                    # Flash all LEDs
                    for _ in range(3):
                        for led in range(3):
                            ioexp.write_pin(led, 1)
                        time.sleep(0.2)
                        for led in range(3):
                            ioexp.write_pin(led, 0)
                        time.sleep(0.2)
                
                if buttons['reset']:
                    print("System reset requested")
                    relay_state = False
                    ioexp.write_pin(3, 0)  # Turn off relay
                    time.sleep(0.3)  # Debounce
                
                if buttons['emergency']:
                    print("EMERGENCY STOP!")
                    relay_state = False
                    ioexp.write_pin(3, 0)  # Emergency relay off
                    # Flash error LED
                    for _ in range(10):
                        ioexp.write_pin(2, 1)  # Error LED on
                        time.sleep(0.1)
                        ioexp.write_pin(2, 0)  # Error LED off
                        time.sleep(0.1)
            
            # Update status LEDs based on system state
            ioexp.write_pin(0, 1 if system_mode == 0 else 0)  # Normal status
            ioexp.write_pin(1, int(time.ticks_ms() / 500) % 2)  # Activity blink
            
            # Battery monitoring
            info = fuel_gauge.info()
            if info.charge_percent < 20:
                ioexp.write_pin(2, int(time.ticks_ms() / 250) % 2)  # Low battery warning
            elif info.isCritical:
                ioexp.write_pin(2, 1)  # Critical battery error
            else:
                ioexp.write_pin(2, 0)  # No error
            
            # Relay control based on battery and mode
            if system_mode != 2 and info.charge_percent > 15 and not info.isCritical:
                if not relay_state:
                    relay_state = True
                    ioexp.write_pin(3, 1)
                    print("Relay activated")
            else:
                if relay_state:
                    relay_state = False
                    ioexp.write_pin(3, 0)
                    print("Relay deactivated")
            
            # Status report every 5 seconds
            if current_time % 5000 < 100:
                print(f"Status: Mode={['Normal', 'Test', 'Maintenance'][system_mode]}, "
                      f"Battery={info.charge_percent}%, Relay={'ON' if relay_state else 'OFF'}")
            
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("\nShutting down control panel...")
    finally:
        # Safe shutdown
        for pin in range(4):
            ioexp.write_pin(pin, 0)
        fuel_gauge.deinit()

if __name__ == "__main__":
    control_panel_loop()
```

## Debugging and Troubleshooting

### I2C Bus Scanning

```python
import machine

# Scan I2C bus for connected devices
i2c = machine.I2C(0, scl=20, sda=21, freq=100000)
devices = i2c.scan()

print("I2C devices found:")
for device in devices:
    print(f"  0x{device:02X}")

# Expected devices:
# 0x20 - PCAL6408A IO Expander
# 0x55 - BQ27421 Fuel Gauge
```

### Component Health Check

```python
def health_check():
    """Comprehensive system health check"""
    print("=== SGW3501-F1-StarterKit Health Check ===")
    
    # I2C Bus Check
    print("\n1. I2C Bus Scan:")
    i2c = machine.I2C(0, scl=20, sda=21, freq=100000)
    devices = i2c.scan()
    
    expected_devices = {0x20: "PCAL6408A IO Expander", 0x55: "BQ27421 Fuel Gauge"}
    for addr, name in expected_devices.items():
        if addr in devices:
            print(f"  ✅ {name} found at 0x{addr:02X}")
        else:
            print(f"  ❌ {name} NOT FOUND at 0x{addr:02X}")
    
    # Fuel Gauge Check
    print("\n2. Fuel Gauge Test:")
    try:
        fuel_gauge.init()
        info = fuel_gauge.info()
        print(f"  ✅ Fuel gauge initialized")
        print(f"  ✅ Battery voltage: {info.voltage_mV} mV")
        print(f"  ✅ Battery charge: {info.charge_percent}%")
        fuel_gauge.deinit()
    except Exception as e:
        print(f"  ❌ Fuel gauge error: {e}")
    
    # IO Expander Check
    print("\n3. IO Expander Test:")
    try:
        ioexp.init()
        # Test basic pin operations
        ioexp.set_direction(0, 0)  # Output
        ioexp.write_pin(0, 1)
        ioexp.write_pin(0, 0)
        print("  ✅ IO expander initialized and tested")
    except Exception as e:
        print(f"  ❌ IO expander error: {e}")
    
    print("\n=== Health Check Complete ===")

# Run health check
health_check()
```

## Performance Considerations

### I2C Bridge Overhead

The I2C bridge adds minimal overhead:
- **Latency**: ~1-2ms additional per I2C transaction
- **Memory**: ~2KB RAM for bridge component
- **CPU**: Negligible impact during normal operation

### Optimization Tips

1. **Batch Operations**: Group multiple I2C operations when possible
2. **Appropriate Delays**: Don't poll I2C devices too frequently
3. **Error Handling**: Implement proper error recovery for I2C timeouts
4. **Resource Management**: Always deinitialize components when done

## Integration with Other sg-sdk Features

### LoRa Monitoring Example

```python
import fuel_gauge
import ioexp
import lora

def lora_battery_monitor():
    """Monitor battery via LoRa with local LED indication"""
    fuel_gauge.init()
    ioexp.init()
    lora.mode(lora.LORA)
    
    # Configure status LED
    ioexp.set_direction(0, 0)  # Output
    
    while True:
        info = fuel_gauge.info()
        
        # Create battery status message
        message = f"BAT:{info.charge_percent}%,{info.voltage_mV}mV,{info.temp_degC:.1f}C"
        
        # Send via LoRa
        lora.send(message)
        
        # Update local LED
        ioexp.write_pin(0, 1 if info.charge_percent > 20 else 0)
        
        time.sleep(60)  # Send every minute
```

This comprehensive documentation demonstrates the power and flexibility of the I2C bridge
architecture in real-world applications.

<!--- end of file --->