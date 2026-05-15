# MicroPython Automated Testing Framework

This directory contains automated testing tools for validating MicroPython functionality in sg-sdk firmware.

## Overview

The testing framework provides automated validation of MicroPython features via serial REPL connection, enabling continuous integration and systematic feature validation.

## Files Structure

```
tests/micropython/
├── repl_tester.py              # Main testing script
├── test-cases/                 # Test configuration files
│   ├── basic_functionality.json   # Basic system health tests
│   ├── usb_device.json            # USB Runtime Device tests
│   └── ...                        # Additional test suites
├── run_test_suite.sh           # Legacy test runner
├── test_suite_manifest.json    # Legacy test manifest
└── README.md                   # This file
```

## Quick Start

### 1. Basic System Test
```bash
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --test-file tests/micropython/test-cases/basic_functionality.json
```

### 2. USB Device Test
```bash
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --test-file tests/micropython/test-cases/usb_device.json
```

### 3. Custom Commands
```bash
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --commands "import machine" "print(machine.freq())"
```

### 4. Interactive Mode
```bash
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --interactive
```

## Test Configuration Format

Test suites are defined in JSON format:

```json
{
  "name": "Test Suite Name",
  "description": "Purpose of this test suite",
  "commands": [
    {
      "command": "import machine",
      "description": "Human-readable description",
      "timeout": 2.0,
      "expected_response": "exact match (optional)",
      "expected_contains": ["must contain these strings"],
      "expected_not_contains": ["must not contain these"],
      "should_succeed": true
    }
  ]
}
```

### Response Validation Options

- **`expected_response`**: Exact string match (after removing prompts and echo)
- **`expected_contains`**: List of strings that must be present in response
- **`expected_not_contains`**: List of strings that must NOT be present
- **`timeout`**: Maximum wait time for command execution (default: 2.0s)
- **`should_succeed`**: Whether command should succeed (default: true)

## Available Test Suites

### basic_functionality.json
Tests core MicroPython functionality:
- Module imports (machine, gc)
- System information (frequency, memory, reset cause)
- Device identification (unique ID)

**Use case**: Quick system health check after firmware flash

### usb_device.json
Tests USB Runtime Device functionality:
- USB device class availability
- Instance creation and methods
- Built-in configuration constants
- Method availability validation

**Use case**: Validate USB Runtime Device restoration after MicroPython upgrade

## Script Features

### Command Line Options
```bash
python3 repl_tester.py [options]

Required:
  --port PORT         Serial port (e.g., /dev/ttyUSB0)

Modes (choose one):
  --test-file FILE    Run JSON test suite
  --commands CMD...   Run specific commands
  --interactive       Interactive REPL mode

Optional:
  --baudrate RATE     Serial baud rate (default: 115200)
  --debug            Enable debug output
  --timeout SECONDS   Default timeout (default: 2.0)
```

### Exit Codes
- **0**: All tests passed
- **1**: One or more tests failed or connection error

### Output Format
```
Running 7 test commands...

[1/7] Import machine module
  ✅ PASS (2.01s)

[2/7] Check system frequency
  ❌ FAIL (2.01s)
  Error: Response missing required content: 'System frequency:'

==================================================
TEST SUMMARY
==================================================
Total:  7
Passed: 6 ✅
Failed: 1 ❌
Success Rate: 85.7%

Failed Tests:
  • print('System frequency:', machine.freq())
    Response missing required content: 'System frequency:'
```

## Creating New Test Suites

1. **Create JSON file** in `test-cases/` directory
2. **Define test commands** with appropriate validation
3. **Test the suite** manually before adding to automation
4. **Document the purpose** in this README

### Best Practices
- Use descriptive command descriptions
- Set appropriate timeouts for slow operations
- Use `expected_contains` for flexible matching
- Use `expected_not_contains` to catch error conditions
- Test both success and failure cases where appropriate

## Integration with sg-sdk

### Post-Build Validation
```bash
# After successful firmware build and flash
./fw_builder.sh --board SGW3501-F1-StarterKit flash --port /dev/ttyUSB0
python3 tests/micropython/repl_tester.py --port /dev/ttyUSB0 --test-file tests/micropython/test-cases/basic_functionality.json
```

### Feature Validation
When restoring disabled features (see `MICROPYTHON_V1_26_1_DISABLED_FEATURES.md`):

1. Build firmware with feature enabled
2. Flash to device
3. Run appropriate test suite to validate functionality
4. Update feature status in documentation

### Continuous Integration
The framework is designed for CI/CD integration:
- Consistent exit codes for automated success/failure detection
- JSON output format support (future enhancement)
- Configurable timeouts for different environments
- Comprehensive logging and error reporting

## Troubleshooting

### Connection Issues
```bash
# Check device connection
ls -la /dev/ttyUSB*

# Test direct connection
minicom -D /dev/ttyUSB0 -b 115200

# Check permissions
sudo usermod -a -G dialout $USER  # logout/login required
```

### Test Failures
1. **Enable debug mode**: `--debug` for detailed output
2. **Increase timeout**: `--timeout 10.0` for slow operations  
3. **Test interactively**: `--interactive` to manually verify behavior
4. **Check firmware**: Ensure correct firmware version flashed

### Common Issues
- **Device not responding**: Check baud rate, connection, power
- **Timeout errors**: Increase timeout for slow operations
- **Permission denied**: Add user to dialout group
- **Command echo**: Use `expected_contains` instead of `expected_response`

---

**Author**: sg-sdk automated testing framework  
**Date**: October 2025  
**Version**: 1.0