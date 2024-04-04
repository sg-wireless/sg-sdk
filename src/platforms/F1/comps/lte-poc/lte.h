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
 * @author  Ahmed Sabry (SG Wireless)       -- Translated it into C
 *
 * @brief   This file is a pure translation to "LTE.py" interface to C language
 * --------------------------------------------------------------------------- *
 */
#ifndef __LTE_H__
#define __LTE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "lte_ppp.h"

/** -------------------------------------------------------------------------- *
 * APIs as defined in "LTE.py"
 * --------------------------------------------------------------------------- *
 */

typedef enum {
    __LTE_OK = 0,
    __LTE_ERROR = 1,
    __LTE_SMALL_BUF = 2,
} lte_error_t;

typedef enum {
    __LTE_MODE__CATM1 = 0,
    __LTE_MODE__NBIOT = 1
} lte_mode_t;

#define __lte_init_default_carrier  "standard"
#define __lte_init_default_cid      1
#define __lte_init_default_mode     __LTE_MODE__CATM1
#define __lte_init_default_baudrate 115200     
#define __lte_init_default_debug    false

lte_error_t lte_init(const char* carrier, uint32_t cid, lte_mode_t mode,
    uint32_t baudrate, bool debug);

bool lte_check_power(void);

void lte_print_pretty_response(const char* rsp, bool flush, const char* prefix);

int lte_return_pretty_response(const char* resp, char* out_buf, uint32_t len);

#define __lte_read_rsp_default_timeout          (-1)
#define __lte_read_rsp_default_wait_ok_error    (false)
#define __lte_read_rsp_default_check_error      (false)
lte_error_t lte_read_rsp(int timeout, bool wait_ok_error, bool check_error,
    char* rsp, int rsp_buf_size);

#define __lte_send_at_cmd_def_cmd           "AT"
#define __lte_send_at_cmd_def_timeout       (-1)
#define __lte_send_at_cmd_def_wait_ok_error false
#define __lte_send_at_cmd_def_check_error   false
lte_error_t lte_send_at_cmd(const char* cmd, int timeout,
    bool wait_ok_error, bool check_error, char* rsp, int resp_size);

lte_error_t lte_get_mode(int * p_mode);

#define __lte_attach_default_apn    NULL
#define __lte_attach_default_type   "IP"
#define __lte_attach_default_cid    (-1)
#define __lte_attach_default_band   (-1)
#define __lte_attach_default_bands  NULL
lte_error_t lte_attach(const char* apn, const char* type, int cid,
    int band, int* bands, int bands_count);

bool lte_is_attached(void);

lte_error_t lte_reset(void);

#define __lte_connect_default_cid   (-1)
lte_error_t lte_connect(int cid);

lte_error_t lte_mode(lte_mode_t new_mode);

lte_error_t lte_ifconfig(lte_ppp_ifconfig_t * p_if_config);

lte_error_t lte_disconnect(void);

bool lte_is_connected(void);

bool lte_isconnected(void);

bool lte_isattached(void);

lte_error_t lte_detach(void);

lte_error_t lte_deinit(void);

bool lte_check_sim_present(void);

bool lte_power_on(bool wait_ok);

lte_error_t lte_power_off(bool force);

lte_error_t lte_pause_ppp(void);

lte_error_t lte_resume_ppp(void);

bool lte_check_ppp(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LTE_H__ */
