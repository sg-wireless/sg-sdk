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

Description: IO Expander Interface (PCAL6408A) Documentation
-->

# IO Expander Interface (PCAL6408A)

## Contents

* [Introduction](#introduction)
* [PCAL6408A Features](#pcal6408a-features)
* [MicroPython Interface](#micropython-interface)
* [Complete Example](#complete-example)
* [C/C++ Interface](#cc-interface)
* [Hardware Connection](#hardware-connection)
* [Troubleshooting](#troubleshooting)
* [Register Map](#register-map)

## Introduction

The IO Expander interface provides access to the PCAL6408A 8-bit I2C GPIO expander
available on SGW3501-F1-StarterKit boards. This component extends the available GPIO 
pins and provides interrupt-capable I/O expansion.

### I2C Bridge Architecture

The IO expander implementation uses MicroPython I2C Bridge architecture for
ESP-IDF v5.4+ compatibility:

- **Driver**: PCAL6408A 8-bit I2C GPIO expander
- **Bridge**: `mp_i2c_bridge` component provides C interface to MicroPython I2C
- **Benefits**: Unified I2C management, modern ESP-IDF compatibility, future-proof

### Implementation Details

- **Device Address**: 0x20 (7-bit I2C address)
- **I2C Pins**: SCL=20, SDA=21 (configurable)
- **Frequency**: 100kHz I2C bus frequency
- **Integration**: Uses `ioexp_mp.c` with I2C bridge for MicroPython builds

## PCAL6408A Features

- 8-bit I2C-bus GPIO with interrupt and weak pull-up
- 5V tolerant inputs
- Polarity inversion register
- Low current consumption
- Interrupt output for pin change detection
- Compatible with standard GPIO operations

## MicroPython Interface

### Initialization

```python
import ioexp

# Initialize the IO expander
ioexp.init()
```

### Pin Configuration

```python
# Set pin direction (1 = input, 0 = output)
ioexp.set_direction(pin_number, direction)

# Example: Set pin 0 as output, pin 1 as input
ioexp.set_direction(0, 0)  # Output
ioexp.set_direction(1, 1)  # Input
```

### Digital I/O Operations

```python
# Write to output pin
ioexp.write_pin(pin_number, value)

# Example: Set pin 0 high
ioexp.write_pin(0, 1)

# Read from input pin
value = ioexp.read_pin(pin_number)

# Example: Read pin 1 state
state = ioexp.read_pin(1)
print(f"Pin 1 state: {state}")
```

### Port Operations

```python
# Write entire 8-bit port at once
ioexp.write_port(0b10101010)  # Set pins 1,3,5,7 high

# Read entire port
port_state = ioexp.read_port()
print(f"Port state: 0x{port_state:02X}")
```

### Pull-up Configuration

```python
# Enable/disable weak pull-up resistors
ioexp.set_pullup(pin_number, enable)

# Example: Enable pull-up on pin 2
ioexp.set_pullup(2, 1)
```

### Polarity Inversion

```python
# Configure polarity inversion
ioexp.set_polarity(pin_number, inverted)

# Example: Invert polarity on pin 3
ioexp.set_polarity(3, 1)
```

## Complete Example

```python
import ioexp
import time

# Initialize the IO expander
ioexp.init()

# Configure pins: 0-3 as outputs, 4-7 as inputs with pull-ups
for pin in range(4):
    ioexp.set_direction(pin, 0)  # Output
    
for pin in range(4, 8):
    ioexp.set_direction(pin, 1)  # Input
    ioexp.set_pullup(pin, 1)     # Enable pull-up

# Blink LEDs on output pins
while True:
    # Set outputs high
    for pin in range(4):
        ioexp.write_pin(pin, 1)
    
    # Read and display input states
    for pin in range(4, 8):
        state = ioexp.read_pin(pin)
        print(f"Input pin {pin}: {state}")
    
    time.sleep(0.5)
    
    # Set outputs low
    for pin in range(4):
        ioexp.write_pin(pin, 0)
    
    time.sleep(0.5)
```

## C/C++ Interface

For native C/C++ applications, the same functionality is available through the
header interface:

```c
#include "ioexp.h"

// Initialize
esp_err_t err = ioexp_init();

// Configure pin direction
ioexp_set_direction(0, IOEXP_OUTPUT);
ioexp_set_direction(4, IOEXP_INPUT);

// Digital operations
ioexp_write_pin(0, 1);  // Set pin 0 high
int state = ioexp_read_pin(4);  // Read pin 4
```

## Hardware Connection

The PCAL6408A is typically connected as follows on SGW3501-F1-StarterKit:

- **VCC**: 3.3V power supply
- **GND**: Ground
- **SCL**: I2C clock line (GPIO 20)
- **SDA**: I2C data line (GPIO 21)
- **INT**: Interrupt output (optional, board-dependent)
- **ADDR**: Address select pin (determines I2C address)

## Troubleshooting

### Common Issues

1. **I2C Communication Errors**
   - Verify I2C connections (SCL, SDA, power, ground)
   - Check I2C address (0x20 default)
   - Ensure proper pull-up resistors on I2C lines

2. **Pin State Issues**
   - Verify pin direction configuration
   - Check for polarity inversion settings
   - Ensure proper voltage levels (5V tolerance)

3. **Initialization Failures**
   - Confirm device presence with I2C scan
   - Verify power supply stability
   - Check for I2C bus conflicts

### Debug Commands

```python
# Test I2C communication
import machine
i2c = machine.I2C(0, scl=20, sda=21)
devices = i2c.scan()
print(f"I2C devices found: {[hex(d) for d in devices]}")

# Expected output should include 0x20 for PCAL6408A
```

## Register Map

For advanced users, the PCAL6408A register map:

| Register | Address | Function |
|----------|---------|----------|
| Input Port | 0x00 | Read input levels |
| Output Port | 0x01 | Write output levels |
| Polarity Inversion | 0x02 | Configure input polarity |
| Configuration | 0x03 | Set pin direction |
| Output Drive Strength | 0x40 | Configure drive strength |
| Input Latch | 0x42 | Input latch control |
| Pull-up/Pull-down Enable | 0x43 | Pull resistor enable |
| Pull-up/Pull-down Select | 0x44 | Pull resistor direction |
| Interrupt Mask | 0x45 | Interrupt enable |
| Interrupt Status | 0x46 | Interrupt status |
| Output Port Configuration | 0x4F | Output port configuration |

<!--- end of file --->