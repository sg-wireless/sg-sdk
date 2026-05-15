<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 !
 ! @brief   Documentation file for eFuse Interface.
 !----------------------------------------------------------------------------->

# eFuse Interface

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Introduction](#intro)
* [eFuse Read Functions](#read-funcs)
* [eFuse Layout](#layout)
* [Example](#example)

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="intro"></div>

## Introduction

The eFuse interface provides read access to the ESP32-S3 one-time programmable
(OTP) fuse memory. eFuses store factory-provisioned board identity data such as
the LoRa-WAN DevEUI, WiFi MAC address, serial number, and hardware identifiers.

The values stored in eFuses are written once during manufacturing and cannot be
changed afterwards. This module allows micropython scripts to retrieve these
values at runtime.

An optional internal RAM cache can be enabled
(`SDK_BOARD_EFUSE_INTERNAL_CACHE_ENABLE`) so that the eFuse block is read from
hardware only once at startup, with subsequent reads served from memory.

<!------------------------------------------------------------------------------
 ! eFuse Read Functions
 !----------------------------------------------------------------------------->
<div id="read-funcs"></div>

## eFuse Read Functions

All read functions take no arguments and return a `bytes` object containing the
raw value read from the eFuse block. The module is imported as `efuse_if`.

* `efuse_if.layout_version()` returns the eFuse user-data block layout version
  (1 byte).

* `efuse_if.lora_mac()` returns the LoRa-WAN DevEUI (8 bytes).

* `efuse_if.serial_number()` returns the board serial number (6 bytes).

* `efuse_if.hw_id()` returns the manufacturer hardware ID (3 bytes).

* `efuse_if.project_id()` returns the project-specific ID (3 bytes).

* `efuse_if.wifi_mac()` returns the WiFi MAC address (6 bytes).

### Optional LoRa Key Functions

The following functions are only available when LoRa key storage on eFuses is
enabled (`SDK_BOARD_LORA_WAN_KEYS_ON_EFUSES`):

* `efuse_if.lora_app_key()` returns the LoRa-WAN OTAA AppKey (16 bytes).

* `efuse_if.lora_nwk_key()` returns the LoRa-WAN OTAA NwkKey (16 bytes).

### Test Function (Virtual eFuse Mode Only)

When building with `CONFIG_EFUSE_VIRTUAL` enabled for development or testing:

* `efuse_if.write_test_lora_keys(AppKey=<bytes>, NwkKey=<bytes>)` writes test
  LoRa keys to the virtual eFuse block. Both keyword arguments must be provided
  as 16-byte buffers.

<!------------------------------------------------------------------------------
 ! eFuse Layout
 !----------------------------------------------------------------------------->
<div id="layout"></div>

## eFuse Layout

The board identity data is stored in **EFUSE_BLK3** (User Data) on the
ESP32-S3. The current layout (version 0) is:

| Field           | Size     | Description              |
|-----------------|----------|--------------------------|
| LPWAN_MAC       | 8 bytes  | LoRa-WAN DevEUI          |
| SERIAL_NUMBER   | 6 bytes  | Board serial number      |
| HW_ID           | 3 bytes  | Hardware ID              |
| PROJECT_ID      | 3 bytes  | Project-specific ID      |
| Reserved        | 4 bytes  | Reserved for future use  |
| LAYOUT_VERSION  | 1 byte   | Layout version number    |
| WIFI_MAC        | 6 bytes  | WiFi MAC address         |
| CRC8            | 1 byte   | CRC8 checksum            |

When LoRa key storage is enabled, the AppKey and NwkKey (16 bytes each) are
stored in a separate eFuse KEY block (configurable, default KEY5).

<!------------------------------------------------------------------------------
 ! Example
 !----------------------------------------------------------------------------->
<div id="example"></div>

## Example

```python
import efuse_if

# read the LoRa-WAN DevEUI
deveui = efuse_if.lora_mac()
print("DevEUI:", deveui.hex())

# read the WiFi MAC address
wifi = efuse_if.wifi_mac()
print("WiFi MAC:", wifi.hex())

# read the board serial number
sn = efuse_if.serial_number()
print("Serial:", sn.hex())

# read hardware and project identifiers
hw = efuse_if.hw_id()
proj = efuse_if.project_id()
print("HW ID:", hw.hex())
print("Project ID:", proj.hex())

# read the layout version
ver = efuse_if.layout_version()
print("Layout version:", int.from_bytes(ver, 'little'))
```

Example output:
```
DevEUI: 70b3d54992a6920f
WiFi MAC: 7c5189020340
Serial: 000000000000
HW ID: 010000
Project ID: 000000
Layout version: 0
```

<!--- end of file ------------------------------------------------------------->
