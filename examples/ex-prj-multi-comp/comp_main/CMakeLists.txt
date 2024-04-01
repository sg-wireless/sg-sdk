__sdk_add_component( user_comp_main
    SRCS        "${CMAKE_CURRENT_LIST_DIR}/*.c"
    REQUIRED_SDK_LIBS
        log_lib     utils_lib   f1_logs_if  user_comp_a user_comp_b
    REQUIRED_ESP_LIBS
        freertos    spi_flash
)
