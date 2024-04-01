#include <stdio.h>
#include <stdbool.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "log_lib.h"
#include "utils_time.h"

#include "head_1.h"
#include "head_2.h"

static void board_startup(void)
{
    void init_log_system(void);
    init_log_system();
}

static void demo_project_definitions(void)
{
    bool is_x_defined =
    #ifdef CONFIG_EXAMPLE_PROJECT_DEF_X
    true;
    #else
    false;
    #endif

    bool is_y_defined =
    #ifdef CONFIG_EXAMPLE_PROJECT_DEF_Y
    true;
    #else
    false;
    #endif

    bool is_z_defined =
    #ifdef CONFIG_EXAMPLE_PROJECT_DEF_Z
    true;
    #else
    false;
    #endif

    __log_output("project def --> "__cyan__
        "CONFIG_EXAMPLE_PROJECT_DEF_X"__default__" = %s\n",
        g_yes_no[is_x_defined]);
    __log_output("project def --> "__cyan__
        "CONFIG_EXAMPLE_PROJECT_DEF_Y"__default__" = %s\n",
        g_yes_no[is_y_defined]);
    __log_output("project def --> "__cyan__
        "CONFIG_EXAMPLE_PROJECT_DEF_Z"__default__" = %s\n",
        g_yes_no[is_z_defined]);
}

static void demo_patched_lib(void)
{
    uint32_t t_gps = 12345678;
    uint32_t t_unx = time_gps_to_unix(t_gps);
    uint32_t t_gps_2 = time_unix_to_gps(t_unx);
    __log_output("conversion is correct? %s\n", g_yes_no[t_gps == t_gps_2]);
}

void app_main(void)
{
    board_startup();

    __log_output_header("welcome to example project", 100, '=');

    while(true)
    {
        example_function_a();
        example_function_b();

        demo_project_definitions();

        demo_patched_lib();

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
