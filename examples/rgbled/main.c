/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @author  Ahmed Sabry (SG Wireless)
 * 
 * @brief   An example demo for lora continuous reception.
 * --------------------------------------------------------------------------- *
 */

#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"

#define __log_subsystem     application
#define __log_component     main
#include "log_lib.h"
__log_subsystem_def(application, green, 1, 1)
__log_component_def(application, main, default, 1, 1)

#include "rgbled.h"

static void board_startup(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    gpio_install_isr_service(0);

    void init_log_system(void);
    init_log_system();
}

void app_main(void)
{
    board_startup();

    __log_output_header("welcome to rgbled c example", 100, '=');

    rgbled_init();

    while(true)
    {
        __log_output("(10 sec) hearbeat service for every 1 second - blue\n");
        rgbled_heartbeat_service_config(0x00000011, 1000, 5);
        rgbled_heartbeat_service_start();
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        __log_output("(10 sec) hearbeat service for every 1 second - yellow\n");
        rgbled_heartbeat_service_config(0x00111100, 1000, 5);
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        __log_output("(2 sec) set color to green\n");
        rgbled_color_u32(0x0000FF00);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        __log_output("(2 sec) set color to red\n");
        rgbled_color_u32(0x00FF0000);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        __log_output("(2 sec) set color to blue\n");
        rgbled_color_u32(0x000000FF);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        __log_output("(2 sec) set color to yellow\n");
        rgbled_color_u32(0x00FFFF00);
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        __log_output("(20 sec) decoration example in the docs\n");
        rgbled_light_cycle_desc_t deco[] = {
            //  color       period      percent repeat
            {   0x00001100, 100,        50,     2},
            {   0,          2000 - 200, 0,      1},
            {   0x00000011, 100,        50,     2},
            {   0,          2000 - 200, 0,      1},
            {   0x00110000, 500,        100,    1},
            {   0x00111100, 500,        100,    1},
            {   0,          1000,       0,      1}
        };

        rgbled_light_decoration_service_start(
            deco, sizeof(deco)/ sizeof(deco[0]), true
        );
        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}

/* --- end of file ---------------------------------------------------------- */
