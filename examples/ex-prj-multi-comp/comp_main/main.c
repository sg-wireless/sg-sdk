#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "log_lib.h"
#include "utils_time.h"

#include "comp_a.h"
#include "comp_b.h"

static void board_startup(void)
{
    void init_log_system(void);
    init_log_system();
}

void app_main(void)
{
    board_startup();

    __log_output_header("welcome to example project", 100, '=');

    while(true)
    {
        __log_output(__yellow__"project main loop"__default__"\n");
        comp_a_run();
        comp_b_run();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
