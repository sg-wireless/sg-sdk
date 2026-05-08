/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
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
 * --------------------------------------------------------------------------- *
 * Copyright (c) 2022, Pycom Limited.
 *
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * @author  (Pycom)
 * 
 * @brief   This file represents a uPython c-module interface to the exported
 *          APIs of system information.
 * --------------------------------------------------------------------------- *
 */


#include "esp_sleep.h"
#include "esp_attr.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "spi_flash_mmap.h"
#include "esp_http_client.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#ifdef CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "mp_lite_if.h"

// Use SG-SDK structured logging
#define __log_subsystem     F1
#define __log_component     fuota
#include "log_lib.h"

// Register the fuota component with cyan color
__log_component_def(F1, fuota, cyan, 1, 0)

/* --- forward declarations ------------------------------------------------- */

/* --- External C interface for soft reset ---------------------------------- */

// Cancel any ongoing OTA operation (called from MicroPython soft reset)
void fuota_cancel_on_soft_reset(void);
// Validate ota partition after reset if pending (called from MicroPython start)
void fuota_validate_ota_partition(void);

/* --- module functions definitions ----------------------------------------- */

static esp_ota_handle_t esp_ota_handle;

/* --- OTA state management ------------------------------------------------- */

typedef enum {
    OTA_STATE_IDLE,
    OTA_STATE_RUNNING,
    OTA_STATE_SUCCESS,
    OTA_STATE_FAILED,
    OTA_STATE_ABORTED
} ota_state_t;

/* Tunables for robust (resume-capable) OTA download. Defaults are tuned for
 * slow / lossy cellular (NB-IoT / LTE-M) links where a single TLS flow is
 * likely to be torn down (NAT rebind, RRC release, PSM, etc.) during a
 * multi-minute firmware download. */
typedef struct {
    uint32_t max_retries;          // total attempts (including the first)
    uint32_t backoff_initial_ms;   // first retry delay
    uint32_t backoff_max_ms;       // cap for exponential backoff
    uint32_t timeout_ms;           // per-HTTP-request timeout
    uint32_t rx_buffer_size;       // receive buffer + ota write block size
    uint32_t max_chunk_bytes;      // 0 = unlimited; else re-connect every N
                                   // bytes to dodge NAT / idle timeouts
    bool     force_restart_on_200; // if server ignores Range (returns 200),
                                   // start over from offset 0
} ota_config_t;

#define OTA_DEFAULT_MAX_RETRIES          30
#define OTA_DEFAULT_BACKOFF_INITIAL_MS   2000
#define OTA_DEFAULT_BACKOFF_MAX_MS       60000
// 180s per HTTP call. First TLS handshake over 1NCE LTE-M NB-IoT can take
// well over a minute before any application bytes flow, and steady-state
// paging gaps on congested cells occasionally exceed 120 s even once
// data has started flowing.
#define OTA_DEFAULT_TIMEOUT_MS           180000
#define OTA_DEFAULT_RX_BUF_SIZE          4096
// 128 KB per connection. On NB-IoT the carrier / NAT / PGW will frequently
// tear down a long-lived TLS flow; capping each HTTP GET to 128 KB forces
// periodic clean reconnects and lets the resume-with-Range logic keep
// advancing instead of stalling on a zombie socket. Benign on WiFi (just
// ~20 extra HEAD/GETs spread over the download)./
#define OTA_DEFAULT_MAX_CHUNK_BYTES      (128 * 1024)

typedef struct {
    ota_state_t state;
    char *url;                  // URL for OTA download
    int bytes_written;          // Total bytes written
    int total_size;             // Total firmware size (if known)
    char *err_msg;              // The error message
    esp_err_t error_code;       // Error code if failed
    TaskHandle_t task_handle;   // FreeRTOS task handle
    bool is_background_task;    // True if running as background task
    volatile bool abort_flag;   // Cooperative abort request
    uint32_t attempts;          // Number of HTTP attempts so far
    uint32_t resume_offset;     // Offset of current attempt (for visibility)
    ota_config_t cfg;           // Active tunables for the current upgrade
} ota_context_t;

static ota_context_t s_ota_ctx = {
    .state = OTA_STATE_IDLE,
    .url = NULL,
    .bytes_written = 0,
    .total_size = 0,
    .error_code = ESP_OK,
    .err_msg = NULL,
    .task_handle = NULL,
    .is_background_task = false,
    .abort_flag = false,
    .attempts = 0,
    .resume_offset = 0,
};

static void ota_ctx_set_err(const char *msg, esp_err_t err)
{
    if (s_ota_ctx.err_msg) {
        free(s_ota_ctx.err_msg);
        s_ota_ctx.err_msg = NULL;
    }
    s_ota_ctx.error_code = err;
    if (!msg) {
        return;
    }
    char buf[256];
    if (err != ESP_OK) {
        snprintf(buf, sizeof(buf), "%s [%s]", msg, esp_err_to_name(err));
    } else {
        snprintf(buf, sizeof(buf), "%s", msg);
    }
    s_ota_ctx.err_msg = strdup(buf);
}

