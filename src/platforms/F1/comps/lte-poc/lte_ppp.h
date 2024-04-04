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
 * @brief   This file represents the interface to the PPP client for LTE
 *          data exchange with the TCP/IP stack.
 *          IMPORTANT to mension:
 *              The source code of this file is originally copied from the
 *              micropython project specifically from this file:
 *                  micropython/ports/esp32/network_ppp.c
 *              and modified for the 
 * --------------------------------------------------------------------------- *
 */
#ifndef __LTE_PPP_H__
#define __LTE_PPP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

void lte_ppp_init(void);
void lte_ppp_deinit(void);

void lte_ppp_activate(void);
void lte_ppp_deactivate(void);
bool lte_ppp_is_active(void);

#define __lte_pp_connect_authmode_default   0
#define __lte_pp_connect_username_default   NULL
#define __lte_pp_connect_password_default   NULL
void lte_ppp_connect(int authmode, const char* username, const char* password);

typedef struct {
    /* address format  b3.b2.b1.b0" */
    uint8_t ip_addr[4];
    uint8_t gw[4];
    uint8_t netmask[4];
    uint8_t dns[4];
} lte_ppp_ifconfig_t;
void lte_ppp_get_ifconfig(lte_ppp_ifconfig_t* p_ifconfig);

bool lte_ppp_isconnected(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LTE_PPP_H__ */
