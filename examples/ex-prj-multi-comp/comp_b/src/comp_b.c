

#include "log_lib.h"

void comp_b_run(void)
{
    #ifdef CONFIG_EXAMPLE_PROJECT_COMP_B_ENABLE
    __log_output(__cyan__"-- running comp b -- its config integer "
        __yellow__"%d\n"__default__, CONFIG_EXAMPLE_PROJECT_COMP_B_INTEGER);
    #else
    __log_output(__red__"-- comp b is disabled\n"__default__);
    #endif
}