static bool ota_sleep_with_abort(uint32_t ms)
{
    const uint32_t step = 100;
    uint32_t waited = 0;
    while (waited < ms) {
        if (s_ota_ctx.abort_flag) {
            return false;
        }
        uint32_t d = (ms - waited) < step ? (ms - waited) : step;
        vTaskDelay(pdMS_TO_TICKS(d));
        waited += d;
    }
    return true;
}

__mp_mod_name(fuota, FUOTA);

// Field names for fuota.info() named tuple
static const qstr fuota_info_fields[] = {
	MP_QSTR_next_update_partition,
	MP_QSTR_running_partition,
	MP_QSTR_boot_partition
};

__mp_mod_fun_0(fuota, info)(void)
{
	const esp_partition_t *partition;
	partition = esp_ota_get_next_update_partition(NULL);
	__log_output("Next update partition: %s\n", partition->label);
	partition = esp_ota_get_running_partition();
	__log_output("Running partition: %s\n", partition->label);
	partition = esp_ota_get_boot_partition();
	__log_output("Boot partition: %s\n", partition->label);
    return mp_const_none;
}

__mp_mod_fun_0(fuota, partition_info)(void)
{
    const esp_partition_t *next_partition = esp_ota_get_next_update_partition(NULL);
    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    const esp_partition_t *boot_partition = esp_ota_get_boot_partition();

    // Create string objects for each partition label
    mp_obj_t next_str = mp_obj_new_str(next_partition->label, strlen(next_partition->label));
    mp_obj_t running_str = mp_obj_new_str(running_partition->label, strlen(running_partition->label));
    mp_obj_t boot_str = mp_obj_new_str(boot_partition->label, strlen(boot_partition->label));
    
    // Create tuple items
    mp_obj_t items[3] = { next_str, running_str, boot_str };
    
    // Create the named tuple
    mp_obj_t info_tuple = mp_obj_new_attrtuple(fuota_info_fields, 3, items);
    
    return info_tuple;

}

static void handle_exceptions(esp_err_t result)
{
    const char* err_msg = NULL;

    if(ESP_ERR_NO_MEM == result)
	{
		err_msg = "Cannot allocate memory for OTA operation.";
	}
	else if(ESP_ERR_OTA_PARTITION_CONFLICT == result)
	{
		err_msg = "Partition holds the currently running firmware, "
                    "cannot update in place.";
	}
	else if(ESP_ERR_NOT_FOUND == result)
	{
		err_msg = "Requested resource not found.";
	}
	else if(ESP_ERR_INVALID_SIZE == result)
	{
		err_msg = "Partition doesn't fit in configured flash size.";
	}
	else if(ESP_ERR_FLASH_OP_TIMEOUT == result)
	{
		err_msg = "Flash write failed.";
	}
	else if(ESP_ERR_FLASH_OP_FAIL == result)
	{
		err_msg = "Flash write failed.";
	}
	else if(ESP_ERR_OTA_ROLLBACK_INVALID_STATE == result)
	{
		err_msg = "Before performing an update, the application must be valid.";
	}
	else if (ESP_ERR_OTA_VALIDATE_FAILED == result)
	{
		err_msg = "First byte of image contains invalid app image magic byte."
                "or OTA image is invalid.";
	}
	else if (ESP_ERR_OTA_SELECT_INFO_INVALID == result)
	{
		err_msg = "OTA data partition has invalid contents.";
	}
	else if (ESP_ERR_INVALID_STATE == result)
	{
		err_msg = "Flash write failed.";
	}
	else if (ESP_ERR_OTA_ROLLBACK_FAILED == result)
	{
		err_msg = "The rollback is not possible "
                  "due to flash does not have any apps.";
	}
    else if(result != ESP_OK)
    {
        err_msg = "the requested operation failed";
    }

    if(err_msg) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(err_msg));
    }
}

__mp_mod_fun_0(fuota, start)(void)
{
    const esp_partition_t *partition_new;
	const esp_partition_t *partition_now;
	esp_err_t result;
	partition_new = esp_ota_get_next_update_partition(NULL);
	partition_now = esp_ota_get_running_partition();
	result = esp_ota_begin(partition_new, 0, &esp_ota_handle);

	__log_output("OTA esp_ota_begin returned %d\n", result);
	__log_output("Writing to partition: %s\n", partition_new->label);
	__log_output("Current boot partition: %s\n", partition_now->label);

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    return mp_const_none;
}

__mp_mod_fun_1(fuota, write)(mp_obj_t data)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
    esp_err_t result;
    result = esp_ota_write(esp_ota_handle, bufinfo.buf, bufinfo.len);

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    return mp_const_none;
}

__mp_mod_fun_0(fuota, finish)(void)
{
	const esp_partition_t *partition_new;
	const esp_partition_t *partition_now;
	esp_err_t result;
	partition_new = esp_ota_get_boot_partition();
	partition_now = esp_ota_get_running_partition();
	result = esp_ota_begin(partition_new, 0, &esp_ota_handle);
	result = esp_ota_end(esp_ota_handle);
	__log_output("OTA esp_ota_end returned %d\n", result);

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    partition_new = esp_ota_get_next_update_partition(NULL);
	result = esp_ota_set_boot_partition(partition_new);
	__log_output("OTA esp_ota_set_boot_partition returned %d\n", result);
	__log_output("NEXT boot from partition: %s\n", partition_new->label);
	__log_output("Current boot partition: %s\n", partition_now->label);
	if (ESP_OK != result) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Error setting new ota partition"));
	}

    return mp_const_none;
}

