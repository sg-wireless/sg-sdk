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

Description: Build commands and testing reference for sg-sdk
-->

# Build Commands Reference

For secure boot + flash encryption workflows (`--secure`), provisioning constraints, and OTA signed `application.bin` guidance, see [SECURITY.md](SECURITY.md).

## Quick Reference Commands

### Main Build Commands

#### Standard Build
```bash
cd /home/ehlers/sg-sdk
./fw_builder.sh --board SGW3501-F1-StarterKit build
```

#### Alternative Boards
```bash
# For different boards (from help output):
./fw_builder.sh --board SGW3101-F1W-OEM build
./fw_builder.sh --board SGW3131-F1WS-OEM build
./fw_builder.sh --board SGW3201-F1L-OEM build
./fw_builder.sh --board SGW3201-F1L-StarterKit build
./fw_builder.sh --board SGW3231-F1LS-OEM build
./fw_builder.sh --board SGW3401-F1C-OEM build
./fw_builder.sh --board SGW3401-F1C-StarterKit build
./fw_builder.sh --board SGW3431-F1CS-OEM build
./fw_builder.sh --board SGW3501-F1-EVB build
./fw_builder.sh --board SGW3501-F1-OEM build
./fw_builder.sh --board SGW3531-F1S-OEM build
```

#### Build Variants
```bash
# MicroPython variant (default)
./fw_builder.sh --board SGW3501-F1-StarterKit --variant micropython build

# Native C/C++ variant (without MicroPython)
./fw_builder.sh --board SGW3501-F1-StarterKit --variant native build
```

### Clean Build
```bash
cd /home/ehlers/sg-sdk
rm -rf build/
./fw_builder.sh --board SGW3501-F1-StarterKit build
```

### Other Build Commands

#### Clean Only
```bash
./fw_builder.sh --board SGW3501-F1-StarterKit clean
```

#### Configuration Menu
```bash
./fw_builder.sh --board SGW3501-F1-StarterKit config
```

#### Flash Firmware
```bash
# Requires --port option
./fw_builder.sh --board SGW3501-F1-StarterKit flash --port /dev/ttyUSB0
```

#### Erase Flash
```bash
./fw_builder.sh --board SGW3501-F1-StarterKit erase --port /dev/ttyUSB0
```

### Combination Flags (NEW)

The build system now supports combination flags that can be used together to streamline the build-flash workflow.

#### Clean Before Build
```bash
# Clean and then build
./fw_builder.sh --board SGW3501-F1-StarterKit --clean build
```

#### Build, Erase, and Flash in One Command
```bash
# Build, erase device, then flash
./fw_builder.sh --board SGW3501-F1-StarterKit --erase --flash --port /dev/ttyUSB0
```

#### Complete Rebuild and Flash Workflow
```bash
# Clean, build, erase, and flash in one command
./fw_builder.sh --board SGW3501-F1-StarterKit --clean --erase --flash --port /dev/ttyUSB0
```

#### Execution Order
When using combination flags, commands execute in this order:
1. **Clean** (if `--clean` specified)
2. **Build** (always executed unless command is `clean` or `config`)
3. **Erase** (if `--erase` specified)
4. **Flash** (if `--flash` specified or command is `flash`)

### Default Configuration File (NEW)

You can now create a `defaults.sdk` file in the sg-sdk root directory to set default values for common options.

#### Creating defaults.sdk
```bash
cd /home/ehlers/sg-sdk
cp defaults.sdk.example defaults.sdk
# Edit with your preferred defaults
nano defaults.sdk
```

#### Example defaults.sdk
```toml
# Target board (required if no --board specified on command line)
board = "SGW3501-F1-StarterKit"

# Serial port for flashing (optional)
port = "/dev/ttyUSB0"

# Build variant (optional, defaults to 'micropython')
variant = "micropython"

# Default project directory (optional)
project-dir = "./examples/hello_world"
```

#### Using defaults.sdk
With the above `defaults.sdk` file, you can simplify your commands:

```bash
# Instead of:
./fw_builder.sh --board SGW3501-F1-StarterKit --port /dev/ttyUSB0 flash

# You can now use:
./fw_builder.sh flash

# CLI arguments always override defaults:
./fw_builder.sh --board SGW3401-F1C-StarterKit flash  # Uses different board
```

