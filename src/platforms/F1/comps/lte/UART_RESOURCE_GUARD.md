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

Author:     Christian Ehlers (SG Wireless)

Description: UART Resource Guard Implementation Documentation
-->

# UART Resource Guard Implementation

## Overview

This document describes the UART resource guard system that prevents conflicts between the LTE modem and MicroPython UART usage on UART1.

## Problem Statement

The F1 platform has two LTE implementations that both use UART1:
1. **Legacy LTE.py** - Python-based LTE driver
2. **import lte** - New espmodem-based C implementation

Without protection, users could accidentally try to use `UART(1)` in MicroPython while the LTE modem is active, causing conflicts and undefined behavior.

## Solution Architecture

The implementation uses a simple, automatic approach:

1. **uart-guard component** - C-level resource tracking
2. **MicroPython UART patch** - Automatically reserves/releases UART on init/deinit
3. **LTE espmodem integration** - Reserves UART1 during modem initialization

### Key Design Principle

**UARTs are automatically locked when initialized and released when deinitialized.** No manual reserve/release calls needed!

- When you call `UART(1)` → Automatically reserves UART1 as "MicroPython user"
- When you call `uart.deinit()` → Automatically releases UART1
- When `import lte; lte.init()` runs → Reserves UART1 as "LTE modem (import lte)"
- When `lte.deinit()` runs → Releases UART1

This allows users to use UART1 when LTE is not active, but prevents conflicts when LTE is initialized.

### Component Structure

```
src/platforms/F1/
├── comps/
│   ├── uart-guard/              # UART resource tracking component
│   │   ├── uart_resource_guard.h
│   │   ├── uart_resource_guard.c
│   │   └── CMakeLists.txt
│   └── lte/
│       ├── espmodem.c           # Modified: reserves UART1 on init
│       ├── LTE.py               # Modified: reserves UART1 on init
│       ├── mod_lte_uart.c       # New: Python bindings for UART guard
│       └── CMakeLists.txt       # Updated: includes mod_lte_uart
└── mpy-hooks/
    ├── board_hooks.h            # Modified: added UART init hook
    ├── machine_uart_hooks.c     # New: hook implementation
    ├── modified_sources/
    │   └── machine_uart.c       # Modified: checks UART reservation
    ├── patches/
    │   └── machine_uart.c.patch # Generated patch file
    ├── CMakeLists.txt           # Updated: registers patch
    ├── mpy_hooks.config         # Updated: added Kconfig option
    └── update_patches.sh        # Updated: generates machine_uart patch
```

## Implementation Details

### 1. UART Guard Component

**Location:** `src/platforms/F1/comps/uart-guard/`

**API Functions:**
```c
bool uart_reserve(uint8_t uart_num, uart_owner_t owner, const char* owner_name);
bool uart_release(uint8_t uart_num, uart_owner_t owner);
bool uart_is_reserved(uint8_t uart_num, uart_owner_t* current_owner, const char** owner_name);
const char* uart_get_owner_description(uint8_t uart_num);
```

**Owner Types:**
- `UART_OWNER_NONE` - UART is available
- `UART_OWNER_LTE_LEGACY` - Reserved by LTE.py
- `UART_OWNER_LTE_ESPMODEM` - Reserved by import lte
- `UART_OWNER_USER` - Reserved by user code

**Features:**
- Thread-safe ownership tracking
- Detailed logging with SG-SDK log system
- Descriptive error messages showing current owner
- Supports up to 3 UARTs (configurable via `UART_GUARD_MAX_PORTS`)

### 2. MicroPython Patch

**Hook Definition** (`board_hooks.h`):
```c
extern void hook_mpy_machine_uart_init(int uart_num, bool *p_uart_reserved, const char **p_owner_name);
#define __hook_mpy_machine_uart_init(_uart_num, _p_reserved, _p_owner) \
    hook_mpy_machine_uart_init(_uart_num, _p_reserved, _p_owner)
```

**Hook Implementation** (`machine_uart_hooks.c`):
- On UART init: Checks if reserved by another component, if not reserves it for `UART_OWNER_USER`
- On UART deinit: Releases the UART if owned by `UART_OWNER_USER`
- Returns error information to MicroPython if UART is already in use

