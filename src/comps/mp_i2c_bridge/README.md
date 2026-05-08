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

Description: MicroPython I2C Bridge component for ESP-IDF v5.4 compatibility
-->

# MicroPython I2C Bridge Component

## Contents

* [Overview](#overview)
* [Architecture](#architecture)
* [API Reference](#api-reference)
* [Usage Example](#usage-example)
* [Implementation Components](#implementation-components)
* [Error Handling](#error-handling)
* [Build Integration](#build-integration)
* [Thread Safety](#thread-safety)
* [Debugging](#debugging)

## Overview

The `mp_i2c_bridge` component provides a C/C++ interface to MicroPython's I2C implementation, enabling native ESP-IDF components to communicate with I2C devices through MicroPython's unified I2C driver. This architecture prevents I2C driver conflicts and ensures compatibility with ESP-IDF v5.4+ I2C API changes.

## Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────────┐
│   C Drivers     │───▶│  I2C Bridge      │───▶│   MicroPython I2C   │
│ (fuel_gauge_mp, │    │ (mp_i2c_bridge)  │    │   (machine.I2C)     │
│  ioexp_mp)      │    │                  │    │                     │
└─────────────────┘    └──────────────────┘    └─────────────────────┘
```

### Benefits

- **Unified I2C Management**: Single MicroPython I2C instance prevents driver conflicts
- **Modern API Usage**: Uses MicroPython's ESP-IDF v5.4+ compatible I2C implementation  
- **Future-Proof**: Automatically inherits MicroPython's I2C driver updates
- **Clean Architecture**: Hardware drivers separated from I2C transport layer

## API Reference

### Initialization

```c
esp_err_t mp_i2c_bridge_init(int scl_pin, int sda_pin, int frequency);
```

Initialize the I2C bridge with specified GPIO pins and frequency.

**Parameters:**
- `scl_pin`: GPIO number for I2C SCL (clock) line
- `sda_pin`: GPIO number for I2C SDA (data) line  
- `frequency`: I2C bus frequency in Hz (typically 100000 or 400000)

**Returns:** `ESP_OK` on success, error code on failure

### Write Operations

```c
esp_err_t mp_i2c_bridge_write(uint8_t device_addr, const uint8_t *data, size_t length);
```

Write data to an I2C device.

**Parameters:**
- `device_addr`: 7-bit I2C device address
- `data`: Pointer to data buffer to write
- `length`: Number of bytes to write

**Returns:** `ESP_OK` on success, error code on failure

### Read Operations

```c
esp_err_t mp_i2c_bridge_read(uint8_t device_addr, uint8_t *data, size_t length);
```

Read data from an I2C device.

**Parameters:**
- `device_addr`: 7-bit I2C device address
- `data`: Pointer to buffer for received data
- `length`: Number of bytes to read

**Returns:** `ESP_OK` on success, error code on failure

### Write-Read Operations

```c
esp_err_t mp_i2c_bridge_write_read(uint8_t device_addr, 
                                   const uint8_t *write_data, size_t write_length,
                                   uint8_t *read_data, size_t read_length);
```

Write data to device then read response (common for register access).

**Parameters:**
- `device_addr`: 7-bit I2C device address
- `write_data`: Pointer to data buffer to write
- `write_length`: Number of bytes to write
- `read_data`: Pointer to buffer for received data
- `read_length`: Number of bytes to read

**Returns:** `ESP_OK` on success, error code on failure

## Usage Example

### Basic Integration

```c
#include "mp_i2c_bridge.h"

// Initialize I2C bridge
esp_err_t err = mp_i2c_bridge_init(20, 21, 100000); // SCL=20, SDA=21, 100kHz
if (err != ESP_OK) {
    return err;
}

// Write data to device
uint8_t write_data[] = {0x10, 0x20}; // Register 0x10, value 0x20
err = mp_i2c_bridge_write(0x48, write_data, sizeof(write_data));

// Read data from device  
uint8_t read_buffer[2];
err = mp_i2c_bridge_read(0x48, read_buffer, sizeof(read_buffer));

// Register read pattern
uint8_t reg = 0x10;
uint8_t value;
err = mp_i2c_bridge_write_read(0x48, &reg, 1, &value, 1);
```

### Component Integration

Add to your component's `CMakeLists.txt`:

```cmake
idf_component_register(
    SRCS "your_component.c"
    INCLUDE_DIRS "."
    REQUIRES mp_i2c_bridge  # Add bridge dependency
)
```

## Implementation Components

The bridge is used by the following sg-sdk components:

### Fuel Gauge (BQ27421)
- **File**: `src/platforms/F1/comps/fuel-gauge-if/src/fuel_gauge_mp.c`
- **Device**: BQ27421 fuel gauge IC
- **I2C Address**: 0x55
- **Usage**: Battery state monitoring and reporting

### IO Expander (PCAL6408A)  
- **File**: `src/platforms/F1/comps/ioexp-if/src/ioexp_mp.c`
- **Device**: PCAL6408A 8-bit I2C GPIO expander
- **I2C Address**: 0x20
- **Usage**: Additional GPIO pins for board peripherals

## Error Handling

The bridge translates MicroPython exceptions to ESP error codes:

- `ESP_OK`: Operation successful
- `ESP_ERR_INVALID_ARG`: Invalid parameters
- `ESP_ERR_TIMEOUT`: I2C transaction timeout
- `ESP_FAIL`: General I2C communication failure

## Build Integration

The component is automatically included when building with MicroPython support:

```bash
./fw_builder.sh --board SGW3501-F1-StarterKit build
```

## Thread Safety

The I2C bridge is designed for single-threaded use from the MicroPython main thread. For multi-threaded applications, implement appropriate synchronization around bridge calls.

## Debugging

Enable I2C bridge logging by setting component log level:

```c
esp_log_level_set("mp_i2c_bridge", ESP_LOG_DEBUG);
```

This will show detailed I2C transaction information for troubleshooting.

<!--- end of file --->