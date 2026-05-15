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
 * @brief   PPP Bridge Implementation using ESP-IDF's built-in PPP netif system
 *          Based on esp_modem examples and ESP-IDF's esp_netif_lwip_ppp.c
 * --------------------------------------------------------------------------- *
 */

#include "ppp_bridge.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

// Use SG-SDK structured logging system
#define __log_subsystem     F1
#define __log_component     ppp_bridge
#include "platform_inc/mp_lite_if.h"

// Event bits for PPP connection status
#define PPP_CONNECTED_BIT    BIT0
#define PPP_DISCONNECTED_BIT BIT1

typedef struct {
    esp_netif_t *ppp_netif;
    esp_modem_dce_t *dce_handle;
    EventGroupHandle_t event_group;
    bool initialized;
    bool connected;
} ppp_bridge_context_t;

static ppp_bridge_context_t s_ppp_bridge = {0};

/**
 * Event handler for IP events (connection/disconnection)
 */
static void ppp_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT) {
        switch (event_id) {
        case IP_EVENT_PPP_GOT_IP: {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            __log_info("PPP connected, IP acquired: " IPSTR, IP2STR(&event->ip_info.ip));
            s_ppp_bridge.connected = true;
            xEventGroupSetBits(s_ppp_bridge.event_group, PPP_CONNECTED_BIT);
            break;
        }
        case IP_EVENT_PPP_LOST_IP:
            __log_info("PPP disconnected, IP lost");
            s_ppp_bridge.connected = false;
            xEventGroupSetBits(s_ppp_bridge.event_group, PPP_DISCONNECTED_BIT);
            break;
        default:
            break;
        }
    }
}

esp_err_t ppp_bridge_init(esp_modem_dce_t* dce_handle)
{
    if (!dce_handle) {
        __log_error("Invalid DCE handle");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_ppp_bridge.initialized) {
        __log_warn("PPP bridge already initialized");
        return ESP_OK;
    }
    
    __log_info("Initializing PPP bridge using ESP-IDF's built-in PPP netif system");
    
    // Store DCE handle
    s_ppp_bridge.dce_handle = dce_handle;
    
    // Create event group for PPP connection status
    s_ppp_bridge.event_group = xEventGroupCreate();
    if (!s_ppp_bridge.event_group) {
        __log_error("Failed to create event group");
        return ESP_ERR_NO_MEM;
    }
    
    // Configure PPP netif using ESP-IDF's default configuration
    esp_netif_config_t ppp_netif_config = ESP_NETIF_DEFAULT_PPP();
    s_ppp_bridge.ppp_netif = esp_netif_new(&ppp_netif_config);
    
    if (!s_ppp_bridge.ppp_netif) {
        __log_error("Failed to create PPP netif");
        vEventGroupDelete(s_ppp_bridge.event_group);
        return ESP_ERR_NO_MEM;
    }
    
    // Register event handler for IP events
    esp_err_t err = esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, 
                                              &ppp_event_handler, NULL);
    if (err != ESP_OK) {
        __log_error("Failed to register IP event handler: %s", esp_err_to_name(err));
        esp_netif_destroy(s_ppp_bridge.ppp_netif);
        vEventGroupDelete(s_ppp_bridge.event_group);
        return err;
    }
    
    s_ppp_bridge.initialized = true;
    s_ppp_bridge.connected = false;
    
    __log_info("PPP bridge initialized successfully");
    return ESP_OK;
}

