<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
 !
 ! Permission is hereby granted, free of charge, to any person obtaining a copy
 ! of this software and associated documentation files(the “Software”), to deal
 ! in the Software without restriction, including without limitation the rights
 ! to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 ! copies  of  the  Software,  and  to  permit  persons to whom the Software is
 ! furnished to do so, subject to the following conditions:
 !
 ! The above copyright notice and this permission notice shall be included in
 ! all copies or substantial portions of the Software.
 !
 ! THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 ! IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 ! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 ! AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 ! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ! OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 ! THE SOFTWARE.
 !
 ! @brief   Change log of the SDK project
 !----------------------------------------------------------------------------->


<!------------------------------------------------------------------------------
 ! Header
 !----------------------------------------------------------------------------->

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

<!------------------------------------------------------------------------------
 ! v1.4.0
 !----------------------------------------------------------------------------->
## [1.4.0] - 2026-05-08

### Platform & Build System

- Update to MicroPython v1.26.1 and ESP-IDF v5.4
- Switch CI workflow to GitHub Actions
- Rename component from f1_legacy_fuota to f1_fuota
- Remove cbor2-lib (now picked up from micropython-libs)
- Update builder scripts to support new esp-idf install and release deployment
- Refactor clean command to directly remove build directory
- Enable deflate compression support
- Add backward compatibility for legacy RTC state handling in bootloader
- Add partition erase and reset functions for NVS and OTA data
- Update NVS interface to include usage statistics and documentation

### LTE

- New unified LTE module (modlte) replacing lte-poc
- ESP modem v2.0.0 middle layer for Sequans Monarch 2 GM02S (GM02SP4)
- Fix espmodem: setup PDP context consistently for GM02S
- Add PPP bridge implementation
- Add UART resource guard component
- Use LTE event handler in CTRL client; lock handler to avoid accidental redefinition
- Set keepalive and send ping message every 5 minutes to avoid disconnects
- Remove support for old LTE driver option
- Print appropriate error message when SIM card is locked or missing
- Add new LTE events documentation

### CTRL Client

- Update CTRL client and documentation
- Send both MQTT ping and CTRL ping message for keep_alive
- Update CTRL API documentation
- Print a warning when ctrl.send_signal() is used (no longer supported)
- Fix: send fw OTA messages only when status changed
- Support ctrl.activate() from safeboot
- Add backward-compatible stubs and NVS checks for CTRL client boot handling
- Fix remove sensor error when no sensor is provisioned
- Fix ctrl+c not working in _inisetup.py

### FUOTA

- Include ESP_IDF error name when available if a custom message has been set
- Use smaller CONFIG_MBEDTLS_CERTIFICATE_BUNDLE_DEFAULT_CMN
- Remove redundant blocking parameter from fuota.start_upgrade()
- Enhance FUOTA with improved resume capabilities and configurable options
  (HTTP Range requests, exponential backoff, artifact identity checks)
- Add chunk-cap handling to OTA task for improved download reliability

### LoRa / LoRaWAN

- Fix LoRaWAN issues
- Add LoRaWAN patches for LoRaMac, LoRaMacClassB, RegionUS915, and radio
- Update CA certificate for server communication

### SSL / Networking

- Fix SSL issues
- Add bundle-networking support via micropython-libs

### Security

- Add security component
- Mark NVS keys partition as encrypted in platform configuration

### Logging

- Implement dynamic logging subsystem and component registration
- Add silent option when updating log filters

### New Components

- Add fw-version component
- Add mp_i2c_bridge component
- Add ws2812_rmt driver component

### Platform Features

- Full USB device support with TinyUSB integration
- Ethernet LAN support
- Virtual timer support using FreeRTOS timers

### Tests

- Add MicroPython REPL automated test framework
- Add test cases for basic functionality, Ethernet LAN, I2C bridge, IO expander
  debug, and USB device

### Documentation

- Full documentation review
- Update build system documentation for platform component registration and feature management
- Add comprehensive component registration guide (BUILD_SYSTEM.md)
- Add build commands reference (COMMANDS.md)
- Add logging system integration guide (LOGGING.md)
- Add I2C bridge practical examples (I2C_Bridge_Examples.md)
- Reorganized documentation into docs/ directory

### Copyright

- Update copyright years to 2023-2026 across all SG Wireless source files
- Add missing copyright notices to all new source files


<!------------------------------------------------------------------------------
 ! v1.3.0
 !----------------------------------------------------------------------------->
## [1.3.0] - 2025-05-23
### Added

- CBOR2 library to support Control Platform Ctrl-1.5

### Changes and Fixes

- SG Wireless Control Platform Client
  - Migrate to support Control Platform Ctrl-1.5
  - Add API to support new provisioning routine  

<!------------------------------------------------------------------------------
 ! v1.2.0
 !----------------------------------------------------------------------------->
## [1.2.0] - 2025-04-26
### Added

- New support for CAN bus

### Changes and Fixes

- SG Wireless Control Platform Client
  - Sensor reading related fixes
  - NTP sync upon internet connection establishment

<!------------------------------------------------------------------------------
 ! v1.1.0
 !----------------------------------------------------------------------------->
## [1.1.0] - 2024-11-05
### Added