#### Benefits
- Reduces typing for repetitive builds
- Consistent settings across your development sessions
- Easy to share team defaults (not checked into git by default)
- Command-line arguments always take precedence

### Advanced Options

#### Custom Definitions
```bash
./fw_builder.sh --board SGW3501-F1-StarterKit --defs SOME_DEF_VAR="one two" SOME_COUNTER=1 build
```

#### Custom Project Directory
```bash
./fw_builder.sh --board SGW3501-F1-StarterKit --project-dir /path/to/project build
```

#### Custom Version String
```bash
./fw_builder.sh --board SGW3501-F1-StarterKit --custom-version-string "v2.0.0-custom" build
```

## Patch Management Commands

### Generate ESP-IDF Patches
```bash
cd src/platforms/F1/esp-idf-patches/
./update_patches.sh
```

### Manual Patch Generation (if needed)
```bash
# Example for machine_pin modifications:
diff -u ext/micropython/ports/esp32/machine_pin.c /tmp/machine_pin_modified.c > /tmp/machine_pin.patch
```

### Check Patch Status
```bash
# View current patches
cat src/platforms/F1/mpy-hooks/patches/machine_pin.c.patch
cat src/platforms/F1/esp-idf-patches/*/patches/*.patch
```

## Troubleshooting Commands

### View Build Logs
```bash
# Check overall build log
tail -50 build.log

# Check specific build output
tail -50 build/sdk-default/F1/SGW3501-F1-StarterKit/micropython/log/idf_py_stderr_output_*
tail -50 build/sdk-default/F1/SGW3501-F1-StarterKit/micropython/log/idf_py_stdout_output_*
```

### Check Build Progress
```bash
# Monitor build in real-time
./fw_builder.sh --board SGW3501-F1-StarterKit build 2>&1 | grep -E "(Building|Linking|Generating)"
```

### Specific Configuration Builds

#### USB Configuration Build
```bash
# Note: Based on context, this might require special config handling
./fw_builder.sh --board SGW3501-F1-StarterKit build
# Then manually verify USB configs in:
# src/platforms/F1/configs/sdkconfig.usb
```

## Directory Structure Reference

### Important Paths
- **Build Output**: `build/sdk-default/F1/SGW3501-F1-StarterKit/`
- **ESP-IDF Patches**: `src/platforms/F1/esp-idf-patches/`
- **MicroPython Patches**: `src/platforms/F1/mpy-hooks/patches/`
- **Platform Sources**: `src/platforms/F1/`
- **Examples**: `examples/`

### Key Files
- **Build Script**: `fw_builder.sh`
- **Build Log**: `build.log`  
- **Configuration**: `src/platforms/F1/configs/sdkconfig.*`
- **MicroPython Config**: `src/platforms/F1/sdk-main/micropython/mpconfigport.h`

## Build System Notes

### Command Syntax
```
./fw_builder.sh --board BOARD [--variant VARIANT] [--defs DEFS...] [COMMAND] [options]
```

### Available Boards
- **SGW3501-F1-StarterKit** - Main development board  
- **SGW3501-F1-OEM** - OEM version
- **SGW3501-F1-EVB** - Evaluation board
- **SGW3401-F1C-StarterKit** - Alternative with different features
- Plus other variants (SGW31xx, SGW32xx series)

### Build Variants
- **micropython** (default) - Full MicroPython support
- **native** - C/C++ only, no MicroPython

### Commands
- **build** (default) - Build firmware
- **clean** - Clean build artifacts  
- **flash** - Build and flash (requires --port)
- **erase** - Erase flash (requires --port)
- **config** - Open configuration menu

## Testing Commands

### Automated REPL Testing

The `tests/micropython/repl_tester.py` script provides automated testing of MicroPython functionality via serial REPL connection.

#### Quick Test Examples
```bash
# Test USB functionality using predefined test suite
cd /home/ehlers/sg-sdk
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --test-file tests/micropython/test-cases/usb_device.json

# Run specific commands directly
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --commands "import machine" "hasattr(machine, 'USBDevice')" "machine.USBDevice()"

# Interactive mode for manual testing
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --interactive
```

