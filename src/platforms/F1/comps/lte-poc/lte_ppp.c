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
 * @brief   This file implements the interface to the PPP client for LTE
 *          data exchange with the TCP/IP stack.
 *          IMPORTANT to mension:
 *              The source code of this file is originally copied from the
 *              micropython project specifically from this file:
 *                  micropython/ports/esp32/network_ppp.c
 *              and modified for the following purposes:
 *                  - to be independent of micropython interfaces
 *                  - to be a single object for LTE PoC implementation.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "esp_system.h"

// #include "esp_wifi.h"
#include "esp_netif.h"

#include "netif/ppp/ppp.h"
#include "netif/ppp/pppos.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "netif/ppp/pppapi.h"

#include "lte_uart.h"
#include "lte_ppp.h"

#define __log_subsystem     lte
#define __log_component     ppp
#include "log_lib.h"
__log_component_def(lte, ppp, yellow, 1, 1);


/** -------------------------------------------------------------------------- *
 * definitions and data structs
 * --------------------------------------------------------------------------- *
 */
#define PPP_CLOSE_TIMEOUT_MS (4000)

typedef struct _ppp_if_obj_t {
    bool active;
    bool connected;
    volatile bool clean_close;
    ppp_pcb *pcb;
    void*   stream;
    SemaphoreHandle_t inactiveWaitSem;
    volatile TaskHandle_t client_task_handle;
    struct netif pppif;
} ppp_if_obj_t;

static ppp_if_obj_t s_ppp_obj;

/** -------------------------------------------------------------------------- *
 * time handling
 * --------------------------------------------------------------------------- *
 */
#define __hal_timestamp_ms()    (esp_timer_get_time() / 1000)
#define __hal_delay_ms(ms)  do{ vTaskDelay(ms/portTICK_PERIOD_MS); } while(0)

/** -------------------------------------------------------------------------- *
 * implementation of the LTE PPP client
 * --------------------------------------------------------------------------- *
 */

static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx) {
    ppp_if_obj_t *obj = ctx;
    struct netif *pppif = ppp_netif(obj->pcb);

    switch (err_code) {
        case PPPERR_NONE:
            obj->connected = (pppif->ip_addr.u_addr.ip4.addr != 0);
            break;
        case PPPERR_USER:
            obj->clean_close = true;
            break;
        case PPPERR_CONNECT:
            obj->connected = false;
            break;
        default:
            break;
    }
}

static bool s_initialized = false;

static void esp_initialize()
{
    static int initialized = 0;
    if (!initialized) {
        __log_info("lte-ppp --> Initializing TCP/IP");
        esp_netif_init();
        initialized = 1;
    }
}

void lte_ppp_init(void)
{
    if(s_initialized) {
        return;
    }
    __log_info("ctor() -> ppp");
    esp_initialize();
    s_ppp_obj.active = false;
    s_ppp_obj.connected = false;
    s_ppp_obj.clean_close = false;
    s_ppp_obj.client_task_handle = NULL;

    s_initialized = true;
}

