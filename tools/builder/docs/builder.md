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
 ! @brief   Documentation file for the main firmware builder script.
 !----------------------------------------------------------------------------->

# SDK Builder

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Introduction](#intro)
* [New Project Organization](#new-prj-org)
* [Build System Command Line Interface( CLI )](#cli)
* [The Final Output Image](#out-image)
* [Firmware Application Project Development](fw_app_prj.md)
* [Configuration Management](config-mgr.md)

---

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->

<div id="intro"></div>

## Introduction

This SDK provides a powerful builder that tackles the firmware building and the
configuration management of the SDK components and underlying dependent
libraries and platforms such as `micropython` and `esp-idf`.

<!------------------------------------------------------------------------------
 ! New Project Organization
 !----------------------------------------------------------------------------->

<div id="new-prj-org"></div>

## New Project Directory Organization

This is the recommended structure to start creating project based on this SDK.

```shell
<project-dir>/
    ├── sg-sdk           # a git submodule for the SDK
    ├── fw-builder.sh    # a new script to call the sg-sdk build-system
    ╰── app-dir          # project source directory
        ├── CMakeLists.txt    # main cmake file of the project
        │..                   # other project contents
        │..
```

This layout structure consists mainly of the following:
* `sg-sdk` which is a git submodule to the SDK.<br>
  ```git submodule add https://github.com/sg-wireless/sg-sdk.git```

* `fw-builder.sh` it is a simple script to call the underlying SDK build system
  and its contents should be like this:
  ```shell
    #!/bin/sh
    python3 sg-sdk/tools/builder/scripts/builder.py $*
  ```
* `app-dir` a directory with a proper application name where the project
  contents will reside.<br>
  The project source directory contents organization description can be found
  [here](new_fw_prj.md)

The user can construct different applications by the following layout structure:

```shell
<project-dir>/
    ├── sg-sdk           # a git submodule for the SDK
    ├── fw-builder.sh    # a new script to call the sg-sdk build-system
    ╰── projects         # applications directory
        ├── app-1-dir              # 1st application project
        │   ├── CMakeLists.txt     # main cmake file of the project
        │   │..                    # other project contents
        │   
        ├── app-2-dir              # 2nd application project
        │   ├── CMakeLists.txt     # main cmake file of the project
        │   │..                    # other project contents
        .   .
        .   .
        ├── app-N-dir              # Nth application project
        │   ├── CMakeLists.txt     # main cmake file of the project
        │   │..                    # other project contents
```

`sg-sdk` and `fw-builder.sh` are same as described before.

The `projects` directory contains as many directories as needed. Each directory
carry a stand alone project(application) sources.

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->

<div id="cli"></div>

## Build System Command Line Interface( CLI )

The build system offers a simple command line interface with colorful display.
After you created your new project directory, you can type:
```fw-builder.sh --help``` to see the help of the build system which is similar
to this screen shot:

![Build System Help Screen](images/builder_help.png)

Then you will make your own build command that matches your project requirements

Examples of build commands:

> Assume you have the F1 Starter Kit and you want to build a `micropython` app,
> then the build and flash commands will be:
> ```shell
> # build command
> ./fw-builder.sh --board SGW3501-F1-StarterKit     \
>                 --project-dir <app-dir>
> # flash command
> ./fw-builder.sh --board SGW3501-F1-StarterKit     \
>                 --project-dir <app-dir>           \
>                 flash --port <board-serial-port>
> ```

> Assume you have the F1 SGW3201(F1-L) based custom board and you want to build
> a `native` C application,
> then the build and flash commands will be:
> ```shell
> # build command
> ./fw-builder.sh --board SGW3201-F1L-OEM           \
>                 --variant native                  \
>                 --project-dir <app-dir>
> # flash command
> ./fw-builder.sh --board SGW3201-F1L-OEM           \
>                 --variant native                  \
>                 --project-dir <app-dir>           \
>                 flash --port <board-serial-port>
> ```

<!------------------------------------------------------------------------------
 ! Output Image (Binaries)
 !----------------------------------------------------------------------------->

<div id="out-image"></div>

## F1 Output Image (Binaries)

All build outputs are compressed in a `.tar.gz` file with a name syntax:
    `<board>-<fw-version>.tar.gz`
and contain the following files:
```shell
<board>-<fw-version>.tar.gz
    ├── bootloader.bin          # the bootloader binary
    ├── partition-table.bin     # the partition table binary
    ├── application.bin         # the main Firmware application binary
    ├── ota_data_initial.bin    # the OTA initial data partition binary
    ╰── flash_args              # the suitable esptool flashing arguments
                                # to be used by the flasher tool
```

The build outputs are placed under the following path based on the build command
variables:
```sg-sdk/build/<app-dir-name>/F1/<board>/<variant>/<file>.tar.gz```

and this is a hierarchy example showing the shape of the build tree with the
build outputs.

```shell
<project-dir>/
    ├── sg-sdk           # a git submodule for the SDK
    │   ╰── build
    │       ├── app-1-dir
    │       │   ├── F1
    │       │   │   ├── SGW3101-F1W-OEM
    │       │   │   │   ├── micropython
    │       │   │   │   │   ├── SGW3131-F1-WS-OEM-v0.0.0.tar.gz
    │       │   │   │   ├── native
    │       │   │   │   │   ├── SGW3131-F1-WS-OEM-v0.0.0.tar.gz
    │       │   │   │   
    │       │   │   ├── SGW3201-F1L-OEM
    │       │   │   │   ├── micropython
    │       │   │   │   │   ├── SGW3201-F1L-OEM-v0.0.0.tar.gz
    │       │   │   │   ├── native
    │       │   │   │   │   ├── SGW3201-F1L-OEM-v0.0.0.tar.gz
    │       .   .   .   .   .
    │       # and so on and so forth for all apps/boards/variants
    │       .   .   .   .   .
    │       ├── app-2-dir
    │       │   ├── F1
    │       │   │   ├── SGW3101-F1W-OEM
    │       │   │   │   ├── micropython
    │       │   │   │   │   ├── SGW3131-F1-WS-OEM-v0.0.0.tar.gz
    │       │   │   │   ├── native
    │       │   │   │   │   ├── SGW3131-F1-WS-OEM-v0.0.0.tar.gz
    │       │   │   │   
    │       │   │   ├── SGW3201-F1L-OEM
    │       │   │   │   ├── micropython
    │       │   │   │   │   ├── SGW3201-F1L-OEM-v0.0.0.tar.gz
    │       │   │   │   ├── native
    │       │   │   │   │   ├── SGW3201-F1L-OEM-v0.0.0.tar.gz
    │       .   .   .   .   .
    │       # and so on and so forth for all apps/boards/variants
    │       .   .   .   .   .
    │       ╰── app-N-dir
    │           ╰── F1
    │               ├── SGW3101-F1W-OEM
    │               │   ├── micropython
    │               │   │   ├── SGW3131-F1-WS-OEM-v0.0.0.tar.gz
    │               │   ├── native
    │               │   │   ├── SGW3131-F1-WS-OEM-v0.0.0.tar.gz
    │               │   
    │               ├── SGW3201-F1L-OEM
    │               │   ├── micropython
    │               │   │   ├── SGW3201-F1L-OEM-v0.0.0.tar.gz
    │               │   ├── native
    │               │   │   ├── SGW3201-F1L-OEM-v0.0.0.tar.gz
    │               .   .   .
    │   
    │   
    │   
    ├── fw-builder.sh    # a new script to call the sg-sdk build-system
    ╰── projects         # applications directory
        ├── app-1-dir                 # 1st application project
        │   ├── CMakeLists.txt     # main cmake file of the project
        │   │..                    # other project contents
        │   
        ├── app-2-dir                 # 2nd application project
        │   ├── CMakeLists.txt     # main cmake file of the project
        │   │..                    # other project contents
        .   .
        .   .
        ├── app-N-dir                 # Nth application project
        │   ├── CMakeLists.txt     # main cmake file of the project
        │   │ .                    # other project contents

```

<!--- end of file ------------------------------------------------------------->
