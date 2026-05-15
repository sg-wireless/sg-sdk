/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @author  Christian Ehlers (SG Wireless)
 * 
 * @brief   PPP Bridge Header - Combining working PPP with ESP Modem AT Commands
 * --------------------------------------------------------------------------- *
 */

#ifndef __PPP_BRIDGE_H__
#define __PPP_BRIDGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "esp_netif.h"
#include "esp_modem_api.h"

/**
 * Initialize PPP bridge with working ESP modem DCE handle
 * 
 * @param dce_handle Valid ESP modem DCE handle (from successful AT command setup)
 * @return ESP_OK on success
 */
esp_err_t ppp_bridge_init(esp_modem_dce_t* dce_handle);

/**
 * Start PPP bridge (switch from AT command mode to PPP data mode)
 * 
 * @return ESP_OK on success
 */
esp_err_t ppp_bridge_start(void);

/**
 * Establish PPP connection (start data session)
 * 
 * @return ESP_OK on success
 */
esp_err_t ppp_bridge_connect(void);

/**
 * Check if PPP connection is established
 * 
 * @return true if connected and has IP address
 */
bool ppp_bridge_is_connected(void);

/**
 * Get IP configuration from PPP interface
 * 
 * @param ip_info Pointer to structure to fill with IP info
 * @return ESP_OK on success
 */
esp_err_t ppp_bridge_get_ip_info(esp_netif_ip_info_t* ip_info);

/**
 * Stop PPP connection and return to AT command mode
 * 
 * @return ESP_OK on success
 */
esp_err_t ppp_bridge_stop(void);

/**
 * Cleanup PPP bridge resources
 * 
 * @return ESP_OK on success
 */
esp_err_t ppp_bridge_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __PPP_BRIDGE_H__ */