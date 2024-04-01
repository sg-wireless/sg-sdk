
#include "log_lib.h"

void example_function_b(void)
{
    __log_output("-- call >> %s()\n", __func__);
}
