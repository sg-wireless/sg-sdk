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
 * @brief   This file contains the main hosttest of the logs libraray
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdio.h>
#include "log_lib.h"

/* --- subsystems/components definitions examples --------------------------- */

__log_subsystem_def(lora, blue, 1, 1)
__log_component_def(lora, spi, blue, 1, 1)

__log_subsystem_def(lte, default, 1, 1)
__log_component_def(lte, uart, green, 1, 1)

/* --- testing implementation ----------------------------------------------- */

static void ____long_function_name_____(void)
{
    __log_info("long function started");
    __log_info("long function started");
}

int main(void)
{
    log_init(NULL);
    __log_info("hello");

    #undef __log_subsystem
    #define __log_subsystem   lte
    __log_info("another");

    #undef __log_component
    #define __log_component   uart

    __log_output("sajdhjkb\n");

    __log_info("%c %c \n(%+05.6d)", 'y', 'g', 0x44);
    __log_info("%c %c \n(%+010.6d)", 'y', 'g', 0x44);
    __log_info("%c %c \n(%+05.6d)", 'y', 'g', 0x44);

    int arr[] = {0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5, 'r', 'y','a','z','%'};

    #undef __log_subsystem
    #define __log_subsystem   lora
    #undef __log_component
    #define __log_component   spi

    __log_dump((void*)arr,sizeof(arr),16,
        __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs,
        __word_len_8);

    ____long_function_name_____();

    __log_info("this is a long line to %Crbe lo%Cdgged%Cy out"
            "aslkdhs ;dshfalkjd h\n\tal;kdhf%Cd dkhf;ldaskj hldjh dlfjhdf"
            "dsfjh kjh jdhlj hdljk hdj dk djhljdh jldh d");
        
    __log_debug("This is a debug message %d -- %d", 100, 1000);
    __log_warn("This is a warning message %d -- %d", 100, 1000);
    __log_error("This is an %Cger%Cpror%Cd message %d -- %d", 100, 1000);

    log_filter_list_stats();

    __log_printf("%10s", "some");
    __log_printf("%Cc%+10d", 1234);
    __log_printf("\n");

    __log_printf("\n\t%Cd(=%Cy%p%Cd=)\n", arr);

    __log_printf("%+10d", 1234);
    __log_printf("%+10d%Cd default color %Cpnot \nagain\n", 1234);
    __log_output("\n\n");
    log_filter_header("compoent", false);
    __log_info(" -- ");
    __log_info(" -- ");
    __log_info(" -- ");
    __log_output("\n\n");

    log_filter_header("component", false);
    __log_info(" -- ");
    __log_info(" -- ");
    __log_info(" -- ");
    __log_output("\n\n");

    log_filter_header("subsystem", false);
    __log_info(" -- ");
    __log_info(" -- ");
    __log_info(" -- ");
    __log_output("\n\n");

    log_filter_list_stats();

    log_filter_header("component", true);
    __log_info(" -- ");
    __log_info(" -- ");
    __log_info(" -- ");
    __log_output("\n\n");

    log_filter_type("info", false);
    log_filter_type("error", true);

    log_filter_list_stats();


    log_filter_subsystem("lora", true);
    log_filter_component("lora", "spi", false);

    log_filter_list_stats();

    log_filter_header("timestamp", false);
    __log_info(" -- ");
    __log_info(" -- ");
    __log_info(" -- ");
    __log_output("\n\n");

    __log_warn("warning");
    __log_assert(0, "asserrt");

    printf("\n\n");

    #undef __log_subsystem
    #define __log_subsystem   lte
    #undef __log_component
    #define __log_component   uart


    __log_dump((void*)arr,sizeof(arr),16,
        __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs,
        __word_len_8);

    __log_dump((void*)arr,sizeof(arr),16,
        __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs,
        __word_len_16);

    __log_dump((void*)arr,sizeof(arr),16,
        __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs,
        __word_len_32);
    
    __log_printf("->|%-10.3f|<-", -0.14);
    __log_endl();
    __log_printf("->|%-+10.3f|<-", 0.14);
    __log_endl();
    __log_printf("->|%10.4f|<-", -0.14);
    __log_endl();
    __log_printf("->|%010.1f|<-", -0.14);
    __log_endl();
    __log_printf("->|%+10.3f|<-", 0.14);
    __log_endl();
    __log_printf("->|%+10.2f|<-", 0.14);
    __log_endl();
    __log_printf("->|%+010.3f|<-", 0.14);
    __log_endl();
    __log_printf("->|%+010.4f|<-", 1.14);
    __log_endl();
    __log_printf("->|%+010.4f|<-", 1.004);
    __log_endl();
    __log_printf("->|%+010d|<-", 14);
    __log_endl();

    __log_printf("->|%+7.5d|<-", 14);
    __log_endl();

    return 0;
}

/* --- end of file ---------------------------------------------------------- */
