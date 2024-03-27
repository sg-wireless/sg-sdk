#define MICROPY_HW_BOARD_NAME               "SGWireless "SDK_BOARD
#define MICROPY_HW_MCU_NAME                 "ESP32S3"

#define MICROPY_PY_MACHINE_DAC              (0)

// Enable UART REPL for modules that have an external USB-UART and don't use native USB.
#define MICROPY_HW_ENABLE_UART_REPL         (1)

#define MICROPY_HW_I2C0_SCL                 (7)
#define MICROPY_HW_I2C0_SDA                 (8)

#define DEFAULT_AP_SSID                     "f1-wlan"

// board specifics
#define MICROPY_PY_SYS_PLATFORM 			SDK_BOARD_NAME
