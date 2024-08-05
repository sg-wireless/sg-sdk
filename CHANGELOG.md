<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
  - Fix the micropython virtual timers byt redirecting it to the RTOS timers.

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