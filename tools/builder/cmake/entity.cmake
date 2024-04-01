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
# Desc      Entity management is a build registery to organise the build
#           components and its attributes.
# ---------------------------------------------------------------------------- #

# ---------------------------------------------------------------------------- #
# synopsis:
#       __entity_init( <entity-name> )
#
# description:
#       Initializes new entity for further use by setting/getting attributes.
# ---------------------------------------------------------------------------- #
function( __entity_init entity_name )

    cmake_parse_arguments( _ "GLOBAL" "" "" ${ARGN})

    if( NOT TARGET __reg_tgt_main_entity )
        log_dbg("-- create the SDK main CMake entity target" yellow)
        add_library( __reg_tgt_main_entity STATIC IMPORTED GLOBAL )
    endif()

    set( target_name __reg_tgt_${entity_name} )

    if("${entity_name}" STREQUAL "main_entity")
        return()
    endif()

    __entity_get_attribute(main_entity
        REGISTERED_ENTITIES __registered_entities)

    if( NOT ${entity_name} IN_LIST __registered_entities )
        log_dbg("-- init new entity register '${entity_name}'" green)
        if(__GLOBAL)
            add_library( ${target_name} STATIC IMPORTED GLOBAL )
        else()
            add_library( ${target_name} STATIC IMPORTED )
        endif()
        __entity_set_attribute(main_entity
            REGISTERED_ENTITIES ${entity_name} APPEND)
    else()
        log_dbg("-- init already existing entity '${entity_name}'" red)
    endif()

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __entity_exists( <entity-name> <output-var> )
#
# description:
#       checks an existence of an entity.
# ---------------------------------------------------------------------------- #
function( __entity_exists entity_name output_variable)

    cmake_parse_arguments( _ "LOG_OFF" "" "" ${ARGN})

    if( NOT TARGET __reg_tgt_main_entity )
        log_dbg("-- create the SDK main CMake entity target" yellow)
        add_library( __reg_tgt_main_entity STATIC IMPORTED GLOBAL )
    endif()

    set( target_name __reg_tgt_${entity_name} )

    if("${entity_name}" STREQUAL "main_entity")
        set(${output_variable} ON PARENT_SCOPE)
        return()
    endif()

    if(__LOG_OFF)
        set(__disable_logs LOG_OFF)
    endif()
    __entity_get_attribute(main_entity REGISTERED_ENTITIES __entities
        ${__disable_logs})

    if( ${entity_name} IN_LIST __entities )
        set(text "entity ${__green__}${entity_name}${__default__} exists")
        set(${output_variable} ON PARENT_SCOPE)
    else()
        set(text "entity ${__red__}${entity_name}${__default__} doesn't exists")
        set(${output_variable} OFF PARENT_SCOPE)
    endif()

    if(NOT __LOG_OFF)
        log_dbg(${text})
    endif()

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __entity_set_attribute( <entity>
#                               <attribute>
#                               <value> [ <value> ...]
#                               [ APPEND ]
#                             )
#
# description:
#       Sets a value for a certain entity attribute. If 'APPEND' is specified
#       the value list will be appended to the current value list of the
#       specified attribute.
#       If the attribute is already exist and no APPEND specified, the attribute
#       value-list will be overridden by the new provided value list.
# ---------------------------------------------------------------------------- #
function(__entity_set_attribute entity attribute value)

    cmake_parse_arguments( _ "APPEND;LOG_OFF" "" "" ${ARGN})

    set( target_name __reg_tgt_${entity} )

    if(NOT TARGET ${target_name})
        log_msg("-- error: try set non existing entity '${entity}'"
            fatal_error)
    endif()

    set(__values)
    foreach(_v ${ARGV})
        if(NOT("${_v}" STREQUAL "APPEND" OR
            "${_v}" STREQUAL "${entity}" OR
            "${_v}" STREQUAL "${attribute}"))
            list(APPEND __values ${_v})
        endif()
    endforeach()

    set(__key __key_${attribute})

    get_property(old_value TARGET ${target_name} PROPERTY ${__key})

    if(NOT __LOG_OFF)
        set(__msg "entity::${__yellow__}${entity}${__default__} ")
        string(APPEND __msg "@[${__cyan__}${attribute}${__default__}] ")
        if(NOT "${old_value}" STREQUAL "")
            string(APPEND __msg "value: ${__cyan__}${old_value}${__default__} ")
            if(__APPEND)
                string(APPEND __msg "append-value: ${__green__}${__values}")
            else()
                string(APPEND __msg "change-to: ${__green__}${__values}")
            endif()
        else()
            string(APPEND __msg "set-value: ${__green__}${__values}")
        endif()
        # string(REGEX REPLACE
        #       "\/(([a-zA-Z0-9_]|-)+\/)+" "" __new_msg "${__msg}")
        # log_dbg("${__new_msg}${__default__}")
        log_dbg("${__msg}${__default__}")
    endif()

    set(__new_values ${old_value})
    if(__APPEND)
        list(APPEND __new_values ${__values})
    else()
        set(__new_values ${__values})
    endif()
    list(REMOVE_DUPLICATES __new_values)
    set_property(TARGET ${target_name} PROPERTY ${__key} ${__new_values})

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __entity_get_attribute( <entity>
#                               <attribute>
#                               <output-variable>
#                             )
#
# description:
#       Gets the current value list of a certain entity attribute.
# ---------------------------------------------------------------------------- #
function(__entity_get_attribute entity attribute variable)

    cmake_parse_arguments( _ "LOG_OFF" "" "" ${ARGN})

    set( target_name __reg_tgt_${entity} )

    if(NOT TARGET ${target_name})
        log_msg("-- error: try get non existing entity '${entity}'"
            fatal_error)
    endif()

    set(__key __key_${attribute})

    get_property(value TARGET ${target_name} PROPERTY ${__key})

    if(NOT __LOG_OFF)
        set(__msg "entity::${__yellow__}${entity}${__default__} ")
        string(APPEND __msg "@[${__cyan__}${attribute}${__default__}] ")
        string(APPEND __msg "get-value: ${__cyan__}${value}${__default__}")
        log_dbg("${__msg}")
    endif()

    set(${variable} ${value} PARENT_SCOPE)

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __entity_find(  <output-variable>
#                       <attribute>
#                       <value>
#                    )
#
# description:
#       Look for all entities that have the given attribute name.
#       If the provided value is
#           * empty string, it will look for all entities that have this
#             attribute set before regardless of its value list
#           * non-empty string, it will look for all entities that have this
#             attribute set before and one of its values equals the given value.
# ---------------------------------------------------------------------------- #
function(__entity_find variable attribute value)

    cmake_parse_arguments( _ "LOG_OFF" "" "" ${ARGN})

    __entity_get_attribute(main_entity REGISTERED_ENTITIES
        __registered_entities)

    set(__key __key_${attribute})

    set(found_entities)
    foreach(_ent ${__registered_entities})
        get_property(_ret TARGET __reg_tgt_${_ent} PROPERTY ${__key})
        # log_msg("--> check entity ${_ent}: values: ${_ret}")
        if( ( ("${value}" STREQUAL "") AND (NOT "${_ret}" STREQUAL "") )
            OR ("${value}" IN_LIST _ret))
            string(REGEX REPLACE "^__reg_tgt_" "" __entity "${_ent}")
            list(APPEND found_entities ${__entity})
        endif()
    endforeach()

    set(${variable} ${found_entities} PARENT_SCOPE)

endfunction()

# ---------------------------------------------------------------------------- #
# synopsis:
#       __entity_list(  [ ATTRIBUTE <attribute> ]
#                       [ VALUES    <value> ... ]
#                       [ DISP      <attribute> ... ]
#                    )
#
# description:
#       A list method for debugging purposes. It lists all entities with
#       specified attribute set. If a given values are given, it will list only
#       entities that have the attribute set with at least one of its value list
#       is one of the given values to this function.
#       The caller can specify a list of attributes with DISP option to display
#       their values along with the found entities.
# ---------------------------------------------------------------------------- #
function(__entity_list)
    cmake_parse_arguments( _ "" "ATTRIBUTE" "VALUES;DISP" ${ARGN})

    set(__msg)
    if(DEFINED __ATTRIBUTE)
        string(APPEND __msg "--- entities which have attribute")
        string(APPEND __msg " ${__yellow__}${ATTRIBUTE}${__default__}")
        if(DEFINED __VALUES)
            string(APPEND __msg " which contain values")
            string(APPEND __msg " ${__green__}${__VALUES}${__default__}")
        else()
            set(__VALUES ALL)
        endif()
    else()
        string(APPEND __msg "--- all registered entities")
        set(__ATTRIBUTE ALL)
    endif()

    __entity_get_attribute(main_entity REGISTERED_ENTITIES __entities LOG_OFF)

    log_msg("${__msg} -- ATTRIBUTE: ${__ATTRIBUTE} -- VALUES: ${__VALUES}")

    set(__found_entities)
    foreach(__entity ${__entities})
        set(__found_entity OFF)
        if("${__ATTRIBUTE}" STREQUAL "ALL")
            set(__found_entity ON)
        else()
            __entity_get_attribute(${__entity} ${__ATTRIBUTE}
                __attr_value LOG_OFF)
            if(NOT "${__attr_value}" STREQUAL "")
                if("${__VALUES}" STREQUAL "ALL")
                    set(__found_entity ON)
                else()
                    set(__found ON)
                    foreach(_v ${__VALUES})
                        if(NOT "${_v}" IN_LIST __attr_value)
                            set(__found OFF)
                            break()
                        endif()
                    endforeach()
                    if(__found)
                        set(__found_entity ON)
                    endif()
                endif()
            endif()
        endif()
        if(__found_entity)
            list(APPEND __found_entities ${__entity})
        endif()
    endforeach()

    set(__cols_lens)
    set(__cols_headers)
    set(__max_len "0")
    foreach(__entity ${__found_entities})
        string(LENGTH ${__entity} __temp_len)
        if(${__max_len} LESS ${__temp_len})
            set(__max_len ${__temp_len})
        endif()
    endforeach()
    list(APPEND __cols_headers "ENTITY")
    math(EXPR __max_len "${__max_len} + 1")
    list(APPEND __cols_lens ${__max_len})

    if(DEFINED __DISP)
        foreach(__col ${__DISP})
            string(LENGTH "${__col}" __max_len)
            foreach(__entity ${__found_entities})
                __entity_get_attribute(${__entity} ${__col} __col_value LOG_OFF)
                string(LENGTH "${__col_value}" __temp_len)
                if(${__max_len} LESS ${__temp_len})
                    set(__max_len ${__temp_len})
                endif()
            endforeach()
            list(APPEND __cols_headers "${__col}")
            math(EXPR __max_len "${__max_len} + 1")
            list(APPEND __cols_lens ${__max_len})
        endforeach()
    endif()

    set(__idx 0)
    set(__row_str)
    foreach(__col_header ${__cols_headers})
        list(GET __cols_lens ${__idx} __col_len)

        string(LENGTH "${__col_header}" __str_len)
        math(EXPR __pad_len "${__col_len} - ${__str_len}")
        set(__msg "${__col_header}")
        string(REPEAT " " ${__pad_len} __pad_str)
        string(APPEND __msg "${__pad_str}")
        string(APPEND __row_str "${__msg}")

        math(EXPR __idx "${__idx} + 1")
    endforeach()
    log_msg("${__row_str}" yellow)


    foreach(__entity ${__found_entities})
        set(__idx 0)
        set(__row_str)
        foreach(__col ${__entity};${__DISP})
            list(GET __cols_lens ${__idx} __col_len)

            set(__str)
            if("${__col}" STREQUAL "${__entity}")
                set(__str ${__entity})
            else()
                __entity_get_attribute(${__entity} ${__col} __str LOG_OFF)
            endif()
            string(LENGTH "${__str}" __str_len)
            math(EXPR __pad_len "${__col_len} - ${__str_len}")
            set(__msg "${__str}")
            string(REPEAT " " ${__pad_len} __pad_str)
            string(APPEND __msg "${__pad_str}")
            string(APPEND __row_str "${__msg}")

            math(EXPR __idx "${__idx} + 1")
        endforeach()
        log_msg("${__row_str}")
    endforeach()

endfunction()

# --- end of file ------------------------------------------------------------ #
