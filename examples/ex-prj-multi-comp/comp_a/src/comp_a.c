

#include "log_lib.h"

void comp_a_run(void)
{
    #ifdef CONFIG_EXAMPLE_PROJECT_COMP_A_ENABLE
    __log_output(__green__"-- running comp a -- its config integer "
        __yellow__"%d\n"__default__, CONFIG_EXAMPLE_PROJECT_COMP_A_INTEGER);
    #else
    __log_output(__red__"-- comp a is disabled\n"__default__);
    #endif
}
