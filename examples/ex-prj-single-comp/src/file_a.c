
#include "log_lib.h"

void example_function_a(void)
{
    __log_output("-- call >> %s()\n", __func__);
}
