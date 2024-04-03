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
# Desc      This file is responsible for managing SDK menu configurations.
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# SDK menu configuration organization
# ===================================
#
#   § Each component in the system can optionally add one or more menu config.
#
#   § The component can add menu config specifications using:
#       * __sdk_add_component(): within the parameters of this function there
#         exist an optional parameters to specify a menu config. Using which,
#         only one menu config can be specified.
#       * __sdk_menu_config_add_component_menu(): through this function, more
#         than one menu config specifications can be added for a single
#         component by calling this function multiple times.
#
#   § hierarchy and grouping:
#     The component can specify a special hierarchy for its appearance by
#     the following syntax: <gid1>.<gid2>. -- .<gidN>, where gid is Group-ID
#     The groups shall be added first by __sdk_menu_config_group_add()
#     The following groups are defined by default by the build system.
#       * MAIN      the main menu config screen
#       * MAIN.SDK  the default group for any added SDK component
#       * MAIN.SDK.CLIBS  the default group for common C libraries
#       * MAIN.SDK.NETWORK  the default group for netowrking libraries
#       * MAIN.USR  the default group for any added user project component
#       * MAIN.DEMO a menu group for all demonstration examples.
#     Example:
#       __sdk_menu_config_add_component_menu( component_abc
#           MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/cfg/basic.kconfig
#           MENU_PROMPT     "Component ABC Basics"
#           MENU_GROUP      MAIN.SDK
#           )
#       The output hierarchy will be like this:
#           <MAIN-SCREEN> -> "SDK Components" -> "Component ABC Basics"
#
#   § A group can be added using __sdk_menu_config_group_add(). It takes
#     the following parameters:
#       * GROUP_ID      an identifier to the group.
#       * GROUP_PROMPT  a prompt of this group in the menu hierarchy.
#
#   § Adding menu config shall be specified by the following parameters
#     whether in both of functions __sdk_menu_config_add_component_menu()
#     and __sdk_add_component().
#       * MENU_CONFIG   The actual file containing the kconfig syntax.
#       * MENU_PROMPT   (optional) The appearance name of this menu config.
#                       If not specified, the component name will be used.
#       * MENU_GROUP    (optional) The hierarchical appearance of this menu
#                       config If not specified, the default group will be
#                       SDK.<comp_name>
#
#   § Finally the build system can request the globbing and generation
#     processing of the final menu config file that contain everything.
#     this function __sdk_menu_config_generate() can be called to generate
#     the final menu config file. It shall take a path to the final menu
#     config file to be written.
#
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_group_add(
#               <group-id>
#               <prompt-string> )
#
# description:
#   adds a new group id <group-id> and sets it prompt string to <prompt-string>
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_group_add group_id prompt_string)

    cmake_parse_arguments( _ "" "GID_OUT_VAR" "" ${ARGN})
    log_dbg("-> request adding a new "
            "group-id: ${__purple__}${group_id}${__default__}"
            ", prompt-string: ${__cyan__}${prompt_string}${__default__}")

    # -- add root group-id if not exists
    __entity_exists(root_menu_gid __exists)
    if(NOT __exists)
        __entity_init(root_menu_gid)
    endif()

    # -- parsing
    if(NOT group_id)
        log_msg("parse error: group-id is empty" fatal_error)
    endif()
    set(text_to_parse ${group_id})
    set(group_list)
    set(__last_entity)
    set(__prev_entity root_menu_gid)
    set(__semantic_error OFF)
    
    while(text_to_parse)
        if(__last_entity)
            __entity_exists(${__last_entity} __exists)
            if(NOT __exists)
                log_msg("group-id ${__red__}${__last_entity}${__yellow__}"
                    " while adding ${__purple__}${group_id}" warning)
                set(__semantic_error ON)
            endif()
        endif()

        # consume the first word (it shall start with alphabet or underscore)
        string(REGEX MATCH "^[a-zA-Z_][a-zA-Z_0-9]*" __token "${text_to_parse}")

        # if not matched, stop processing and threw an error
        if(NOT __token)
            log_msg("parse error: expected group-id word but found: "
                    "${__purple__}${text_to_parse}" fatal_error)
        endif()

        # remove it from the 'text_to_parse'
        log_var(__token)
        if(__last_entity)
            set(__prev_entity ${__last_entity})
        endif()
        set(__last_entity ${__prev_entity}.${__token})
        list(APPEND group_list ${__token})
        string(REGEX REPLACE "^${__token}" "" text_to_parse "${text_to_parse}")

        if(text_to_parse)
            # dot must exist check if there is a dot '.'
            string(REGEX MATCH "^\\." found_dot ${text_to_parse})
            if(NOT found_dot)
                log_msg("parse error: expected '.' but found: "
                        "${__purple__}${text_to_parse}" fatal_error)
            endif()
            string(REGEX REPLACE "^\\." "" text_to_parse "${text_to_parse}")
        endif()
    endwhile()

    # stop processing if found a gid not registered in the requested chain
    if(__semantic_error)
        log_msg("adding group-id error" fatal_error)
    endif()

    __entity_exists(${__last_entity} __exists)
    if(__exists)
        log_msg("adding an already existing group-id" fatal_error)
    endif()
    __entity_init(${__last_entity})
    __entity_set_attribute(${__last_entity} PROMPT_STRING ${prompt_string})
    __entity_set_attribute(${__prev_entity} CHILD_GROUPS
        ${__last_entity} APPEND)

    log_dbg("-> group-id chain ${__green__}${group_id}${__default__}"
            " has been added successfully")
    if(__GID_OUT_VAR)
        set(${__GID_OUT_VAR} ${__last_entity} PARENT_SCOPE)
    endif()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_group_exists( <group-id> <output-var> )