__mp_mod_fun_0(fuota, rollback)(void)
{
	esp_err_t result;
	result = esp_ota_mark_app_invalid_rollback_and_reboot();

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    return mp_const_none;
}

/* --- Background OTA task -------------------------------------------------- *
 *
 * Robust resume-capable OTA download.
 *
 * Strategy
 *  - esp_ota_begin() is called once; the OTA partition is erased exactly once
 *    per logical upgrade. esp_ota_handle is kept alive across TCP/TLS failures.
 *  - Each HTTP attempt uses a fresh esp_http_client connection. On resume, a
 *    "Range: bytes=<offset>-" header is sent. Server returning 200 (ignored
 *    Range) restarts the upgrade from offset 0; 206 is treated as a proper
 *    resume. Content-Range "/<total>" is used to learn/confirm total size.
 *  - Server artifact identity (ETag / Last-Modified / total size) is captured
 *    on the first successful response and re-checked on every subsequent
 *    response; mismatch restarts the upgrade from offset 0.
 *  - Transient failures (read error, premature close, 5xx, transport open
 *    failure) trigger exponential backoff (cfg.backoff_initial_ms up to
 *    cfg.backoff_max_ms) until cfg.max_retries is exhausted.
 *  - Optional cfg.max_chunk_bytes caps how much data is pulled per HTTP
 *    connection; this helps on NB-IoT where long-lived flows get dropped by
 *    NAT / PSM / radio transitions even when progress is being made.
 *  - Abort is cooperative (s_ota_ctx.abort_flag is checked in all loops);
 *    no vTaskDelete() is performed from Python-visible APIs.
 */