esp_err_t ppp_bridge_start(void)
{
    if (!s_ppp_bridge.initialized) {
        __log_error("PPP bridge not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    __log_info("Starting PPP bridge - switching ESP modem to CMUX mode");
    
    // Switch ESP modem to CMUX mode to enable simultaneous AT commands and data
    esp_err_t err = esp_modem_set_mode(s_ppp_bridge.dce_handle, ESP_MODEM_MODE_CMUX);
    if (err != ESP_OK) {
        __log_error("Failed to set CMUX mode: %s", esp_err_to_name(err));
        return err;
    }
    
    __log_info("CMUX mode activated - ESP modem now supports concurrent AT commands and PPP data");
    return ESP_OK;
}

esp_err_t ppp_bridge_connect(void)
{
    if (!s_ppp_bridge.initialized) {
        __log_error("PPP bridge not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    __log_info("Establishing PPP connection using ESP-IDF's netif system");
    
    // The ESP modem library should automatically handle PPP connection
    // when in CMUX mode through the netif interface
    // We just need to wait for the connection event
    
    return ESP_OK;
}

bool ppp_bridge_is_connected(void)
{
    return s_ppp_bridge.connected;
}

esp_err_t ppp_bridge_get_ip_info(esp_netif_ip_info_t* ip_info)
{
    if (!s_ppp_bridge.initialized || !s_ppp_bridge.ppp_netif) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!s_ppp_bridge.connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    return esp_netif_get_ip_info(s_ppp_bridge.ppp_netif, ip_info);
}

esp_err_t ppp_bridge_stop(void)
{
    if (!s_ppp_bridge.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    __log_info("Stopping PPP bridge");
    
    // Switch ESP modem back to command mode
    esp_err_t err = esp_modem_set_mode(s_ppp_bridge.dce_handle, ESP_MODEM_MODE_COMMAND);
    if (err != ESP_OK) {
        __log_warn("Failed to switch back to command mode: %s", esp_err_to_name(err));
    }
    
    s_ppp_bridge.connected = false;
    xEventGroupSetBits(s_ppp_bridge.event_group, PPP_DISCONNECTED_BIT);
    
    return ESP_OK;
}

esp_err_t ppp_bridge_deinit(void)
{
    if (!s_ppp_bridge.initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    __log_info("Deinitializing PPP bridge");
    
    // Stop first if still connected
    if (s_ppp_bridge.connected) {
        ppp_bridge_stop();
    }
    
    // Unregister event handler
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &ppp_event_handler);
    
    // Clean up resources
    if (s_ppp_bridge.ppp_netif) {
        esp_netif_destroy(s_ppp_bridge.ppp_netif);
        s_ppp_bridge.ppp_netif = NULL;
    }
    
    if (s_ppp_bridge.event_group) {
        vEventGroupDelete(s_ppp_bridge.event_group);
        s_ppp_bridge.event_group = NULL;
    }
    
    s_ppp_bridge.initialized = false;
    s_ppp_bridge.connected = false;
    s_ppp_bridge.dce_handle = NULL;
    
    __log_info("PPP bridge deinitialized");
    return ESP_OK;
}

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_netif.h"
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppos.h"
#include "lwip/err.h"
#include "netif/ppp/pppapi.h"

#include "driver/uart.h"
#include "esp_modem_api.h"

#define __log_subsystem     F1
#define __log_component     ppp_bridge
#include "log_lib.h"
__log_component_def(F1, ppp_bridge, cyan, 1, 0)

// PPP Bridge State
typedef struct {
    bool active;
    bool connected;
    volatile bool clean_close;
    ppp_pcb *pcb;
    SemaphoreHandle_t uart_mutex;
    volatile TaskHandle_t client_task_handle;
    struct netif pppif;
    esp_modem_dce_t* dce_handle;  // Reference to ESP modem DCE
} ppp_bridge_t;

static ppp_bridge_t s_ppp_bridge = {0};

// Ahmed's proven PPP status callback
static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx) {
    ppp_bridge_t *obj = ctx;
    struct netif *pppif = ppp_netif(obj->pcb);

    switch (err_code) {
        case PPPERR_NONE:
            obj->connected = (pppif->ip_addr.u_addr.ip4.addr != 0);
            __log_info("PPP connected, IP acquired");
            break;
        case PPPERR_USER:
            obj->clean_close = true;
            __log_info("PPP clean close requested");
            break;
        case PPPERR_CONNECT:
            obj->connected = false;
            __log_error("PPP connection failed");
            break;
        default:
            __log_warn("PPP status: %d", err_code);
            break;
    }
}

// Direct UART output for PPP (bypassing ESP modem for data)
static u32_t ppp_output_callback(ppp_pcb *pcb, const void *data, u32_t len, void *ctx)
{
    // Write directly to UART1 (same as ESP modem uses)
    int written = uart_write_bytes(UART_NUM_1, (const char*)data, len);
    if (written < 0) {
        __log_error("UART write failed");
        return 0;
    }
    __log_debug("PPP-TX: %d bytes", written);
    return written;
}

// PPP client task - reads from UART and feeds to PPP stack
static void pppos_client_task(void *self_in) {
    uint8_t buf[256];
    __log_info("PPP client task started");

    while (ulTaskNotifyTake(pdTRUE, 0) == 0) {
        // Read directly from UART1 (same as ESP modem uses)
        int len = uart_read_bytes(UART_NUM_1, buf, sizeof(buf), 10 / portTICK_PERIOD_MS);
        if (len > 0) {
            __log_debug("PPP-RX: %d bytes", len);
            // Feed data to PPP stack
            pppos_input_tcpip(s_ppp_bridge.pcb, (u8_t *)buf, len);
        }
    }

    __log_info("PPP client task ending");
    s_ppp_bridge.client_task_handle = NULL;
    vTaskDelete(NULL);
}

// Initialize PPP bridge
esp_err_t ppp_bridge_init(esp_modem_dce_t* dce_handle) {
    if (s_ppp_bridge.active) {
        return ESP_OK;
    }
    
    __log_info("Initializing PPP bridge with working ESP modem DCE");
    
    s_ppp_bridge.dce_handle = dce_handle;
    s_ppp_bridge.active = false;
    s_ppp_bridge.connected = false;
    s_ppp_bridge.clean_close = false;
    s_ppp_bridge.client_task_handle = NULL;
    
    // Create mutex for UART coordination
    s_ppp_bridge.uart_mutex = xSemaphoreCreateMutex();
    if (!s_ppp_bridge.uart_mutex) {
        __log_error("Failed to create UART mutex");
        return ESP_ERR_NO_MEM;
    }
    
    return ESP_OK;
}

// Start PPP mode (after successful ESP modem attachment)
esp_err_t ppp_bridge_start(void) {
    if (s_ppp_bridge.active) {
        __log_warn("PPP already active");
        return ESP_OK;
    }
    
    __log_info("Starting PPP bridge - switching from AT to PPP mode");
    
    // Step 1: Tell ESP modem to enter data mode (this is what was crashing before)
    // Instead of using ESP modem's PPP, we'll use Ahmed's proven implementation
    
    // Step 2: Create PPP interface using Ahmed's proven approach
    s_ppp_bridge.pcb = pppapi_pppos_create(&s_ppp_bridge.pppif,
        ppp_output_callback, ppp_status_cb, &s_ppp_bridge);

    if (s_ppp_bridge.pcb == NULL) {
        __log_error("Failed to create PPP interface");
        return ESP_FAIL;
    }
    
    s_ppp_bridge.active = true;
    __log_info("PPP bridge activated successfully");
    return ESP_OK;
}

// Connect PPP (establish data session)
esp_err_t ppp_bridge_connect(void) {
    if (!s_ppp_bridge.active) {
        __log_error("PPP bridge not active");
        return ESP_ERR_INVALID_STATE;
    }

    if (s_ppp_bridge.client_task_handle != NULL) {
        __log_warn("PPP already connecting/connected");
        return ESP_OK;
    }

    __log_info("Establishing PPP connection");

    // Set PPP as default interface
    if (pppapi_set_default(s_ppp_bridge.pcb) != ESP_OK) {
        __log_error("Failed to set PPP as default");
        return ESP_FAIL;
    }

    // Enable DNS
    ppp_set_usepeerdns(s_ppp_bridge.pcb, true);

    // Start PPP connection
    if (pppapi_connect(s_ppp_bridge.pcb, 0) != ESP_OK) {
        __log_error("PPP connect failed");
        return ESP_FAIL;
    }

    // Create PPP client task using Ahmed's approach
    if (xTaskCreate(pppos_client_task, "ppp_bridge", 2048, &s_ppp_bridge, 5,
        (TaskHandle_t *)&s_ppp_bridge.client_task_handle) != pdPASS) {
        __log_error("Failed to create PPP client task");
        return ESP_ERR_NO_MEM;
    }

    __log_info("PPP connection initiated successfully");
    return ESP_OK;
}

// Check if PPP is connected
bool ppp_bridge_is_connected(void) {
    return s_ppp_bridge.connected;
}

// Get IP configuration
esp_err_t ppp_bridge_get_ip_info(esp_netif_ip_info_t* ip_info) {
    if (!s_ppp_bridge.connected || !ip_info) {
        return ESP_ERR_INVALID_STATE;
    }
    
    struct netif *pppif = ppp_netif(s_ppp_bridge.pcb);
    if (!pppif) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ip_info->ip.addr = pppif->ip_addr.u_addr.ip4.addr;
    ip_info->gw.addr = pppif->gw.u_addr.ip4.addr;
    ip_info->netmask.addr = pppif->netmask.u_addr.ip4.addr;
    
    return ESP_OK;
}

// Stop PPP connection
esp_err_t ppp_bridge_stop(void) {
    if (!s_ppp_bridge.active) {
        return ESP_OK;
    }
    
    __log_info("Stopping PPP bridge");
    
    // Stop PPP task and connection
    if (s_ppp_bridge.client_task_handle != NULL) {
        pppapi_close(s_ppp_bridge.pcb, 0);
        
        // Wait for clean close
        uint32_t timeout = 4000; // 4 seconds
        uint32_t start = xTaskGetTickCount() * portTICK_PERIOD_MS;
        while (!s_ppp_bridge.clean_close && 
               (xTaskGetTickCount() * portTICK_PERIOD_MS - start) < timeout) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }

        // Stop task
        xTaskNotifyGive(s_ppp_bridge.client_task_handle);
        start = xTaskGetTickCount() * portTICK_PERIOD_MS;
        while (s_ppp_bridge.client_task_handle != NULL && 
               (xTaskGetTickCount() * portTICK_PERIOD_MS - start) < timeout) {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }

    // Release PPP interface
    if (s_ppp_bridge.pcb) {
        pppapi_free(s_ppp_bridge.pcb);
        s_ppp_bridge.pcb = NULL;
    }
    
    s_ppp_bridge.active = false;
    s_ppp_bridge.connected = false;
    s_ppp_bridge.clean_close = false;
    
    __log_info("PPP bridge stopped");
    return ESP_OK;
}

// Cleanup
esp_err_t ppp_bridge_deinit(void) {
    ppp_bridge_stop();
    
    if (s_ppp_bridge.uart_mutex) {
        vSemaphoreDelete(s_ppp_bridge.uart_mutex);
        s_ppp_bridge.uart_mutex = NULL;
    }
    
    __log_info("PPP bridge deinitialized");
    return ESP_OK;
}