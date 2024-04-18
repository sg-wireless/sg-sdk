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
 ! @author  Ahmed Sabry (SG Wireless)
 !
 ! @brief   Documentation file for System Information.
 !----------------------------------------------------------------------------->

# System Information

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Introduction](#intro)
* [Board Information](#board-info)
* [version Information](#ver-info)
* [Other Information](#others)

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="intro"></div>

## Introduction

System information collects different information from different system
components and displays it to the user.

<!------------------------------------------------------------------------------
 ! Board Information
 !----------------------------------------------------------------------------->
<div id="board-info"></div>

## Board Information

`sysinfo.board()` This method used to get the board related information.
It returns a dict object with the following keyword fields:

* `full_name` a complete name for the board comrising the OEM module name,
  part number and the shield name if available.
* `platform` a platform name of the used OEM module.
* `module_name` the used OEM module name such as `F1`, `F1-C`, `F1-L`, ...
* `module_number` the corresponding part number of the module such as:
  `SGW3201`, `SGW3501`, ...
* `shield` the used shield for the OEM module. such as `StarterKit`. If the
  shield is not available, it will be `OEM`.

Example

```python
import sysinfo
board_info = sysinfo.board()
print( board_info )
print( f'full name     : {board_info.full_name}' )
print( f'platform      : {board_info.platform}' )
print( f'module name   : {board_info.module_name}' )
print( f'module number : {board_info.module_number}' )
print( f'shield        : {board_info.shield}' )
```
the output will be as similar to the following:
```
(full_name='SGW3201-F1-L-OEM', platform='F1', module_name='F1-L', module_number='SGW3201', shield='OEM')
full name     : SGW3201-F1-L-OEM
platform      : F1
module name   : F1-L
module number : SGW3201
shield        : OEM
```

`sysinfo.show_board()` displays directly the board information on screen as
in the following example:

```
>>> sysinfo.show_board()
================================== board info ==================================
 - board full name            SGW3201-F1-L-OEM
 - platform                   F1
 - board name                 F1-L
 - board number               SGW3201
 - board shield               OEM

 - micropython board name     SGWireless SGW3201-F1-L-OEM
 - micropython MCU name       ESP32S3
 - micropython system name    F1-L
 ```

<!------------------------------------------------------------------------------
 ! Board Information
 !----------------------------------------------------------------------------->
<div id="ver-info"></div>

## Version Information

`sysinfo.version()` This method used to get the version related information.
It returns a dict object with the following keyword fields:

(major=0, minor=1, patch=0, git_delta=1, git_tag='0b532043', build_date='2024.04.18', build_time='07:30', custom='dirty', release='v0.1.0', build='v0.1.0-1-0b532043-20240418-dirty')

* `major` an integer containing the version major number.
* `minor` an integer containing the version minor number.
* `patch` an integer containing the version patch number.
* `git_delta` an integer containing the number of commits between the base
  release git commit and the current build git commit.
* `git_tag` a string representing the build git commit short hash(8 characters).
* `build_date` a string representing the date of this build in format
  (yyyy.mm.dd).
* `build_time` a string representing the time of this build in format (hh:mm).
* `custom` a string containing a user given custom string in the build command
  or it contains `dirty` word if no provided custom version string in the build
  command and the build source has uncommitted changes. Otherwise, it is empty.
* `release` contains the base release version of this build.
* `build` contains the complete build version of this build.


Example

```python
import sysinfo
version_info = sysinfo.version()
print( version_info )
print( f'base release  : {version_info.release}' )
print( f'build version : {version_info.build}' )
print( f'date and time : {version_info.build_date} - {version_info.build_time}')
```
the output will be as similar to the following:
```
(major=0, minor=1, patch=0, git_delta=1, git_tag='0b532043', build_date='2024.04.18', build_time='07:30', custom='dirty', release='v0.1.0', build='v0.1.0-1-0b532043-20240418-dirty')
base release  : v0.1.0
build version : v0.1.0-1-0b532043-20240418-dirty
date and time : 2024.04.18 - 07:30
```

`sysinfo.show_version()` displays directly the version information on screen as
in the following example:

```
>>> sysinfo.show_version()
=============================== firmware version ===============================
 - firmware version           v0.1.0-1-0b532043-20240418-bug#123-test-fix
 - firmware base release      v0.1.0
 - custom version string      bug#123-test-fix

 - build date and time        2024.04.18 - 08:44

 - git hash long              0b532043c19433554dbb65b33878fb8ba6643516
 - git hash short             0b532043
 - git delta                  1

 - micropython build          v1.19.1-796-gf4811b0b4
 - micropython build date     2024-04-18
================================================================================
```

<!------------------------------------------------------------------------------
 ! Other Information
 !----------------------------------------------------------------------------->
<div id="others"></div>

## Other Information

`sysinfo.show_efuses()` displays the eFuses information of the board. as in the
following example
```
>>> sysinfo.show_efuses()
============================= efuses for user data =============================
 - Layout Version             00
 - LoRa MAC                   70 b3 d5 49 92 a6 92 0f
 - Serial Number              00 00 00 00 00 00
 - HW ID                      01 00 00
 - Project ID                 00 00 00
 - WiFi MAC                   7c 51 89 02 03 40
================================================================================
```

`sysinfo.show_flash()` displays the flash storage information and the current
 deployed partition table as in the following example:

```
>>> sysinfo.show_flash()
================================= flash stats ==================================
 - flash frequency            40 MHz
 - flash size                 16777216  Bytes ~= 16 MB
 - partition table:
    label     type  subtype     enc   start      end      size_b  size_kb size_mb 
    nvs       data  nvs         no  00011000  0001bfff     45056     44.0   0.0   
    otadata   data  factory     no  0001c000  0001dfff      8192      8.0   0.0   
    config    data  data-undef  no  0001e000  0001efff      4096      4.0   0.0   
    rfu1      data  data-undef  no  0001f000  0001ffff      4096      4.0   0.0   
    factory   app   factory     no  00020000  0029ffff   2621440   2560.0   2.5   
    ota_0     app   ota         no  002a0000  0051ffff   2621440   2560.0   2.5   
    ota_1     app   ota         no  00520000  0079ffff   2621440   2560.0   2.5   
    rfu2      data  data-undef  no  007a0000  007a017f       384      0.4   0.0   
    vfs       data  data-undef  no  00800000  00ffffff   8388608   8192.0   8.0   
================================================================================
```

`sysinfo.show_spiram()` displays the spiram related information as in the
following example:

```
>>> sysinfo.show_spiram()
================================= spiram stats =================================
 - ram size                   8388608 Bytes ~= 8 MB
================================================================================
```

`sysinfo.show_all()` displays all above system information together.

<!--- end of file ------------------------------------------------------------->