static void ota_task(void *pvParameters)
{
    char *url = (char *)pvParameters;
    ota_config_t cfg = s_ota_ctx.cfg;
    esp_err_t err = ESP_OK;
    const char *err_msg = NULL;
    const esp_partition_t *update_partition = NULL;
    esp_ota_handle_t ota_handle = 0;
    bool ota_begin_done = false;
    size_t offset = 0;
    size_t total_size = 0;
    char etag[128] = {0};
    char last_modified[64] = {0};
    bool server_id_known = false;
    uint32_t attempt = 0;
    uint32_t backoff_ms = cfg.backoff_initial_ms;
    char *rx_buf = NULL;
    TickType_t t_start_ticks = xTaskGetTickCount();
    // Tristate: assume the server supports Range (206) until a Range request
    // comes back with 200. Once cleared we stop chunk-capping, otherwise we'd
    // chunk-close -> reconnect with Range -> server ignores -> restart from 0
    // forever (observed against Python http.server / SimpleHTTPRequestHandler
    // and any other server that doesn't implement RFC 7233).
    bool server_supports_range = true;

    s_ota_ctx.state = OTA_STATE_RUNNING;
    s_ota_ctx.bytes_written = 0;
    s_ota_ctx.total_size = 0;
    s_ota_ctx.error_code = ESP_OK;
    s_ota_ctx.attempts = 0;
    s_ota_ctx.resume_offset = 0;
    s_ota_ctx.abort_flag = false;

    // Check network connectivity before attempting OTA
    bool has_ip = false;
    esp_netif_ip_info_t ip_info;

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0) {
        has_ip = true;
        __log_info("WiFi connected, IP: " IPSTR, IP2STR(&ip_info.ip));
    }
    if (!has_ip) {
        netif = esp_netif_get_handle_from_ifkey("PPP_DEF");
        if (netif && esp_netif_get_ip_info(netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0) {
            has_ip = true;
            __log_info("LTE connected, IP: " IPSTR, IP2STR(&ip_info.ip));
        }
    }
    if (!has_ip) {
        __log_error("Network not connected - no IP address on any interface");
        err = ESP_ERR_INVALID_STATE;
        err_msg = "Network not connected - please connect to WiFi or LTE first";
        goto fail;
    }

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (!update_partition) {
        err = ESP_ERR_NOT_FOUND;
        err_msg = "No OTA partition available";
        goto fail;
    }
    __log_info("OTA target partition: %s", update_partition->label);

    rx_buf = malloc(cfg.rx_buffer_size);
    if (!rx_buf) {
        err = ESP_ERR_NO_MEM;
        err_msg = "RX buffer allocation failed";
        goto fail;
    }

    while (attempt < cfg.max_retries) {
        if (s_ota_ctx.abort_flag) {
            err_msg = "aborted by user";
            goto fail_aborted;
        }

        attempt++;
        s_ota_ctx.attempts = attempt;
        s_ota_ctx.resume_offset = offset;

        __log_info("OTA attempt %u/%u starting at offset %u/%u (t=%u s)",
                     (unsigned)attempt, (unsigned)cfg.max_retries,
                     (unsigned)offset, (unsigned)total_size,
                     (unsigned)((xTaskGetTickCount() - t_start_ticks)
                                * portTICK_PERIOD_MS / 1000));

        esp_http_client_config_t http_cfg = {
            .url = url,
            .timeout_ms = (int)cfg.timeout_ms,
            .keep_alive_enable = false,
            .buffer_size = (int)cfg.rx_buffer_size,
            .buffer_size_tx = 1024,
#ifdef CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
            .crt_bundle_attach = esp_crt_bundle_attach,
#endif
        };
        esp_http_client_handle_t client = esp_http_client_init(&http_cfg);
        if (!client) {
            err = ESP_ERR_NO_MEM;
            err_msg = "esp_http_client_init failed";
            goto retry_wait;
        }

        if (offset > 0) {
            char range_hdr[64];
            snprintf(range_hdr, sizeof(range_hdr),
                     "bytes=%u-", (unsigned)offset);
            esp_http_client_set_header(client, "Range", range_hdr);
        }

        err = esp_http_client_open(client, 0);
        if (err != ESP_OK) {
            __log_warn("esp_http_client_open failed: %s", esp_err_to_name(err));
            err_msg = "esp_http_client_open failed";
            esp_http_client_cleanup(client);
            goto retry_wait;
        }

        int header_len = esp_http_client_fetch_headers(client);
        int status_code = esp_http_client_get_status_code(client);

        if (header_len < 0) {
            __log_warn("esp_http_client_fetch_headers failed");
            err = ESP_FAIL;
            err_msg = "Failed to read HTTP headers";
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            goto retry_wait;
        }

        // Parse total size from either Content-Range (206) or Content-Length (200)
        size_t resp_total = 0;
        if (status_code == 206) {
            char *cr_hdr = NULL;
            esp_http_client_get_header(client, "Content-Range", &cr_hdr);
            if (cr_hdr) {
                // Expected format: "bytes <start>-<end>/<total>"
                const char *slash = strchr(cr_hdr, '/');
                if (slash && slash[1] && slash[1] != '*') {
                    resp_total = (size_t)strtoul(slash + 1, NULL, 10);
                }
            }
            if (resp_total == 0 && header_len > 0) {
                resp_total = offset + (size_t)header_len;
            }
        } else if (status_code == 200) {
            if (header_len > 0) {
                resp_total = (size_t)header_len;
            }
        } else if (status_code >= 400 && status_code < 500) {
            char msg[64];
            snprintf(msg, sizeof(msg), "HTTP %d (client error, non-retryable)",
                     status_code);
            err = ESP_FAIL;
            ota_ctx_set_err(msg, err);
            err_msg = NULL;  // already set via ota_ctx_set_err
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            goto fail_already_set;
        } else {
            char msg[64];
            snprintf(msg, sizeof(msg), "HTTP %d (transient)", status_code);
            err = ESP_FAIL;
            __log_warn("%s", msg);
            err_msg = "HTTP transient error";
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            goto retry_wait;
        }

        // Handle server ignoring Range: restart upgrade from byte 0
        if (status_code == 200 && offset > 0) {
            __log_warn("Server ignored Range (status 200) at offset %u; "
                       "restarting upgrade from byte 0 "
                       "(disabling chunk cap for this upgrade)",
                       (unsigned)offset);
            // Mark the server as not supporting Range so the download loop
            // stops honouring max_chunk_bytes. Without this we'd loop
            // forever: chunk cap -> reconnect -> server ignores Range ->
            // restart from 0 -> chunk cap ...
            server_supports_range = false;
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            if (ota_begin_done) {
                esp_ota_abort(ota_handle);
                ota_begin_done = false;
            }
            offset = 0;
            total_size = 0;
            s_ota_ctx.bytes_written = 0;
            s_ota_ctx.total_size = 0;
            server_id_known = false;
            etag[0] = 0;
            last_modified[0] = 0;
            // Not counted as a retry failure: reset backoff
            backoff_ms = cfg.backoff_initial_ms;
            attempt--;
            s_ota_ctx.attempts = attempt;
            continue;
        }

        // Artifact identity check across retries
        char *new_etag = NULL;
        char *new_lm = NULL;
        esp_http_client_get_header(client, "ETag", &new_etag);
        esp_http_client_get_header(client, "Last-Modified", &new_lm);

        if (server_id_known) {
            bool mismatch = false;
            if (etag[0] && new_etag && strcmp(etag, new_etag) != 0) {
                mismatch = true;
            } else if (!etag[0] && last_modified[0] && new_lm &&
                       strcmp(last_modified, new_lm) != 0) {
                mismatch = true;
            } else if (total_size != 0 && resp_total != 0 &&
                       total_size != resp_total) {
                mismatch = true;
            }
            if (mismatch) {
                __log_warn("Server artifact changed mid-download "
                           "(etag/lm/size), restarting from byte 0");
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                if (ota_begin_done) {
                    esp_ota_abort(ota_handle);
                    ota_begin_done = false;
                }
                offset = 0;
                total_size = 0;
                s_ota_ctx.bytes_written = 0;
                s_ota_ctx.total_size = 0;
                server_id_known = false;
                etag[0] = 0;
                last_modified[0] = 0;
                backoff_ms = cfg.backoff_initial_ms;
                attempt--;
                s_ota_ctx.attempts = attempt;
                continue;
            }
        } else {
            if (new_etag) {
                strncpy(etag, new_etag, sizeof(etag) - 1);
                etag[sizeof(etag) - 1] = 0;
            }
            if (new_lm) {
                strncpy(last_modified, new_lm, sizeof(last_modified) - 1);
                last_modified[sizeof(last_modified) - 1] = 0;
            }
            server_id_known = true;
        }

        if (resp_total > 0) {
            total_size = resp_total;
            s_ota_ctx.total_size = (int)total_size;
        }

        // Begin OTA on the first successful header fetch
        if (!ota_begin_done) {
            __log_info("esp_ota_begin partition=%s size=%u",
                       update_partition->label, (unsigned)total_size);
            err = esp_ota_begin(update_partition,
                                total_size > 0 ? total_size : OTA_SIZE_UNKNOWN,
                                &ota_handle);
            if (err != ESP_OK) {
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                err_msg = "esp_ota_begin failed";
                goto fail;
            }
            ota_begin_done = true;
        }

        // Download & write loop for this HTTP attempt
        bool read_err = false;
        bool idle_timeout = false;
        bool chunk_cap_hit = false;
        size_t chunk_written = 0;
        int last_log_pct = -1;
        if (total_size > 0) {
            last_log_pct = (int)((offset * 100) / total_size);
        }
        TickType_t last_rx_tick = xTaskGetTickCount();
        TickType_t last_debug_tick = last_rx_tick;
        size_t last_debug_offset = offset;
        while (!s_ota_ctx.abort_flag) {
            int rd = esp_http_client_read(client, rx_buf,
                                          (int)cfg.rx_buffer_size);
            if (rd < 0) {
                __log_warn("esp_http_client_read error at offset %u",
                           (unsigned)offset);
                read_err = true;
                break;
            }
            if (rd == 0) {
                // Could be true EOF or just no data available right now.
                if (esp_http_client_is_complete_data_received(client)) {
                    break;  // true EOF for this HTTP response
                }
                TickType_t now = xTaskGetTickCount();
                uint32_t idle_ms =
                    (uint32_t)((now - last_rx_tick) * portTICK_PERIOD_MS);
                if (idle_ms >= cfg.timeout_ms) {
                    __log_warn("No data for %u ms at offset %u/%u; "
                               "forcing reconnect",
                               (unsigned)idle_ms, (unsigned)offset,
                               (unsigned)total_size);
                    idle_timeout = true;
                    break;
                }
                // Heartbeat while idle so operators see the stall.
                uint32_t since_debug_ms =
                    (uint32_t)((now - last_debug_tick) * portTICK_PERIOD_MS);
                if (since_debug_ms >= 5000) {
                    size_t delta = offset - last_debug_offset;
                    __log_debug("OTA rx: +%u bytes in %u ms (%u B/s), "
                                "offset=%u/%u (idle %u ms)",
                                (unsigned)delta, (unsigned)since_debug_ms,
                                since_debug_ms ?
                                    (unsigned)((delta * 1000U) / since_debug_ms)
                                                : 0U,
                                (unsigned)offset, (unsigned)total_size,
                                (unsigned)idle_ms);
                    last_debug_tick = now;
                    last_debug_offset = offset;
                }
                vTaskDelay(pdMS_TO_TICKS(200));
                continue;
            }
            last_rx_tick = xTaskGetTickCount();
            err = esp_ota_write(ota_handle, rx_buf, rd);
            if (err != ESP_OK) {
                __log_error("esp_ota_write failed at offset %u: %s",
                            (unsigned)offset, esp_err_to_name(err));
                esp_http_client_close(client);
                esp_http_client_cleanup(client);
                err_msg = "esp_ota_write failed";
                goto fail;
            }
            offset += (size_t)rd;
            chunk_written += (size_t)rd;
            s_ota_ctx.bytes_written = (int)offset;

            if (total_size > 0) {
                int pct = (int)((offset * 100) / total_size);
                if (pct > last_log_pct) {
                    last_log_pct = pct;
                    __log_debug("OTA progress: %u/%u (%d%%)",
                                (unsigned)offset, (unsigned)total_size, pct);
                }
                if (offset >= total_size) {
                    break;  // complete
                }
            }
            // Byte-level heartbeat so sub-percent progress is visible.
            TickType_t now = xTaskGetTickCount();
            uint32_t since_debug_ms =
                (uint32_t)((now - last_debug_tick) * portTICK_PERIOD_MS);
            if (since_debug_ms >= 5000) {
                size_t delta = offset - last_debug_offset;
                __log_debug("OTA rx: +%u bytes in %u ms (%u B/s), "
                            "offset=%u/%u",
                            (unsigned)delta, (unsigned)since_debug_ms,
                            since_debug_ms ?
                                (unsigned)((delta * 1000U) / since_debug_ms)
                                            : 0U,
                            (unsigned)offset, (unsigned)total_size);
                last_debug_tick = now;
                last_debug_offset = offset;
            }
            if (cfg.max_chunk_bytes > 0 && server_supports_range &&
                chunk_written >= cfg.max_chunk_bytes) {
                __log_info("Chunk cap reached (%u bytes); reconnecting to "
                           "refresh transport",
                           (unsigned)chunk_written);
                chunk_cap_hit = true;
                break;
            }
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        if (s_ota_ctx.abort_flag) {
            err_msg = "aborted by user";
            goto fail_aborted;
        }

        if (total_size > 0 && offset >= total_size) {
            break;  // success
        }

        if (read_err) {
            err = ESP_FAIL;
            err_msg = "transport read error";
            goto retry_wait;
        }

        if (idle_timeout) {
            err = ESP_FAIL;
            err_msg = "idle timeout during read";
            goto retry_wait;
        }

        // Voluntary chunk-cap reconnect: not a failure. Don't burn a retry
        // attempt and don't apply backoff -- reset it so any real failure
        // that follows starts from backoff_initial_ms.
        if (chunk_cap_hit && total_size > 0 && offset < total_size) {
            if (attempt > 0) {
                attempt--;
                s_ota_ctx.attempts = attempt;
            }
            backoff_ms = cfg.backoff_initial_ms;
            continue;
        }

        // Clean EOF but not yet complete -> server closed early; retry
        if (total_size > 0 && offset < total_size) {
            __log_warn("Premature EOF at offset %u/%u; will resume",
                       (unsigned)offset, (unsigned)total_size);
            err = ESP_FAIL;
            err_msg = "premature EOF";
            goto retry_wait;
        }

        // Unknown total size + clean EOF -> treat as success
        break;

    retry_wait:
        if (attempt >= cfg.max_retries) {
            if (!err_msg) err_msg = "max retries exceeded";
            goto fail;
        }
        __log_info("OTA retrying in %u ms (attempt %u/%u, offset %u/%u, "
                     "reason=%s)",
                     (unsigned)backoff_ms, (unsigned)attempt,
                     (unsigned)cfg.max_retries,
                     (unsigned)offset, (unsigned)total_size,
                     err_msg ? err_msg : "unknown");
        if (!ota_sleep_with_abort(backoff_ms)) {
            err_msg = "aborted by user";
            goto fail_aborted;
        }
        if (backoff_ms < cfg.backoff_max_ms) {
            backoff_ms *= 2;
            if (backoff_ms > cfg.backoff_max_ms) {
                backoff_ms = cfg.backoff_max_ms;
            }
        }
    }

    if (s_ota_ctx.abort_flag) {
        err_msg = "aborted by user";
        goto fail_aborted;
    }

    // Finalize OTA
    err = esp_ota_end(ota_handle);
    ota_begin_done = false;
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            err_msg = "Image validation failed, image is corrupted";
        } else {
            err_msg = "esp_ota_end failed";
        }
        goto fail;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        err_msg = "esp_ota_set_boot_partition failed";
        goto fail;
    }

    __log_info("OTA upgrade successful: %u bytes, %u attempt(s), partition %s",
               (unsigned)offset, (unsigned)s_ota_ctx.attempts,
               update_partition->label);
    s_ota_ctx.state = OTA_STATE_SUCCESS;
    goto cleanup;

