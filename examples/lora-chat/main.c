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

#include "lora.h"

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

    void ioexp_init(void);
    ioexp_init();
}

void app_main(void)
{
    board_startup();

    __log_output_header("welcome to lora c example", 100, '=');

    // -- init lora stack
    lora_ctor();

    lora_connect_callback_stub();

    __log_output("-- change lora mode to lora-raw\n");
    lora_change_mode(__LORA_MODE_RAW);

    lora_raw_param_t param = {
        .type   = __LORA_RAW_PARAM_REGION,
        .region = __LORA_REGION_EU868
    };

    __log_output("-- set the region to EU-868 with default configuration\n");
    lora_ioctl(__LORA_IOCTL_SET_PARAM, &param);

    lora_stats();

    __log_output("-- start rx continuous mode\n");
    lora_ioctl(__LORA_IOCTL_RX_CONT_START, NULL);

    while(true)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/* --- end of file ---------------------------------------------------------- */
