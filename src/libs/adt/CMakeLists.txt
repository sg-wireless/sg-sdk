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
# Desc      It supports the build of the lib_adt
# ---------------------------------------------------------------------------- #

__sdk_add_component( adt_lib

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/src/idll.c
        ${CMAKE_CURRENT_LIST_DIR}/src/buffers_chain.c


    MPY_MODS
        "${CMAKE_CURRENT_LIST_DIR}/tst/*.c"

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
        "${CMAKE_CURRENT_LIST_DIR}/tst/*.c"

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}/inc
    
    REQUIRED_SDK_LIBS
        utils_lib
)

__sdk_menu_config_group_add(MAIN.DEMO.CLIBS "Common C-Libraries Demos")

__sdk_menu_config_add_component_menu(adt_buffers_chain
    ${CMAKE_CURRENT_LIST_DIR}/cfg/buffers_chain_demo.config
    MENU_PROMPT "buffers chains example demo"
    MENU_GROUP  MAIN.DEMO.CLIBS
    )

__sdk_menu_config_add_component_menu(adt_buffers_chain
    ${CMAKE_CURRENT_LIST_DIR}/cfg/buffers_chain.config
    MENU_PROMPT "buffers chains"
    MENU_GROUP  MAIN.SDK.CLIBS
    )

__sdk_menu_config_add_component_menu(adt_idll
    ${CMAKE_CURRENT_LIST_DIR}/cfg/idll_demo.config
    MENU_PROMPT "indexed double linked list (IDLL) example demo"
    MENU_GROUP  MAIN.DEMO.CLIBS
    )

__sdk_menu_config_add_component_menu(adt_idll
    ${CMAKE_CURRENT_LIST_DIR}/cfg/idll.config
    MENU_PROMPT "indexed double linked list (IDLL)"
    MENU_GROUP  MAIN.SDK.CLIBS
    )

# --- end of file ------------------------------------------------------------ #