fail_aborted:
    __log_info("OTA aborted at offset %u (attempt %u)",
               (unsigned)offset, (unsigned)s_ota_ctx.attempts);
    if (ota_begin_done) {
        esp_ota_abort(ota_handle);
        ota_begin_done = false;
    }
    s_ota_ctx.state = OTA_STATE_ABORTED;
    ota_ctx_set_err(err_msg ? err_msg : "aborted", ESP_OK);
    goto cleanup;

fail:
    if (ota_begin_done) {
        esp_ota_abort(ota_handle);
        ota_begin_done = false;
    }
    __log_error("OTA upgrade failed after %u attempt(s) at offset %u: %s (%s)",
                (unsigned)s_ota_ctx.attempts, (unsigned)offset,
                err_msg ? err_msg : "unknown", esp_err_to_name(err));
    s_ota_ctx.state = OTA_STATE_FAILED;
    ota_ctx_set_err(err_msg ? err_msg : "OTA failed",
                    err != ESP_OK ? err : ESP_FAIL);
    goto cleanup;

fail_already_set:
    if (ota_begin_done) {
        esp_ota_abort(ota_handle);
        ota_begin_done = false;
    }
    __log_error("OTA upgrade failed: %s",
                s_ota_ctx.err_msg ? s_ota_ctx.err_msg : "unknown");
    s_ota_ctx.state = OTA_STATE_FAILED;
    // err_msg/error already populated by ota_ctx_set_err()
    goto cleanup;

