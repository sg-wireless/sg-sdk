
__sdk_add_component( project_x_main
    SRCS        "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
    INCS_PRIV   ${CMAKE_CURRENT_LIST_DIR}/inc
    MPY_MODS
        ${CMAKE_CURRENT_LIST_DIR}/src/mpy_ex_c_module.c
        ${CMAKE_CURRENT_LIST_DIR}/src/mpy_unused_app_main.c
    REQUIRED_SDK_LIBS
        log_lib     utils_lib   f1_logs_if
    REQUIRED_ESP_LIBS
        freertos    spi_flash
    MENU_CONFIG
        ${CMAKE_CURRENT_LIST_DIR}/cfg/kconfig
)

__sdk_add_micropython_frozen_manifest(
    ${CMAKE_CURRENT_LIST_DIR}/mpy/manifest.py )

__sdk_add_kconfig_default( ${CMAKE_CURRENT_LIST_DIR}/cfg/kconfig.default )

set(__patches_dir ${CMAKE_CURRENT_LIST_DIR}/patches) # to shorten paths

__sdk_add_patch(
    ENTITY_NAME    utils_lib
    ORIGINAL_FILE  ${__dir_src}/libs/utils/utils_time.c
    PATCH_FILE     ${__patches_dir}/utils_lib/utils_time.c.patch
)
