__sdk_menu_config_group_add(MAIN.PLATFORM.F1.CAN "CAN")

__sdk_add_component( f1_can_if

    LOGS_DEFS
        ${CMAKE_CURRENT_LIST_DIR}/can.c

    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/mod_can.c

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    INCS_IF
        ${CMAKE_CURRENT_LIST_DIR}

    MENU_CONFIG     ${CMAKE_CURRENT_LIST_DIR}/can.config
    MENU_PROMPT     "can interface component"
    MENU_GROUP      MAIN.PLATFORM.F1.CAN
)



__sdk_add_kconfig_default(${CMAKE_CURRENT_LIST_DIR}/defaults.config)