cleanup:
    if (rx_buf) {
        free(rx_buf);
    }
    if (url) {
        free(url);
    }
    s_ota_ctx.task_handle = NULL;
    if (s_ota_ctx.is_background_task) {
        vTaskDelete(NULL);
    }
}

/* --- Python API functions ------------------------------------------------- */

/* Populate cfg with defaults, optionally overridden by a Python dict of
 * {str: int}. Unknown keys are silently ignored. Returns true on success. */
static void ota_cfg_from_opts(ota_config_t *cfg, mp_obj_t opts)
{
    cfg->max_retries        = OTA_DEFAULT_MAX_RETRIES;
    cfg->backoff_initial_ms = OTA_DEFAULT_BACKOFF_INITIAL_MS;
    cfg->backoff_max_ms     = OTA_DEFAULT_BACKOFF_MAX_MS;
    cfg->timeout_ms         = OTA_DEFAULT_TIMEOUT_MS;
    cfg->rx_buffer_size     = OTA_DEFAULT_RX_BUF_SIZE;
    cfg->max_chunk_bytes    = OTA_DEFAULT_MAX_CHUNK_BYTES;
    cfg->force_restart_on_200 = true;

    if (opts == MP_OBJ_NULL || opts == mp_const_none) {
        return;
    }
    if (!mp_obj_is_type(opts, &mp_type_dict)) {
        mp_raise_TypeError(MP_ERROR_TEXT("opts must be a dict"));
    }

    struct { const char *k; uint32_t *p; } keys[] = {
        { "max_retries",        &cfg->max_retries },
        { "backoff_initial_ms", &cfg->backoff_initial_ms },
        { "backoff_max_ms",     &cfg->backoff_max_ms },
        { "timeout_ms",         &cfg->timeout_ms },
        { "rx_buffer_size",     &cfg->rx_buffer_size },
        { "max_chunk_bytes",    &cfg->max_chunk_bytes },
    };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        mp_obj_t key = mp_obj_new_str(keys[i].k, strlen(keys[i].k));
        mp_map_elem_t *e = mp_map_lookup(mp_obj_dict_get_map(opts),
                                         key, MP_MAP_LOOKUP);
        if (e) {
            mp_int_t v = mp_obj_get_int(e->value);
            if (v < 0) v = 0;
            *keys[i].p = (uint32_t)v;
        }
    }

    // Sanity clamps
    if (cfg->max_retries == 0) cfg->max_retries = 1;
    if (cfg->rx_buffer_size < 512) cfg->rx_buffer_size = 512;
    if (cfg->rx_buffer_size > 16384) cfg->rx_buffer_size = 16384;
    if (cfg->timeout_ms < 5000) cfg->timeout_ms = 5000;
    if (cfg->backoff_initial_ms < 100) cfg->backoff_initial_ms = 100;
    if (cfg->backoff_max_ms < cfg->backoff_initial_ms) {
        cfg->backoff_max_ms = cfg->backoff_initial_ms;
    }
}