static u32_t ppp_output_callback(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{
    return lte_uart_write(data, len);
}

static void pppos_client_task(void *self_in) {
    uint8_t buf[256];

    while (ulTaskNotifyTake(pdTRUE, 0) == 0) {
        int len = lte_uart_read(buf, sizeof(buf), 0);
        if (len > 0) {
            pppos_input_tcpip(s_ppp_obj.pcb, (u8_t *)buf, len);
        }
    }

    s_ppp_obj.client_task_handle = NULL;
    vTaskDelete(NULL);
}

void lte_ppp_activate(void)
{
    if (s_ppp_obj.active) {
        return;
    }

    s_ppp_obj.pcb = pppapi_pppos_create(&s_ppp_obj.pppif,
        ppp_output_callback, ppp_status_cb, &s_ppp_obj);

    if (s_ppp_obj.pcb == NULL)
    {
        __log_error("failed to activate ppp");
        return;
    }
    s_ppp_obj.active = true;
}

void lte_ppp_deactivate(void)
{
    if (!s_ppp_obj.active) {
        return;
    }

    if (s_ppp_obj.client_task_handle != NULL) {
        // is connecting or connected?
        // Wait for PPPERR_USER, with timeout
        pppapi_close(s_ppp_obj.pcb, 0);
        uint32_t t0 = __hal_timestamp_ms();
        while (!s_ppp_obj.clean_close &&
            __hal_timestamp_ms() - t0 < PPP_CLOSE_TIMEOUT_MS)
        {
            __hal_delay_ms(10);
        }

        // Shutdown task
        xTaskNotifyGive(s_ppp_obj.client_task_handle);
        t0 = __hal_timestamp_ms();
        while (s_ppp_obj.client_task_handle != NULL && 
            __hal_timestamp_ms() - t0 < PPP_CLOSE_TIMEOUT_MS)
        {
            __hal_delay_ms(10);
        }
    }

    // Release PPP
    pppapi_free(s_ppp_obj.pcb);
    s_ppp_obj.pcb = NULL;
    s_ppp_obj.active = false;
    s_ppp_obj.connected = false;
    s_ppp_obj.clean_close = false;
}

bool lte_ppp_is_active(void)
{
    return s_ppp_obj.active;
}

void lte_ppp_connect(int authmode, const char* username, const char* password)
{
    if (!s_ppp_obj.active) {
        __log_error("ppp must be active");
        return;
    }

    if (s_ppp_obj.client_task_handle != NULL) {
        __log_error("ppp already");
        return;
    }

    switch (authmode) {
        case PPPAUTHTYPE_NONE:
        case PPPAUTHTYPE_PAP:
        case PPPAUTHTYPE_CHAP:
            break;
        default:
            __log_error("invalid authmode");
            return;
    }

    if (authmode != PPPAUTHTYPE_NONE) {
        if(! username || ! password) {
            __log_error("missing username or password with authmode: %d",
                authmode);
        }
        pppapi_set_auth(s_ppp_obj.pcb, authmode, username, password);
    }
    if (pppapi_set_default(s_ppp_obj.pcb) != ESP_OK) {
        __log_error("set default failed");
        return;
    }

    ppp_set_usepeerdns(s_ppp_obj.pcb, true);

    if (pppapi_connect(s_ppp_obj.pcb, 0) != ESP_OK) {
        __log_error("connect failed");
        return;
    }

    if (xTaskCreate(pppos_client_task, "ppp", 2048, &s_ppp_obj, 1,
        (TaskHandle_t *)&s_ppp_obj.client_task_handle) != pdPASS) {
        __log_error("failed to create worker task");
    }

    return;
}

void lte_ppp_delete(void)
{
    if(!s_initialized) {
        return;
    }
    __log_info("dtor() -> ppp");
    lte_ppp_deactivate();

    s_initialized = false;
}

static void copy_ip_addr(uint8_t* dst, uint8_t* src, bool is_little )
{
    if(src && dst)
    {
        if(is_little)
        {
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            dst[3] = src[3];
        }
        else
        {
            dst[0] = src[3];
            dst[1] = src[2];
            dst[2] = src[1];
            dst[3] = src[0];
        }
    }
}

void lte_ppp_get_ifconfig(lte_ppp_ifconfig_t* p_ifconfig)
{
    if(p_ifconfig && s_ppp_obj.pcb)
    {
        const ip_addr_t *dns = dns_getserver(0);
        struct netif *pppif = ppp_netif(s_ppp_obj.pcb);
        copy_ip_addr(p_ifconfig->ip_addr, (uint8_t *)&pppif->ip_addr, false);
        copy_ip_addr(p_ifconfig->gw, (uint8_t *)&pppif->gw, false);
        copy_ip_addr(p_ifconfig->netmask, (uint8_t *)&pppif->netmask, false);
        copy_ip_addr(p_ifconfig->dns, (uint8_t *)dns, false);
    }
    else if(p_ifconfig)
    {
        memset(p_ifconfig, 0, sizeof(lte_ppp_ifconfig_t));
    }
}

bool lte_ppp_isconnected(void)
{
    return s_ppp_obj.connected;
}

/* --- end of file ---------------------------------------------------------- */