#### Test Configuration Files

Test suites are defined in JSON format under `tests/micropython/test-cases/`:

```json
{
  "name": "Example Test Suite",
  "description": "Description of test purpose",
  "commands": [
    {
      "command": "import machine",
      "description": "Import machine module",
      "timeout": 2.0,
      "expected_not_contains": ["Error", "Traceback"],
      "should_succeed": true
    },
    {
      "command": "hasattr(machine, 'USBDevice')",
      "description": "Check USB device availability",
      "timeout": 2.0,
      "expected_response": "True",
      "should_succeed": true
    }
  ]
}
```

#### Available Test Options

**Response Validation:**
- `expected_response`: Exact response match
- `expected_contains`: Response must contain all listed strings
- `expected_not_contains`: Response must not contain any listed strings
- `timeout`: Maximum wait time for command execution (default: 2.0s)

**Available Test Files:**
- `usb_device.json` - USB Runtime Device functionality tests

#### Advanced Usage
```bash
# Enable debug output for troubleshooting
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --test-file tests/micropython/test-cases/usb_device.json --debug

# Custom timeout for slow operations
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --commands "import slow_module" --timeout 10.0

# Test multiple commands with custom timeouts
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 \
  --commands "import machine" "print('System info:', machine.freq())" "import gc; gc.collect()"
```

#### Exit Codes
- **0**: All tests passed
- **1**: One or more tests failed or connection error

### Manual Testing Commands

#### Basic Serial Connection
```bash
# Direct serial connection for manual testing
minicom -D /dev/ttyUSB0 -b 115200

# Alternative using screen
screen /dev/ttyUSB0 115200
```

#### Common Test Commands
```python
# Basic system test
import machine
print("System frequency:", machine.freq())
import gc; print("Free memory:", gc.mem_free())

# USB device test
import machine
print("USB available:", hasattr(machine, 'USBDevice'))
if hasattr(machine, 'USBDevice'):
    usb = machine.USBDevice()
    print("USB active:", usb.active())
    print("USB methods:", [m for m in dir(usb) if not m.startswith('_')])

# Hardware test
import machine
print("Unique ID:", machine.unique_id().hex())
print("Reset cause:", machine.reset_cause())
```

## MicroPython Remote Control (mpremote)

The `mpremote` tool provides powerful remote control capabilities for MicroPython devices. It's located at `/home/ehlers/sg-sdk/ext/micropython/tools/mpremote/mpremote.py`.

### Quick Connection

#### Basic Device Connection
```bash
# Connect to USB0 (most common for SGW3501-F1-StarterKit)
cd /home/ehlers/sg-sdk/ext/micropython/tools/mpremote
python3 mpremote.py u0

# Alternative connection methods
python3 mpremote.py connect /dev/ttyUSB0
python3 mpremote.py a0  # for /dev/ttyACM0
```

#### Device Discovery
```bash
# List available serial ports
python3 mpremote.py devs
```

### File System Operations

#### File Management
```bash
# List files on device
python3 mpremote.py u0 ls

# Create directory
python3 mpremote.py u0 mkdir /lib

# Copy file to device
python3 mpremote.py u0 cp main.py :main.py

# Copy file from device to local
python3 mpremote.py u0 cp :boot.py ./boot_backup.py

# Remove file from device
python3 mpremote.py u0 rm main.py

# View file contents
python3 mpremote.py u0 cat boot.py

# File system information
python3 mpremote.py u0 df
python3 mpremote.py u0 tree
```

#### Directory Operations
```bash
# Show directory tree
python3 mpremote.py u0 tree

# Create and remove directories
python3 mpremote.py u0 mkdir /test
python3 mpremote.py u0 rmdir /test

# File checksums
python3 mpremote.py u0 sha256sum boot.py
```

### Code Execution

#### Running Scripts
```bash
# Run local Python script on device
python3 mpremote.py u0 run /home/ehlers/LoRaWAN/main.py

# Execute single command
python3 mpremote.py u0 eval "print('Hello from SGW3501')"

# Execute multi-line code
python3 mpremote.py u0 exec "
import machine
print('Freq:', machine.freq())
print('Free RAM:', machine.mem_free())
"
```

