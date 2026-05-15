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
 * @brief   ESP Modem Configuration Definitions
 *          These configurations bypass ESP-IDF's Kconfig system since custom
 *          ESP modem configurations are not recognized by standard build.
 *          Values match those in sdkconfig.espmodem.
 * --------------------------------------------------------------------------- *
 */

#ifndef ESP_MODEM_CONFIG_H
#define ESP_MODEM_CONFIG_H

// ESP Modem C API Configuration
#define CONFIG_ESP_MODEM_C_API_STR_MAX                        128

// ESP Modem CMUX Configuration  
#define CONFIG_ESP_MODEM_CMUX_DEFRAGMENT_PAYLOAD              1
#define CONFIG_ESP_MODEM_CMUX_DELAY_AFTER_DLCI_SETUP          100
#define CONFIG_ESP_MODEM_CMUX_USE_SHORT_PAYLOADS_ONLY         0

// ESP Modem Feature Configuration
// Note: CONFIG_ESP_MODEM_ADD_CUSTOM_MODULE should NOT be defined when disabled
// to ensure #ifdef CONFIG_ESP_MODEM_ADD_CUSTOM_MODULE evaluates to false
// #define CONFIG_ESP_MODEM_ADD_CUSTOM_MODULE                    0  // DISABLED: Don't define this
// When custom module is disabled, we still need to define the header to prevent preprocessor errors
// even though it won't be included due to the conditional compilation
#define CONFIG_ESP_MODEM_CUSTOM_MODULE_HEADER                 "esp_modem_config.h"
#define CONFIG_ESP_MODEM_URC_HANDLER                          1
#define CONFIG_ESP_MODEM_PPP_ESCAPE_BEFORE_EXIT               0
#define CONFIG_ESP_MODEM_ADD_DEBUG_LOGS                       0
#define CONFIG_ESP_MODEM_ENABLE_DEVELOPMENT_MODE              0
#define CONFIG_ESP_MODEM_USE_PPP_MODE                         1
#define CONFIG_ESP_MODEM_USE_INFLATABLE_BUFFER_IF_NEEDED      1

#endif // ESP_MODEM_CONFIG_H