__mp_mod_fun_var_between(fuota, start_upgrade, 1, 2)(
    size_t n_args, const mp_obj_t *args)
{
    if (s_ota_ctx.state == OTA_STATE_RUNNING) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("OTA upgrade already in progress"));
    }

    const char *url = mp_obj_str_get_str(args[0]);
    ota_cfg_from_opts(&s_ota_ctx.cfg, n_args > 1 ? args[1] : MP_OBJ_NULL);

    __log_info("Starting OTA upgrade from: %s (retries=%u, timeout_ms=%u, "
                 "chunk=%u)",
                 url, (unsigned)s_ota_ctx.cfg.max_retries,
                 (unsigned)s_ota_ctx.cfg.timeout_ms,
                 (unsigned)s_ota_ctx.cfg.max_chunk_bytes);

    // Clear previous state and error message
    s_ota_ctx.bytes_written = 0;
    s_ota_ctx.total_size = 0;
    s_ota_ctx.error_code = ESP_OK;
    s_ota_ctx.attempts = 0;
    s_ota_ctx.resume_offset = 0;
    s_ota_ctx.abort_flag = false;
    if (s_ota_ctx.err_msg) {
        free(s_ota_ctx.err_msg);
        s_ota_ctx.err_msg = NULL;
    }

    char *url_copy = strdup(url);
    if (!url_copy) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Failed to allocate memory for URL"));
    }

    if (s_ota_ctx.url) {
        free(s_ota_ctx.url);
    }
    s_ota_ctx.url = strdup(url);

    s_ota_ctx.is_background_task = true;
    BaseType_t result = xTaskCreate(
        ota_task,
        "ota_task",
        8192,
        (void *)url_copy,
        5,
        &s_ota_ctx.task_handle
    );

    if (result != pdPASS) {
        free(url_copy);
        free(s_ota_ctx.url);
        s_ota_ctx.url = NULL;
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Failed to create OTA task"));
    }

    return mp_const_none;
}

__mp_mod_fun_0(fuota, status)(void)
{
    mp_obj_t dict = mp_obj_new_dict(8);

    const char *state_str = "unknown";
    switch (s_ota_ctx.state) {
        case OTA_STATE_IDLE:    state_str = "idle"; break;
        case OTA_STATE_RUNNING: state_str = "running"; break;
        case OTA_STATE_SUCCESS: state_str = "success"; break;
        case OTA_STATE_FAILED:  state_str = "failed"; break;
        case OTA_STATE_ABORTED: state_str = "aborted"; break;
    }

    mp_obj_dict_store(dict, mp_obj_new_str("state", 5),
                     mp_obj_new_str(state_str, strlen(state_str)));
    mp_obj_dict_store(dict, mp_obj_new_str("bytes_written", 13),
                     mp_obj_new_int(s_ota_ctx.bytes_written));
    mp_obj_dict_store(dict, mp_obj_new_str("attempts", 8),
                     mp_obj_new_int(s_ota_ctx.attempts));
    mp_obj_dict_store(dict, mp_obj_new_str("resume_offset", 13),
                     mp_obj_new_int(s_ota_ctx.resume_offset));

    if (s_ota_ctx.total_size > 0) {
        mp_obj_dict_store(dict, mp_obj_new_str("total_size", 10),
                         mp_obj_new_int(s_ota_ctx.total_size));
        int pct = (int)(((int64_t)s_ota_ctx.bytes_written * 100)
                        / s_ota_ctx.total_size);
        mp_obj_dict_store(dict, mp_obj_new_str("percent", 7),
                         mp_obj_new_int(pct));
    }

    if (s_ota_ctx.state == OTA_STATE_FAILED) {
        mp_obj_dict_store(dict, mp_obj_new_str("error_code", 10),
                         mp_obj_new_int(s_ota_ctx.error_code));
    }
    if (s_ota_ctx.err_msg) {
        mp_obj_dict_store(dict, mp_obj_new_str("err_msg", 7),
                         mp_obj_new_str(s_ota_ctx.err_msg, strlen(s_ota_ctx.err_msg)));
    }

    return dict;
}

