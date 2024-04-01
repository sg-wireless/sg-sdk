# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author    Ahmed Sabry (SG Wireless)
#
# Desc      This file is responsible for managing SDK build components and
#           attributes
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_add_component( <comp-name>
#                   -------------- generic --------------
#                   [ COMP_TYPE     <component-type> ]
#                   [ SRCS          <file> ... ]
#                   [ INCS          <include-dir> ... ]
#                   [ INCS_IF       <include-dir> ... ]
#                   [ INCS_PRIV     <include-dir> ... ]
#                   [ DEFS          <compile-definition> ... ]
#                   [ FLAGS         <compiler-flag> ... ]
#                   [ LOGS_DEFS     <logs-definition-file> ... ]
#                   [ MENU_CONFIG <menu-config-file> ]
#                   [ MENU_PROMPT <menu-prompt> ]
#                   [ MENU_GROUP  <menu-group> ]
#                   [ REQUIRED_SDK_LIBS  <lib> ... ]
#                   ------- micropython specific --------
#                   [ MPY_MODS      <mpy-mod-file> ... ]
#                   [ MPY_EXCLUDE_FILES <mpy-orig-file> ... ]
#                   --------- esp-idf specific ----------
#                   [ REQUIRED_ESP_LIBS  <lib> ... ]
#       )
# 
# description
# -----------
#   § It is used to add a software component library. It offers many attributes
#     associated to the required component.
# 
# generic arguments
# -----------------
#   <comp-name>     an identifier name for the component
#   
#   COMP_TYPE       a user specified type for this component to be easily
#                   retrieved and filtered based on this type.
#                   example the patching system is marking all patched
#                   components with a type 'patched' to be collected afterwards
#                   processed.
#
#   SRCS            all C source files. it can be referenced by wildcard
#                   character '*'. ex: "${CMAKE_CURRENT_LIST_DIR}/*.c"
#
#   INCS or INCS_IF both are used to provide a list of interface include
#                   directories. So that the user component of this component
#                   can directly include any file within those directories
#                   without the need to specify its include directory
#
#   INCS_PRIV       a list of include directory to be used only for the source
#                   files of this component.
#
#   DEFS            a list of private compilation definitions
#
#   FLAGS           a list of private compilation flags
#
#   LOGS_DEFS       a list of files that have a log_lib subsystem/component
#                   definitions to be used by the log_lib generator
#
#   REQUIRED_SDK_LIBS a list of other dependent SDK components
#
#   MENU_CONFIG     a kconfig file to specify the component menu config
#
#   MENU_PROMPT     a prompt name for this component in the menu display
#
#   MENU_GROUP      a group chain hierarchy to locate this component configs
# 
# micropython specific arguments
# ------------------------------
#   MPY_MODS        a subset of the SRC files above that contains a micropython
#                   c modules to be used by the micropython binder generator.
#                   These files will be exmpted from the compilation in case of
#                   native C build variant.
#
#   MPY_EXCLUDE_FILES   a list of files of the original micropython sources.
#                   these files will be excluded from the compilation within
#                   micropython compilation. This cases is needed if the user
#                   wants to overwrites completely an existing micropython file.
#
# ESP-IDF specific arguments
# --------------------------
#   REQUIRED_ESP_LIBS   a list of dependent components from the esp-idf platform
#
# ---------------------------------------------------------------------------- #
function( __sdk_add_component  __comp_name )
    set( options)
    set( oneValueArgs
        COMP_TYPE
        MENU_CONFIG
        MENU_PROMPT
        MENU_GROUP
        )
    set( multiValueArgs
        SRCS        # source files list
        INCS        # include directories list
        INCS_IF     # include directories list
        INCS_PRIV   # include directories list
        DEFS        # compile definiiotns list
        FLAGS       # c compile flags list
        MPY_MODS    # list of micropython c module files
        LOGS_DEFS   # file containing the log-lib subsystems/components defs
        MPY_EXCLUDE_FILES
        REQUIRED_SDK_LIBS
        REQUIRED_ESP_LIBS
        )
    cmake_parse_arguments( __arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    log_dbg("add new SDK component ${__purple__}${__comp_name}${__default__}")
    __entity_init(${__comp_name})

    if(DEFINED __arg_COMP_TYPE)
        __entity_set_attribute(${__comp_name} ENTITY_TYPE ${__arg_COMP_TYPE}
            APPEND)
    else()
        __entity_set_attribute(${__comp_name} ENTITY_TYPE library APPEND)
    endif()

    if(DEFINED __arg_MENU_PROMPT)
        __entity_set_attribute(${__comp_name} MENU_PROMPT
            "${__arg_MENU_PROMPT}")
    endif()

    __get_global_attribute( USER_COMPONENT_SWITCH __is_user_comp)
    if(__is_user_comp)
        __entity_set_attribute(${__comp_name} ENTITY_TYPE userlib APPEND)
    else()
        __entity_set_attribute(${__comp_name} ENTITY_TYPE sdklib APPEND)
    endif()
 
    if(DEFINED __arg_INCS)
        __entity_set_attribute(${__comp_name} INCS_IF ${__arg_INCS} APPEND)
    endif()
    if(DEFINED __arg_INCS_IF)
        __entity_set_attribute(${__comp_name} INCS_IF ${__arg_INCS_IF} APPEND)
        __set_global_attribute( SDK_INCS ${__arg_INCS_IF} APPEND)
    endif()
    if(DEFINED __arg_INCS_PRIV)
        __entity_set_attribute(${__comp_name}
            INCS_PRIV ${__arg_INCS_PRIV} APPEND)
    endif()
    if(DEFINED __arg_DEFS)
        __entity_set_attribute(${__comp_name} DEFINITIONS ${__arg_DEFS})
    endif()
    if(DEFINED __arg_FLAGS)
        __entity_set_attribute(${__comp_name} CFLAGS ${__arg_FLAGS})
    endif()
    if(DEFINED __arg_REQUIRES)
        __entity_set_attribute(${__comp_name} REQUIRES ${__arg_FLAGS})
    endif()

    file(GLOB __sources ${__arg_SRCS})

    if(DEFINED __arg_MPY_MODS)
        file(GLOB __cmods ${__arg_MPY_MODS})
        __entity_set_attribute(${__comp_name} MPY_CMODS ${__cmods} APPEND)
        if(DEFINED __arg_SRCS AND
            NOT "${__build_variant}" STREQUAL "micropython")
            list(REMOVE_ITEM __sources ${__cmods})
        endif()
    endif()

    if(__sources)
        __entity_set_attribute(${__comp_name} SOURCES ${__sources})
    endif()

    if(DEFINED __arg_LOGS_DEFS)
        file(GLOB __sources ${__arg_LOGS_DEFS})
        __set_global_attribute( LOGS_DEFS ${__sources} APPEND)
    endif()

    if(DEFINED __arg_MPY_EXCLUDE_FILES)
        file(GLOB __sources ${__arg_MPY_EXCLUDE_FILES})
        __set_global_attribute( MPY_EXCLUDE_FILES ${__sources} APPEND )
    endif()

    if(DEFINED __arg_REQUIRED_SDK_LIBS)
        __entity_set_attribute(${__comp_name}
            REQUIRED_SDK_LIBS ${__arg_REQUIRED_SDK_LIBS})
    endif()

    if(DEFINED __arg_REQUIRED_ESP_LIBS)
        __entity_set_attribute(${__comp_name}
            REQUIRED_ESP_LIBS ${__arg_REQUIRED_ESP_LIBS} APPEND)
    endif()

    if(DEFINED __arg_MENU_CONFIG)
        if(__arg_MENU_PROMPT)
            set(__menu_prompt MENU_PROMPT ${__arg_MENU_PROMPT})
        endif()
        if(__arg_MENU_GROUP)
            set(__menu_group MENU_GROUP ${__arg_MENU_GROUP})
        endif()
        __sdk_menu_config_add_component_menu(${__comp_name}
            ${__arg_MENU_CONFIG} ${__menu_group} ${__menu_prompt}
            )
    endif()

endfunction()

function(__sdk_enable_user_comps)
    __set_global_attribute( USER_COMPONENT_SWITCH ON)
endfunction()

function(__sdk_disable_user_comps)
    __set_global_attribute( USER_COMPONENT_SWITCH OFF)
endfunction()


# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_add_logs_defs_files( <logs-defs-file>... )
#
# description:
#       adds files to the files list that contain logs subsystems and components
#       definitions
# ---------------------------------------------------------------------------- #
function(__sdk_add_logs_defs_files)

    file(GLOB __files ${ARGN})
    __entity_set_attribute(main_entity LOGS_DEFS ${__files} APPEND)

endfunction()

function(__sdk_get_logs_defs_files __out_var)
    __entity_get_attribute(main_entity LOGS_DEFS __files)
    set(${__out_var} ${__files} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_add_comp_dirs( <comp-dir>... )
#
# description:
#       adds components directories
# ---------------------------------------------------------------------------- #
function(__sdk_add_comp_dirs)
    foreach(__arg ${ARGN})
        file(GLOB __dirs ${__arg})
        foreach(__dir ${__dirs})
            set(__list_file ${__dir}/CMakeLists.txt)
            if(IS_DIRECTORY ${__dir})
                if(NOT EXISTS ${__list_file})
                    log_msg("${__list_file} does not exist" fatal_error)
                endif()
                log_dbg("-- add comp list file ${__list_file}")
                __entity_set_attribute(main_entity COMPONENTS_LIST_FILES
                    ${__list_file} APPEND)
            endif()
        endforeach()
    endforeach()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_add_comp_list_files( <comp-list-file>... )
#
# description:
#       adds components list files
# ---------------------------------------------------------------------------- #
function(__sdk_add_comp_list_files)
    foreach(__arg ${ARGN})
        file(GLOB __files ${__arg})
        foreach(__file ${__files})
            if(NOT EXISTS ${__file})
                log_msg("${__file} does not exist" fatal_error)
            endif()
            log_dbg("-- add comp list file ${__file}")
            __entity_set_attribute(main_entity COMPONENTS_LIST_FILES
                ${__file} APPEND)
        endforeach()
    endforeach()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_get_comp_list_files( <out-var> )
#
# description:
#       gets current added component list files
# ---------------------------------------------------------------------------- #
function(__sdk_get_comp_list_files __out_var)
    __entity_get_attribute(main_entity COMPONENTS_LIST_FILES __files)
    set(${__out_var} ${__files} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_add_micropython_frozen_manifest( <files-list> )
#       __sdk_get_micropython_frozen_manifest( <output-variable> )
#
# description:
#       adds/gets a frozen manifest list of files
# ---------------------------------------------------------------------------- #
function( __sdk_add_micropython_frozen_manifest )
    file(GLOB __files ${ARGN})
    __entity_set_attribute(main_entity MPY_FROZEN_MANIFEST ${__files} APPEND)
endfunction()

function( __sdk_get_micropython_frozen_manifest __out_var)
    __entity_get_attribute(main_entity MPY_FROZEN_MANIFEST __files)
    set(${__out_var} ${__files} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_add_compile_options(
#                       [SDK_LIBS]
#                       [USR_LIBS]
#                       <option-string> )
# description:
#       adds a compilation options to all contributing libraries in the project
#       only for SDK libraries if optional 'SDK_LIBS' is specified and only for
#       only user libraries if 'USR_LIBS' is specified. If both 'SDK_LIBS' and
#       'USR_LIBS' are specified it will be added to SDK and USER libraries.
# ---------------------------------------------------------------------------- #
function( __sdk_add_compile_options )

    set(arg_opts SDK_LIBS USR_LIBS)
    cmake_parse_arguments( _ "${arg_opts}" "" "" ${ARGN})

    set(__cc)
    foreach(__arg ${ARGN})
        if(NOT ${__arg} IN_LIST arg_opts)
            list(APPEND __cc ${__arg})
        endif()
    endforeach()
    

    if(__SDK_LIBS)
        __entity_set_attribute(main_entity COMPILE_FLAGS_SDK ${__cc} APPEND)
    endif()
    if(__USR_LIBS)
        __entity_set_attribute(main_entity COMPILE_FLAGS_USR ${__cc} APPEND)
    endif()
    if(NOT (__SDK_LIBS OR __USR_LIBS))
        __entity_set_attribute(main_entity COMPILE_FLAGS ${ARGN} APPEND)
    endif()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_get_compile_options(
#                       <out-var>
#                       [SDK_LIBS]
#                       [USR_LIBS] )
# description:
#       gets a currently set compilation options for any library of only for
#       SDK libraries if 'SDK_LIBS' is set or only for user libraries if
#       'USR_LIBS' is set.
# ---------------------------------------------------------------------------- #
function( __sdk_get_compile_options __out_var)

    cmake_parse_arguments( _ "SDK_LIBS;USR_LIBS" "" "" ${ARGN})

    if(__SDK_LIBS)
        __entity_get_attribute(main_entity COMPILE_FLAGS_SDK __output)
    elseif(__USR_LIBS)
        __entity_get_attribute(main_entity COMPILE_FLAGS_USR __output)
    else()
        __entity_get_attribute(main_entity COMPILE_FLAGS __output)
    endif()
    set(${__out_var} ${__output} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_add_kconfig_default( <option-string> )
#               __sdk_get_kconfig_default( <output-variable> )
# ---------------------------------------------------------------------------- #
function( __sdk_add_kconfig_default )
    foreach(__cnf_file ${ARGN})
        file(GLOB __file ${__cnf_file})
        if(__file)
            __entity_set_attribute(main_entity SDK_CONFIGS ${__file} APPEND)
        endif()
    endforeach()
endfunction()

function( __sdk_get_kconfig_default __out_var)
    __entity_get_attribute(main_entity SDK_CONFIGS __output)
    set(${__out_var} ${__output} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __set_global_attribute(
#                   <attribute> <value> [ <value>... ]
#                   [ APPEND ]
#               )
# ---------------------------------------------------------------------------- #
function( __set_global_attribute attribute )
    __entity_set_attribute(main_entity ${attribute} ${ARGN})
endfunction()

function( __get_global_attribute attribute __out_var )
    __entity_get_attribute(main_entity ${attribute} __values)
    set(${__out_var} ${__values} PARENT_SCOPE)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_add_patch(
#                   ENTITY_NAME     <entity-name>
#                   ( 
#                       PATCH_FILE      <patch-file>
#                       [ ORIGINAL_FILE   <original-file> ]
#                   ||
#                       PATCHES_DIRS     <patches-dirs-list>
#                   )
#                   [ FINAL_DIR   <final-patched-files-directory> ]
#                   [ ESP_IDF ]
#                   [ EXTERNAL ]
#                   [ EXTRA_INCLUDES <include-dirs-list> ]
#               )
#
# description:
#
#   § ENTITY_NAME   refers to an external component used in the project build
#     it could be "micropython" for micropython libraray or one of the ESP-IDF
#     components
#
#   § the function can take either of the following
#       * an individual PATCH_FILE with an optional original file identification
#         using ORIGINAL_FILE argument
#       * a group of patches in a given directories PATCHES_DIR, each patch in
#         the directory will be applied to its corresponding original file from
#         the entity sources
#
#   § FINAL_DIR it is used to redirect the generation of the final modified
#     patched files into a dedicated directory.
#     if the global option PATCHED_FILES_IN_BUILD_DIR is defined, the special
#     option MODIFIED_FILE_DIR will be ignored and the generated files will be
#     always directed to the directory <CMAKE_BINARY_DIR>/patches/<ENTITY_NAME>
#
#   § EXTRA_INCLUDES additional include dirs to be added to the patched target
#     which are needed while compiling target with the new added patch
# ---------------------------------------------------------------------------- #
function(__sdk_add_patch)
    set( options
        ESP_IDF
        EXTERNAL
        )
    set( oneValueArgs
        ENTITY_NAME
        ORIGINAL_FILE
        PATCH_FILE
        PATCHES_DIR
        FINAL_DIR
        )
    set( multiValueArgs
        PATCHES_DIRS
        EXTRA_INCLUDES
        )
    cmake_parse_arguments( _
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(err)
    set(__fun ${CMAKE_CURRENT_FUNCTION})

    if(NOT DEFINED __ENTITY_NAME)
        string(APPEND err "-- missing ENTITY_NAME in ${__fun}()")
    endif()

    if((DEFINED __PATCH_FILE AND DEFINED __PATCHES_DIRS) OR
        NOT (DEFINED __PATCH_FILE OR DEFINED __PATCHES_DIRS))
        string(APPEND err "-- in func ${__fun}(); either PATCH_FILE or ")
        string(APPEND err "PATCHES_DIRS shall be specified but not both")
    endif()

    if(DEFINED __PATCHES_DIRS AND DEFINED __ORIGINAL_FILE)
        string(APPEND err "-- in func ${__fun}(); specifying ORIGINAL_FILE")
        string(APPEND err " is alowed with PATCH_FILE but not with PATCHES_DIRS")
        string(APPEND err " PATCHES_DIRS")
    endif()

    if(NOT "${err}" STREQUAL "")
        log_msg(${err} fatal_error)
    endif()

    log_dbg("add patching info for entity ${__ENTITY_NAME}")

    if(NOT DEFINED __FINAL_DIR OR DEFINED PATCHED_FILES_IN_BUILD_DIR)
        set(__FINAL_DIR ${CMAKE_BINARY_DIR}/patches/${__ENTITY_NAME} )
    endif()

    __entity_init( ${__ENTITY_NAME} )
    __entity_set_attribute( ${__ENTITY_NAME} ENTITY_TYPE patched APPEND )

    foreach(_opt ${options})
        if(__${_opt})
            __entity_set_attribute( ${__ENTITY_NAME} ${_opt} on )
        endif()
    endforeach()

    if(DEFINED __PATCH_FILE)
        log_dbg("patch entity ${__purple__}${__ENTITY_NAME}${__default__} "
            "with file ${__blue__}${__PATCH_FILE}${__default__}")
        __entity_set_attribute( ${__ENTITY_NAME} PATCH_FILE
            "(${__ORIGINAL_FILE},${__PATCH_FILE},${__FINAL_DIR})"
            APPEND )
    else()
        foreach(__patch_dir ${__PATCHES_DIRS})
            log_dbg("patch entity ${__purple__}${__ENTITY_NAME}${__default__} "
                "with patches dir ${__blue__}${__patch_dir}${__default__}")
            __entity_set_attribute( ${__ENTITY_NAME} PATCH_DIR
                "(${__patch_dir},${__FINAL_DIR})"
                APPEND )
        endforeach()
    endif()

    if(DEFINED __EXTRA_INCLUDES)
        __entity_set_attribute(${__ENTITY_NAME} EXTRA_INCLUDES
            ${__EXTRA_INCLUDES})
    endif()

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_process_patching(
#                   <patch-file>
#                   <original-file>
#                   <final-dir>
#                   [ <output-final-file> ]
#               )
#
# description:
#
#   § This function processes single file patching
#   
#   § It takes the following arguments:
#       <patch-file>        The file containing the patch hunks
#       <original-file>     The original source file to be patched
#       <final-dir>         The directory at which the patched file will be
#                           saved
#       <output-final-file> An optional variable name to be set with the new
#                           saved patched final file
# ---------------------------------------------------------------------------- #
function(__sdk_process_patching
        patch_file
        orig_file
        final_dir
        output_final_file)

    # -- apply the 'patch_file' on 'orig_file'
    #    and save the output file at 'final_dir'

    # checkers
    if( NOT EXISTS ${orig_file} )
        log_msg("error: patching non-exist original file ${orig_file}"
            fatal_error)
    endif()
    if( NOT EXISTS ${patch_file} )
        log_msg("error: patching non-exist patch file ${patch_file}"
            fatal_error)
    endif()

    if(__log_show_patched_files)
        log_msg("-- process patching ${__cyan__}${orig_file}${__default__}")
    else()
        log_dbg("-- process patching ${__cyan__}${orig_file}${__default__}")
    endif()

    # create final dir if not exists
    if( NOT EXISTS ${final_dir} )
        file( MAKE_DIRECTORY ${final_dir} )
    endif()

    # create temporary folder in the build location
    set(__temp_dir ${CMAKE_BINARY_DIR}/temp_patching)
    if( NOT EXISTS ${__temp_dir} )
        file( MAKE_DIRECTORY ${__temp_dir} )
    endif()

    # -- patch first at a dummy file
    get_filename_component( __file_name ${orig_file} NAME )
    set(__temp_file ${__temp_dir}/${__file_name}.tmp)
    set(__final_file ${final_dir}/${__file_name})
    execute_process(
        COMMAND patch --ignore-whitespace ${orig_file}
            -i ${patch_file} -o ${__temp_file}
        OUTPUT_VARIABLE __process_output
        RESULT_VARIABLE __process_result
        )
    if(${__process_result} )
        log_msg("${__red__}patching process error:\n"
            "${__process_output}${__default__}" fatal_error)
    endif()

    # -- compare and copy if necessary
    file(COPY_FILE ${__temp_file} ${__final_file} ONLY_IF_DIFFERENT)

    if(output_final_file)
        set(${output_final_file} ${__final_file} PARENT_SCOPE)
    endif()

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_patch_target(
#                   <entity>
#                   <target>
#               )
#
# description:
#
#   § This function applies the previously recorded patching info from the
#     entity and applies it to a real target
#   
#   § It takes the following arguments:
#       <entity>    The SDK entity name.
#       <target>    The real build target to patch its equivalent sources as in
#                   the patch information in the entity.
# ---------------------------------------------------------------------------- #
function(__sdk_patch_target __entity __target)

    log_dbg("patch process from entity "
        "${__blue__}${__entity}${__default__} to target"
        " ${__cyan__}${__target}${__default__}")

    set(__step_idx 1)

    log_dbg(" -- step (${__step_idx}): fetch original target sources" green)
    math(EXPR __step_idx "${__step_idx} + 1")
    get_target_property(__orig_srcs ${__target} SOURCES)
    get_target_property(__orig_incs ${__target} INCLUDE_DIRECTORIES)
    # log_list(__orig_srcs)

    log_dbg(" -- step (${__step_idx}): fetch individual patch files" green)
    math(EXPR __step_idx "${__step_idx} + 1")

    __entity_get_attribute(${__entity} PATCH_FILE __pairs)
    foreach(__pair ${__pairs})
        if(${__pair} MATCHES "^\\((.+)?,(.+)?,(.+)?\\)\$")
            set(__orig_file ${CMAKE_MATCH_1})
            set(__patch_file ${CMAKE_MATCH_2})
            set(__final_dir ${CMAKE_MATCH_3})
            log_dbg("--> patch file ${__cyan__}${__patch_file}${__default__}")
            log_dbg("    orig file  ${__cyan__}${__orig_file}${__default__}")
            log_dbg("    final dir  ${__cyan__}${__final_dir}${__default__}")

            # check if original file among target sources
            if(NOT "${__orig_file}" IN_LIST __orig_srcs)
                log_dbg("orig file ${__cyan__}${__orig_file}${__default__}"
                    " does not exist among target "
                    "${__yellow__}${__target}${__default__} sources,"
                    " skip this patch ...")
                continue()
            endif()

            # process file patching
            __sdk_process_patching(${__patch_file} ${__orig_file} ${__final_dir}
                __final_file)

            # replace the new patched file in the target sources files
            list(REMOVE_ITEM __orig_srcs ${__orig_file})
            list(APPEND __orig_srcs ${__final_file})

            # add final dir to the target includes list
            get_filename_component(__orig_dir ${__orig_file} DIRECTORY)
            list(APPEND __orig_incs ${__final_dir} ${__orig_dir})
        endif()
    endforeach()

    log_dbg(" -- step (${__step_idx}): fetch directory groups" green)
    math(EXPR __step_idx "${__step_idx} + 1")
    __entity_get_attribute(${__entity} PATCH_DIR __pairs)
    foreach(__pair ${__pairs})
        if(${__pair} MATCHES "^\\((.+)?,(.+)?\\)\$")
            set(__patch_dir ${CMAKE_MATCH_1})
            set(__final_dir ${CMAKE_MATCH_2})
            log_dbg(" --> found patch dir ${__cyan__}${__patch_dir}${__default__}")
            log_dbg("     final dir ${__cyan__}${__final_dir}${__default__}")
            if(NOT EXISTS ${__patch_dir})
                log_dbg("error: patch dir ${__patch_dir} does not exist"
                    fatal_error)
            endif()

            # pick up all patch files in the patches directory
            file(GLOB __patch_files "${__patch_dir}/*.patch")

            foreach(__patch_file ${__patch_files})
                # look for the corresponding original file in the target sources
                string(REGEX REPLACE ".*/(.*)\.patch\$" "\\1"
                    __filename ${__patch_file})
                foreach(__src_file ${__orig_srcs})
                    if( ${__src_file} MATCHES "^.*/${__filename}$" )
                        __sdk_process_patching(
                            ${__patch_file} ${__src_file} ${__final_dir}
                            __final_file)

                        list(REMOVE_ITEM __orig_srcs ${__src_file})
                        list(APPEND __orig_srcs ${__final_file})
                        get_filename_component(src_dir ${__src_file} DIRECTORY)
                        list(APPEND __orig_incs ${src_dir})
                        break()
                    endif()
                endforeach()
            endforeach()
            if(__patch_files)
                list(APPEND __orig_incs ${__final_dir})
            endif()
        endif()
    endforeach()

    log_dbg(" -- step (${__step_idx}): update target with new sources" green)
    math(EXPR __step_idx "${__step_idx} + 1")

    __entity_get_attribute(${__entity} EXTRA_INCLUDES __extra_incs)
    list(APPEND __orig_incs ${__extra_incs})
    list(REMOVE_DUPLICATES __orig_incs)

    set_target_properties(${__target} PROPERTIES
        SOURCES             "${__orig_srcs}"
        INCLUDE_DIRECTORIES "${__orig_incs}")

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_get_user_project_dir( <output-var>)
#
# description:
#   This function retrieves the user project dir passed in the CLI in the
#   variable APP_DIR
# ---------------------------------------------------------------------------- #
function(__sdk_get_user_project_dir __user_prj_dir)

    if(DEFINED APP_DIR)
        set(${__user_prj_dir} ${APP_DIR} PARENT_SCOPE)
    endif()

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:     __sdk_process_cli_variables( )
#
# description:
#   This function do the required processing on the passed variabled from the
#   CLI. The expected passed CLI vars are as follows:
# 
#   variables needed by build system only
#       __partition_table       STR     path to the partition table csv file
#       __sdkconfigs_files      list    list of sdkconfig files for esp-idf env
#
#   variablled needed by both the build system and the source code
#       __build_variant         STR     to determine micropython or native build
#       __feature_<feature>     BOOL    specify a specific feature switch
#       __user_def_<def>        any     a provided user specific define
#       
#       APP_NAME                STR     required application name
#       APP_DIR                 STR     path to user project dir
#       
#       SDK_PLATFORM            STR     the platform name(ex: F1)
#       SDK_BOARD_NAME          STR     the board name (ex: F1-L)
#       SDK_BOARD_NUMBER        STR     the board part number (ex: SGW3501)
#       SDK_BOARD_SHIELD        STR     the used shield if any
#       SDK_BOARD               STR     the combined board name
#
# ---------------------------------------------------------------------------- #
function(__sdk_process_cli_variables)
    get_cmake_property(__all_vars VARIABLES)

    foreach(var ${__all_vars})
        # if(${var} MATCHES "^\\((.+)?,(.+)?,(.+)?\\)\$")
        if("${var}" MATCHES "^__feature_\(.+\)")
            set(feat ${CMAKE_MATCH_1})
            if(${var})
                __sdk_add_compile_options( "-D${var}" )
                log_dbg("-- feature enabled  >> ${feat}" cyan)
            else()
                log_dbg("-- feature disabled >> ${feat}" red)
            endif()
            continue()
        endif()

        if("${var}" MATCHES "^__user_def_(.+)\$")
            set(user_def ${CMAKE_MATCH_1})
            if("${${var}}" MATCHES "^((0x[0-9a-fA-F]+)|([0-9]+))$")
                log_dbg("-- add user-defined-var (${__green__}Integer"
                    "${__default__}) ${__cyan__}${user_def}${__default__}="
                    "${__purple__}${${var}}${__default__}")
                    __sdk_add_compile_options("-D${user_def}=${${var}}")
            else()
                log_dbg("-- add user-defined-var (${__green__}String"
                    "${__default__}) ${__cyan__}${user_def}${__default__}="
                    "${__purple__}\"${${var}}\"${__default__}")
                    __sdk_add_compile_options("-D${user_def}=\"${${var}}\"")
            endif()
            continue()
        endif()

    endforeach()

    set(__sdk_board_vars SDK_PLATFORM SDK_BOARD_NAME SDK_BOARD_NUMBER
        SDK_BOARD_SHIELD SDK_BOARD)
    foreach(var ${__sdk_board_vars})
        if( DEFINED ${var} )
            set( __ignore_variable__ ${var} )
            log_dbg("-- add board-var: ${var}=\"${${var}}\"")
            __sdk_add_compile_options( "-D${var}=\"${${var}}\"" )
        endif()
    endforeach()

    if( DEFINED FW_GENERATED_BUILD_VERSION_HEADER )
        set(__full_path ${FW_GENERATED_BUILD_VERSION_HEADER})
        get_filename_component(__dir ${__full_path} DIRECTORY)
        get_filename_component(__file ${__full_path} NAME)
        __sdk_add_compile_options(
            "-DFW_GENERATED_BUILD_VERSION_HEADER=\"${__file}\"" )
        __set_global_attribute( SDK_INCS ${__dir} APPEND)
    endif()

    if("${__build_variant}" STREQUAL "micropython")
        __sdk_add_compile_options("-DMICROPYTHON_BUILD")
    endif()

endfunction()

# --- end of file ------------------------------------------------------------ #