#
# description:
#   checks if the group <group-id> exists, the <output-var> will be set to ON
#   if exists, else will be set to OFF
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_group_exists group_id is_exist_var )
    cmake_parse_arguments( _ "" "GID_OUT_VAR" "" ${ARGN})
    __entity_exists(root_menu_gid.${group_id} __exists)
    set(${is_exist_var} ${__exists} PARENT_SCOPE)
    if(__GID_OUT_VAR)
        set(${__GID_OUT_VAR} root_menu_gid.${group_id} PARENT_SCOPE)
    endif()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_group_get_parent( <group-id> <parent_group_id> )
#
# description:
#   returns a parent chain of a given group-is chain. no existence checking
#   is performed, but syntax checking only is performed
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_group_get_parent group_id parent_group_id )
    set(__gid root_menu_gid.${group_id})
    set(__w "[a-zA-Z_][a-zA-Z_0-9]*")
    string(REGEX MATCH "^${__w}(\\.(${__w}(\\.${__w})*))?\\.${__w}\$"
        found "${__gid}")
    if(found)
        set(${parent_group_id} "${CMAKE_MATCH_2}" PARENT_SCOPE)
        return()
    endif()
    log_msg("error finding parent group-id: ${group_id}" fatal_error)
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_init(  )
#
# description:
#   init default hierarchy MAIN.SDK, MAIN.USR and MAIN.DEMO
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_init)
    __sdk_menu_config_group_add(MAIN                "Main Menu")
    __sdk_menu_config_group_add(MAIN.SDK            "*** SDK Components")
    __sdk_menu_config_group_add(MAIN.SDK.CLIBS      "Common C Libraries")
    __sdk_menu_config_group_add(MAIN.SDK.NETWORK    "Network Components")
    __sdk_menu_config_group_add(MAIN.PLATFORM       "*** SDK Platforms")
    __sdk_menu_config_group_add(MAIN.DEMO           "*** SDK Demo Examples")
    __sdk_menu_config_group_add(MAIN.USR        "*** User Project Components")
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_list(  )
#
# description:
#   lists the current hierarchy for debugging
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_list)

    cmake_parse_arguments( _ "" "GROUP_ID;INDENT" "" ${ARGN})
    if(NOT DEFINED __GROUP_ID)
        set(__gid root_menu_gid)
    else()
        set(__gid ${__GROUP_ID})
    endif()

    __entity_exists(${__gid} __exists LOG_OFF)
    if(NOT __exists)
        return()
    endif()

    __entity_get_attribute(${__gid} PROMPT_STRING __str LOG_OFF)
    log_msg("${__INDENT}${__cyan__}${__gid}${__default__}"
            "(${__green__}${__str}${__default__})")
    __entity_get_attribute(${__gid} CHILD_GROUPS __childs LOG_OFF)
    __entity_get_attribute(${__gid} KCONFIG_FILES __kconfigs LOG_OFF)

    
    foreach(__child ${__childs})
        __sdk_menu_config_list(GROUP_ID ${__child} INDENT "${__INDENT}    ")
    endforeach()
    foreach(__file ${__kconfigs})
        log_msg("${__INDENT}    ${__blue__}kconfig-file:${__default__}${__file}")
    endforeach()
endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_add_component_menu(
#               <comp_name>
#               <menu-config-file>
#               [ MENU_PROMPT     <prompt-string>   ]
#               [ MENU_GROUP      <group-chain>     ]
#               )
#
# description:
#   adds a menu config specifications for a single component
#
# parameters:
#   <comp_name>         the component name
#   <menu-config-file>  a file containing the required menu config syntax
#                       (kconfig linux kernel tool syntax)
#   [MENU_PROMPT <prompt-string>]
#                       an optional prompt string for this added menu config
#                       if not specified, the '<comp_name>' will be used instead
#   [MENU_GROUP <group-chain>]
#                       an optional group-chain to specify the hierarchical
#                       appearance of this menu config.
#                       if specified, the appearance will be at:
#                           <group-chain>.<comp_name>
#                       if not specified, the following default hierarchy will
#                       considered:
#                         * MAIN.SDK.<comp_name> for SDK component
#                         * MAIN.USR.<comp_name> for user project component
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_add_component_menu comp_name menu_config_file)

    cmake_parse_arguments( _ "" "MENU_PROMPT;MENU_GROUP" "" ${ARGN})

    if(NOT __MENU_PROMPT)
        set(__MENU_PROMPT ${comp_name})
    endif()

    if(NOT __MENU_GROUP)
        __get_global_attribute( USER_COMPONENT_SWITCH __is_user_comp)
        if(__is_user_comp)
            set(__MENU_GROUP MAIN.USR)
        else()
            set(__MENU_GROUP MAIN.SDK)
        endif()
    endif()

    __sdk_menu_config_group_exists(${__MENU_GROUP} __exists)
    if(NOT __exists)
        log_msg("error component group-id ${__purple__}${__MENU_GROUP}"
                "${__red__} doesn't exist while adding "
                "${__yellow__}${comp_name}" fatal_error)
    endif()

    set(__comp_group ${__MENU_GROUP}.${comp_name})
    __sdk_menu_config_group_exists(${__comp_group} __exists
        GID_OUT_VAR __gid)

    if(NOT __exists)
        __sdk_menu_config_group_add(${__comp_group} ${__MENU_PROMPT}
            GID_OUT_VAR __gid)
    endif()

    __entity_set_attribute(${__gid} KCONFIG_FILES ${menu_config_file}
        APPEND)

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __sdk_menu_config_generate( <final-output-file> )
#
# description:
#   generate the final menu config file for the build system
# ---------------------------------------------------------------------------- #
function(__sdk_menu_config_generate __output_final_file)
    set(__text)
    function(__run_generator __gid __indent __out_text)
        __entity_exists(${__gid} __exists LOG_OFF)
        if(NOT __exists)
            return()
        endif()

        set(text "")

        __entity_get_attribute(${__gid} CHILD_GROUPS __childs LOG_OFF)
        __entity_get_attribute(${__gid} KCONFIG_FILES __kconfigs LOG_OFF)

        if(NOT (__childs OR __kconfigs))
            set(${__out_text} ${text} PARENT_SCOPE)
            return()
        endif()

        if(NOT "${__gid}" STREQUAL "root_menu_gid.MAIN")
            __entity_get_attribute(${__gid} PROMPT_STRING __str LOG_OFF)
            string(APPEND text "${__indent}menu \"${__str}\"\n")
            set(sub_indent "${__indent}    ")
        else()
            set(sub_indent "${__indent}")
        endif()

        foreach(__child ${__childs})
            __run_generator(${__child} "${sub_indent}" output)
            string(APPEND text ${output})
        endforeach()
        foreach(__file ${__kconfigs})
            string(APPEND text "${sub_indent}source \"${__file}\"\n")
        endforeach()

        if(NOT "${__gid}" STREQUAL "root_menu_gid.MAIN")
            string(APPEND text "${__indent}endmenu\n")
        endif()

        set(${__out_text} ${text} PARENT_SCOPE)
    endfunction()

    __run_generator(root_menu_gid.MAIN "" __text)
    # log_msg("${__text}")

    __sdk_update_file_contents(${__output_final_file} ${__text})

endfunction()

# --- end of file ------------------------------------------------------------ #
