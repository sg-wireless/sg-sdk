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
 * @brief   This file contains the extended interface for the log type test.
 * --------------------------------------------------------------------------- *
 */

#ifndef __LOG_TEST_H__
#define __LOG_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -- includes -------------------------------------------------------------- */

#include "log_lib.h"

/* -- macros ---------------------------------------------------------------- */

#define __log_test(id, verdict, test_case_name, msg...) \
    __opt_paste(__get_log_type_opt(test), y,        \
        do {                                        \
            log_info_test_t info = {                \
                { .p_type_info = &g_log_type_test },\
                id, verdict, test_case_name         \
            };                                      \
            __log(test, (void*)&info, msg);         \
        } while(0)                                  \
    )

/* -- typedefs -------------------------------------------------------------- */

typedef struct {
    log_info_t      log_info_base;
    uint32_t        test_id;
    bool            verdict;
    const char*     test_case_name;
} log_info_test_t;

/* -- type declaration ------------------------------------------------------ */

__log_type_dec(test);

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LOG_MEMDUMP_H__ */
