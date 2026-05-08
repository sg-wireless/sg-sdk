<!--
Copyright (c) 2023-2026 SG Wireless - All Rights Reserved

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

Author:     Christian Ehlers (SG Wireless)
Maintainer: Christian Ehlers (SG Wireless)

Description: Logging system documentation and integration guide
-->

# SG Wireless SDK Logging System

The SG Wireless SDK includes a sophisticated logging system (`log_lib`) that provides structured, colorized, and filterable logging capabilities for C code development. This document explains how to integrate logging into your code.

## Table of Contents

1. [System Overview](#system-overview)
2. [Quick Start](#quick-start)
3. [Setting Up Logging in Your Component](#setting-up-logging-in-your-component)
4. [Available Log Levels](#available-log-levels)
5. [Component Registration](#component-registration)
6. [CMakeLists.txt Integration](#cmakeliststxt-integration)
7. [Log Filtering and Control](#log-filtering-and-control)
8. [Color Support](#color-support)
9. [Advanced Features](#advanced-features)
10. [MicroPython Logging Interface](#micropython-logging-interface)
11. [Examples](#examples)
12. [Best Practices](#best-practices)
13. [Troubleshooting](#troubleshooting)

## System Overview

The logging system is built around these core concepts:

- **Subsystems**: High-level logical groupings (e.g., `lora`, `F1`, `drivers`)
- **Components**: Specific modules within subsystems (e.g., `ioexp`, `sx126x`, `mgr_api`)
- **Log Types**: Different log levels (info, debug, warn, error, assert)
- **Compile-time Control**: Logs can be enabled/disabled at build time
- **Runtime Filtering**: Logs can be filtered by subsystem, component, or type

## Quick Start

### 1. Basic Setup in Your C File

```c
// my_component.c

#include <stdio.h>
// ... other includes ...

// Define your logging context
#define __log_subsystem     F1          // Your subsystem name
#define __log_component     my_comp     // Your component name
#include "log_lib.h"                    // Include logging system

// Register your component (must be after log_lib.h include)
__log_component_def(F1, my_comp, green, 1, 0)

void my_function(void) {
    __log_info("This is an info message");
    __log_debug("Debug value: %d", some_variable);
    __log_warn("Warning: unusual condition detected");
    __log_error("Error occurred: %s", error_string);
}
```

### 2. Create a logs_defs.h File (if needed)

If you have multiple components, create a `logs_defs.h` file:

```c
// src/my_module/inc/logs_defs.h

#include "log_lib.h"

// Define subsystem
__log_subsystem_def(my_module, blue, 1, 0)

// Define components within the subsystem
__log_component_def(my_module, parser,      yellow,     1, 0)
__log_component_def(my_module, processor,   green,      1, 0)
__log_component_def(my_module, interface,   cyan,       1, 0)
```

### 3. Update CMakeLists.txt

Add logging support to your component's CMakeLists.txt:

```cmake
# src/my_module/CMakeLists.txt

__component_register(
    SRCS
        "my_component.c"
        "another_file.c"
    INCLUDE_DIRS
        "inc"
    LOGS_DEFS
        "${CMAKE_CURRENT_LIST_DIR}/inc/logs_defs.h"  # If you have logs_defs.h
    REQUIRES
        log_lib
)
```

## Setting Up Logging in Your Component

### Step 1: Choose Subsystem and Component Names

Pick meaningful names that reflect your code's organization:

- **Subsystem**: Logical grouping (e.g., `drivers`, `F1`, `lora`, `network`)
- **Component**: Specific functionality (e.g., `ioexp`, `sx126x`, `tcp`, `parser`)

### Step 2: Add Logging Headers to Your C File

```c
// At the top of your .c file, after other includes
#define __log_subsystem     my_subsystem
#define __log_component     my_component
#include "log_lib.h"

// Component registration (choose your color)
__log_component_def(my_subsystem, my_component, purple, 1, 0)
```

### Step 3: Use Logging Macros

```c
void example_function(int value, const char* name) {
    __log_info("Function called with value=%d, name='%s'", value, name);
    
    if (value < 0) {
        __log_warn("Negative value detected: %d", value);
        return;
    }
    
    if (value > 100) {
        __log_error("Value too high: %d (max: 100)", value);
        return;
    }
    
    __log_debug("Processing value %d", value);
    // ... processing logic ...
    __log_info("Processing completed successfully");
}
```

## Available Log Levels

| Macro | Purpose | Usage |
|-------|---------|-------|
| `__log_info(...)` | General information | Status updates, normal operations |
| `__log_debug(...)` | Debug information | Detailed tracing, variable dumps |
| `__log_warn(...)` | Warnings | Recoverable issues, unusual conditions |
| `__log_error(...)` | Errors | Error conditions, failures |
| `__log_assert(cond, ...)` | Assertions | Critical checks (will crash if false) |
| `__log_printf(...)` | Raw output | Unformatted output |

### Example Usage

```c
// Info - general status
__log_info("Device initialized successfully");

// Debug - detailed information
__log_debug("Register value: 0x%02X, status: %s", reg_val, status_str);

// Warning - recoverable issues
__log_warn("Retry attempt %d/%d failed", attempt, max_attempts);

// Error - error conditions
__log_error("I2C communication failed: %s (code: %d)", esp_err_to_name(err), err);

// Assert - critical checks
__log_assert(ptr != NULL, "Pointer must not be NULL");

// Memory/pointer debugging
int *my_ptr = malloc(sizeof(int));
__log_ptr(my_ptr);  // Logs: "pointer: my_ptr = 0x3c0a1b40"
```

## Component Registration

### Component Definition Syntax

```c
__log_component_def(subsystem, component, color, compile_flag, enable_flag)
```

**Parameters:**
- `subsystem`: Subsystem name (e.g., `F1`, `lora`, `drivers`)
- `component`: Component name (e.g., `ioexp`, `sx126x`, `parser`)
- `color`: Display color (see [Color Support](#color-support))
- `compile_flag`: 1 = include in build, 0 = exclude
- `enable_flag`: 1 = enabled by default, 0 = disabled by default

### Subsystem Definition

```c
__log_subsystem_def(subsystem_name, color, compile_flag, enable_flag)
```

**Example:**
```c
#include "log_lib.h"

// Define subsystem
__log_subsystem_def(sensors, cyan, 1, 0)

// Define components
__log_component_def(sensors, temperature, blue,    1, 0)
__log_component_def(sensors, humidity,    green,   1, 0)
__log_component_def(sensors, pressure,    yellow,  1, 0)
```

## CMakeLists.txt Integration

### Basic Integration

```cmake
__component_register(
    SRCS
        "src/my_file.c"
    INCLUDE_DIRS
        "inc"
    REQUIRES
        log_lib
)
```

### With Custom logs_defs.h

```cmake
__component_register(
    SRCS
        "src/file1.c"
        "src/file2.c"
    INCLUDE_DIRS
        "inc"
    LOGS_DEFS
        "${CMAKE_CURRENT_LIST_DIR}/inc/logs_defs.h"
    REQUIRES
        log_lib
)
```

### Real-world Example

Based on `src/platforms/F1/comps/ioexp-if/CMakeLists.txt`:

```cmake
__component_register(
    SRCS
        "ioexp.c"
        "ioexp_mp.c"
        "mod_ioexp.c"
    INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}"
    PRIV_INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}"
    REQUIRES
        log_lib
        drivers
        utils_lib
        pcal6408a
        freertos
        F1-BSP
        LOGS_DEFS
            "${CMAKE_CURRENT_LIST_DIR}/ioexp.c"
)
```

## Log Filtering and Control

The logging system provides runtime filtering capabilities:

### Filter by Subsystem

```c
// Enable/disable entire subsystem
log_filter_subsystem("lora", true);   // Enable lora subsystem
log_filter_subsystem("lora", false);  // Disable lora subsystem
```

### Filter by Component

```c
// Enable/disable specific component
log_filter_component("lora", "sx126x", true);   // Enable lora/sx126x
log_filter_component("lora", "sx126x", false);  // Disable lora/sx126x
```

### Filter by Log Type

```c
// Enable/disable log types
log_filter_type("debug", false);  // Disable all debug logs
log_filter_type("error", true);   // Enable all error logs
```

### Save and Restore Filter State

```c
log_filter_save_state_t saved_state = {
    .subsystem_name = "lora",
    .component_name = "sx126x"
};

// Save current state and disable
log_filter_save_state(&saved_state, false);

// ... do something with logs disabled ...

// Restore previous state
log_filter_restore_state(&saved_state);
```

## Color Support

Available colors for component definition:

| Color Name | Description |
|------------|-------------|
| `default` | System default |
| `black` | Black text |
| `red` | Red text |
| `green` | Green text |
| `yellow` | Yellow text |
| `blue` | Blue text |
| `purple` | Purple text |
| `cyan` | Cyan text |
| `white` | White text |

### Using Colors in Log Messages

```c
__log_info("Status: " __green_str("OK") " - Device ready");
__log_error("Error: " __red_str("FAILED") " - Device not responding");
__log_warn("Warning: " __yellow_str("TIMEOUT") " - Retrying...");
```

## Advanced Features

### Memory Dump Logging

```c
#include "log_mem_dump.h"

uint8_t buffer[16] = {0x01, 0x02, 0x03, /* ... */};
__log_mem_dump("Buffer contents:", buffer, sizeof(buffer));
```

### Conditional Compilation

Logs can be conditionally compiled using preprocessor macros:

```c
#if CONFIG_DEBUG_ENABLED
    __log_debug("Debug build - extra information");
#endif
```

### Custom Log Types

You can define custom log types for specialized output:

```c
// In header file
__log_type_info_struct_def(custom, 
    int level;
    const char* category;
);

// Usage
__log_opr_macro(custom, .level = 3, .category = "network");
```

## MicroPython Logging Interface

The logging system is also exposed to MicroPython through the built-in `logs` module. This allows MicroPython applications to produce structured, colorized, filterable log output that is indistinguishable from C-level logs.

### Overview

MicroPython logs flow through the same C logging pipeline as native code. They appear with the same structured header format including timestamp, log type, task name, subsystem, and component columns. Dynamic subsystems and components are registered at runtime and participate in the same filtering system.

### Module: `logs`

```python
import logs
```

### Registering Subsystems and Components

Before logging, you must register at least one subsystem and one component.

#### `logs.register_subsystem(name, *, color='default', enabled=True, silent=True)`

Register a new dynamic subsystem.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `name` | str | *(required)* | Subsystem name (max 24 characters) |
| `color` | str | `'default'` | Display color for the subsystem column |
| `enabled` | bool | `True` | Whether the subsystem is enabled for output |
| `silent` | bool | `True` | If `False`, prints a registration confirmation log |

```python
logs.register_subsystem("myapp", color='cyan', enabled=True, silent=False)
```

#### `logs.register_component(subsystem, component, *, color='default', enabled=True, silent=True)`

Register a new dynamic component under an existing subsystem.

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `subsystem` | str | *(required)* | Parent subsystem name (must already be registered) |
| `component` | str | *(required)* | Component name (max 24 characters) |
| `color` | str | `'default'` | Display color for the component column |
| `enabled` | bool | `True` | Whether the component is enabled for output |
| `silent` | bool | `True` | If `False`, prints a registration confirmation log |

```python
logs.register_component("myapp", "network", color='blue', enabled=True)
logs.register_component("myapp", "sensors", color='green', enabled=True)
```

### Logging Functions

All logging functions share the same signature:

```python
logs.<level>(subsystem, component, message)
```

| Function | Log Type | Typical Use |
|----------|----------|-------------|
| `logs.info(subsystem, component, message)` | info | General status and progress |
| `logs.debug(subsystem, component, message)` | debug | Detailed tracing information |
| `logs.warn(subsystem, component, message)` | warn | Recoverable issues and unusual conditions |
| `logs.error(subsystem, component, message)` | error | Error conditions and failures |
| `logs.output(subsystem, component, message)` | output | Direct output (no log-type decoration) |

```python
logs.info("myapp", "network", "Connected to MQTT broker")
logs.debug("myapp", "sensors", "Raw reading: {}".format(value))
logs.warn("myapp", "network", "Connection timeout, retrying...")
logs.error("myapp", "network", "Failed to connect: {}".format(err))
logs.output("myapp", "sensors", "Temperature: 24.5°C")
```

### Log Output Format

MicroPython logs appear in the same structured format as C logs:

```
|000:00:06-822|  info  |1:mp_task      |mpy            |  myapp  |  network    |Connected to MQTT broker
|000:00:06-833|  error |1:mp_task      |mpy            |  myapp  |  sensors    |Sensor read failed: timeout
```

The columns are:
- **Timestamp**: Time since boot (`HHH:MM:SS-mmm`)
- **Log type**: `info`, `debug`, `warn`, `error`, or `output`
- **OS context**: Core ID and task name (e.g., `1:mp_task`)
- **Source**: Always `mpy` for MicroPython-originated logs
- **Subsystem**: The registered subsystem name
- **Component**: The registered component name
- **Message**: Your log message

### Filtering

Dynamic subsystems and components participate in the existing filter system:

```python
# Filter by subsystem
logs.filter_subsystem("myapp", True)    # Enable
logs.filter_subsystem("myapp", False)   # Disable

# Filter by component
logs.filter_component("myapp", "sensors", True)    # Enable
logs.filter_component("myapp", "sensors", False)   # Disable

# View all filter stats (includes dynamic registrations)
logs.filter_stats()
```

### Capacity Limits

The dynamic registry supports:
- Up to **8 dynamic subsystems**
- Up to **32 dynamic components** (shared across all subsystems)
- Names up to **24 characters**

These limits are separate from compile-time C subsystems/components.

### Complete MicroPython Example

```python
import logs

# Register subsystem (silent=False to confirm in log output)
logs.register_subsystem("myapp", color='cyan', enabled=True, silent=False)

# Register components with different colors
logs.register_component("myapp", "main", color='green', enabled=True, silent=False)
logs.register_component("myapp", "network", color='blue', enabled=True)
logs.register_component("myapp", "sensors", color='yellow', enabled=True)

# Log messages from different components
logs.info("myapp", "main", "Application starting")
logs.info("myapp", "network", "Connecting to WiFi...")
logs.debug("myapp", "network", "SSID: MyNetwork, RSSI: -45")
logs.info("myapp", "network", "WiFi connected")
logs.info("myapp", "sensors", "Reading temperature sensor")
logs.info("myapp", "sensors", "Temperature: 24.5°C, Humidity: 45.2%")
logs.warn("myapp", "sensors", "Battery level low: 15%")
logs.error("myapp", "network", "MQTT connection lost")
```

### CTRL Client Integration

The CTRL client library (`ctrl_debug.py`) uses the `logs` module internally. Each CTRL component registers its own logging component under the `"ctrl"` subsystem:

| Component File | Component Name | Color |
|----------------|---------------|-------|
| `ctrl_debug.py` | `main` | green |
| `ctrl_config.py` | `config` | yellow |
| `ctrl_connection.py` | `connection` | blue |
| `ctrl_protocol.py` | `protocol` | white |
| `ctrl_library.py` | `library` | green |
| `ctrl_sensors.py` | `sensors` | cyan |
| `ctrl_pyconfig.py` | `pyconfig` | purple |

#### Enabling Debug Output

Use `ctrl.debug(level)` to set the global debug level for **all** components at once. This stores the level in NVS (`ctrl_debug`) and updates every component's level in `ctrl.dbg()`:

```python
ctrl.debug(100)   # Enable all components at level 100
ctrl.debug(0)     # Disable all components
ctrl.debug()      # Returns current global debug level
```

When `ctrl.debug()` sets a level, it also populates the per-component dict so you can fine-tune individual components afterward with `ctrl.dbg()`.

#### Per-Component Debug Levels (`ctrl.dbg()`)

Fine-grained control over individual component output. Levels are stored in NVS (`ctrl_dbg`) as a JSON dict and persist across reboots.

```python
# Query
ctrl.dbg()                  # Returns full dict, e.g. {'sensors': 0, 'connection': 100, ...}
ctrl.dbg("connection")      # Returns level for 'connection' (0 if not set)

# Set individual components
ctrl.dbg("connection", 100) # Enable connection at level 100
ctrl.dbg("sensors", 0)      # Disable sensors (silenced on next boot)

# Mass update via dict
ctrl.dbg({"connection": 10, "protocol": 10, "sensors": 0, "config": 0})
```

**Behavior details:**
- Components set to `0` are remembered in the dict. On next boot they are registered with `enabled=False` and `silent=True` — no log output at all.
- Components with a level `> 0` are registered as enabled.
- The `"*"` wildcard key sets the default level for any component not explicitly listed: `ctrl.dbg({"*": 100, "sensors": 0})` enables everything except sensors.
- `ctrl.debug(N)` sets all components to level `N` and updates both `ctrl_debug` and `ctrl_dbg` in NVS.

#### Typical Workflow

```python
# 1. Enable everything to see what's happening
ctrl.debug(100)

# 2. Too noisy — disable sensors and config, keep connection/protocol
ctrl.dbg({"sensors": 0, "config": 0, "connection": 10, "protocol": 10})

# 3. Check current state
ctrl.dbg()
# {'sensors': 0, 'config': 0, 'library': 100, 'connection': 10, 'protocol': 10, ...}

# 4. Re-enable a single component
ctrl.dbg("sensors", 50)

# 5. Disable everything
ctrl.debug(0)
```

All settings persist to NVS automatically — after a reboot, each component resumes at its saved level.

#### Custom Components

The `print_debug()` helper routes through the logging system using the `component` keyword argument:

```python
from ctrl_debug import print_debug, register_component

DEBUG_COMPONENT = "my_module"
register_component(DEBUG_COMPONENT, color='yellow')

print_debug(5, "Something happened", component=DEBUG_COMPONENT)
```

## Examples

### Example 1: I2C Driver with Logging

```c
// i2c_driver.c
#include <stdio.h>
#include <esp_err.h>

#define __log_subsystem     drivers
#define __log_component     i2c
#include "log_lib.h"
__log_component_def(drivers, i2c, yellow, 1, 0)

esp_err_t i2c_read_register(uint8_t addr, uint8_t reg, uint8_t* data) {
    __log_debug("Reading register 0x%02X from device 0x%02X", reg, addr);
    
    esp_err_t err = /* ... actual I2C read ... */;
    
    if (err != ESP_OK) {
        __log_error("I2C read failed: %s (addr=0x%02X, reg=0x%02X)", 
                   esp_err_to_name(err), addr, reg);
        return err;
    }
    
    __log_debug("Read value: 0x%02X", *data);
    return ESP_OK;
}
```

### Example 2: State Machine with Detailed Logging

```c
// state_machine.c
#define __log_subsystem     control
#define __log_component     fsm
#include "log_lib.h"
__log_component_def(control, fsm, purple, 1, 0)

typedef enum {
    STATE_IDLE,
    STATE_CONNECTING,
    STATE_CONNECTED,
    STATE_ERROR
} device_state_t;

void state_transition(device_state_t from, device_state_t to) {
    const char* state_names[] = {"IDLE", "CONNECTING", "CONNECTED", "ERROR"};
    
    __log_info("State transition: %s -> %s", state_names[from], state_names[to]);
    
    if (to == STATE_ERROR) {
        __log_error("Entering error state from %s", state_names[from]);
    }
    
    // Handle state transition logic...
    __log_debug("State transition completed");
}
```

### Example 3: Component with logs_defs.h

```c
// src/network/inc/logs_defs.h
#include "log_lib.h"

__log_subsystem_def(network, cyan, 1, 0)

__log_component_def(network, tcp,       blue,   1, 0)
__log_component_def(network, udp,       green,  1, 0)
__log_component_def(network, http,      yellow, 1, 0)
__log_component_def(network, mqtt,      purple, 1, 0)
```

```c
// src/network/tcp_client.c
#define __log_subsystem     network
#define __log_component     tcp
#include "log_lib.h"

int tcp_connect(const char* host, int port) {
    __log_info("Connecting to %s:%d", host, port);
    
    // Connection logic...
    
    __log_info("TCP connection established");
    return 0;
}
```

## Best Practices

### 1. Meaningful Component Names
```c
// Good
#define __log_component     ioexp
#define __log_component     sx126x_driver
#define __log_component     tcp_client

// Avoid
#define __log_component     comp1
#define __log_component     module
#define __log_component     driver
```

### 2. Appropriate Log Levels
```c
// Use info for important status changes
__log_info("Device initialized successfully");
__log_info("Connection established");

// Use debug for detailed tracing
__log_debug("Processing packet %d, size: %d bytes", pkt_id, size);

// Use warn for recoverable issues
__log_warn("Retry attempt %d failed, retrying...", attempt);

// Use error for actual problems
__log_error("Configuration validation failed: %s", reason);
```

### 3. Structured Log Messages
```c
// Good - structured and informative
__log_info("I2C transaction completed: addr=0x%02X, reg=0x%02X, value=0x%02X", 
          device_addr, register_addr, value);

// Less helpful
__log_info("I2C done");
```

### 4. Error Context
```c
// Provide context with errors
esp_err_t err = i2c_master_write();
if (err != ESP_OK) {
    __log_error("I2C write failed: %s (addr=0x%02X, len=%d)", 
               esp_err_to_name(err), addr, len);
}
```

### 5. Conditional Debug Logging
```c
// For expensive debug operations
#if LOG_LOCAL_LEVEL >= ESP_LOG_DEBUG
    char buffer[256];
    format_complex_data(buffer, sizeof(buffer), data);
    __log_debug("Complex data: %s", buffer);
#endif
```

## Troubleshooting

### Problem: Logs Not Appearing

1. **Check component registration in CMakeLists.txt**:
   ```cmake
   LOGS_DEFS
       "${CMAKE_CURRENT_LIST_DIR}/your_file.c"  # or logs_defs.h
   ```

2. **Verify component definition**:
   ```c
   __log_component_def(subsystem, component, color, 1, 0)
   //                                               ^
   //                                    compile_flag must be 1
   ```

3. **Check log filtering**:
   ```c
   // Ensure subsystem/component is enabled
   log_filter_subsystem("your_subsystem", true);
   log_filter_component("your_subsystem", "your_component", true);
   ```

### Problem: Build Errors

1. **Missing log_lib dependency**:
   ```cmake
   REQUIRES
       log_lib  # Add this
   ```

2. **Include order matters**:
   ```c
   // Correct order
   #define __log_subsystem     my_sys
   #define __log_component     my_comp
   #include "log_lib.h"
   __log_component_def(my_sys, my_comp, color, 1, 0)
   ```

### Problem: Component ID Not Found

This usually means the component wasn't properly registered. Check:

1. Component definition syntax
2. CMakeLists.txt LOGS_DEFS entry
3. Build system regeneration (clean build)

### Problem: Colors Not Working

Colors require terminal support. Check:
1. Terminal supports ANSI color codes
2. Log output is going to terminal (not redirected to file)
3. Color support is enabled in build configuration

---

This logging system provides powerful debugging and monitoring capabilities for your ESP32 firmware. Start with the basic setup and gradually explore advanced features as needed.

<!--- end of file --->