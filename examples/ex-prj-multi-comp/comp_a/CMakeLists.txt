__sdk_add_component( user_comp_a
    SRCS        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
    INCS_IF     ${CMAKE_CURRENT_LIST_DIR}/inc
    REQUIRED_SDK_LIBS
        log_lib     utils_lib   f1_logs_if
    MENU_CONFIG
        ${CMAKE_CURRENT_LIST_DIR}/kconfig
)