- New support for LoRa OTAA security keys existence on the ESP32 eFuses.
- New matured ESP32 NVS interface component to easily inspect and change the
  underlying NVS partitions.
- New F1 boards configurations support `SGW3201-F1L-StarterKit` and
  `SGW3401-F1C-StarterKit`

### Changes and Fixes

- SG Wireless Control Platform Client Feature supported:
  - LoRa support in control-client and adding LoRa network over-the-air
    configuration.
  - Support of ctrl-client automatic activation (ZTP "Zero-Touch-Provisioning")
    over LoRa network.

- LoRa Changes and Fixes
  - Unify lora callback interface to receive a single context object as tuple
    carrying all related information to the incoming lora event.
  - New changes to fine tune and control accurately the RX windows timing.
  - LoRa RX/TX messages parameters enhancements and fixes.
  - LoRa docs and examples updates.

- Micropython related
  - Enforce micropython hooked virtual timers de-init at soft reset

- Others
  - Enforce system panics by default in all log assertions.

<!------------------------------------------------------------------------------
 ! v1.0.0
 !----------------------------------------------------------------------------->
## [1.0.0] - 2024-08-05
### Added
- SG Wireless Control Platform Client Feature supported:
  - FUOTA Feature support
  - OTA Network Pereferneces Support

### Changes and Fixes

- WiFi Speed Enhancement
  - Tuning the WiFi Driver and Memory configuration to improve the Wifi
    performance and speed.

- Micropython
  - Fix the micropython virtual timers by redirecting it to the RTOS timers.

- Fuel-Guage
  - Support of battery parmeters (designCapacity, minSysVoltage, taperCurrent)
    in the API.

- LoRa Changes and Fixes
  - LoRa-WAN LCTT mode support
  - Support RSSI and SNR info feeds with RX event data
  - Fixed LoRa-WAN payload size pre-check
  - Fixed the sync-obj racing between wait and signal requests

- LTE Changes and Fixes
  - Use time instead of utime as umodules will be deprecated in future uPython
  - Fix hang issue when switching PPP from active(False) to active(True)
  - Fix inconsistent use of ppp variables
  - Remove ATO command during connect as investigation of its use is ongoing
  - Add IMEI and ICCID functions for convenience
  - update LTE API documentation.
  - other LTE.py fixes.

- Tools
  - Fix flashing over USB-C from MacOS by inhertiting the ESP Fix from this PR:
    espressif/esptool/pull/718

<!------------------------------------------------------------------------------
 ! v0.5.0
 !----------------------------------------------------------------------------->
## [0.5.0] - 2024-04-29
### Added
- Fuel-Gauge Support
- Import Control Platform Client "ctrl-client"
- Adding an example to transfer a file over lora
- Import legacy FUOTA component

## Changes
- Update boards filenames
- Updating the micropython main.py and boot.py to by created if they are not
  present
- firmware versioning updates and system information to reflect the version
  for scripting purposes
- LTE.py Changes
  - Function is_connected returns ppp stack connection state
  - in_ppp flag is cleared during ppp_suspend
  - Fix mode selection when called from ctrl_client
  - Formatting updates
  - Check if power is enabled during init
  - More graceful power_off
- LoRa changes and fixes
  - disable lora-raw async message until buffer copying is supported
  - enable payload change in lora-raw
  - Fix Region US915 channels frequencies verification
  - Fix lora sync objects access mutex initialisation

<!------------------------------------------------------------------------------
 ! v0.1.0
 !----------------------------------------------------------------------------->
## [0.1.0] - 2024-04-09
### Added
- Adding This Changelog tracking file.
- Adding StarterKit RGB-LED Firmware component with its driver submodule.
- Adding new power management enhancement patch for ESP-IDF peripheral ctrl
- Importing F1 Pins definitions patch
- Importing LTE POC implementation in both C and Python languages
- Importing The test engine, its documentations, examples and CI test-suites of
  micropython, LoRa, and SafeBoot.
- Importing LoRa Stack and its F1 interfacing component.
- Adding F1 eFuse interface component
- Importing F1 system inspection component and adding its documentation and
  menu-config.
- Adding Safeboot Feature Menu config and Documentations.
- Importing Safeboot Feature and its required micropython hooks.
- Importing F1 IO Expander Interfacing Component and its required micropython
  hooks
- Importing IO Expander driver PCAL6408A
- Importing the common State Machine Library
- Importing the common Abstract Data Types (ADT library)
- Adding Firmware versioning component and its generation tool
- Build System Updates
    - Support micropython modules and frozen contents updates dependencies to
      sdkconfig and c-modules generated files
    - Support adding components recursively
    - Support Build system Menu Config structuring
    - Adding Build system Documentation, Examples
- Importing the common C logging library
- Importing micropython binding abstractionlayer for C-modules development
- Importing the common C utils library from internal development
### Fixed
- Fix Build System to include patched micropython files into QSTR generation.

<!------------------------------------------------------------------------------
 ! v0.0.0
 !----------------------------------------------------------------------------->
## [0.0.0] - 2024-03-27
### Added
- SDK Basic Build System for both micropython and native C building variants.
- F1 Platform Infrastructure and its boards basic config files
- Helpful common python scripts (pylog, pycli, pytext)


<!--- end of file ------------------------------------------------------------->