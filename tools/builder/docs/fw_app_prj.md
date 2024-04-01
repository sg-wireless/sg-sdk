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
 ! @brief   Documentation file for the building a Firmware application project.
 !----------------------------------------------------------------------------->

# Firmwar Application Project Development

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Project Preparation](#prj-prep)
* [Default Variables](#default-vars)
* [Single Firmware Application Component](#single-sw-comp)
* [Multiple Firmware Application Components](#multiple-sw-comp)
* [Component CMakeLists.txt Contents](#comp-cmake-file)

---

<!------------------------------------------------------------------------------
 ! Preparing the Project
 !----------------------------------------------------------------------------->

<div id="prj-prep"></div>

## Project Preparation

* Create a new project directory anywhere. The project directory will be
  referred as **`<user-project-dir>`**

* Create the main cmake list file in this location
  **`<user-project-dir>/CMakeLists.txt`** and we will be referring to the main
  cmake list file by **`<user-prj-main-cmake-file>`**

* There are two paradigms for the project, single component or multiple
  components. Please refer to the respective section for each paradigm.

---

<!------------------------------------------------------------------------------
 ! Default Variables
 !----------------------------------------------------------------------------->

<div id="default-vars"></div>

## Default Variables

There are some default variables that can be used to determine the paths in
the components cmake list files.

The following table describes those paths:

|variable|description|
|:-------|:----------|
|`__dir_root`|contains the path to the main SDK cloned repository directory|
|`__dir_esp_idf`|contains the path to esp-idf source directory|
|`__dir_micropython`|contains the path to micropython source directory|
|`__dir_platform`|contains the path to SDK selected board platform directory|
|`__dir_tools`|contains the path to SDK tools directory|
|`__dir_sdk_comps`|contains the path to SDK components directory|
|`__dir_platform_comps`|contains the path to SDK selected board platform components directory|
|`__dir_drivers`|contains the path to SDK drivers directory|

---

<!------------------------------------------------------------------------------
 ! Single Firmware Component
 !----------------------------------------------------------------------------->

<div id="single-sw-comp"></div>

## Single Firmware Application Component

In this project paradigm, the project folder will be considered as a single
component. All what you need is to fill the **`<user-prj-main-cmake-file>`**
with the needed descriptors as will be described in the following
**[Component CMakeLists.txt Contents](#comp-cmake-file)** description section.

```cmake
Example:

# ---------------------------------------------------------------------------- #
# assume the following project directory hierarchy:
# ---------------------------------------------------------------------------- #
<user-project-dir>/
    CMakeLists.txt              # component cmake list file
    src/                        # project source files directory
        file_a.c
        file_b.c
        file_mpy_c_module.c     # contain micropython ecosystem related stuff
    inc/                        # project include directory
        head_1.h
        head_2.h
    mpy/                        # micropython files
        manifest.py
        module_a.py
        module_b.py
        module_c.py
    patches/                    # patches directory
        sdk_comp_x/
            sdk_comp_x_file.c.patch
        esp_idf_comp_x/
            esp_idf_comp_x_file.c.patch
    cfg/                        # project configurations
        kconfig                 # project menu config specificatrion
        kconfig.default         # default global SDK config

# ---------------------------------------------------------------------------- #
# contents of the <user-project-dir>/CMakeLists.txt will be:
# ---------------------------------------------------------------------------- #

    # to specify the contents of the application source files
    __sdk_add_component( project_x_main

        # all source files
        SRCS        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"

        # private include (visible only at the component level)
        INCS_PRIV   ${CMAKE_CURRENT_LIST_DIR}/inc

        # micropython C-modules
        MPY_MODS    ${CMAKE_CURRENT_LIST_DIR}/src/file_mpy_c_module.c

        # dependent SDK libraries
        REQUIRED_SDK_LIBS
            log_lib     utils_lib   logs_if

        # dependent ESP-IDF libraries
        REQUIRED_ESP_LIBS
            freertos    spi_flash

        # menu config file (following kconfig language syntax)
        MENU_CONFIG  ${CMAKE_CURRENT_LIST_DIR}/cfg/kconfig

        # appearance name in the menu config screen
        MENU_PROMPT  "project X configuration"
    )

    # To add a developped stuff in micropython to become a frozen code
    # within the final Firmware image
    __sdk_add_micropython_frozen_manifest(
        ${CMAKE_CURRENT_LIST_DIR}/mpy/manifest.py )

    # to add a default sdkconfig. any config here overwrites the default
    # of the SDK config if any
    __sdk_add_kconfig_default(
        ${CMAKE_CURRENT_LIST_DIR}/cfg/kconfig.default )

    # to patch any file whether in the SDK or in the ESP-IDF
    set(__patches_dir ${CMAKE_CURRENT_LIST_DIR}/patches)
    __sdk_add_patch(
        # the component name which owns the file to be patched
        ENTITY_NAME    esp_idf_comp_x

        # the file to be patched path
        ORIGINAL_FILE  ${__dir_esp_idf}/path/to/esp_idf_comp_x.c

        # the file containing the patch
        PATCH_FILE
            ${__patches_dir}/esp_idf_comp_x/esp_idf_comp_x.c.patch

        # the path where the final file after patching shall go.
        FINAL_DIR      ${__patches_dir}/modified_files
    )

    __sdk_add_patch(
        ENTITY_NAME    sdk_comp_x
        ORIGINAL_FILE  ${__dir_sdk_comps}/path/to/sdk_comp_x_file.c
        PATCH_FILE     ${__patches_dir}/sdk_comp_x/sdk_comp_x_file.c.patch
        FINAL_DIR      ${__patches_dir}/modified_files
    )
```

---

<!------------------------------------------------------------------------------
 ! Multiple Software Components
 !----------------------------------------------------------------------------->

<div id="multiple-sw-comp"></div>

## Multiple Firmware Application Components

In this project paradigm, the project is splitted into different components.
The **`<user-prj-main-cmake-file>`** shall contain an include cmake statement
for each components CMaksLists.txt. And each component CMakeLists.txt shall
specify the component descriptor as explained in
**[Component CMakeLists.txt Contents](#comp-cmake-file)** description section.

```cmake
Example:

# ---------------------------------------------------------------------------- #
# assume the following project directory hierarchy:
# ---------------------------------------------------------------------------- #
<user-project-dir>/
    CMakeLists.txt
    project_components/
        comp_a/
            CMakeLists.txt          # component cmake list file
            kconfig                 # optional component level configuration
            src/                    # component source files directory
                comp_a_file_1.c
                comp_a_file_2.c
            inc/                    # component interface headers directory 
                comp_a_interface.h
        comp_b/
            CMakeLists.txt
            kconfig
            src/
                comp_b_file_1.c
                comp_b_file_1.h
                comp_b_file_2.c
            inc/
                comp_b_interface.h
        comp_c/
            CMakeLists.txt
            comp_c_file_1.c
            comp_c_file_1.h
    mpy/                            # micropython source files
        manifest.py
        module_a.py
        module_b.py
        module_c.py
    patches/                        # patches directory
        sdk_comp_x/
            sdk_comp_x_file.patch
        esp_idf_comp_x/
            esp_idf_comp_x_file.patch
    cfg/
        kconfig                     # global project menu config description
        kconfig.default             # default configurations for the overall SDK

# ---------------------------------------------------------------------------- #
# contents of the <user-project-dir>/CMakeLists.txt will be:
# ---------------------------------------------------------------------------- #

    # include all contributing component CMakeLists.txt files
    set(__user_proj_comps_dir ${CMAKE_CURRENT_LIST_DIR}/project_components)
    include(${__user_proj_comps_dir}/comp_a/CMakeLists.txt)
    include(${__user_proj_comps_dir}/comp_b/CMakeLists.txt)
    include(${__user_proj_comps_dir}/comp_c/CMakeLists.txt)

    __sdk_add_micropython_frozen_manifest(
        ${CMAKE_CURRENT_LIST_DIR}/mpy/manifest.py )

    __sdk_add_kconfig_default( ${CMAKE_CURRENT_LIST_DIR}/cfg/kconfig.default )

    set(__patches_dir ${CMAKE_CURRENT_LIST_DIR}/patches) # to shorten paths
    __sdk_add_patch(
        ENTITY_NAME    esp_idf_comp_x
        ORIGINAL_FILE  ${__dir_esp_idf}/path/to/esp_idf_comp_x_file.c
        PATCH_FILE     ${__patches_dir}/esp_idf_comp_x/esp_idf_comp_x_file.patch
        FINAL_DIR      ${__patches_dir}/modified_files
    )

    __sdk_add_patch(
        ENTITY_NAME    sdk_comp_x
        ORIGINAL_FILE  ${__dir_sdk_comps}/path/to/sdk_comp_x_file.c
        PATCH_FILE     ${__patches_dir}/sdk_comp_x/sdk_comp_x_file.patch
        FINAL_DIR      ${__patches_dir}/modified_files
    )

# ---------------------------------------------------------------------------- #
# contents of the ${__user_proj_comps_dir}/comp_a/CMakeLists.txt will be:
# ---------------------------------------------------------------------------- #
    __sdk_add_component( user_comp_a
        SRCS        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
        INCS_IF     ${CMAKE_CURRENT_LIST_DIR}/inc
        REQUIRED_SDK_LIBS
            log_lib     utils_lib   logs_if
        MENU_CONFIG
            ${CMAKE_CURRENT_LIST_DIR}/kconfig
    )

```

<!------------------------------------------------------------------------------
 ! Component CMakeLists.txt Contents
 !----------------------------------------------------------------------------->

<div id="comp-cmake-file"></div>

## Component ```CMakeLists.txt``` Contents

The typical component cmake list file contains the following descriptors:

|descriptor|required|description|
|:---------|:------:|:----------|
|```__sdk_add_component()```|mandatory|sets the component specs|
|```__sdk_add_micropython_frozen_manifest()```|optional|specify micropython frozen manifests|
|```__sdk_add_compile_options()```|optional|add global compile flags|
|```__sdk_add_kconfig_default()```|optional|add default SDK configuration settings|
|```__sdk_add_patch()```|optional|specify a particular component patching|

The following CMake snippet describes the usage of all descriptors:

```cmake
# -- adding the component to the SDK builder
__sdk_add_component(

    <component-name>    # a name of the component and it is mandatory

    # any option below is optional

    SRCS
        # list here the list of source C files that are needed.
        # uncomment the following line if you want all c files to be
        # compiled
        # "${CMAKE_CURRENT_LIST_DIR}/*.c"

    INCS_PRIV
        # list here the list of needed include directories that are needed
        # only while compiling the sources of this particular component

    INCS_IF
        # list of an interface include directories of this component
        # this include directories will be automatically available to
        # any other SDK component while its building without even
        # specifying it in its private include list.

    DEFS
        # list of private definitions for this particular component

    FLAGS
        # list of private compilation flags for this particular component

    LOGS_DEFS
        # list of files that have a log subsystems and log components
        # definitions

    MPY_MODS
        # list of files that contain micropython c-modules developed using
        # the sdk micropython lite interface

    REQUIRED_SDK_LIBS
        # list of SDK dependent components
    
    REQUIRED_ESP_LIBS
        # list of ESP-IDF dependent components
    
    MENU_CONFIG
        # a single kconfig file
    MENU_PROMPT
        # a display name in the menu config screen
    MENU_GROUP
        # a hierarchical tree placement to change the default location
        # of appearance of this menu config in the menu-config screen
        # a detailed description will be explained in the section of the
        # menu config organization.
)

# -- [micropython specific] adding micropython frozen code
__sdk_add_micropython_frozen_manifest(
    # add the list of micropython manifest files to be added as
    # a frozen code
)

# -- adding a global project compile options. will be applied to all
#    project component for SDK components or 3rd party components
__sdk_add_compile_options(
    # list of compile flags as a string
    # example "-DSOME_FLAG=10"
    # if SDK_LIBS option flag is specified, it will affect only SDK libs
    # if USR_LIBS option flag is specified, it will affect only USER
    #       project components
    # example:  __sdk_add_compile_options("-DSOME_FLAG=10" USR_LIBS)
    # example:  __sdk_add_compile_options("-DSOME_FLAG=10" SDK_LIBS)
    # example:  __sdk_add_compile_options("-DSOME_FLAG=10")
)

# -- adding a default kconfig options
__sdk_add_kconfig_default(
    # add the list of files that contains the default configuration
    # newer configuration default overwrites the earliest one
)

# -- adding patch for an existing component whether SDK or 3rd party
# -- to add patch you can specify a single patch file or a dirctory
#    containing some patches

# -- specifying single patch file
__sdk_add_patch(
    # a name of the component to be patched
    ENTITY_NAME     <patched-component>

    # the name of the original file along with its full path in the
    # component source
    ORIGINAL_FILE   <original-filename>

    # the file containing the patch hunks
    PATCH_FILE      <patch-file>

    # a directory where the output file after patching will be placed
    # this is an optional argument
    FINAL_DIR       <output-dir>
)

# -- specifying group of patches files
__sdk_add_patch(
    # a name of the component to be patched
    ENTITY_NAME     <patched-component>

    # list of the directories containing group of patch files
    PATCHES_DIRS    <patches-dirs>

    # a directory where the output files after patching will be placed
    # this is an optional argument
    FINAL_DIR       <output-dir>
)

```

<!--- end of file ------------------------------------------------------------->
