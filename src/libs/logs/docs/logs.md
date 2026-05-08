<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
 !
 ! Permission is hereby granted, free of charge, to any person obtaining a copy
 ! of this software and associated documentation files(the "Software"), to deal
 ! in the Software without restriction, including without limitation the rights
 ! to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 ! copies  of  the  Software,  and  to  permit  persons to whom the Software is
 ! furnished to do so, subject to the following conditions:
 !
 ! The above copyright notice and this permission notice shall be included in
 ! all copies or substantial portions of the Software.
 !
 ! THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 ! IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 ! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 ! AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 ! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ! OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 ! THE SOFTWARE.
 !
 ! @author  Ahmed Sabry (SG Wireless)
 ! @maintainer  Christian Ehlers (SG Wireless)
 !
 ! @brief   Documentation file for the Logging Library.
 !----------------------------------------------------------------------------->

# Logging Library

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Introduction](#intro)
* [Architecture](#architecture)
* [C API — Subsystem and Component Definitions](#c-defs)
* [C API — Logging Macros](#c-macros)
* [C API — Color Macros](#c-colors)
* [C API — Filtering Functions](#c-filter)
* [C API — Dynamic Registration](#c-dynamic)
* [MicroPython API — `logs` Module](#mpy-module)
* [MicroPython API — Registration](#mpy-registration)
* [MicroPython API — Logging Functions](#mpy-logging)
* [MicroPython API — Filtering](#mpy-filtering)
* [Log Output Format](#output-format)
* [Kconfig Options](#kconfig)
* [Capacity Limits](#limits)
* [Example — C Component](#example-c)
* [Example — MicroPython](#example-mpy)

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="intro"></div>

## Introduction

The logging library provides a unified, structured logging system for both C
native code and MicroPython applications. All log messages pass through the same
pipeline and appear in a consistent columnar format with timestamp, log type,
OS context, source location, subsystem, component, and message fields.

Key features:

* **Structured output** — fixed-width columns with configurable widths
* **Compile-time definitions** — subsystems and components defined via macros,
  enabling per-module compile-time gating
* **Runtime filtering** — enable/disable output by subsystem, component,
  log type, or header column at runtime
* **Terminal coloring** — ANSI color support with per-subsystem and
  per-component colors
* **Dynamic registration** — MicroPython code can register subsystems and
  components at runtime and produce log output identical to native C logs
* **Soft-reset safe** — dynamic registrations are cleared on MicroPython
  soft reset, allowing clean re-registration

<!------------------------------------------------------------------------------
 ! Architecture
 !----------------------------------------------------------------------------->
<div id="architecture"></div>

## Architecture

```
┌──────────────────────┐     ┌──────────────────────┐
│   C source files     │     │  MicroPython scripts  │
│  __log_info(...)     │     │  logs.info(...)       │
└──────────┬───────────┘     └──────────┬───────────┘
           │                            │
           │  compile-time              │  runtime dynamic
           │  subsys/comp IDs           │  subsys/comp names
           │                            │
           ▼                            ▼
    ┌──────────────────────────────────────────┐
    │           log pipeline (log_main.c)       │
    │  ┌─────────┐  ┌──────────┐  ┌─────────┐ │
    │  │ filter   │→│ header   │→│ output  │ │
    │  │ checks   │  │ render   │  │ serial  │ │
    │  └─────────┘  └──────────┘  └─────────┘ │
    └──────────────────────────────────────────┘
```

The static registry holds subsystems and components defined at compile time
using `__log_subsystem_def` and `__log_component_def`. The dynamic registry
holds subsystems and components registered at runtime from MicroPython via
`log_register_subsystem()` and `log_register_component()`. Both registries
participate in the same filtering and output pipeline.

<!------------------------------------------------------------------------------
 ! C API - Subsystem and Component Definitions
 !----------------------------------------------------------------------------->
<div id="c-defs"></div>

## C API — Subsystem and Component Definitions

Each C source file selects its subsystem and component before including the
log header:

```c
#define __log_subsystem     mysubsys
#define __log_component     mycomp
#include "log_lib.h"
```

Subsystems and components are defined (typically once per compilation unit)
using the following macros:

* `__log_subsystem_def(name, color, compile_flag, on)` — define a subsystem.

    | Parameter | Description |
    |-----------|-------------|
    | `name` | Subsystem identifier (used as a C token) |
    | `color` | Color identifier: `default`, `red`, `green`, `yellow`, `blue`, `purple`, `cyan`, `white`, `black` |
    | `compile_flag` | `1` to compile log calls, `0` to strip them at compile time |
    | `on` | `1` to enable output at startup, `0` to start disabled |

* `__log_component_def(subsystem, name, color, compile_flag, on)` — define a
  component under an existing subsystem. Same parameters as above, with an
  additional `subsystem` parameter naming the parent.

```c
// mydriver.c
#define __log_subsystem     drivers
#define __log_component     spi
#include "log_lib.h"

__log_subsystem_def(drivers, blue, 1, 1)
__log_component_def(drivers, spi, yellow, 1, 1)
```

<!------------------------------------------------------------------------------
 ! C API - Logging Macros
 !----------------------------------------------------------------------------->
<div id="c-macros"></div>

## C API — Logging Macros

All macros use `printf`-style format strings. Output is gated by the compile
flag and runtime filter state of the current subsystem and component.

| Macro | Log Type | Description |
|-------|----------|-------------|
| `__log_info(fmt, ...)` | info | General status and progress |
| `__log_debug(fmt, ...)` | debug | Detailed tracing information |
| `__log_warn(fmt, ...)` | warn | Recoverable issues, unusual conditions |
| `__log_error(fmt, ...)` | error | Error conditions and failures |
| `__log_output(fmt, ...)` | output | Direct standard output (no header) |
| `__log_printf(fmt, ...)` | printf | Continuation of a previous log line |
| `__log_assert(cond, fmt, ...)` | assert | Fatal assertion (triggers crash if `cond` is false) |
| `__log_ptr(ptr)` | debug | Convenience: logs `"pointer: name = 0xADDR"` |

```c
__log_info("SPI transfer complete, %d bytes", len);
__log_debug("Register 0x%02X = 0x%02X", reg, val);
__log_warn("Retry %d/%d", attempt, max_retries);
__log_error("SPI timeout after %d ms", timeout_ms);
__log_assert(buf != NULL, "buffer must not be NULL");
```

<!------------------------------------------------------------------------------
 ! C API - Color Macros
 !----------------------------------------------------------------------------->
<div id="c-colors"></div>

## C API — Color Macros

Inline color escapes can be embedded in log format strings. These are stripped
when `SDK_LOG_LIB_TERMINAL_COLORING_ENABLE` is disabled.

| Macro | Color |
|-------|-------|
| `__red__` | Red |
| `__green__` | Green |
| `__yellow__` | Yellow |
| `__blue__` | Blue |
| `__purple__` | Purple |
| `__cyan__` | Cyan |
| `__white__` | White |
| `__black__` | Black |
| `__default__` | Terminal default |

```c
__log_info("Status: "__green__"OK"__default__);
__log_error("Status: "__red__"FAIL"__default__" (code %d)", err);
```

String wrapper macros are also available: `__red_str("text")`,
`__green_str("text")`, etc. These automatically reset to the default color
after the string.

<!------------------------------------------------------------------------------
 ! C API - Filtering Functions
 !----------------------------------------------------------------------------->
<div id="c-filter"></div>

## C API — Filtering Functions

All filtering functions work on both static (compile-time) and dynamic
(runtime) registrations.

* `void log_filter_subsystem(name, state, silent)` — enable (`true`) or
  disable (`false`) all output from a subsystem. When `silent` is `false`,
  a confirmation message is logged.

* `void log_filter_component(subsys_name, comp_name, state, silent)` — enable
  or disable a specific component.

* `void log_filter_type(type_name, state, silent)` — enable or disable a log
  type globally (e.g., `"debug"`, `"info"`).

* `void log_filter_header(column_name, state, silent)` — show or hide a header
  column (e.g., `"timestamp"`, `"func_name"`).

* `void log_filter_header_reorder(name, new_order)` — change the display order
  of a header column.

* `void log_filter_list_stats()` — print the current filter state for all
  subsystems, components, log types, and header columns.

* `bool log_filter_subsystem_get_state(name)` — query the current enable state
  of a subsystem.

* `bool log_filter_component_get_state(subsys_name, comp_name)` — query the
  current enable state of a component.

* `void log_filter_save_state(p_state, new_state)` — save the current filter
  state for a subsystem/component pair and set a new state. Useful for
  temporarily enabling output around a specific operation.

* `void log_filter_restore_state(p_state)` — restore a previously saved filter
  state.

<!------------------------------------------------------------------------------
 ! C API - Dynamic Registration
 !----------------------------------------------------------------------------->
<div id="c-dynamic"></div>

## C API — Dynamic Registration

These functions are the C-level API backing the MicroPython `logs` module.
They can also be called directly from C code that needs runtime registration.

* `int log_register_subsystem(name, color_name, enabled, silent)` — register
  a dynamic subsystem. Returns `0` on success, `-1` if the name is a duplicate,
  clashes with a static subsystem, or the registry is full.

* `int log_register_component(subsys_name, comp_name, color_name, enabled, silent)`
  — register a dynamic component under an existing dynamic subsystem.
  Returns `0` on success, `-1` on failure.

* `void log_dynamic_message(subsys_name, comp_name, p_type_info, msg)` — push
  a message through the log pipeline using dynamic subsystem/component names.

* `void log_dynamic_registry_clear()` — clear all dynamic registrations.
  Called automatically on MicroPython soft reset to allow re-registration.

<!------------------------------------------------------------------------------
 ! MicroPython API - logs Module
 !----------------------------------------------------------------------------->
<div id="mpy-module"></div>

## MicroPython API — `logs` Module

```python
import logs
```

The `logs` module is a built-in C module enabled by `SDK_LOG_LIB_MPY_CMOD_ENABLE`.
It provides Python access to log registration, message output, and filtering.

<!------------------------------------------------------------------------------
 ! MicroPython API - Registration
 !----------------------------------------------------------------------------->
<div id="mpy-registration"></div>

## MicroPython API — Registration

Before logging, at least one subsystem and one component must be registered.

* `logs.register_subsystem(name, *, color='default', enabled=True, silent=True)`

    Register a new dynamic subsystem.

    | Parameter | Type | Default | Description |
    |-----------|------|---------|-------------|
    | `name` | str | *(required)* | Subsystem name (max 23 characters) |
    | `color` | str | `'default'` | Display color for the subsystem column |
    | `enabled` | bool | `True` | Whether the subsystem starts enabled |
    | `silent` | bool | `True` | If `False`, logs a registration confirmation |

    Raises `ValueError` if the registry is full or the name is a duplicate.

* `logs.register_component(subsystem, component, *, color='default', enabled=True, silent=True)`

    Register a dynamic component under an existing subsystem.

    | Parameter | Type | Default | Description |
    |-----------|------|---------|-------------|
    | `subsystem` | str | *(required)* | Parent subsystem name (must exist) |
    | `component` | str | *(required)* | Component name (max 23 characters) |
    | `color` | str | `'default'` | Display color for the component column |
    | `enabled` | bool | `True` | Whether the component starts enabled |
    | `silent` | bool | `True` | If `False`, logs a registration confirmation |

    Raises `ValueError` if the subsystem is not found or the component is a
    duplicate.

<!------------------------------------------------------------------------------
 ! MicroPython API - Logging Functions
 !----------------------------------------------------------------------------->
<div id="mpy-logging"></div>

## MicroPython API — Logging Functions

All logging functions share the same signature:

```python
logs.<level>(subsystem, component, message)
```

| Function | Log Type | Typical Use |
|----------|----------|-------------|
| `logs.info(subsystem, component, message)` | info | General status and progress |
| `logs.debug(subsystem, component, message)` | debug | Detailed tracing information |
| `logs.warn(subsystem, component, message)` | warn | Recoverable issues |
| `logs.error(subsystem, component, message)` | error | Error conditions and failures |
| `logs.output(subsystem, component, message)` | output | Direct output (no log-type decoration) |

All three arguments are required strings. The subsystem and component must have
been previously registered.

```python
logs.info("myapp", "network", "Connected to broker")
logs.debug("myapp", "sensors", "Raw value: {}".format(val))
logs.warn("myapp", "network", "Connection timeout, retrying...")
logs.error("myapp", "network", "Failed: {}".format(err))
logs.output("myapp", "sensors", "Temperature: 24.5°C")
```

<!------------------------------------------------------------------------------
 ! MicroPython API - Filtering
 !----------------------------------------------------------------------------->
<div id="mpy-filtering"></div>

## MicroPython API — Filtering

* `logs.filter_subsystem(subsystem, state, *, silent=False)` — enable or
  disable a subsystem. Works on both static and dynamic subsystems.

* `logs.filter_component(subsystem, component, state, *, silent=False)` —
  enable or disable a specific component.

* `logs.filter_header(header_item, state, *, silent=False)` — show or hide a
  header column. Valid names: `"timestamp"`, `"log_type"`, `"os_info"`,
  `"subsystem"`, `"component"`, `"filename"`, `"line_num"`, `"func_name"`.

* `logs.filter_log_type(log_type, state, *, silent=False)` — enable or disable
  a log type globally. Valid names: `"info"`, `"debug"`, `"warn"`, `"error"`,
  `"output"`, `"printf"`, `"assert"`.

* `logs.header_reorder(header_item, new_order)` — change the display position
  of a header column.

* `logs.filter_stats()` — print the current filter state for all subsystems,
  components, types, and header columns.

```python
# Disable all debug output
logs.filter_log_type("debug", False)

# Hide the function name column
logs.filter_header("func_name", False)

# Disable a specific component
logs.filter_component("ctrl", "sensors", False)

# Print full filter status
logs.filter_stats()
```

<!------------------------------------------------------------------------------
 ! Log Output Format
 !----------------------------------------------------------------------------->
<div id="output-format"></div>

## Log Output Format

All log messages (C and MicroPython) appear in the same structured format:

```
|000:00:06-822|  info  |1:mp_task      |mpy             |  ctrl   |  connection | Connected to MQTT broker
```

The default column layout:

| Column | Header Name | Default Width | Description |
|--------|-------------|---------------|-------------|
| Timestamp | `timestamp` | 13 | Time since boot `HHH:MM:SS-mmm` |
| Log type | `log_type` | 8 | `info`, `debug`, `warn`, `error`, `output` |
| OS context | `os_info` | 12 | Core ID and task name (e.g. `1:mp_task`) |
| Function | `func_name` | 15 | Source function name (C) or `mpy` (Python) |
| Subsystem | `subsystem` | 9 | Registered subsystem name |
| Component | `component` | 13 | Registered component name |
| Message | — | — | The log message text |

Columns can be enabled/disabled, reordered, and resized via Kconfig and runtime
filter APIs. The `filename` and `line_num` columns are disabled by default.

<!------------------------------------------------------------------------------
 ! Kconfig Options
 !----------------------------------------------------------------------------->
<div id="kconfig"></div>

## Kconfig Options

All options are under the `SDK_LOG_LIB_ENABLE` menu.

| Option | Default | Description |
|--------|---------|-------------|
| `SDK_LOG_LIB_ENABLE` | y | Master enable for the logging library |
| `SDK_LOG_LIB_MEMORY_BUFFER_SIZE_KB` | 4 | Internal buffer size in KB |
| `SDK_LOG_LIB_TERMINAL_COLORING_ENABLE` | y | ANSI color output |
| `SDK_LOG_LIB_MPY_CMOD_ENABLE` | y | Enable MicroPython `logs` module |

**Log types** — each can be individually compiled in or out:

| Option | Default |
|--------|---------|
| `SDK_LOG_LIB_TYPE_INFO` | y |
| `SDK_LOG_LIB_TYPE_DEBUG` | y |
| `SDK_LOG_LIB_TYPE_WARN` | y |
| `SDK_LOG_LIB_TYPE_ERROR` | y |
| `SDK_LOG_LIB_TYPE_PRINTF` | y |
| `SDK_LOG_LIB_TYPE_ASSERT` | y |
| `SDK_LOG_LIB_TYPE_MEM_DUMP` | y |
| `SDK_LOG_LIB_TYPE_ENFORCE` | y |

**Header fields** — each can be enabled/disabled and has a configurable width:

| Field | Enable Option | Width Option | Default Width |
|-------|---------------|--------------|---------------|
| Timestamp | `SDK_LOG_LIB_HEADER_TIMESTAMP` (y) | `..._TIMESTAMP_WIDTH` | 13 |
| Log type | `SDK_LOG_LIB_HEADER_LOG_TYPE` (y) | `..._LOG_TYPE_WIDTH` | 8 |
| OS context | `SDK_LOG_LIB_HEADER_OS_CONTEXT_INFO` (y) | `..._OS_CONTEXT_INFO_WIDTH` | 12 |
| File name | `SDK_LOG_LIB_HEADER_FILENAME` (n) | `..._FILENAME_WIDTH` | 12 |
| Line number | `SDK_LOG_LIB_HEADER_LINE_NUM` (n) | `..._LINE_NUM_WIDTH` | 6 |
| Function | `SDK_LOG_LIB_HEADER_FUNC_NAME` (y) | `..._FUNC_NAME_WIDTH` | 15 |
| Subsystem | `SDK_LOG_LIB_HEADER_SUBSYSTEM` (y) | `..._SUBSYSTEM_WIDTH` | 9 |
| Component | `SDK_LOG_LIB_HEADER_COMPONENT` (y) | `..._COMPONENT_WIDTH` | 13 |

Line wrapping is optionally enabled with `SDK_LOG_LIB_TOTAL_LINE_WRAPPING_ENABLE`
(default: off) and `SDK_LOG_LIB_TOTAL_LINE_WIDTH` (default: 86).

<!------------------------------------------------------------------------------
 ! Capacity Limits
 !----------------------------------------------------------------------------->
<div id="limits"></div>

## Capacity Limits

**Dynamic registry** (runtime registrations from MicroPython or C):

| Resource | Limit |
|----------|-------|
| Dynamic subsystems | 8 |
| Dynamic components (shared across all subsystems) | 32 |
| Name length | 23 characters |

These limits are separate from compile-time static subsystems and components,
which have no practical limit beyond available flash.

The dynamic registry is cleared on every MicroPython soft reset
(`log_dynamic_registry_clear()`), allowing modules to re-register cleanly.

<!------------------------------------------------------------------------------
 ! Example - C Component
 !----------------------------------------------------------------------------->
<div id="example-c"></div>

## Example — C Component

```c
// spi_driver.c
#define __log_subsystem     drivers
#define __log_component     spi
#include "log_lib.h"

__log_subsystem_def(drivers, blue, 1, 1)
__log_component_def(drivers, spi, yellow, 1, 1)

esp_err_t spi_transfer(uint8_t* buf, size_t len) {
    __log_debug("SPI transfer: %d bytes at %p", len, buf);

    esp_err_t err = /* ... actual transfer ... */;

    if (err != ESP_OK) {
        __log_error("SPI transfer failed: %s", esp_err_to_name(err));
        return err;
    }

    __log_info("SPI transfer complete: %d bytes", len);
    return ESP_OK;
}
```

Output:

```
|000:00:01-234|  debug |0:spi_task    |spi_driver.c    | drivers |     spi     | SPI transfer: 64 bytes at 0x3FC90000
|000:00:01-235|  info  |0:spi_task    |spi_driver.c    | drivers |     spi     | SPI transfer complete: 64 bytes
```

<!------------------------------------------------------------------------------
 ! Example - MicroPython
 !----------------------------------------------------------------------------->
<div id="example-mpy"></div>

## Example — MicroPython

```python
import logs

# Register subsystem and components
logs.register_subsystem("myapp", color='cyan', enabled=True, silent=False)
logs.register_component("myapp", "main", color='green', enabled=True)
logs.register_component("myapp", "network", color='blue', enabled=True)
logs.register_component("myapp", "sensors", color='yellow', enabled=True)

# Log messages
logs.info("myapp", "main", "Application starting")
logs.info("myapp", "network", "Connecting to WiFi...")
logs.debug("myapp", "network", "SSID: MyNetwork, RSSI: -45")
logs.warn("myapp", "sensors", "Battery level low: 15%")
logs.error("myapp", "network", "MQTT connection lost")

# Runtime filtering
logs.filter_component("myapp", "sensors", False)     # silence sensors
logs.filter_component("myapp", "sensors", True)      # re-enable sensors
logs.filter_stats()                                   # show all filter states
```

Output:

```
|000:00:06-100|  info  |1:mp_task     |mpy             |  myapp  |    main     | Application starting
|000:00:06-110|  info  |1:mp_task     |mpy             |  myapp  |   network   | Connecting to WiFi...
|000:00:06-120|  debug |1:mp_task     |mpy             |  myapp  |   network   | SSID: MyNetwork, RSSI: -45
|000:00:06-130|  warn  |1:mp_task     |mpy             |  myapp  |   sensors   | Battery level low: 15%
|000:00:06-140|  error |1:mp_task     |mpy             |  myapp  |   network   | MQTT connection lost
```