**Patch Applied** (`machine_uart.c.patch`):
- Adds `#include "board_hooks.h"` to machine_uart.c
- Inserts guard check in `mp_machine_uart_make_new()`:
```c
#ifdef __hook_mpy_machine_uart_init
bool uart_reserved = false;
const char *owner_name = NULL;
__hook_mpy_machine_uart_init(uart_num, &uart_reserved, &owner_name);
if (uart_reserved) {
    mp_raise_msg_varg(&mp_type_OSError, 
        MP_ERROR_TEXT("UART(%d) is in use by %s"), 
        uart_num, owner_name ? owner_name : "another component");
}
#endif
```
- Inserts release in `mp_machine_uart_deinit()`:
```c
#ifdef __hook_mpy_machine_uart_deinit
__hook_mpy_machine_uart_deinit(self->uart_num);
#endif
```

**Kconfig Option:**
- `CONFIG_SDK_MPY_HOOK_MACHINE_UART_INIT_ENABLE` (default: y)
- Can be disabled via menuconfig if guard not needed

### 3. LTE Integration

#### espmodem (import lte)

**Modified:** `src/platforms/F1/comps/lte/espmodem.c`

**Initialization** (`espmodem_init()`):
```c
// Reserve UART1 for LTE modem use
if (!uart_reserve(1, UART_OWNER_LTE_ESPMODEM, "LTE modem (import lte)")) {
    const char* owner = uart_get_owner_description(1);
    __log_error("Cannot initialize LTE: UART1 already in use by %s", owner);
    return ESPMODEM_ERR_INVALID_ARG;
}
```

**Cleanup** (`espmodem_deinit()`):
```c
// Release UART1 reservation
uart_release(1, UART_OWNER_LTE_ESPMODEM);
```

#### Legacy LTE.py

**Modified:** `LTE.py`

**No special handling needed!** LTE.py simply calls `UART(1, ...)` which automatically reserves UART1.

**Cleanup** (`deinit()`):
```python
# Deinitialize UART to release the resource
if LTE.lte_uart is not None:
    LTE.lte_uart.deinit()  # Automatically releases UART1
    LTE.lte_uart = None
```

## Build System Integration

### Component Registration

**Platform CMakeLists.txt** (`src/platforms/F1/CMakeLists.txt`):
- Added `${__dir_platform_comps}/uart-guard` to `__sdk_add_comp_dirs()`

**LTE Component** (`comps/lte/CMakeLists.txt`):
- Added `mod_lte_uart.c` to `MPY_MODS` and `SRCS`
- Added `f1_uart_guard` to `REQUIRES`

**MicroPython Hooks** (`mpy-hooks/CMakeLists.txt`):
- Registered machine_uart.c patch via `__sdk_add_patch()`
- Added `f1_uart_guard` to `REQUIRED_SDK_LIBS`

### Patch Generation

The patch is generated automatically via `update_patches.sh`:
```bash
cd src/platforms/F1/mpy-hooks
bash update_patches.sh
```

This creates `patches/machine_uart.c.patch` from the modified source in `modified_sources/machine_uart.c`.

## Usage Examples

### Scenario 1: LTE.py in use, user tries UART(1)

```python
from LTE import LTE
lte = LTE()  # Calls UART(1, ...) which auto-reserves UART1

from machine import UART
uart = UART(1)  # Raises OSError: UART(1) is in use by MicroPython user
```

### Scenario 2: import lte in use, user tries UART(1)

```python
import lte
lte.init()  # Reserves UART1 as "LTE modem (import lte)"

from machine import UART
uart = UART(1)  # Raises OSError: UART(1) is in use by LTE modem (import lte)
```

### Scenario 3: User reserves UART1 first

```python
from machine import UART
uart = UART(1)  # Auto-reserves UART1 as "MicroPython user"

# Later...
from LTE import LTE
lte = LTE()  # Raises OSError: UART(1) is in use by MicroPython user
```

### Scenario 4: Proper cleanup allows reuse

```python
import lte
lte.init()    # Reserves UART1
lte.deinit()  # Releases UART1

from machine import UART
uart = UART(1)  # Now works - UART1 is available again
```

### Scenario 5: User can release UART1 manually

```python
from machine import UART
uart = UART(1)  # Auto-reserves UART1
uart.deinit()   # Releases UART1

# Now LTE can use it
from LTE import LTE
lte = LTE()  # Works!
```