#### Interactive REPL
```bash
# Enter interactive REPL session
python3 mpremote.py u0 repl

# Resume previous session without soft reset
python3 mpremote.py u0 resume
```

### Advanced Features

#### Device Management
```bash
# Soft reset device (restart MicroPython)
python3 mpremote.py u0 soft-reset

# Hard reset device
python3 mpremote.py u0 reset

# Enter bootloader mode
python3 mpremote.py u0 bootloader

# Set/get device RTC
python3 mpremote.py u0 rtc
python3 mpremote.py u0 rtc --set "2025-10-01 14:30:00"
```

#### Package Management
```bash
# Install packages from micropython-lib
python3 mpremote.py u0 mip install umqtt.simple
python3 mpremote.py u0 mip install urequests

# Install from specific source
python3 mpremote.py u0 mip install github:org/repo
```

#### Directory Mounting
```bash
# Mount local directory on device (for development)
python3 mpremote.py u0 mount /home/ehlers/LoRaWAN

# Unmount when done
python3 mpremote.py u0 umount
```

#### File Editing
```bash
# Edit file directly on device (uses local editor)
python3 mpremote.py u0 edit main.py
```

### Utility Commands

#### Timing and Delays
```bash
# Add delay between commands
python3 mpremote.py u0 sleep 2 eval "print('After 2 seconds')"
```

#### ROM File System
```bash
# Manage ROM partitions (advanced)
python3 mpremote.py u0 romfs --help
```

### Common Usage Patterns

#### Development Workflow
```bash
# Quick development cycle
cd /home/ehlers/sg-sdk/ext/micropython/tools/mpremote

# 1. Upload and test script
python3 mpremote.py u0 cp /home/ehlers/LoRaWAN/main.py :main.py
python3 mpremote.py u0 run main.py

# 2. Monitor and debug
python3 mpremote.py u0 repl

# 3. File system cleanup
python3 mpremote.py u0 ls
python3 mpremote.py u0 rm old_file.py
```

#### LoRaWAN Testing
```bash
# Upload LoRaWAN test script
cd /home/ehlers/sg-sdk/ext/micropython/tools/mpremote
python3 mpremote.py u0 cp /home/ehlers/LoRaWAN/main.py :main.py

# Run with output monitoring
python3 mpremote.py u0 soft-reset run main.py

# Interactive debugging after test
python3 mpremote.py u0 repl
```

#### System Information Gathering
```bash
# Get comprehensive device info
python3 mpremote.py u0 exec "
import machine, gc, sys
print('Platform:', sys.platform)
print('Version:', sys.version)
print('Frequency:', machine.freq(), 'Hz')
print('Unique ID:', machine.unique_id().hex())
print('Flash size:', machine.flash_size() if hasattr(machine, 'flash_size') else 'Unknown')
print('Free memory:', gc.mem_free(), 'bytes')
print('Reset cause:', machine.reset_cause())
"
```

### Troubleshooting

#### Connection Issues
```bash
# Check available devices
python3 mpremote.py devs

# Try different connection methods
python3 mpremote.py connect /dev/ttyUSB0
python3 mpremote.py connect /dev/ttyACM0

# Reset device if unresponsive
python3 mpremote.py u0 reset
```

#### Permission Issues
```bash
# Ensure user is in dialout group (run once)
sudo usermod -a -G dialout $USER
# Then logout/login or use: newgrp dialout
```

### mpremote vs Other Tools

| Feature | mpremote | minicom/screen | repl_tester.py |
|---------|----------|----------------|----------------|
| File transfer | ✅ Built-in | ❌ Manual | ❌ No |
| Script execution | ✅ Native | ⚠️ Copy/paste | ✅ Automated |
| Interactive REPL | ✅ Yes | ✅ Yes | ❌ No |
| Package management | ✅ mip command | ❌ No | ❌ No |
| Automated testing | ⚠️ Manual | ❌ No | ✅ JSON-based |
| File system ops | ✅ Full support | ❌ No | ❌ No |

**When to use each:**
- **mpremote**: Development, file management, package installation
- **minicom/screen**: Low-level debugging, manual exploration  
- **repl_tester.py**: Automated test suites, CI/CD integration

<!--- end of file --->