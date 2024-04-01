/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This file is copied and modified from ESP-IDF examples
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"

#include "log_lib.h"

void app_main(void)
{
    // -- init logging system
    void init_log_system(void);
    init_log_system();

    __log_output_header("welcome to hello world example", 100, '=');

    // -- display the current logging stats
    log_filter_list_stats();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    __log_output("This is "__yellow__"%s"__default__
            " chip with "__green__"%d"__default__" CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    unsigned major_rev = chip_info.full_revision / 100;
    unsigned minor_rev = chip_info.full_revision % 100;
    __log_output("silicon revision v%d.%d, ", major_rev, minor_rev);

    __log_output("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    __log_output("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

    for (int i = 3; i >= 0; i--) {
        __log_output("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    __log_output("Restarting now.\n");
    __log_output_fill(100, '=', true);
    // fflush(stdout);
    esp_restart();
}