## Error Messages

All error messages include the current owner for debugging:

- **MicroPython UART init:** `OSError: UART(1) is in use by LTE modem (import lte)`
- **LTE.py init:** `OSError: Cannot initialize LTE: UART1 already in use by user`
- **espmodem init:** Logs error and returns `ESPMODEM_ERR_INVALID_ARG`

## Testing

To test the implementation:

1. **Build the firmware:**
   ```bash
   cd sg-sdk
   ./fw_builder.sh build examples/hello_world
   ```

2. **Test LTE.py reservation:**
   ```python
   from LTE import LTE
   lte = LTE()  # UART(1) auto-reserves
   from machine import UART
   uart = UART(1)  # Should raise OSError
   ```

3. **Test import lte reservation:**
   ```python
   import lte
   lte.init()
   from machine import UART
   uart = UART(1)  # Should raise OSError
   ```

4. **Test automatic release:**
   ```python
   from machine import UART
   uart = UART(1)  # Auto-reserves
   uart.deinit()   # Auto-releases
   
   import lte
   lte.init()  # Should work now
   ```

## Configuration

### Enable/Disable UART Guard

Via menuconfig:
```bash
cd build/sdk-default
idf.py menuconfig
# Navigate to: SG Wireless SDK -> F1 Platform -> MicroPython Hooks
# Toggle: "Enable UART resource guard for UART initialization"
```

Or via sdkconfig:
```
CONFIG_SDK_MPY_HOOK_MACHINE_UART_INIT_ENABLE=y
```

### Adjust Maximum UART Ports

Edit `src/platforms/F1/comps/uart-guard/uart_resource_guard.h`:
```c
#define UART_GUARD_MAX_PORTS 3  // Change to support more UARTs
```

## Logging

The UART guard uses SG-SDK's structured logging:

```c
__log_component_def(UART_GUARD, uart_guard, cyan, 1, 1);
__log_component_def(MOD_LTE_UART, mod_lte_uart, cyan, 1, 1);
```

**Log Messages:**
- `UART1 reserved by LTE modem (import lte)`
- `UART1 released by LTE modem (import lte)`
- `UART1 reservation failed - already owned by LTE (LTE.py)`

## Future Enhancements

Potential improvements:

1. **Auto-release on component cleanup** - Track component lifecycle
2. **UART remapping** - Suggest alternative UARTs if UART1 is busy
3. **Reservation timeout** - Auto-release if component doesn't cleanup
4. **Global reservation API** - Extend to other resources (SPI, I2C)
5. **Debug command** - Add MicroPython command to show all UART reservations

## Implementation Notes

- Uses SG-SDK patch mechanism to avoid modifying external MicroPython source
- Follows existing I2C hook pattern for consistency
- Thread-safe via ESP-IDF's UART driver mutex
- Minimal runtime overhead (simple flag check)
- Gracefully degrades if CONFIG disabled (no guard check)

## Files Modified/Created

**Created:**
- `src/platforms/F1/comps/uart-guard/uart_resource_guard.h`
- `src/platforms/F1/comps/uart-guard/uart_resource_guard.c`
- `src/platforms/F1/comps/uart-guard/CMakeLists.txt`
- `src/platforms/F1/comps/lte/logs_defs.h`
- `src/platforms/F1/mpy-hooks/machine_uart_hooks.c`
- `src/platforms/F1/mpy-hooks/modified_sources/machine_uart.c`
- `src/platforms/F1/mpy-hooks/patches/machine_uart.c.patch`

**Modified:**
- `src/platforms/F1/comps/lte/espmodem.c`
- `src/platforms/F1/comps/lte/LTE.py`
- `src/platforms/F1/comps/lte/CMakeLists.txt`
- `src/platforms/F1/mpy-hooks/board_hooks.h`
- `src/platforms/F1/mpy-hooks/CMakeLists.txt`
- `src/platforms/F1/mpy-hooks/mpy_hooks.config`
- `src/platforms/F1/mpy-hooks/update_patches.sh`
- `src/platforms/F1/CMakeLists.txt`

---

**Author:** Christian Ehlers  
**Date:** 2024  
**Status:** Completed - Ready for testing

<!--- end of file --->
