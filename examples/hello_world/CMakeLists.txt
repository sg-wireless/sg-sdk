
__sdk_add_component( hello_world

    SRCS
        "${CMAKE_CURRENT_LIST_DIR}/*.c"

    LOGS_DEFS
        "${CMAKE_CURRENT_LIST_DIR}/main.c"

    INCS_PRIV
        ${__dir_ext}/esp-idf/components/log/include
        ${__dir_ext}/esp-idf/components/spi_flash/include
        ${__dir_ext}/esp-idf/components/nvs_flash/include

    REQUIRED_ESP_LIBS
        freertos
        spi_flash
    
    REQUIRED_SDK_LIBS
        log_lib
        f1_logs_if
)