__mp_mod_fun_0(fuota, abort_upgrade)(void)
{
    if (s_ota_ctx.state != OTA_STATE_RUNNING) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("No OTA upgrade in progress"));
    }

    // Cooperative abort: the download loop will notice the flag, close the
    // HTTP client cleanly, release the ota handle and exit.
    s_ota_ctx.abort_flag = true;

    __log_output("OTA upgrade abort requested by user\n");

    return mp_const_none;
}

__mp_mod_fun_0(fuota, valid)(void)
{
	esp_err_t result;
	result = esp_ota_mark_app_valid_cancel_rollback();

	if (ESP_OK != result)
    {
        handle_exceptions(result);
    }

	    return mp_const_none;
}

__mp_mod_fun_var_between(fuota, upgrade, 1, 2)(
    size_t n_args, const mp_obj_t *args)
{
    // Blocking upgrade function
    if (s_ota_ctx.state == OTA_STATE_RUNNING) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("OTA upgrade already in progress"));
    }

    const char *url = mp_obj_str_get_str(args[0]);
    ota_cfg_from_opts(&s_ota_ctx.cfg, n_args > 1 ? args[1] : MP_OBJ_NULL);

    __log_info("Starting OTA upgrade from: %s (retries=%u, timeout_ms=%u, "
                 "chunk=%u)",
                 url, (unsigned)s_ota_ctx.cfg.max_retries,
                 (unsigned)s_ota_ctx.cfg.timeout_ms,
                 (unsigned)s_ota_ctx.cfg.max_chunk_bytes);

    s_ota_ctx.bytes_written = 0;
    s_ota_ctx.total_size = 0;
    s_ota_ctx.error_code = ESP_OK;
    s_ota_ctx.attempts = 0;
    s_ota_ctx.resume_offset = 0;
    s_ota_ctx.abort_flag = false;
    if (s_ota_ctx.err_msg) {
        free(s_ota_ctx.err_msg);
        s_ota_ctx.err_msg = NULL;
    }

    char *url_copy = strdup(url);
    if (!url_copy) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Failed to allocate memory for URL"));
    }

    if (s_ota_ctx.url) {
        free(s_ota_ctx.url);
    }
    s_ota_ctx.url = strdup(url);

    s_ota_ctx.is_background_task = false;
    ota_task((void *)url_copy);
    return (s_ota_ctx.state == OTA_STATE_SUCCESS) ? mp_const_true : mp_const_false;
}

/* --- External C interface for soft reset ---------------------------------- */

void fuota_cancel_on_soft_reset(void)
{
    if (s_ota_ctx.state != OTA_STATE_RUNNING || !s_ota_ctx.task_handle) {
        return;
    }

    __log_output("Soft reset: canceling ongoing OTA upgrade\n");

    // Ask the task to stop cooperatively, then give it a short window to
    // close its HTTP client / free mbedtls / abort the ota handle before we
    // yank it out from under any pending sockets.
    s_ota_ctx.abort_flag = true;
    for (int i = 0; i < 20; ++i) {
        if (!s_ota_ctx.task_handle ||
            s_ota_ctx.state != OTA_STATE_RUNNING) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Last-resort fallback: if the task is still running, delete it. This
    // may leak transport state, but we're in a teardown path.
    if (s_ota_ctx.task_handle &&
        s_ota_ctx.state == OTA_STATE_RUNNING) {
        vTaskDelete(s_ota_ctx.task_handle);
        s_ota_ctx.task_handle = NULL;
        s_ota_ctx.state = OTA_STATE_ABORTED;
    }

    if (s_ota_ctx.url) {
        free(s_ota_ctx.url);
        s_ota_ctx.url = NULL;
    }
}

/* --- External C interface for start ---------------------------------- */

void fuota_validate_ota_partition(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    if (strcmp(running->label, "factory") == 0) {
        return;
    }
    esp_ota_img_states_t ota_state;
    esp_err_t err = esp_ota_get_state_partition(running, &ota_state);
    if ( err == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            err = esp_ota_mark_app_valid_cancel_rollback();
        }
    } else {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(esp_err_to_name(err)));
    }
}

/* --- end of file ---------------------------------------------------------- */
