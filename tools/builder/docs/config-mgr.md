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
 ! @brief   Documentation file for the SDK configuration management.
 !----------------------------------------------------------------------------->

# Firmware Configuration Management

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents


* [Introduction](#intro)
* [Board `.toml` config file](#board-toml)
* [Components Kconfig](#comp-kconfig)

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="intro"></div>

## Introduction

The configuration is categorized as follows:

* `Static` A configuration that can not be changed in run-time and is managed by
    the build system. They are set by configuration files such as:
        * `.toml` files which carries the basic default system configuration.
        * `menu-config` files which creates such configurations and identifies
          its default values and types. It is the same as linux kernel config
          files and follows the basic syntax of kconfig language.
        * `sdkconfig defaults` which are default kconfig configuation selection
          if it is different than the default value of config specification
          in the `menu-config` files.

* `Dynamic` A configuration that can be changed in the run-time.
    The dynamic configurations is tackled by the firmware itself and it is
    stored on the non-volatile memory and it is out of build system scope.

<!------------------------------------------------------------------------------
 ! .toml board configuration
 !----------------------------------------------------------------------------->
<div id="board-toml"></div>

## Board `.toml` config file:

Each board has a `.toml` board config file which carries the basic board
configurations Such as:
* Board identifications (part number, model-name, shield)
* Board Supported features (lora, lte, ...)
* Submodules (esp-idf, micropython, ...)
* Tools (esp-idf, ...)
* Configurations (sdkconfigs, partition-table, ...)

The hierarchical distribution of the `.toml` files is as follows:
```shell
<project-dir>/
    ├── sg-sdk           # a git submodule for the SDK
    │   ├── src
    │   │   ├── platforms
    │   │   │   ├── F1
    │   │   │   │   ├── platform_config.toml     # sdk platform config
    │   │   │   │   ├── boards
    │   │   │   │   │   ├── SGW3101-F1W-OEM.toml # sdk board config
    │   │   │   │   │   ├── SGW3131-F1WS-OEM.toml
    │   │   │   │   │   ├── SGW3201-F1L-OEM.toml
    │   │   │   │   │   ├── SGW3501-F1-StarterKit.toml
    │   .   .   .   .   .   ...
    │
    │
    ├── fw-builder.sh
    ╰── app-dir         # user application project directory
        ├── CMakeLists.txt
        ├── board_config.toml   # user board configuration
        │..
        │..
```

We have three different priority levels of `.toml` configuration:
1. User Board config file       -> Highest priority
2. SDK board config file
3. Platform board config file   -> Lowest priority

The config in the highest priority level override the config in the lowest
priority level.

The formatting of the `.toml` file is as follows:
```toml
# the board identification
# it is overridable by higher level .toml file
[id]
    board_name      = "F1-S"
    board_number    = "SGW3531"
    shield          = "OEM"

# the supported features by hardware
# it is overridable by higher level .toml file
[features]
    # <feature> = [true|false]
    lora = true
    lte  = true
    secure-element = false

# the needed submodules to be present before building
# it is extendable by higher level .toml file
[submodules]
    # list of prerequisite submodules irrespective of any other variant
    default = [ "esp-idf" ]
    [submodules.variant]
        # list of prerequisite submodules for micropython based building 
        micropython = [  ]
        # list of prerequisite submodules for native based building 
        native = [  ]

    # submodules specific to the existence of specific feature
    [submodules.features]
        # if lora feature is enabled the submodule here will be
        # prepared prior building
        lora = [ 'LoRaMac-node' ]

# list of needed tools to be installed before building
# it is extendable by higher level .toml file, but needs support
# by the build system
[tools]
    default = [ 'esp-idf' ]

# list of default configuration references of the board
[configs]

    # it is overridable by higher level .toml file
    partition-table = [
        # an array of arrays of string carrying the specifications of the
        # esp-idf partition-table
        # Name,     Type,   SubType,    Offset,     Size
        ["nvs"    , "data"  , "nvs",    "0x11000",    "0xB000", ""],
    ]

    # it is extendable by higher level .toml file
    sdkconfig-files = [
        # list of files containing a default sdkconfigs for the 
        # menu-config the files path shall be relative to the
        # base platform path
        "configs/file_1.config",
        "configs/file_2.config"
    ]

    # a list of sdkconfigs in key/value pairs
    # it is overridable by higher level .toml file
    [configs.sdkconfig]
        CONFIG_FOO_ENABLE="y",
        CONFIG_BAR_ENABLE="n"

    # a list of sdkconfigs for micropython build variable only
    # it is overridable by higher level .toml file
    [configs.micropython.sdkconfig]

    # a list of sdkconfigs for native build variable only
    # it is overridable by higher level .toml file
    [configs.native.sdkconfig]

```

<!------------------------------------------------------------------------------
 ! kconfig configurations
 !----------------------------------------------------------------------------->
<div id="comp-kconfig"></div>

## Components Kconfig (`menu-config`) files:

* It is a list of files written in kconfig language syntax. Each file specifies
  a set of configurations and their types and help. It is also called
  `menu-config` files.

* Each component is allowed to add one or more `menu-config` files.

* Each `menu-config` file can be placed at a specifiable menu hierarchy using
  a group-id concept which will be explained later.

* Each component can add an sdkconfigs default files to act as a default config
  selection for any `menu-config` config.

* The component can add menu config specifications using:
    * `__sdk_add_component()`: within the parameters of this function there
      exist an optional parameters to specify a menu config. Using which,
      only one menu config can be specified.
    * `__sdk_menu_config_add_component_menu()`: through this function, more
      than one menu config specifications can be added for a single
      component by calling this function multiple times.

* hierarchy and grouping:
  The component can specify a special hierarchy for its appearance by
  the following syntax: `<gid1>.<gid2>. -- .<gidN>`, where gid is Group-ID
  The groups shall be added first by `__sdk_menu_config_group_add()`
  The following groups are defined by default by the build system.
    * `MAIN`      the main menu config screen
    * `MAIN.SDK`  the default group for any added SDK component
    * `MAIN.SDK.CLIBS`  the default group for common C libraries
    * `MAIN.SDK.NETWORK`  the default group for netowrking libraries
    * `MAIN.USR`  the default group for any added user project component
    * `MAIN.DEMO` a menu group for all demonstration examples.
  Example:
    ```cmake
    __sdk_menu_config_add_component_menu( component_abc
        MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/cfg/basic.kconfig
        MENU_PROMPT     "Component ABC Basics"
        MENU_GROUP      MAIN.SDK
        )
    ```
    The output hierarchy will be like this:<br>
        `<MAIN-SCREEN> -> "SDK Components" -> "Component ABC Basics"`

* A group can be added using `__sdk_menu_config_group_add()`. It takes
  the following parameters:
   * `GROUP_ID`      an identifier to the group.
   * `GROUP_PROMPT`  a prompt of this group in the menu hierarchy.

* Adding menu config shall be specified by the following parameters
  whether in both of functions `__sdk_menu_config_add_component_menu()`
  and `__sdk_add_component()`.
    * `MENU_CONFIG`   The actual file containing the kconfig syntax.
    * `MENU_PROMPT`   (optional) The appearance name of this menu config.
                    If not specified, the component name will be used.
    * `MENU_GROUP`    (optional) The hierarchical appearance of this menu
                    config If not specified, the default group will be
                    `SDK.<comp_name>`

<!--- end of file ------------------------------------------------------------->
