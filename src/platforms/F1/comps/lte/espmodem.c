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
 * @brief   ESP Modem Middle Layer Implementation for Sequans Monarch 2 GM02S
 *          This implementation provides a robust interface to the ESP modem
 *          library with specific optimizations for Sequans modems based on
 *          successful CMUX implementation.
 * --------------------------------------------------------------------------- *
 */

#include "espmodem.h"
#include <string.h>
#include <stdio.h>
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_ppp.h"
#include "esp_modem_api.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"        // For ESP error codes
#include "esp_heap_caps.h"  // For memory checking
#include "uart_resource_guard.h" // For UART resource protection

// Use SG-SDK interfaces  
#include "ioexp.h"      // SG-SDK IO expander interface

// Use SG-SDK structured logging system
#define __log_subsystem     lte
#define __log_component     espmodem
#include "log_lib.h"

// Register the espmodem component in the lte subsystem with cyan color
__log_component_def(lte, espmodem, cyan, 1, 1)

// Event bits for synchronization
#define ESPMODEM_CONNECT_BIT    BIT0
#define ESPMODEM_DISCONNECT_BIT BIT1

// Retry configuration for AT commands
#define AT_CMD_MAX_RETRIES      3
#define AT_CMD_RETRY_DELAY_MS   1000

// Structure to collect multi-line AT responses
typedef struct {
    char *buffer;
    size_t buffer_size;
    size_t current_pos;
    bool overflow;
} at_response_collector_t;

// Global state
static struct {
    bool initialized;
    bool debug_enabled;
    esp_modem_dce_t* dce;
    esp_netif_t* esp_netif;
    EventGroupHandle_t event_group;
    espmodem_status_t status;
    
    // Store APN for DCE configuration (GM02S needs consistent APN)
    char configured_apn[64];
    
    // Store CID (Context Identifier) - default 1, but Verizon uses 3
    int configured_cid;
    
    // Store carrier conformance mode for AT+SQNBANDSEL (e.g., "standard", "verizon", "att")
    char carrier[32];
    
    // Event callbacks
    espmodem_network_event_cb_t network_cb;
    espmodem_signal_event_cb_t signal_cb;
    espmodem_error_event_cb_t error_cb;
    espmodem_unsolicited_cb_t unsolicited_cb;
    
    // Multi-line response collector
    at_response_collector_t response_collector;
    
    // Track whether we're currently executing a solicited AT command
    // When true, suppress URC callbacks (responses are solicited, not unsolicited)
    // IMPORTANT: Volatile ensures visibility across tasks (simpler than atomic for bool flag)
    volatile bool command_in_progress;
    
    // Track +SHUTDOWN URC for reset operations
    // Set by URC handler when +SHUTDOWN is received, checked by espmodem_reset()
    volatile bool shutdown_received;
    
    // Track UART break signals and other terminal errors
    // Incremented by modem_terminal_error_handler when break/error detected
    volatile uint32_t break_signal_count;
    volatile uint32_t frame_error_count;
    volatile uint32_t buffer_overflow_count;
    
    // Crash detection - track timing of break signals
    // Multiple breaks in short time = likely hardware reset/crash
    uint32_t last_break_timestamp_ms;
    uint32_t breaks_in_window;
    
    // Recovery state
    volatile bool recovery_in_progress;
    TaskHandle_t recovery_monitor_task;
} g_espmodem = {0};

// Global CID variable exposed to C++ ESP modem library overrides
// This allows the SQNGM02S::set_data_mode() override to access the CID
// configured by espmodem_attach() without complex API changes
int g_espmodem_configured_cid = 1;  // Default to CID=1

// URC buffer tracking - ESP modem library accumulates URCs until next AT command
// We track what we've already processed to avoid duplicate events
static size_t g_last_processed_len = 0;

// ============================================================================
// Forward Declarations
// ============================================================================

#ifdef CONFIG_ESP_MODEM_URC_HANDLER
static esp_err_t unsolicited_handler(uint8_t *data, size_t len);
#endif

// Forward declaration for modem terminal error handler
static void modem_terminal_error_handler(esp_modem_terminal_error_t err);

// Forward declaration for crash recovery function
static espmodem_err_t espmodem_recover_from_crash(void);

// Forward declaration for recovery monitoring task
static void espmodem_recovery_monitor_task(void *arg);

// Forward declarations for unified event handler system
static espmodem_event_cb_t g_event_handler;
static uint32_t g_event_mask;
static void* g_event_user_ctx;
static void dispatch_event(const espmodem_event_t* event);

// ============================================================================
// AT Command Retry Helper
// ============================================================================

/**
 * @brief Execute an AT command with automatic retry on timeout/failure
 * 
 * @param dce       ESP modem DCE handle
 * @param command   AT command to execute (without \r\n)
 * @param response  Buffer to store response
 * @param timeout   Timeout per attempt in milliseconds
 * @param retries   Maximum number of retry attempts (0 = no retries)
 * 
 * @return ESP_OK on success, ESP_ERR_TIMEOUT if all retries exhausted
 */
static esp_err_t esp_modem_at_with_retry(esp_modem_dce_t* dce, const char* command, 
                                          char* response, int timeout, int retries)
{
    esp_err_t err;
    int attempt = 0;
    
    // Set flag to suppress URCs during command execution
    atomic_store(&g_espmodem.command_in_progress, true);
    
    do {
        if (attempt > 0) {
            __log_warn("Retry %d/%d for: %s", attempt, retries, command);
            vTaskDelay(pdMS_TO_TICKS(AT_CMD_RETRY_DELAY_MS));
        }
        
        err = esp_modem_at(dce, command, response, timeout);
        
        if (err == ESP_OK) {
            if (attempt > 0) {
                __log_info("Command succeeded after %d retries", attempt);
            }
            atomic_store(&g_espmodem.command_in_progress, false);
            return ESP_OK;
        }
        
        __log_warn("AT command '%s' failed: %s (0x%x)", command, esp_err_to_name(err), err);
        attempt++;
        
    } while (attempt <= retries && (err == ESP_ERR_TIMEOUT || err == ESP_FAIL));
    
    __log_error("AT command '%s' failed after %d attempts", command, attempt);
    atomic_store(&g_espmodem.command_in_progress, false);
    return err;
}

// ============================================================================
// Power Management and Initialization State Detection
// ============================================================================

/**
 * @brief Configure carrier conformance mode while modem is at 115200 baud
 * 
 * This must be done before switching to 921600 because AT+SQNCTM triggers
 * a modem reset which returns the modem to factory 115200 baud.
 * 
 * By configuring conformance mode at 115200, after the reset the modem
 * stays at 115200 and we can continue with the normal baudrate switch.
 * 
 * @param carrier Carrier conformance mode (e.g., "standard", "verizon", "att")
 * @param detected_carrier_out Output buffer to store detected carrier (optional, can be NULL)
 * @param detected_carrier_size Size of the output buffer
 * @return true if conformance mode was configured (or already correct), false on error
 */
static bool configure_conformance_mode_at_115200(const char* carrier, char* detected_carrier_out, size_t detected_carrier_size)
{
    if (!carrier) {
        __log_error("Carrier parameter is NULL");
        return false;
    }
    
    // Special handling for "None" - following Pycom Monarch 1 logic:
    // if (args[0].u_obj != mp_const_none) - only configure if user explicitly specified carrier
    // If "None" is passed, it means user didn't specify a carrier, so skip configuration entirely
    // BUT we still need to detect and return the current carrier for SQNBANDSEL commands
    if (strcmp(carrier, "None") == 0) {
        __log_info("Carrier not specified (None) - detecting current modem configuration");
        
        // Query current conformance mode to get actual carrier name
        char response[512];
        uart_flush_input(UART_NUM_1);
        const char* ctm_current = "AT+SQNCTM?\r\n";
        uart_write_bytes(UART_NUM_1, ctm_current, strlen(ctm_current));
        int len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 1000 / portTICK_PERIOD_MS);
        
        if (len > 0 && detected_carrier_out) {
            response[len] = '\0';
            __log_info("Current conformance mode: %s", response);
            
            // Extract carrier name from response (e.g., "+SQNCTM: verizon\r\n")
            char* carrier_start = strstr(response, "+SQNCTM:");
            if (carrier_start) {
                carrier_start += 8;  // Skip "+SQNCTM:"
                while (*carrier_start == ' ') carrier_start++;  // Skip spaces
                
                // Copy carrier name until \r or \n
                int i = 0;
                while (carrier_start[i] != '\0' && carrier_start[i] != '\r' && 
                       carrier_start[i] != '\n' && i < (int)detected_carrier_size - 1) {
                    detected_carrier_out[i] = carrier_start[i];
                    i++;
                }
                detected_carrier_out[i] = '\0';
                __log_info("Detected carrier: %s", detected_carrier_out);
            } else {
                __log_warn("Failed to parse SQNCTM response - defaulting to 'standard'");
                if (detected_carrier_size > 0) {
                    strncpy(detected_carrier_out, "standard", detected_carrier_size - 1);
                    detected_carrier_out[detected_carrier_size - 1] = '\0';
                }
            }
        }
        
        return true;  // Keep current configuration, but detected carrier is returned
    }
    
    __log_info("Checking carrier conformance mode configuration: %s", carrier);
    
    char response[512];
    int len;
    
    // Check current CFUN status
    uart_flush_input(UART_NUM_1);
    const char* cfun_query = "AT+CFUN?\r\n";
    uart_write_bytes(UART_NUM_1, cfun_query, strlen(cfun_query));
    len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 1000 / portTICK_PERIOD_MS);
    
    if (len > 0) {
        response[len] = '\0';
        if (!(strstr(response, "+CFUN: 0") || strstr(response, "+CFUN: 4"))) {
            __log_warn("CFUN not 0 or 4 - conformance mode can only be changed when radio is off");
            __log_warn("Skipping conformance mode configuration");
            return true;  // Not an error - just can't change it now
        }
        __log_info("CFUN is 0 or 4 - can safely configure conformance mode");
    }
    
    // Check current conformance mode
    uart_flush_input(UART_NUM_1);
    const char* ctm_current = "AT+SQNCTM?\r\n";
    uart_write_bytes(UART_NUM_1, ctm_current, strlen(ctm_current));
    len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 1000 / portTICK_PERIOD_MS);
    
    char detected_carrier[32] = {0};
    if (len > 0) {
        response[len] = '\0';
        __log_info("Current conformance mode: %s", response);
        
        // Extract carrier name from response (e.g., "+SQNCTM: verizon\r\n")
        char* carrier_start = strstr(response, "+SQNCTM:");
        if (carrier_start) {
            carrier_start += 8;  // Skip "+SQNCTM:"
            while (*carrier_start == ' ') carrier_start++;  // Skip spaces
            
            // Copy carrier name until \r or \n
            int i = 0;
            while (carrier_start[i] != '\0' && carrier_start[i] != '\r' && 
                   carrier_start[i] != '\n' && i < sizeof(detected_carrier) - 1) {
                detected_carrier[i] = carrier_start[i];
                i++;
            }
            detected_carrier[i] = '\0';
            
            // Trim trailing spaces
            while (i > 0 && detected_carrier[i-1] == ' ') {
                detected_carrier[--i] = '\0';
            }
            
            __log_info("Detected carrier: '%s'", detected_carrier);
        }
    }
    
    // Check if mode needs to be changed
    if (strstr(detected_carrier, carrier)) {
        __log_info("Conformance mode already set to: %s", carrier);
        return true;
    }
    
    __log_info("Carrier mismatch - detected '%s', requested '%s'", detected_carrier, carrier);
    
    // Validate carrier parameter against supported modes (if not "standard")
    if (strcmp(carrier, "standard") != 0) {
        uart_flush_input(UART_NUM_1);
        const char* ctm_query = "AT+SQNCTM=?\r\n";
        uart_write_bytes(UART_NUM_1, ctm_query, strlen(ctm_query));
        len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 1000 / portTICK_PERIOD_MS);
        
        if (len > 0) {
            response[len] = '\0';
            __log_info("Supported conformance modes: %s", response);
            
            if (!strstr(response, carrier)) {
                __log_error("Invalid carrier mode '%s' - not supported by modem", carrier);
                return false;
            }
        }
    }
    
    __log_info("Conformance mode mismatch - changing to: %s", carrier);
    
    // Set CFUN=4 if not already
    char cfun_response[256];
    uart_flush_input(UART_NUM_1);
    const char* cfun_query2 = "AT+CFUN?\r\n";
    uart_write_bytes(UART_NUM_1, cfun_query2, strlen(cfun_query2));
    len = uart_read_bytes(UART_NUM_1, cfun_response, sizeof(cfun_response) - 1, 1000 / portTICK_PERIOD_MS);
    
    if (len > 0) {
        cfun_response[len] = '\0';
        if (!strstr(cfun_response, "+CFUN: 4")) {
            __log_info("Setting CFUN=4 before conformance mode change");
            uart_flush_input(UART_NUM_1);
            const char* cfun4 = "AT+CFUN=4\r\n";
            uart_write_bytes(UART_NUM_1, cfun4, strlen(cfun4));
            len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 2000 / portTICK_PERIOD_MS);
            if (len <= 0 || !strstr(response, "OK")) {
                __log_warn("Failed to set CFUN=4");
            }
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
    
    // Set new conformance mode - modem will automatically reset
    char ctm_cmd[64];
    snprintf(ctm_cmd, sizeof(ctm_cmd), "AT+SQNCTM=\"%s\"\r\n", carrier);
    __log_info("Setting conformance mode: %s", ctm_cmd);
    
    uart_flush_input(UART_NUM_1);
    uart_write_bytes(UART_NUM_1, ctm_cmd, strlen(ctm_cmd));
    len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 1000 / portTICK_PERIOD_MS);
    
    if (len <= 0 || !strstr(response, "OK")) {
        __log_error("Failed to set conformance mode");
        return false;
    }
    
    __log_info("Conformance mode command sent, waiting for modem reset...");
    
    // Wait for +SHUTDOWN (GM02S sends +SHUTDOWN, Monarch 1 sends +SHDOWN)
    bool shutdown_received = false;
    for (int attempt = 0; attempt < 50; attempt++) {  // Max 5 seconds
        len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            response[len] = '\0';
            if (strstr(response, "+S")) {
                __log_info("Modem shutdown detected: %s", response);
                shutdown_received = true;
                break;
            }
        }
    }
    
    if (!shutdown_received) {
        __log_warn("Did not detect +SHUTDOWN - modem may still be resetting");
    }
    
    // Wait for +SYSSTART (modem reboot completion)
    __log_info("Waiting for +SYSSTART after conformance mode change...");
    bool sysstart_received = false;
    for (int attempt = 0; attempt < 100; attempt++) {  // Max 10 seconds
        len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 100 / portTICK_PERIOD_MS);
        if (len > 0) {
            response[len] = '\0';
            if (strstr(response, "+SYSSTART")) {
                __log_info("Modem rebooted successfully after conformance mode change");
                sysstart_received = true;
                break;
            }
        }
    }
    
    if (!sysstart_received) {
        __log_warn("+SYSSTART not detected - verifying modem responsiveness");
    }
    
    // Verify modem is responsive with AT commands (modem is back at 115200)
    __log_info("Verifying modem responsive at 115200 after reset...");
    for (int attempt = 0; attempt < 10; attempt++) {
        vTaskDelay(pdMS_TO_TICKS(500));
        uart_flush_input(UART_NUM_1);
        const char* at_cmd = "AT\r\n";
        uart_write_bytes(UART_NUM_1, at_cmd, strlen(at_cmd));
        len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 500 / portTICK_PERIOD_MS);
        if (len > 0) {
            response[len] = '\0';
            if (strstr(response, "OK")) {
                __log_info("Modem responsive after conformance mode change (attempt %d)", attempt + 1);
                
                // Set CFUN=4 after reset
                __log_info("Setting CFUN=4 after conformance mode change");
                uart_flush_input(UART_NUM_1);
                const char* cfun4 = "AT+CFUN=4\r\n";
                uart_write_bytes(UART_NUM_1, cfun4, strlen(cfun4));
                uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 2000 / portTICK_PERIOD_MS);
                
                return true;
            }
        }
        if (attempt == 9) {
            __log_error("Modem not responding after conformance mode change");
            return false;
        }
    }
    
    return true;
}/**
 * @brief Initialize modem handling three possible power states:
 * 
 * Scenario 1: Modem is powered off
 *   - Power on via IO expander
 *   - Wait for SYSSTART 
 *   - Modem boots to factory 115200 baud
 *   - Switch to 921600 (non-persistent) for CMUX
 * 
 * Scenario 2: Modem powered on in normal AT mode (firmware crash recovery)
 *   - Modem already at 115200 or 921600 (from previous AT+IPR)
 *   - Detect baudrate and switch to 921600 if needed
 *   - No PPP disconnect needed (we don't use old PPP mode anymore)
 * 
 * Scenario 3: Modem stuck in CMUX mode (crash during CMUX operation)
 *   - Modem at 921600 but won't respond to AT commands
 *   - Hardware reset via LTE_RST_N pin (P3 on IO expander)
 *   - Wait for SYSSTART and proceed as Scenario 1
 */
static esp_err_t initialize_modem_with_state_detection(void)
{
    __log_info("Initializing modem with intelligent state detection...");
    
    // Initialize IO expander to check power state
    ioexp_init();
    
    // Check if modem is already powered on
    bool modem_was_on = ioexp_lte_chip_power_status();
    
    if (modem_was_on) {
        __log_info("Modem already powered - checking communication state...");
        
        // Quick UART setup to test communication
        uart_config_t uart_config = {
            .baud_rate = 921600,  // Try 921600 first (CMUX baudrate or persistent IPR)
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
            .rx_flow_ctrl_thresh = 122,
        };
        
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, ESPMODEM_UART_TX_PIN, ESPMODEM_UART_RX_PIN, 
                                      ESPMODEM_UART_RTS_PIN, ESPMODEM_UART_CTS_PIN));
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0));
        
        // Test communication at 921600
        char response[256];
        uart_flush_input(UART_NUM_1);
        const char* at_cmd = "AT\r\n";
        uart_write_bytes(UART_NUM_1, at_cmd, strlen(at_cmd));
        int len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 
                                  1000 / portTICK_PERIOD_MS);
        
        bool responding_921600 = false;
        if (len > 0) {
            response[len] = '\0';
            if (strstr(response, "OK")) {
                __log_info("Modem responding at 921600 - normal AT mode recovery");
                responding_921600 = true;
                g_espmodem.status.current_baudrate = 921600;
            }
        }
        
        if (!responding_921600) {
            // Try 115200
            __log_info("No response at 921600, trying 115200...");
            uart_config.baud_rate = 115200;
            ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
            
            uart_flush_input(UART_NUM_1);
            uart_write_bytes(UART_NUM_1, at_cmd, strlen(at_cmd));
            len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 
                                  1000 / portTICK_PERIOD_MS);
            
            if (len > 0) {
                response[len] = '\0';
                if (strstr(response, "OK")) {
                    __log_info("Modem responding at 115200 - will switch to 921600");
                    g_espmodem.status.current_baudrate = 115200;
                    
                    // Switch to 921600 non-persistent (AT+IPR=921600,0)
                    const char* ipr_cmd = "AT+IPR=921600,0\r\n";
                    uart_write_bytes(UART_NUM_1, ipr_cmd, strlen(ipr_cmd));
                    
                    // Read with early exit on OK
                    int total_len = 0;
                    for (int attempts = 0; attempts < 20; attempts++) {
                        len = uart_read_bytes(UART_NUM_1, response + total_len, 
                                              sizeof(response) - total_len - 1,
                                              100 / portTICK_PERIOD_MS);
                        if (len > 0) {
                            total_len += len;
                            response[total_len] = '\0';
                            if (strstr(response, "OK")) {
                                __log_debug("AT+IPR=921600,0 successful");
                                break;
                            }
                        }
                    }
                    
                    vTaskDelay(pdMS_TO_TICKS(100));
                    
                    // Reconfigure UART to 921600
                    uart_config.baud_rate = 921600;
                    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
                    g_espmodem.status.current_baudrate = 921600;
                    __log_info("Switched to 921600 (non-persistent) for CMUX");
                    
                    responding_921600 = true;  // Now at 921600
                }
            }
        }
        
        if (!responding_921600) {
            // Scenario 3: Modem stuck in CMUX mode or unresponsive state
            __log_warn("Modem not responding at 921600 or 115200 - likely stuck in CMUX mode");
            __log_warn("Performing hardware reset via LTE_RST_N pin...");
            
            uart_driver_delete(UART_NUM_1);
            
            // Hardware reset via IO expander (LTE_RST_N on P3)
            ioexp_lte_chip_reset();
            
            __log_info("Modem reset via LTE_RST_N, waiting for +SYSSTART...");
            
            // Reinstall UART at 115200 (factory default after reset)
            uart_config.baud_rate = 115200;
            ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
            ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, ESPMODEM_UART_TX_PIN, ESPMODEM_UART_RX_PIN, 
                                          ESPMODEM_UART_RTS_PIN, ESPMODEM_UART_CTS_PIN));
            ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0));
            
            // Wait for +SYSSTART to confirm modem is ready
            bool sysstart_received = false;
            int total_len = 0;
            
            for (int boot_attempt = 0; boot_attempt < 80; boot_attempt++) {  // Max 8 seconds
                len = uart_read_bytes(UART_NUM_1, response + total_len,
                                      sizeof(response) - total_len - 1,
                                      100 / portTICK_PERIOD_MS);
                if (len > 0) {
                    total_len += len;
                    response[total_len] = '\0';
                    if (strstr(response, "+SYSSTART")) {
                        __log_info("Received +SYSSTART after reset");
                        sysstart_received = true;
                        break;
                    }
                }
            }
            
            if (!sysstart_received) {
                __log_warn("+SYSSTART not received after reset - proceeding anyway");
            }
            
            // Now switch to 921600 non-persistent
            uart_flush_input(UART_NUM_1);
            const char* ipr_cmd = "AT+IPR=921600,0\r\n";
            uart_write_bytes(UART_NUM_1, ipr_cmd, strlen(ipr_cmd));
            
            // Read with early exit on OK
            total_len = 0;
            for (int attempts = 0; attempts < 20; attempts++) {
                len = uart_read_bytes(UART_NUM_1, response + total_len,
                                      sizeof(response) - total_len - 1,
                                      100 / portTICK_PERIOD_MS);
                if (len > 0) {
                    total_len += len;
                    response[total_len] = '\0';
                    if (strstr(response, "OK")) {
                        __log_debug("AT+IPR=921600,0 successful after reset");
                        break;
                    }
                }
            }
            
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Reconfigure to 921600
            uart_config.baud_rate = 921600;
            ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
            g_espmodem.status.current_baudrate = 921600;
            __log_info("Hardware reset complete, modem at 921600 (non-persistent)");
        }
        
        // Clean up UART - ESP modem library will take over
        uart_driver_delete(UART_NUM_1);
        
    } else {
        // Scenario 1: Modem was powered off - clean boot
        // CRITICAL: Match LTE.py sequence - setup UART FIRST, then power on modem
        // This ensures UART is ready to receive +SYSSTART immediately when modem boots
        __log_info("Modem powered off - performing clean power-on sequence...");
        
        // Step 1: Setup UART at 115200 (factory default) BEFORE powering on modem
        __log_info("Setting up UART at 115200 baud...");
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
            .rx_flow_ctrl_thresh = 122,
        };
        
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, ESPMODEM_UART_TX_PIN, ESPMODEM_UART_RX_PIN, 
                                      ESPMODEM_UART_RTS_PIN, ESPMODEM_UART_CTS_PIN));
        ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0));

        uart_flush_input(UART_NUM_1);
        vTaskDelay(pdMS_TO_TICKS(100)); 
        
        // Step 2: Now power on the modem - UART is ready to receive +SYSSTART
        __log_info("UART ready, now powering on modem...");
        ioexp_lte_chip_power_on();
        
        // Wait for +SYSSTART URC which indicates modem has finished booting
        // GM02S sends this automatically within 2-3 seconds of power-on
        char response[512];
        bool sysstart_received = false;
        int total_len = 0;
        
        __log_info("Waiting for +SYSSTART (modem boot completion indicator)...");
        
        for (int boot_attempt = 0; boot_attempt < 80; boot_attempt++) {  // Max 8 seconds (80 x 100ms)
            int len = uart_read_bytes(UART_NUM_1, response + total_len, 
                                      sizeof(response) - total_len - 1,
                                      100 / portTICK_PERIOD_MS);
            if (len > 0) {
                total_len += len;
                response[total_len] = '\0';
                __log_info("Modem boot output: %s", response);
                if (strstr(response, "+SYSSTART")) {
                    __log_info("Received +SYSSTART after %d ms", (boot_attempt + 1) * 100);
                    sysstart_received = true;
                    break;
                }
            }
        }
        
        if (!sysstart_received) {
            __log_warn("+SYSSTART not received within 8 seconds - proceeding to verify with AT command");
        }
        
        // Now verify modem responds to AT commands
        // If +SYSSTART wasn't received, modem might still be booting, so retry AT several times
        uart_flush_input(UART_NUM_1);
        
        bool modem_ready = false;
        for (int at_retry = 0; at_retry < 10; at_retry++) {  // Try up to 10 times over 5 seconds
            if (at_retry > 0) {
                vTaskDelay(pdMS_TO_TICKS(500));  // Wait 500ms between retries
            }
            
            const char* at_cmd = "AT\r\n";
            uart_write_bytes(UART_NUM_1, at_cmd, strlen(at_cmd));
            
            int len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 
                                      500 / portTICK_PERIOD_MS);
            if (len > 0) {
                response[len] = '\0';
                if (strstr(response, "OK")) {
                    __log_info("Modem responding to AT commands (attempt %d)", at_retry + 1);
                    modem_ready = true;
                    break;
                }
            }
        }
        
        if (!modem_ready) {
            __log_error("Modem did not respond to AT command after 10 attempts - initialization may fail");
        }
        
        // Flush any remaining boot messages
        uart_flush_input(UART_NUM_1);
        
        // Configure carrier conformance mode BEFORE switching baudrate
        // If conformance mode needs to change, modem will reset to 115200
        // and we'll continue with the baudrate switch right after
        // If carrier is "None", we'll detect and store the actual carrier
        if (g_espmodem.carrier[0] != '\0') {
            __log_info("Configuring conformance mode before baudrate switch...");
            char detected_carrier[32] = {0};
            if (!configure_conformance_mode_at_115200(g_espmodem.carrier, detected_carrier, sizeof(detected_carrier))) {
                __log_error("Failed to configure conformance mode - continuing anyway");
            }
            // If carrier was "None", update to the detected value for SQNBANDSEL
            if (strcmp(g_espmodem.carrier, "None") == 0 && detected_carrier[0] != '\0') {
                strncpy(g_espmodem.carrier, detected_carrier, sizeof(g_espmodem.carrier) - 1);
                g_espmodem.carrier[sizeof(g_espmodem.carrier) - 1] = '\0';
                __log_info("Updated carrier from 'None' to detected value: %s", g_espmodem.carrier);
            }
        }
        
        // Switch to 921600 non-persistent (AT+IPR=921600,0)
        const char* ipr_cmd = "AT+IPR=921600,0\r\n";
        uart_write_bytes(UART_NUM_1, ipr_cmd, strlen(ipr_cmd));
        
        // Read response with early exit on "OK"
        total_len = 0;  // Reuse existing total_len variable
        for (int attempts = 0; attempts < 20; attempts++) {  // Max 2 seconds
            int len = uart_read_bytes(UART_NUM_1, response + total_len, 
                                      sizeof(response) - total_len - 1, 
                                      100 / portTICK_PERIOD_MS);
            if (len > 0) {
                total_len += len;
                response[total_len] = '\0';
                // Check if we got OK - exit early
                if (strstr(response, "OK")) {
                    __log_debug("AT+IPR=921600,0 successful");
                    break;
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));  // Brief settling time
        
        // Reconfigure UART to 921600
        uart_config.baud_rate = 921600;
        ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
        g_espmodem.status.current_baudrate = 921600;
        __log_info("Clean boot complete, modem at 921600 (non-persistent)");
        
        // Clean up UART - ESP modem library will take over
        uart_driver_delete(UART_NUM_1);
    }
    
    g_espmodem.status.powered = true;
    __log_info("Modem initialization complete - ready for ESP modem library at 921600 baud");
    return ESP_OK;
}

// ============================================================================
// ESP Event Handlers (from successful cmux_sequans_test.c)
// ============================================================================

static void on_ip_event(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    if (event_id == IP_EVENT_PPP_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        
        __log_info("PPP connection established!");
        __log_info("IP: " IPSTR, IP2STR(&event->ip_info.ip));
        __log_info("Gateway: " IPSTR, IP2STR(&event->ip_info.gw));
        
        g_espmodem.status.ppp_connected = true;
        xEventGroupSetBits(g_espmodem.event_group, ESPMODEM_CONNECT_BIT);
        
        // Dispatch PPP_CONNECTED event to unified event handler
        if (g_event_handler) {
            // Allocate strings for IP addresses (will be copied by event handler)
            char ip_str[16], netmask_str[16], gw_str[16], dns1_str[16], dns2_str[16];
            snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
            snprintf(netmask_str, sizeof(netmask_str), IPSTR, IP2STR(&event->ip_info.netmask));
            snprintf(gw_str, sizeof(gw_str), IPSTR, IP2STR(&event->ip_info.gw));
            
            // Get DNS servers
            dns1_str[0] = '\0';
            dns2_str[0] = '\0';
            esp_netif_dns_info_t dns_info;
            if (esp_netif_get_dns_info(g_espmodem.esp_netif, ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
                snprintf(dns1_str, sizeof(dns1_str), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
            }
            if (esp_netif_get_dns_info(g_espmodem.esp_netif, ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
                snprintf(dns2_str, sizeof(dns2_str), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
            }
            
            espmodem_event_t evt = {
                .event_type = ESPMODEM_EVENT_PPP_CONNECTED,
                .ppp = {
                    .connected = true,
                    .ip = ip_str,
                    .netmask = netmask_str,
                    .gateway = gw_str,
                    .dns1 = dns1_str[0] ? dns1_str : NULL,
                    .dns2 = dns2_str[0] ? dns2_str : NULL
                }
            };
            dispatch_event(&evt);
        }
        
        // Trigger network callback (legacy)
        if (g_espmodem.network_cb) {
            espmodem_network_info_t info;
            sprintf(info.ip, IPSTR, IP2STR(&event->ip_info.ip));
            sprintf(info.gateway, IPSTR, IP2STR(&event->ip_info.gw));
            sprintf(info.netmask, IPSTR, IP2STR(&event->ip_info.netmask));
            info.connected = true;
            g_espmodem.network_cb(true, &info);
        }
        
    } else if (event_id == IP_EVENT_PPP_LOST_IP) {
        __log_warn("PPP connection lost");
        g_espmodem.status.ppp_connected = false;
        g_espmodem.status.cmux_active = false;
        xEventGroupSetBits(g_espmodem.event_group, ESPMODEM_DISCONNECT_BIT);
        
        // NOTE: We do NOT exit CMUX mode here because:
        // 1. The ESP netif is already handling PPP shutdown
        // 2. Calling esp_modem_set_mode() here causes recursive events
        // 3. The modem can stay in CMUX mode - next connect() will handle it
        // The flags above ensure connect() knows to re-establish the connection
        __log_debug("Internal state updated - next connect() will re-establish");
        
        // Dispatch PPP_DISCONNECTED event to unified event handler
        if (g_event_handler) {
            espmodem_event_t evt = {
                .event_type = ESPMODEM_EVENT_PPP_DISCONNECTED,
                .ppp = {
                    .connected = false,
                    .ip = NULL,
                    .netmask = NULL,
                    .gateway = NULL,
                    .dns1 = NULL,
                    .dns2 = NULL
                }
            };
            dispatch_event(&evt);
        }
        
        if (g_espmodem.network_cb) {
            g_espmodem.network_cb(false, NULL);
        }
    }
}

static void on_ppp_changed(void *arg, esp_event_base_t event_base,
                           int32_t event_id, void *event_data)
{
    __log_debug("PPP state changed event %d", (int)event_id);
}

// ============================================================================
// Initialization and Configuration
// ============================================================================
// Core Public Functions
// ============================================================================

espmodem_err_t espmodem_init(const char* carrier)
{
    if (g_espmodem.initialized) {
        __log_info("ESP modem already initialized");
        
        // Check if user is trying to change conformance mode
        const char* requested_carrier = carrier ? carrier : "None";
        if (strcmp(g_espmodem.carrier, requested_carrier) != 0) {
            __log_info("Conformance mode change requested: '%s' -> '%s'", 
                      g_espmodem.carrier, requested_carrier);
            
            // If requesting "None", no change needed
            if (strcmp(requested_carrier, "None") == 0) {
                __log_info("Requested 'None' - keeping current mode '%s'", g_espmodem.carrier);
                return ESPMODEM_OK;
            }
            
            // Change conformance mode at 921600 (will trigger reset)
            espmodem_err_t err = espmodem_change_conformance_mode(requested_carrier);
            if (err != ESPMODEM_OK) {
                __log_error("Failed to change conformance mode");
            }
            return err;
        }
        
        return ESPMODEM_OK;
    }
    
    // Reserve UART1 for LTE modem use (context 2 = C lte module namespace)
    // Context 2 is used to distinguish from LTE.py (context 1) and user code (context 0)
    if (!uart_reserve(1, UART_OWNER_LTE_ESPMODEM, "LTE modem (import lte)", 2)) {
        const char* owner = uart_get_owner_description(1);
        __log_error("Cannot initialize LTE: UART1 already in use by %s", owner ? owner : "another component");
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    __log_info("Initializing ESP modem middle layer with PSRAM-optimized coexistence mode...");
    
    // Enable detailed ESP modem library debugging BEFORE creating DCE
    // Based on actual ESP modem library source code tags:
    // - "esp-modem": Hex dumps of PPP/CMD data
    // - "DCE mode": Mode transition logging
    // - "command_lib": AT command execution
    // - "uart_terminal": UART I/O
    // - "modem_api": API calls
    // esp_log_level_set("esp-modem", ESP_LOG_VERBOSE);      // Main library tag
    // esp_log_level_set("DCE mode", ESP_LOG_VERBOSE);       // Mode transitions (CRITICAL)
    // esp_log_level_set("command_lib", ESP_LOG_VERBOSE);    // AT commands
    // esp_log_level_set("uart_terminal", ESP_LOG_VERBOSE);  // UART traffic
    // esp_log_level_set("modem_api", ESP_LOG_VERBOSE);      // API calls
    // esp_log_level_set("modem_api_target", ESP_LOG_VERBOSE);
    // esp_log_level_set("esp_netif_lwip", ESP_LOG_DEBUG);
    // esp_log_level_set("esp-netif_lwip-ppp", ESP_LOG_DEBUG);
    // __log_info("ESP modem library debug logging enabled (all actual library tags)");
    
    // Initialize state
    memset(&g_espmodem, 0, sizeof(g_espmodem));
    
    // Store carrier conformance mode
    // "None" means user didn't specify (use modem default, only fix if non-standard)
    // Any other value (including explicit "standard") means user wants that specific mode
    if (!carrier) {
        carrier = "None";  // Default if NULL passed
    }
    strncpy(g_espmodem.carrier, carrier, sizeof(g_espmodem.carrier) - 1);
    g_espmodem.carrier[sizeof(g_espmodem.carrier) - 1] = '\0';
    __log_info("Carrier conformance mode requested: %s", g_espmodem.carrier);
    
    // Initialize ESP netif and event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));
    
    // Create event group
    g_espmodem.event_group = xEventGroupCreate();
    if (!g_espmodem.event_group) {
        __log_error("Failed to create event group");
        return ESPMODEM_ERR_NO_MEM;
    }
    
    // Initialize modem with intelligent state detection
    // Handles: 1) Clean power-on, 2) Crash recovery, 3) CMUX stuck state
    esp_err_t err = initialize_modem_with_state_detection();
    if (err != ESP_OK) {
        __log_error("Modem initialization failed");
        return ESPMODEM_ERR_POWER_FAILED;
    }
    
    // Create ESP modem DCE instance for AT commands
    __log_info("Creating ESP modem DCE instance with 921600 baud...");
    
    // CRITICAL: Create netif and DCE together like the working test
    // This ensures ESP modem library properly sets up PPP output callbacks
    esp_netif_config_t netif_ppp_config = ESP_NETIF_DEFAULT_PPP();
    g_espmodem.esp_netif = esp_netif_new(&netif_ppp_config);
    if (!g_espmodem.esp_netif) {
        __log_error("Failed to create PPP netif");
        return ESPMODEM_ERR_NO_MEM;
    }
    
    // Configure DCE - APN will be set later in connect() via esp_modem_sqn_gm02s_connect()
    // For now, use placeholder since SQNGM02S doesn't use DCE config APN
    esp_modem_dce_config_t dce_config = ESP_MODEM_DCE_DEFAULT_CONFIG("placeholder");
    
    // PSRAM-optimized allocation strategy for WiFi/LTE coexistence:
    // 1. UART buffers: Minimal in internal RAM (DMA/ISR requirement)
    // 2. ESP modem DTE buffer: Large in PSRAM (application level)
    // 3. Task stacks: In PSRAM (FreeRTOS supports external stacks)
    esp_modem_dte_config_t dte_config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    
    // UART buffers - sized for high-throughput HTTP/HTTPS over PPP/CMUX
    dte_config.uart_config.rx_buffer_size = 4096;       // 4KB for HTTP responses, internal RAM
    dte_config.uart_config.tx_buffer_size = 2048;       // 2KB for HTTP requests, internal RAM  
    dte_config.uart_config.event_queue_size = 20;       // Reasonable event queue
    
    // Application buffers - can be larger since PSRAM is available
    dte_config.dte_buffer_size = 2048;                   // Larger DTE buffer (in PSRAM via malloc)
    dte_config.task_stack_size = 4096;                   // Full task stack (in PSRAM if configured)
    dte_config.task_priority = 4;                        // WiFi-friendly priority
    
    __log_info("UART buffers: RX=%d, TX=%d (internal RAM)", 
               dte_config.uart_config.rx_buffer_size, 
               dte_config.uart_config.tx_buffer_size);
    __log_info("Application buffers: DTE=%d, Stack=%d (PSRAM preferred)", 
               dte_config.dte_buffer_size, 
               dte_config.task_stack_size);
    
    dte_config.uart_config.tx_io_num = ESPMODEM_UART_TX_PIN;
    dte_config.uart_config.rx_io_num = ESPMODEM_UART_RX_PIN;
    dte_config.uart_config.rts_io_num = ESPMODEM_UART_RTS_PIN;
    dte_config.uart_config.cts_io_num = ESPMODEM_UART_CTS_PIN;
    dte_config.uart_config.flow_control = ESP_MODEM_FLOW_CONTROL_HW;
    dte_config.uart_config.baud_rate = g_espmodem.status.current_baudrate;
    
    // Use SQNGM02S device type for Sequans GM02S modem
    // The SQNGM02S class has native support for correct AT command sequencing:
    g_espmodem.dce = esp_modem_new_dev(ESP_MODEM_DCE_SQNGM02S, &dte_config, &dce_config, g_espmodem.esp_netif);
    if (!g_espmodem.dce) {
        __log_error("Failed to create ESP modem DCE with SQNGM02S device type");
        esp_netif_destroy(g_espmodem.esp_netif);
        g_espmodem.esp_netif = NULL;
        return ESPMODEM_ERR_NO_MEM;
    }
    
    // Verify netif is properly configured after DCE creation
    __log_info("ESP modem DCE created - verifying netif configuration...");
    const char* netif_desc = esp_netif_get_desc(g_espmodem.esp_netif);
    __log_info("Netif description: %s", netif_desc ? netif_desc : "NULL");
    
    // Check if netif has been properly attached by ESP modem library
    esp_netif_flags_t flags = esp_netif_get_flags(g_espmodem.esp_netif);
    __log_info("Netif flags: 0x%x", flags);
    
    // Get the driver handle to verify attachment
    void* driver_handle = esp_netif_get_io_driver(g_espmodem.esp_netif);
    __log_info("Netif driver handle: %p", driver_handle);
    if (driver_handle) {
        __log_info("✓ Netif properly attached to ESP modem driver - PPP should work");
    } else {
        __log_warn("WARNING: Netif driver handle is NULL - checking configuration...");
    }
    
    // Synchronize communication - wait for modem to be fully responsive
    __log_info("Synchronizing modem communication - waiting for modem to be fully ready...");
    
    // First attempt with esp_modem_sync()
    atomic_store(&g_espmodem.command_in_progress, true);
    esp_err_t sync_err = esp_modem_sync(g_espmodem.dce);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (sync_err == ESP_OK) {
        __log_info("✓ Modem sync successful on first attempt");
    } else {
        __log_warn("Modem sync returned %s - will actively wait for modem readiness", esp_err_to_name(sync_err));
        
        // Keep sending AT commands until modem responds (max 10 attempts, 500ms apart)
        char response[64];
        bool modem_ready = false;
        for (int attempt = 0; attempt < 10; attempt++) {
            vTaskDelay(pdMS_TO_TICKS(500));  // Wait 500ms between attempts
            
            atomic_store(&g_espmodem.command_in_progress, true);
            esp_err_t at_err = esp_modem_at(g_espmodem.dce, "AT", response, 2000);
            atomic_store(&g_espmodem.command_in_progress, false);
            __log_debug("AT command response: %s", response);
            if (at_err == ESP_OK && strstr(response, "OK")) {
                __log_info("✓ Modem responded after %d attempts - now fully ready", attempt + 1);
                modem_ready = true;
                break;
            }
            
            __log_debug("Modem sync attempt %d/10 - no response yet", attempt + 1);
        }
        
        if (!modem_ready) {
            __log_error("Modem failed to respond after 10 attempts - initialization may fail");
            // Don't return error - continue and let retry mechanism handle it
        }
    }
    
    // ESP modem DCE is ready - let the library manage its own initialization
    __log_info("ESP modem DCE ready - library will manage mode transitions automatically");
    
    // Register terminal error callback to detect UART break signals and other errors
    // This callback is triggered by ESP modem library when:
    // - UART_BREAK events occur (modem reset, crash, or PPP drop)
    // - UART_FRAME_ERR events occur (baudrate mismatch, electrical issues)
    // - Buffer overflows happen (data loss)
    esp_err_t err_cb_result = esp_modem_set_error_cb(g_espmodem.dce, modem_terminal_error_handler);
    if (err_cb_result == ESP_OK) {
        __log_info("Terminal error callback registered - break signals will be detected");
    } else {
        __log_warn("Failed to register terminal error callback: %s", esp_err_to_name(err_cb_result));
    }
    
    // Check initial ESP modem mode after DCE creation
    esp_modem_dce_mode_t initial_mode = esp_modem_get_mode(g_espmodem.dce);
    __log_info("Initial ESP modem mode after DCE creation: %d", initial_mode);
    
    // Don't try to force mode changes during init - let operations handle this
    __log_info("ESP modem DCE created successfully, AT commands now available");
    
    // Conformance mode was already configured during initialize_modem_with_state_detection()
    // at 115200 baud before switching to 921600. This is much cleaner because if the
    // modem resets during conformance mode change, it stays at 115200 and we just
    // continue with the normal baudrate switch sequence.
    __log_info("Conformance mode already configured during modem initialization");
    
    // CRITICAL: Always register URC handler during init, even without user callback
    // This ensures internal tracking works (+SHUTDOWN detection for resets)
    // The handler will only forward URCs to Python if g_espmodem.unsolicited_cb is set
#ifdef CONFIG_ESP_MODEM_URC_HANDLER
    esp_err_t urc_err = esp_modem_set_urc(g_espmodem.dce, unsolicited_handler);
    if (urc_err == ESP_OK) {
        __log_info("URC handler registered (internal tracking active, user callback: %s)",
                  g_espmodem.unsolicited_cb ? "set" : "not set");
    } else {
        __log_error("Failed to register URC handler: %s", esp_err_to_name(urc_err));
        __log_error("WARNING: +SHUTDOWN detection will not work without URC handler!");
    }
#else
    __log_warn("CONFIG_ESP_MODEM_URC_HANDLER not enabled - URC tracking disabled");
#endif
    
    g_espmodem.initialized = true;
    __log_info("ESP modem middle layer initialized successfully");
    
    // Start independent recovery monitoring task
    // This task runs continuously and watches for crash detection flag
    // It's independent of the UART task so it can safely destroy DCE without deadlock
    BaseType_t task_result = xTaskCreate(
        espmodem_recovery_monitor_task,
        "lte_recovery",
        4096,
        NULL,
        5,  // Higher priority than normal tasks
        &g_espmodem.recovery_monitor_task
    );
    
    if (task_result != pdPASS) {
        __log_error("Failed to create recovery monitoring task");
        return ESPMODEM_ERR_NO_MEM;
    }
    
    __log_info("Recovery monitoring task started - automatic crash recovery enabled");
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_attach(const char* apn, const char* type, int cid, int band, const char* bands)
{
    if (!g_espmodem.initialized || !apn) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    // Validate CID (typically 1-16 depending on modem)
    if (cid < 1 || cid > 16) {
        __log_error("Invalid CID: %d (must be 1-16)", cid);
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    // Default to IP if type not specified
    if (!type || strlen(type) == 0) {
        type = "IP";
    }
    
    // Store APN for later use (GM02S needs consistent APN across commands)
    strncpy(g_espmodem.configured_apn, apn, sizeof(g_espmodem.configured_apn) - 1);
    g_espmodem.configured_apn[sizeof(g_espmodem.configured_apn) - 1] = '\0';
    
    // Store CID for connect() - Verizon requires CID=3, default is 1
    g_espmodem.configured_cid = cid;
    g_espmodem_configured_cid = cid;  // Also update global for C++ ESP modem library access
    __log_info("Stored CID=%d for PPP connection", cid);
    
    if (bands) {
        __log_info("Initiating network attachment with APN: %s, Type: %s, Bands: %s", apn, type, bands);
    } else if (band > 0) {
        __log_info("Initiating network attachment with APN: %s, Type: %s, Band: %d", apn, type, band);
    } else {
        __log_info("Initiating network attachment with APN: %s, Type: %s (auto band)", apn, type);
    }
    
    // DCE should already exist from init, but check just in case
    if (!g_espmodem.dce) {
        __log_error("ESP modem DCE not initialized - call espmodem_init() first");
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    char response[256];
    esp_err_t err;

    // Step 1: Check SIM status using consolidated function
    __log_info("Checking SIM status...");
    espmodem_err_t sim_err = espmodem_check_sim_present();
    if (sim_err != ESPMODEM_OK) {
        return sim_err;  // Error already logged in espmodem_check_sim_present()
    }
    
    // Step 2: Check radio status (CFUN)
    bool radio_enabled = false;
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+CFUN?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err == ESP_OK && strstr(response, "+CFUN: 1")) {
        radio_enabled = true;
        __log_info("Radio is enabled (CFUN=1)");
    }
    
    // Step 3: Configure band selection if specified (requires getting current mode first)
    if (band > 0 || bands) {
        // Query current mode from modem
        // AT+SQNMODEACTIVE returns: 1=CAT-M1, 2=NB-IoT
        // AT+SQNBANDSEL expects: 0=CAT-M1, 1=NB-IoT (mode - 1)
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT+SQNMODEACTIVE?", response, 5000);
        atomic_store(&g_espmodem.command_in_progress, false);
        int sqn_mode = 0; // Default to CAT-M1 mode for SQNBANDSEL (0)
        if (err == ESP_OK) {
            if (strstr(response, "+SQNMODEACTIVE: 1")) {
                sqn_mode = 0; // CAT-M1 (SQNMODEACTIVE returns 1, SQNBANDSEL needs 0)
                __log_info("Current mode: CAT-M1");
            } else if (strstr(response, "+SQNMODEACTIVE: 2")) {
                sqn_mode = 1; // NB-IoT (SQNMODEACTIVE returns 2, SQNBANDSEL needs 1)
                __log_info("Current mode: NB-IoT");
            }
        } else {
            __log_warn("Failed to query mode, using default CAT-M1");
        }
        
        // AT+SQNBANDSEL=<mode>,"<carrier>","<band>" or "<bands>"
        char bandsel_cmd[128];
        if (bands) {
            __log_info("Configuring band selection: %s", bands);
            snprintf(bandsel_cmd, sizeof(bandsel_cmd), 
                     "AT+SQNBANDSEL=%d,\"%s\",\"%s\"", 
                     sqn_mode, g_espmodem.carrier, bands);
        } else {
            __log_info("Configuring band selection: %d", band);
            snprintf(bandsel_cmd, sizeof(bandsel_cmd), 
                     "AT+SQNBANDSEL=%d,\"%s\",\"%d\"", 
                     sqn_mode, g_espmodem.carrier, band);
        }
        
        err = esp_modem_at_with_retry(g_espmodem.dce, bandsel_cmd, response, 5000, AT_CMD_MAX_RETRIES);
        if (err != ESP_OK) {
            __log_error("Failed to configure band selection after retries");
            return ESPMODEM_ERR_NETWORK_FAILED;
        }
        
        if (bands) {
            __log_info("Bands %s configured successfully", bands);
        } else {
            __log_info("Band %d configured successfully", band);
        }
    }
    
    // Step 4: Check if APN configuration needs to be updated
    bool apn_needs_update = false;
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+CGDCONT?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err == ESP_OK) {
        // Check if our APN is in the response
        if (!strstr(response, apn)) {
            apn_needs_update = true;
            __log_info("APN not configured or changed, needs update");
        } else {
            __log_info("APN already configured correctly");
        }
    } else {
        // If query fails, assume we need to configure
        apn_needs_update = true;
    }
    
    if (apn_needs_update) {
        // If radio is enabled, set CFUN=4 before changing APN
        if (radio_enabled) {
            __log_info("Setting CFUN=4 to reconfigure APN...");
            err = esp_modem_at_with_retry(g_espmodem.dce, "AT+CFUN=4", response, 10000, AT_CMD_MAX_RETRIES);
            if (err != ESP_OK) {
                __log_error("Failed to set CFUN=4 after retries");
                return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
            }
            vTaskDelay(pdMS_TO_TICKS(500));
            radio_enabled = false; // Will need to re-enable
        }
        
        // Configure APN with specified type and CID
        char cgdcont_cmd[128];
        snprintf(cgdcont_cmd, sizeof(cgdcont_cmd), "AT+CGDCONT=%d,\"%s\",\"%s\"", cid, type, apn);
        err = esp_modem_at_with_retry(g_espmodem.dce, cgdcont_cmd, response, 5000, AT_CMD_MAX_RETRIES);
        if (err != ESP_OK) {
            __log_error("Failed to configure APN after retries");
            return ESPMODEM_ERR_NETWORK_FAILED;
        }
        __log_info("APN configured: %s", apn);
    }
    
    // Step 5: Enable radio and start network search (CFUN=1)
    __log_info("Enabling radio and starting network search (CFUN=1)...");
    err = esp_modem_at_with_retry(g_espmodem.dce, "AT+CFUN=1", response, 10000, AT_CMD_MAX_RETRIES);
    if (err != ESP_OK) {
        __log_error("Failed to enable radio after retries");
        return ESPMODEM_ERR_NETWORK_FAILED;
    }
    
    // NON-BLOCKING: Return immediately after starting attachment
    // User should poll with isattached() to check status
    __log_info("Network search initiated - use isattached() to monitor registration");
    __log_info("Note: NB-IoT initial attachment may take several minutes");
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_connect(void)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    __log_info("Activating CMUX mode for PPP data connectivity...");
    
    // Check the ACTUAL modem mode first (don't rely solely on our flags)
    // The modem might still be in CMUX from a previous connection
    esp_modem_dce_mode_t current_mode = esp_modem_get_mode(g_espmodem.dce);
    __log_debug("Current modem mode: %d (0=COMMAND, 2=CMUX, 9=UNDEF)", current_mode);
    
    // If already in CMUX and PPP connected, nothing to do
    if (current_mode == ESP_MODEM_MODE_CMUX && g_espmodem.status.ppp_connected) {
        __log_info("Already in CMUX mode and PPP connected");
        return ESPMODEM_OK;
    }
    
    // If in CMUX but not connected, we need to cycle the mode to restart PPP
    // Simply staying in CMUX won't trigger a fresh PPP negotiation
    if (current_mode == ESP_MODEM_MODE_CMUX && !g_espmodem.status.ppp_connected) {
        __log_info("Already in CMUX mode but PPP not connected - cycling mode to restart");
        __log_info("Exiting CMUX mode first...");
        
        esp_err_t exit_err = esp_modem_set_mode(g_espmodem.dce, ESP_MODEM_MODE_COMMAND);
        if (exit_err != ESP_OK) {
            __log_error("Failed to exit CMUX mode: %s", esp_err_to_name(exit_err));
            // Continue anyway - might still work
        }
        
        esp_modem_dce_mode_t mode_after_exit = esp_modem_get_mode(g_espmodem.dce);
        __log_info("Mode after exit: %d (should be 0=COMMAND)", mode_after_exit);
        
        // Give modem AND esp_netif time to fully process the mode transition
        // The esp_netif needs time to stop PPP before we can restart it
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Now fall through to normal CMUX activation below
        current_mode = mode_after_exit;
    }
    
    bool skip_cmux_activation = false;  // Always activate CMUX after cycling
    
    // Verify network is attached before attempting CMUX
    if (!espmodem_is_attached()) {
        __log_error("Network not attached - call attach() first and wait for registration");
        return ESPMODEM_ERR_NETWORK_FAILED;
    }
    
    // Only activate CMUX if not already in CMUX mode
    if (!skip_cmux_activation) {
        // Check current mode
        esp_modem_dce_mode_t current_mode = esp_modem_get_mode(g_espmodem.dce);
        __log_info("ESP modem mode before CMUX: %d (0=COMMAND, 2=CMUX, 9=UNDEF)", current_mode);
        __log_info("Calling esp_modem_set_mode(ESP_MODEM_MODE_CMUX=%d)...", ESP_MODEM_MODE_CMUX);
        
        esp_err_t cmux_err = esp_modem_set_mode(g_espmodem.dce, ESP_MODEM_MODE_CMUX);
        
        __log_info("esp_modem_set_mode returned: %s (0x%x)", esp_err_to_name(cmux_err), cmux_err);
        
        // Check the actual mode - the library sometimes returns ESP_FAIL even when mode transition succeeded
        esp_modem_dce_mode_t final_mode = esp_modem_get_mode(g_espmodem.dce);
        __log_info("Mode after esp_modem_set_mode call: %d", final_mode);
        
        // WORKAROUND: ESP modem library can return ESP_FAIL even when CMUX activation succeeds
        // The actual mode is the source of truth - if we're in CMUX mode (2), it worked!
        if (final_mode != ESP_MODEM_MODE_CMUX) {
            __log_error("Failed to activate CMUX mode - final mode: %d", final_mode);
            __log_error("ESP modem library returned: %s", esp_err_to_name(cmux_err));
            return ESPMODEM_ERR_CMUX_FAILED;
        }
        
        __log_info("✓ CMUX mode successfully activated (mode=2)");
        __log_info("This is a known ESP modem library quirk - mode transition succeeded");
    }
    
    // Update state - whether we just activated CMUX or it was already active
    g_espmodem.status.cmux_active = true;
    
    // NOTE: esp_modem_set_mode(ESP_MODEM_MODE_CMUX) automatically calls setup_data_mode()
    // which sends the dial command (ATD*99#) to start PPP. This happens BOTH on initial
    // connection AND after cycling the mode for reconnection. No manual dial needed!
    
    // PPP negotiation should now start automatically in background via esp_modem_netif glue
    __log_info("CMUX active - PPP negotiation should be starting");
    __log_info("Use isconnected() to monitor PPP connection status");
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_disconnect(void)
{
    if (!g_espmodem.initialized) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    if (g_espmodem.dce && g_espmodem.status.ppp_connected) {
        esp_err_t err = esp_modem_set_mode(g_espmodem.dce, ESP_MODEM_MODE_COMMAND);
        if (err == ESP_OK) {
            g_espmodem.status.ppp_connected = false;
            g_espmodem.status.cmux_active = false;
            __log_info("Disconnected from PPP");
        }
        return (err == ESP_OK) ? ESPMODEM_OK : ESPMODEM_ERR_PPP_FAILED;
    }
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_connect_with_params(const char* apn, int band, espmodem_mode_t mode)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    __log_info("Connecting to network with parameters: APN='%s', band=%d, mode=%d", 
               apn ? apn : "default", band, mode);
    
    // For now, just use the existing atomic connect with default parameters
    // TODO: Modify the existing connect implementation to use these parameters
    // This would involve updating the attachment sequence to use the provided APN and band
    
    return espmodem_connect();
}

espmodem_err_t espmodem_deinit(bool power_off)
{
    if (!g_espmodem.initialized) {
        return ESPMODEM_OK;
    }
    
    __log_info("Deinitializing ESP modem...");
    
    // Cleanup ESP modem
    if (g_espmodem.dce) {
        esp_modem_destroy(g_espmodem.dce);
        g_espmodem.dce = NULL;
    }
    
    if (g_espmodem.esp_netif) {
        esp_netif_destroy(g_espmodem.esp_netif);
        g_espmodem.esp_netif = NULL;
    }
    
    if (power_off) {
        // Power off modem using SG-SDK ioexp interface
        ioexp_lte_chip_power_off();
    }
    
    // Cleanup event group
    if (g_espmodem.event_group) {
        vEventGroupDelete(g_espmodem.event_group);
    }
    
    // Release UART1 reservation (context 1 = system/LTE namespace)
    uart_release(1, UART_OWNER_LTE_ESPMODEM, 1);
    
    memset(&g_espmodem, 0, sizeof(g_espmodem));
    
    __log_info("ESP modem deinitialized");
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_power_on(void)
{
    __log_info("Powering on LTE modem");
    
    // Initialize IO expander if not already done
    ioexp_init();
    
    // Power on the modem
    ioexp_lte_chip_power_on();
    
    // Update status
    g_espmodem.status.powered = true;
    
    __log_info("LTE modem powered on");
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_power_off(bool force)
{
    __log_info("Powering off LTE modem (force=%d)", force);
    
    if (!force && g_espmodem.initialized) {
        // Graceful shutdown: disconnect and detach first
        if (g_espmodem.status.ppp_connected) {
            __log_info("Disconnecting PPP before power off");
            espmodem_disconnect();
        }
        
        if (g_espmodem.status.network_attached) {
            __log_info("Detaching from network before power off");
            espmodem_detach();
        }
    }
    
    espmodem_deinit(true);  // Always power off the modem hardware
    
    __log_info("LTE modem powered off");
    return ESPMODEM_OK;
}

bool espmodem_is_powered(void)
{
    // Check actual hardware state via ioexp
    return ioexp_lte_chip_power_status();
}

bool espmodem_is_initialized(void)
{
    // Check actual hardware state via ioexp
    return g_espmodem.initialized;
}

// ============================================================================
// Status and Information Functions
// ============================================================================

bool espmodem_is_attached(void)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return false;
    }
    
    // Query modem for current registration status
    char response[128];
    atomic_store(&g_espmodem.command_in_progress, true);
    esp_err_t err = esp_modem_at(g_espmodem.dce, "AT+CEREG?", response, 3000);
    atomic_store(&g_espmodem.command_in_progress, false);
    __log_debug("AT+CEREG? response: %s", response);
    
    if (err == ESP_OK) {
        // Check for registered states following LTE.py logic:
        // Must have "+CEREG:" prefix to avoid false positives
        // Format: +CEREG: <n>,<stat>[,<tac>,<ci>[,<AcT>]]
        // Where <n> is 1 or 2 (reporting mode) and <stat> is 1 (home) or 5 (roaming)
        if (strstr(response, "+CEREG: 1,1") || strstr(response, "+CEREG: 1,5") ||
            strstr(response, "+CEREG: 2,1") || strstr(response, "+CEREG: 2,5") ||
            strstr(response, "+CEREG: 3,1") || strstr(response, "+CEREG: 3,5") ||
            strstr(response, "+CEREG: 4,1") || strstr(response, "+CEREG: 4,5") ||
            strstr(response, "+CEREG: 5,1") || strstr(response, "+CEREG: 5,5")) {
            g_espmodem.status.network_attached = true;
            return true;
        }
    }
    
    g_espmodem.status.network_attached = false;
    return false;
}

bool espmodem_is_connected(void)
{
    return g_espmodem.status.ppp_connected;
}

bool espmodem_is_active(void)
{
    return g_espmodem.initialized && g_espmodem.dce != NULL;
}

espmodem_err_t espmodem_wait_for_recovery(uint32_t timeout_ms)
{
    if (!g_espmodem.recovery_in_progress) {
        return ESPMODEM_OK;  // Not in recovery, proceed immediately
    }
    
    __log_info("Crash recovery in progress - waiting for completion (timeout: %lu ms)...", 
              (unsigned long)timeout_ms);
    
    // Wait for the independent monitoring task to complete recovery
    uint32_t start = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while (g_espmodem.recovery_in_progress) {
        uint32_t elapsed = (xTaskGetTickCount() * portTICK_PERIOD_MS) - start;
        if (elapsed >= timeout_ms) {
            __log_error("Timeout waiting for crash recovery to complete");
            return ESPMODEM_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(100));  // Check every 100ms
    }
    
    __log_info("Crash recovery completed, resuming operation");
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_get_status(espmodem_status_t* status)
{
    if (!status) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    memcpy(status, &g_espmodem.status, sizeof(espmodem_status_t));
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_get_error_stats(espmodem_error_stats_t* stats)
{
    if (!stats) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    stats->break_signal_count = g_espmodem.break_signal_count;
    stats->frame_error_count = g_espmodem.frame_error_count;
    stats->buffer_overflow_count = g_espmodem.buffer_overflow_count;
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_get_signal_strength(int* rssi, int* rssi_dbm, int* ber)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    char response[256];
    atomic_store(&g_espmodem.command_in_progress, true);
    esp_err_t err = esp_modem_at(g_espmodem.dce, "AT+CSQ", response, 3000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err == ESP_OK) {
        int rssi_val = -999, ber_val = -999;
        if (sscanf(response, "+CSQ: %d,%d", &rssi_val, &ber_val) == 2) {
            if (rssi) *rssi = rssi_val;
            if (ber) *ber = ber_val;
            if (rssi_dbm) {
                *rssi_dbm = (rssi_val == 99) ? -999 : (-113 + rssi_val * 2);
            }
            
            // Update internal status
            g_espmodem.status.signal_rssi = rssi_val;
            g_espmodem.status.signal_ber = ber_val;
            
            return ESPMODEM_OK;
        }
    }
    
    return ESPMODEM_ERR_TIMEOUT;
}

// Callback for collecting AT response lines
static esp_err_t collect_response_line(uint8_t *data, size_t len)
{
    at_response_collector_t *collector = &g_espmodem.response_collector;
    
    static int callback_count = 0;
    callback_count++;
    
    // WORKAROUND: This printf prevents a compiler optimization bug that causes data duplication
    // The bug manifests as memcmp() failing to detect duplicates in the deduplication logic below
    // DO NOT REMOVE unless the root cause is identified and fixed properly
    // printf(">>> CALLBACK #%d ENTRY: len=%d, current_pos=%d\n", callback_count, (int)len, (int)collector->current_pos);
    vTaskDelay(pdMS_TO_TICKS(100));
    // Debug: log what we received (string representation and hex of first 4 bytes)
    __log_debug("Callback #%d: Received line (%d bytes): [%.*s]", callback_count, (int)len, (int)len, data);
    if (len > 0) {
        __log_debug("First bytes (hex): %02x %02x %02x %02x", 
                    len > 0 ? data[0] : 0,
                    len > 1 ? data[1] : 0,
                    len > 2 ? data[2] : 0,
                    len > 3 ? data[3] : 0);
    }
    
    // Strip leading CR/LF
    size_t start_offset = 0;
    while (start_offset < len && (data[start_offset] == '\r' || data[start_offset] == '\n')) {
        start_offset++;
    }
    
    // Strip trailing CR/LF for comparison
    size_t compare_len = len - start_offset;
    while (compare_len > 0 && (data[start_offset + compare_len-1] == '\r' || data[start_offset + compare_len-1] == '\n')) {
        compare_len--;
    }
    
    __log_debug("After stripping leading/trailing CR/LF: start_offset=%d, compare_len=%d", (int)start_offset, (int)compare_len);
    
    // Skip empty lines FIRST before doing any pattern matching
    // Empty lines have nothing to process, so just continue immediately
    if (compare_len == 0) {
        __log_debug("Empty line detected, returning ESP_ERR_INVALID_RESPONSE to continue");
        return ESP_ERR_INVALID_RESPONSE;  // Continue waiting for more data
    }
    
    // Check for OK or ERROR responses - these signal command completion
    // The data might contain multiple lines, so we need to check if OK/ERROR appears anywhere
    // Search for "\r\nOK\r\n" or "\r\nERROR" pattern in the data
    bool found_ok = false;
    bool found_error = false;
    
    for (size_t i = 0; i < len - 3; i++) {
        // Look for "\r\nOK\r\n" pattern
        if (i + 5 < len && 
            data[i] == '\r' && data[i+1] == '\n' && 
            data[i+2] == 'O' && data[i+3] == 'K' &&
            data[i+4] == '\r' && data[i+5] == '\n') {
            found_ok = true;
            __log_debug("Found OK pattern at offset %d", (int)i);
            break;
        }
        // Look for "\r\nERROR" pattern  
        if (i + 6 < len &&
            data[i] == '\r' && data[i+1] == '\n' &&
            data[i+2] == 'E' && data[i+3] == 'R' && 
            data[i+4] == 'R' && data[i+5] == 'O' && data[i+6] == 'R') {
            found_error = true;
            __log_debug("Found ERROR pattern at offset %d", (int)i);
            break;
        }
    }
    
    // Also check if the line itself (after stripping) is just "OK" or "ERROR"
    if (compare_len == 2 && memcmp(data + start_offset, "OK", 2) == 0) {
        found_ok = true;
        __log_debug("Line is just OK");
    }
    if (compare_len >= 5 && memcmp(data + start_offset, "ERROR", 5) == 0) {
        found_error = true;
        __log_debug("Line is just ERROR");
    }
    
    // Add this line to the buffer (but only the actual content, not OK/ERROR terminator)
    // We need to extract just the useful data, excluding the final \r\nOK\r\n or \r\nERROR
    size_t copy_len = len;
    
    if (found_ok) {
        // Find where "\r\nOK\r\n" starts and only copy up to that point
        for (size_t i = 0; i < len - 5; i++) {
            if (data[i] == '\r' && data[i+1] == '\n' && 
                data[i+2] == 'O' && data[i+3] == 'K') {
                copy_len = i;  // Don't include the \r\nOK\r\n
                __log_debug("Truncating at OK, copy_len=%d", (int)copy_len);
                break;
            }
        }
    } else if (found_error) {
        // Find where "\r\nERROR" starts and copy including ERROR
        for (size_t i = 0; i < len - 6; i++) {
            if (data[i] == '\r' && data[i+1] == '\n' && 
                data[i+2] == 'E' && data[i+3] == 'R' && data[i+4] == 'R') {
                // Include ERROR in output, but not trailing \r\n
                copy_len = i + 7;  // \r\nERROR = 7 chars
                __log_debug("Including ERROR in response, copy_len=%d", (int)copy_len);
                break;
            }
        }
    }
    
    // Copy the data into the buffer (strip leading CR/LF)
    size_t actual_copy = 0;
    if (copy_len > start_offset) {
        actual_copy = copy_len - start_offset;
        
        // DEDUPLICATION: Check if this data is already at the start of our buffer
        // The esp_modem library may accumulate and re-send data when we return ESP_ERR_INVALID_RESPONSE
        // If this chunk starts with what we already have, only copy the NEW part
        if (collector->current_pos > 0 && actual_copy >= collector->current_pos) {
            // Check if the beginning of this data matches what we already have
            if (memcmp(data + start_offset, collector->buffer, collector->current_pos) == 0) {
                // This data starts with what we already have - skip the duplicate part
                size_t skip = collector->current_pos;
                start_offset += skip;
                actual_copy -= skip;
                __log_debug("DEDUP: Skipping %d bytes that match existing buffer content", (int)skip);
                
                // If there's nothing new, just continue
                if (actual_copy == 0) {
                    __log_debug("DEDUP: No new data after skipping duplicates");
                    if (found_ok || found_error) {
                        return ESP_OK;  // Command complete, no new data to add
                    }
                    return ESP_ERR_INVALID_RESPONSE;  // Continue waiting
                }
            }
        }
        
        // Check if we have room for this data BEFORE copying
        size_t remaining = collector->buffer_size - collector->current_pos;
        if (remaining <= actual_copy + 1) {
            // Buffer overflow - mark it and stop collecting
            collector->overflow = true;
            __log_warn("Buffer overflow: current_pos=%d, actual_copy=%d, buffer_size=%d, remaining=%d", 
                      (int)collector->current_pos, (int)actual_copy, (int)collector->buffer_size, (int)remaining);
            return ESP_OK;  // Return OK to signal successful stop (not an error)
        }
        
        memcpy(collector->buffer + collector->current_pos, data + start_offset, actual_copy);
        collector->current_pos += actual_copy;
        __log_debug("Copied %d bytes to buffer (total now: %d)", (int)actual_copy, (int)collector->current_pos);
    }
    
    // Null terminate
    collector->buffer[collector->current_pos] = '\0';
    
    // Return values per esp_modem C API wrapper (esp_modem_c_api.cpp):
    // - ESP_OK → command_result::OK (command complete successfully)
    // - ESP_FAIL → command_result::FAIL (command failed, stop)
    // - Anything else → command_result::TIMEOUT (keep waiting for more data)
    if (found_ok) {
        __log_debug("Command completed successfully (OK found), returning ESP_OK");
        return ESP_OK;  // DONE - command complete
    }
    if (found_error) {
        __log_debug("Command completed with ERROR, returning ESP_OK");
        return ESP_OK;  // DONE - command complete (with ERROR response)
    }
    
    // Return ESP_ERR_INVALID_RESPONSE to keep waiting for more lines
    __log_debug("Normal/empty line, returning ESP_ERR_INVALID_RESPONSE to continue");
    return ESP_ERR_INVALID_RESPONSE;  // NOT DONE - keep waiting
}

espmodem_err_t espmodem_send_at_command(const char* command, char* response, 
                                       size_t response_size, uint32_t timeout_ms)
{
    if (!g_espmodem.initialized || !g_espmodem.dce || !command || !response) {
        __log_error("Invalid arguments: initialized=%d, dce=%p, command=%p, response=%p", 
                   g_espmodem.initialized, g_espmodem.dce, command, response);
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    // Clear response buffer
    memset(response, 0, response_size);
    
    // Mark command as in-progress to suppress URC callbacks during execution
    // (Set flag BEFORE esp_modem_sync to prevent URCs during sync)
    atomic_store(&g_espmodem.command_in_progress, true);
    
    // Sync modem before sending command to clear any pending data
    // This helps prevent buffer overlap between commands
    esp_modem_sync(g_espmodem.dce);
    
    //Setup the response collector for multi-line responses
    g_espmodem.response_collector.buffer = response;
    g_espmodem.response_collector.buffer_size = response_size;
    g_espmodem.response_collector.current_pos = 0;
    g_espmodem.response_collector.overflow = false;
    
    __log_debug("Sending AT command with buffer_size=%d bytes", (int)response_size);

    // esp_modem_command() does not add CR terminator automatically (unlike esp_modem_at)
    // We need to append \r to the command string for proper AT command termination
    char cmd_with_cr[256];
    snprintf(cmd_with_cr, sizeof(cmd_with_cr), "%s\r", command);
    
    __log_debug("UART-TX: %s", command);
    __log_debug("Calling esp_modem_command with timeout=%d ms", (int)timeout_ms);
    
    esp_err_t err = esp_modem_command(g_espmodem.dce, cmd_with_cr, collect_response_line, timeout_ms);
    
    // Command complete - allow URCs again
    atomic_store(&g_espmodem.command_in_progress, false);
    
    __log_debug("esp_modem_command returned with err=%d (%s)", err, esp_err_to_name(err));
    
    // ESP_OK means callback found OK/ERROR (command complete successfully)
    // ESP_FAIL means callback indicated command failure
    // ESP_ERR_TIMEOUT means timeout waiting for response
    if (err == ESP_OK) {
        size_t resp_len = g_espmodem.response_collector.current_pos;
        __log_debug("UART-RX: (%d bytes)", resp_len);  // Don't log the full response, it's huge!
        
        if (g_espmodem.response_collector.overflow) {
            __log_warn("Response buffer overflow - response truncated (buffer size: %d bytes)", response_size);
        }
    } else {
        __log_error("AT command failed with ESP error: %d (%s)", err, esp_err_to_name(err));
    }
    
    return (err == ESP_OK) ? ESPMODEM_OK : ESPMODEM_ERR_TIMEOUT;
}

void espmodem_set_debug(bool enable)
{
    g_espmodem.debug_enabled = enable;
    // Note: SG-SDK logging can be controlled at runtime via log_filter functions
    // This function serves as a compatibility interface
    __log_debug("Debug mode %s", enable ? "enabled" : "disabled");
}

esp_modem_dce_t* espmodem_get_dce_handle(void)
{
    return g_espmodem.dce;
}

esp_netif_t* espmodem_get_netif_handle(void)
{
    return g_espmodem.esp_netif;
}

// ============================================================================
// Event Callback Registration
// ============================================================================

espmodem_err_t espmodem_register_network_callback(espmodem_network_event_cb_t callback)
{
    g_espmodem.network_cb = callback;
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_register_signal_callback(espmodem_signal_event_cb_t callback)
{
    g_espmodem.signal_cb = callback;
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_register_error_callback(espmodem_error_event_cb_t callback)
{
    g_espmodem.error_cb = callback;
    return ESPMODEM_OK;
}

// ============================================================================
// Additional Network Operations
// ============================================================================

espmodem_err_t espmodem_detach(void)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    __log_info("Detaching from network...");
    
    // Disconnect PPP first if connected
    if (g_espmodem.status.ppp_connected) {
        espmodem_disconnect();
    }
    
    char response[256];
    esp_err_t err;
    
    // Check current CFUN state
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+CFUN?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err != ESP_OK) {
        __log_error("Failed to check CFUN status");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    // According to Sequans, should go to CFUN=4 first before going to CFUN=0
    // This ensures SIM card remains writable during detach
    if (strstr(response, "+CFUN: 1")) {
        __log_info("Radio ON, switching to CFUN=4 before detach");
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT+CFUN=4", response, 10000);
        atomic_store(&g_espmodem.command_in_progress, false);
        if (err != ESP_OK) {
            __log_error("Failed to set CFUN=4");
            return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Now go to CFUN=0 (minimum functionality)
    __log_info("Setting CFUN=0 (minimum functionality)");
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+CFUN=0", response, 10000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err != ESP_OK) {
        __log_error("Failed to set CFUN=0");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    g_espmodem.status.network_attached = false;
    __log_info("Network detached successfully");
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_reset(void)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    // Dispatch MODEM_RESET event
    if (g_event_handler) {
        espmodem_event_t event = {
            .event_type = ESPMODEM_EVENT_MODEM_RESET,
            .reset = {
                .user_initiated = true,
                .reason = "User-initiated reset via espmodem_reset()"
            }
        };
        dispatch_event(&event);
    }
    
    // Disconnect PPP first if connected
    if (g_espmodem.status.ppp_connected) {
        espmodem_disconnect();
    }
    
    // CRITICAL: Clear shutdown flag BEFORE sending reset command
    // AT^RESET returns OK immediately, then sends +SHUTDOWN asynchronously
    // We must clear the flag before any URCs can arrive
    g_espmodem.shutdown_received = false;
    __log_debug("Shutdown flag cleared, ready to detect +SHUTDOWN URC");
    
    // Send reset command and wait for +SHUTDOWN URC (like LTE.py)
    char response[512];
    esp_err_t err;
    
    // Send AT^RESET command
    __log_info("Resetting modem via AT^RESET command...");
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT^RESET", response, 1000);
    atomic_store(&g_espmodem.command_in_progress, false);

    if (err != ESP_OK) {
        __log_info("Failed to send AT^RESET command");
        __log_info("Modem hardware reset via IO expander");
        // Hardware reset via IO expander (LTE_RST_N on P3)
        ioexp_lte_chip_reset();
        return espmodem_wait_reset(false);
    } else {
        __log_info("AT^RESET command sent successfully, waiting for +SHUTDOWN...");
    }
    
    // CRITICAL: Add delay to allow ESP modem library command parser to fully release
    // The +SHUTDOWN URC typically arrives 20-50ms after the OK response
    // Without this delay, the command parser may still be active and consume the URC
    // instead of forwarding it to unsolicited_handler()
    vTaskDelay(pdMS_TO_TICKS(100));
    __log_debug("Command parser release delay complete, URC handler should be active");

    return espmodem_wait_reset(true);
}

/**
 * @brief Change carrier conformance mode at 921600 baud (after modem is initialized)
 * 
 * This function handles the modem reset that occurs when changing conformance mode
 * via AT+SQNCTM. Unlike configure_conformance_mode_at_115200() which is called during
 * initialization, this can be called at any time after the modem is initialized.
 * 
 * The function will:
 * 1. Check/set CFUN=4 if needed (radio must be off)
 * 2. Send AT+SQNCTM=<carrier>
 * 3. Call espmodem_wait_reset() to handle the modem reset and baudrate switch
 * 4. Update g_espmodem.carrier with the new value
 * 
 * @param carrier New carrier conformance mode (e.g., "standard", "verizon", "att")
 * @return ESPMODEM_OK on success, error code otherwise
 */
espmodem_err_t espmodem_change_conformance_mode(const char* carrier)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    if (!carrier || strcmp(carrier, "None") == 0) {
        __log_error("Cannot change to 'None' carrier - must specify explicit carrier");
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    __log_info("Changing conformance mode from '%s' to '%s'", g_espmodem.carrier, carrier);
    
    // Disconnect PPP first if connected
    if (g_espmodem.status.ppp_connected) {
        __log_info("Disconnecting PPP before conformance mode change");
        espmodem_disconnect();
    }
    
    char response[512];
    esp_err_t err;
    
    // Check current CFUN status - must be 0 or 4 to change conformance mode
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+CFUN?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err != ESP_OK) {
        __log_error("Failed to query CFUN status");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    // Set CFUN=4 if not already 0 or 4
    if (!strstr(response, "+CFUN: 0") && !strstr(response, "+CFUN: 4")) {
        __log_info("Setting CFUN=4 before conformance mode change");
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT+CFUN=4", response, 10000);
        atomic_store(&g_espmodem.command_in_progress, false);
        if (err != ESP_OK) {
            __log_error("Failed to set CFUN=4");
            return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    // Validate carrier parameter against supported modes (if not "standard")
    if (strcmp(carrier, "standard") != 0) {
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT+SQNCTM=?", response, 5000);
        atomic_store(&g_espmodem.command_in_progress, false);
        if (err != ESP_OK) {
            __log_error("Failed to query supported conformance modes");
            return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
        }
        
        __log_info("Supported conformance modes: %s", response);
        
        if (!strstr(response, carrier)) {
            __log_error("Invalid carrier mode '%s' - not supported by modem", carrier);
            return ESPMODEM_ERR_INVALID_ARG;
        }
    }
    
    // Query current conformance mode
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+SQNCTM?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err == ESP_OK) {
        __log_info("Current conformance mode: %s", response);
        
        // Check if already in requested mode
        if (strstr(response, carrier)) {
            __log_info("Already in '%s' mode, no change needed", carrier);
            strncpy(g_espmodem.carrier, carrier, sizeof(g_espmodem.carrier) - 1);
            g_espmodem.carrier[sizeof(g_espmodem.carrier) - 1] = '\0';
            return ESPMODEM_OK;
        }
    }
    
    // Send AT+SQNCTM=<carrier> - this will trigger modem reset
    char ctm_cmd[64];
    snprintf(ctm_cmd, sizeof(ctm_cmd), "AT+SQNCTM=\"%s\"", carrier);
    __log_info("Sending: %s", ctm_cmd);
    
    // Clear shutdown flag before sending command
    g_espmodem.shutdown_received = false;
    
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, ctm_cmd, response, 2000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err != ESP_OK) {
        __log_error("Failed to send AT+SQNCTM command");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    __log_info("AT+SQNCTM sent successfully, modem will reset");
    
    // Wait for modem reset and recreate DCE
    espmodem_err_t reset_err = espmodem_wait_reset(true);
    if (reset_err != ESPMODEM_OK) {
        __log_error("Failed to handle modem reset after conformance mode change");
        return reset_err;
    }
    
    // Update stored carrier
    strncpy(g_espmodem.carrier, carrier, sizeof(g_espmodem.carrier) - 1);
    g_espmodem.carrier[sizeof(g_espmodem.carrier) - 1] = '\0';
    __log_info("Conformance mode changed successfully to: %s", g_espmodem.carrier);
    
    return ESPMODEM_OK;
}

/**
 * Background task that monitors for crash detection and triggers recovery
 * This task runs independently and is NOT spawned by the UART task,
 * so it can safely destroy the DCE without deadlock
 */
static void espmodem_recovery_monitor_task(void *arg)
{
    (void)arg;
    
    __log_info("Recovery monitor task started");
    
    while (1) {
        // Check every 100ms if recovery is needed
        vTaskDelay(pdMS_TO_TICKS(100));
        
        if (g_espmodem.recovery_in_progress) {
            __log_info("Recovery flag detected - starting crash recovery...");
            
            // Wait a bit for break signals to stop coming in
            // This prevents issues with concurrent UART activity during recovery
            __log_info("Waiting for break signals to settle...");
            vTaskDelay(pdMS_TO_TICKS(500));
            
            // Perform recovery synchronously
            espmodem_err_t result = espmodem_recover_from_crash();
            
            if (result == ESPMODEM_OK) {
                __log_info("Crash recovery completed successfully");
            } else {
                __log_error("Crash recovery failed with error: %d", result);
            }
            
            // Always clear flag so system doesn't hang
            g_espmodem.recovery_in_progress = false;
        }
    }
}

/**
 * Internal function for crash recovery
 * Called by the independent monitoring task when crash detected
 */
static espmodem_err_t espmodem_recover_from_crash(void)
{
    if (!g_espmodem.initialized) {
        __log_error("Cannot recover - modem not initialized");
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    __log_info("Starting modem crash recovery...");
    __log_info("Break signals detected: %lu, Frame errors: %lu",
              (unsigned long)g_espmodem.break_signal_count,
              (unsigned long)g_espmodem.frame_error_count);
    
    // Disconnect PPP first if connected
    if (g_espmodem.status.ppp_connected) {
        __log_info("Disconnecting PPP before recovery");
        espmodem_disconnect();
    }
    
    // // Perform hardware reset first - this physically resets the modem
    __log_info("Performing modem hardware reset via IO expander...");
    ioexp_lte_chip_reset();
    
    // Wait for hardware reset to complete and UART activity to stop
    // This is critical to prevent concurrent access during DCE destruction
    __log_info("Waiting for hardware reset to complete...");
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Now destroy the old DCE - safe to do after hardware reset
    // The modem is already reset, so destroy won't try to communicate with it
    if (g_espmodem.dce) {
        __log_info("Destroying old DCE after hardware reset...");
        esp_modem_destroy(g_espmodem.dce);
        g_espmodem.dce = NULL;
        __log_info("Old DCE destroyed successfully");
    }

    // Wait for modem to boot and reconfigure
    // espmodem_wait_reset() will recreate DCE and re-register handlers
    espmodem_err_t result = espmodem_wait_reset(false);
    
    return result;
}

espmodem_err_t espmodem_wait_reset(bool wait_shutdown)
{
    __log_debug("espmodem_wait_reset called with wait_shutdown=%d", wait_shutdown);
    char response[512];
    esp_err_t err;

    if (wait_shutdown) {

        // Wait for +SHUTDOWN URC (detected by unsolicited_handler)
        // The URC handler sets g_espmodem.shutdown_received when +SHUTDOWN arrives
        __log_info("Waiting for +SHUTDOWN URC...");
        for (int i = 0; i < 50; i++) {  // 5 seconds total (50 * 100ms)
            if (g_espmodem.shutdown_received) {
                __log_info("+SHUTDOWN detected via URC handler");
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        if (!g_espmodem.shutdown_received) {
            __log_warn("+SHUTDOWN not received within timeout - proceeding anyway");
        }
    } else {
        __log_info("Handling modem reset without waiting for +SHUTDOWN URC...");
    }
    
    // Destroy ESP modem DCE - we need to recreate it at 115200 then switch to 921600
    // Check if DCE is already destroyed (e.g., by crash recovery)
    if (g_espmodem.dce != NULL) {
        __log_info("Destroying ESP modem DCE to handle baudrate change...");
        esp_modem_destroy(g_espmodem.dce);
        g_espmodem.dce = NULL;
    } else {
        __log_info("DCE already destroyed - skipping destruction step");
    }
    
    // Wait for modem to shutdown and reboot
    // __log_info("Waiting for modem reboot...");
    // vTaskDelay(pdMS_TO_TICKS(2000));  // Give modem time to shutdown and start booting
    
    // Setup direct UART access at 115200 to catch boot and switch baudrate
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
    };
    
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, ESPMODEM_UART_TX_PIN, ESPMODEM_UART_RX_PIN, 
                                  ESPMODEM_UART_RTS_PIN, ESPMODEM_UART_CTS_PIN));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0));
    
    // Flush any boot messages
    uart_flush_input(UART_NUM_1);
    
    // Wait for modem to be responsive at 115200
    __log_info("Waiting for modem to respond at 115200 baud...");

    bool modem_ready = false;
    for (int i = 0; i < 20; i++) {
        vTaskDelay(pdMS_TO_TICKS(250));
        
        int len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 
                                  500 / portTICK_PERIOD_MS);
        if (len > 0) {
            response[len] = '\0';
            if (strstr(response, "+SYSSTART")) {
                __log_info("Modem +SYSSTART received at 115200 after reset");
                modem_ready = true;
                break;
            }
        }
    }
    for (int i = 0; i < 20; i++) {
        vTaskDelay(pdMS_TO_TICKS(250));
        
        const char* at_cmd = "AT\r\n";
        uart_write_bytes(UART_NUM_1, at_cmd, strlen(at_cmd));
        
        int len = uart_read_bytes(UART_NUM_1, response, sizeof(response) - 1, 
                                  500 / portTICK_PERIOD_MS);
        if (len > 0) {
            response[len] = '\0';
            if (strstr(response, "OK")) {
                __log_info("Modem responding at 115200 after reset");
                modem_ready = true;
                break;
            }
        }
    }
    
    if (!modem_ready) {
        uart_driver_delete(UART_NUM_1);
        __log_error("Modem not responding at 115200 after reset");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    // Switch to 921600 non-persistent
    __log_info("Switching modem to 921600 baud (non-persistent)...");
    const char* ipr_cmd = "AT+IPR=921600,0\r\n";
    uart_write_bytes(UART_NUM_1, ipr_cmd, strlen(ipr_cmd));
    
    // Read response with early exit on OK
    int total_len = 0;
    for (int attempts = 0; attempts < 20; attempts++) {
        int len = uart_read_bytes(UART_NUM_1, response + total_len, 
                                  sizeof(response) - total_len - 1,
                                  100 / portTICK_PERIOD_MS);
        if (len > 0) {
            total_len += len;
            response[total_len] = '\0';
            if (strstr(response, "OK")) {
                __log_debug("AT+IPR=921600,0 successful after reset");
                break;
            }
        }
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Reconfigure UART to 921600
    uart_config.baud_rate = 921600;
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_1, &uart_config));
    __log_info("UART reconfigured to 921600 baud");
    
    // Clean up UART - we'll recreate ESP modem DCE
    uart_driver_delete(UART_NUM_1);
    
    // Recreate ESP modem DCE at 921600
    __log_info("Recreating ESP modem DCE at 921600 baud...");
    
    esp_modem_dce_config_t dce_config = ESP_MODEM_DCE_DEFAULT_CONFIG("");
    esp_modem_dte_config_t dte_config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    
    // Use same buffer configuration as init
    dte_config.uart_config.rx_buffer_size = 4096;
    dte_config.uart_config.tx_buffer_size = 2048;
    dte_config.uart_config.event_queue_size = 20;
    dte_config.dte_buffer_size = 2048;
    dte_config.task_stack_size = 4096;
    dte_config.task_priority = 4;
    
    dte_config.uart_config.tx_io_num = ESPMODEM_UART_TX_PIN;
    dte_config.uart_config.rx_io_num = ESPMODEM_UART_RX_PIN;
    dte_config.uart_config.rts_io_num = ESPMODEM_UART_RTS_PIN;
    dte_config.uart_config.cts_io_num = ESPMODEM_UART_CTS_PIN;
    dte_config.uart_config.flow_control = ESP_MODEM_FLOW_CONTROL_HW;
    dte_config.uart_config.baud_rate = 921600;
    
    // CRITICAL: Use SQNGM02S device type to get correct AT command sequencing
    // Must match the device type used in espmodem_init()
    g_espmodem.dce = esp_modem_new_dev(ESP_MODEM_DCE_SQNGM02S, &dte_config, &dce_config, g_espmodem.esp_netif);
    if (!g_espmodem.dce) {
        __log_error("Failed to recreate ESP modem DCE after reset");
        return ESPMODEM_ERR_NO_MEM;
    }
    
    // Sync communication
    __log_info("Synchronizing ESP modem communication...");
    for (int i = 0; i < 5; i++) {
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT", response, 1000);
        atomic_store(&g_espmodem.command_in_progress, false);
        if (err == ESP_OK && strstr(response, "OK")) {
            __log_info("ESP modem communication synchronized");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    // CRITICAL: Always re-register URC handler after DCE recreation
    // This ensures +SHUTDOWN detection works even without user callback
#ifdef CONFIG_ESP_MODEM_URC_HANDLER
    err = esp_modem_set_urc(g_espmodem.dce, unsolicited_handler);
    if (err == ESP_OK) {
        __log_info("URC handler re-registered after reset (user callback: %s)",
                  g_espmodem.unsolicited_cb ? "set" : "not set");
    } else {
        __log_error("Failed to re-register URC handler after reset: %s", esp_err_to_name(err));
        __log_error("WARNING: +SHUTDOWN detection will not work without URC handler!");
    }
#endif
    
    // Re-register terminal error callback for break signal detection
    err = esp_modem_set_error_cb(g_espmodem.dce, modem_terminal_error_handler);
    if (err == ESP_OK) {
        __log_info("Terminal error callback re-registered after reset");
    } else {
        __log_warn("Failed to re-register terminal error callback: %s", esp_err_to_name(err));
    }
    
    // Reset internal state
    memset(&g_espmodem.status, 0, sizeof(g_espmodem.status));
    g_espmodem.status.powered = true;
    g_espmodem.status.current_baudrate = 921600;

    // IMPORTANT: Query actual conformance mode after reset to sync internal state
    // The modem maintains separate conformance mode configs for CAT-M1 and NB-IoT
    // After a technology switch (or any reset), we need to read what mode is active
    __log_info("Querying conformance mode after reset...");
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+SQNCTM?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err == ESP_OK) {
        // Parse conformance mode from response (e.g., "+SQNCTM: verizon\r\n")
        char* carrier_start = strstr(response, "+SQNCTM:");
        if (carrier_start) {
            carrier_start += 8;  // Skip "+SQNCTM:"
            while (*carrier_start == ' ') carrier_start++;  // Skip spaces
            
            // Copy carrier name until \r or \n
            int i = 0;
            while (carrier_start[i] != '\0' && carrier_start[i] != '\r' && 
                   carrier_start[i] != '\n' && i < (int)sizeof(g_espmodem.carrier) - 1) {
                g_espmodem.carrier[i] = carrier_start[i];
                i++;
            }
            g_espmodem.carrier[i] = '\0';
            
            // Trim trailing spaces
            while (i > 0 && g_espmodem.carrier[i-1] == ' ') {
                g_espmodem.carrier[--i] = '\0';
            }
            
            __log_info("Conformance mode after reset: '%s'", g_espmodem.carrier);
        } else {
            __log_warn("Failed to parse SQNCTM response, defaulting to 'standard'");
            strncpy(g_espmodem.carrier, "standard", sizeof(g_espmodem.carrier) - 1);
            g_espmodem.carrier[sizeof(g_espmodem.carrier) - 1] = '\0';
        }
    } else {
        __log_warn("Failed to query conformance mode after reset, keeping previous value '%s'", 
                  g_espmodem.carrier);
    }
    
    __log_info("Modem reset complete - ready at 921600 baud");
    g_espmodem.recovery_in_progress = false;
    return ESPMODEM_OK;
}

// ============================================================================
// Modem Configuration Functions
// ============================================================================

espmodem_err_t espmodem_check_sim_present(void)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    char response[256];
    esp_err_t err;
    
    // Check if we're in CFUN=1 (radio enabled) - if not, switch to CFUN=4 to read SIM
    // This matches LTE.py behavior: check_sim_present() lines 381-394
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+CFUN?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err == ESP_OK && !strstr(response, "+CFUN: 1")) {
        // Not in CFUN=1, switch to CFUN=4 to read SIM
        __log_info("Radio not enabled, setting CFUN=4 to check SIM...");
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT+CFUN=4", response, 5000);
        atomic_store(&g_espmodem.command_in_progress, false);
        if (err != ESP_OK) {
            __log_error("Failed to set CFUN=4");
            return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
        }
        vTaskDelay(pdMS_TO_TICKS(500));  // Allow modem to switch modes
    }
    
    // Try up to 5 times with 250ms delays (matches LTE.py)
    bool sim_ready = false;
    for (int retry = 0; retry < 15; retry++) {
        __log_info("SIM card check (attempt %d/15)", retry + 1);
        if (retry > 0) {
            vTaskDelay(pdMS_TO_TICKS(250));
        }
        
        atomic_store(&g_espmodem.command_in_progress, true);
        err = esp_modem_at(g_espmodem.dce, "AT+CPIN?", response, 5000);
        atomic_store(&g_espmodem.command_in_progress, false);
        if (err == ESP_OK && strstr(response, "+CPIN: READY")) {
            __log_info("SIM card ready (attempt %d/15)", retry + 1);
            g_espmodem.status.sim_ready = true;
            sim_ready = true;
            break;
        }
    }
    
    if (!sim_ready) {
        __log_error("SIM card not present or PIN protected after 15 attempts");
        return ESPMODEM_ERR_SIM_NOT_READY;
    }
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_get_mode(espmodem_mode_t* mode)
{
    if (!g_espmodem.initialized || !g_espmodem.dce || !mode) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    char response[128];
    esp_err_t err = esp_modem_at_with_retry(g_espmodem.dce, "AT+SQNMODEACTIVE?", response, 5000, AT_CMD_MAX_RETRIES);
    if (err != ESP_OK) {
        __log_error("Failed to get mode after retries: %s", esp_err_to_name(err));
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    // Parse response: +SQNMODEACTIVE: <mode>
    // mode: 1 = CAT-M1, 2 = NB-IoT
    char* mode_str = strstr(response, "+SQNMODEACTIVE:");
    if (mode_str) {
        mode_str += 15; // Skip "+SQNMODEACTIVE:"
        while (*mode_str && (*mode_str == ' ' || *mode_str == '\t')) mode_str++;
        
        int raw_mode = atoi(mode_str);
        *mode = (raw_mode == 1) ? ESPMODEM_MODE_CATM1 : ESPMODEM_MODE_NBIOT;
        
        __log_info("Current mode: %s (raw=%d)", 
                   (*mode == ESPMODEM_MODE_CATM1) ? "CAT-M1" : "NB-IoT", raw_mode);
        return ESPMODEM_OK;
    }
    
    __log_error("Unable to parse mode from response: %s", response);
    return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
}

espmodem_err_t espmodem_set_mode(espmodem_mode_t mode)
{
    if (!g_espmodem.initialized || !g_espmodem.dce) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    if (mode != ESPMODEM_MODE_CATM1 && mode != ESPMODEM_MODE_NBIOT) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    // Get current mode first
    espmodem_mode_t current_mode;
    espmodem_err_t err = espmodem_get_mode(&current_mode);
    if (err != ESPMODEM_OK) {
        return err;
    }
    
    // Check if mode change is needed
    if (current_mode == mode) {
        __log_info("Already in %s mode, no change needed",
                   (mode == ESPMODEM_MODE_CATM1) ? "CAT-M1" : "NB-IoT");
        return ESPMODEM_OK;
    }
    
    __log_info("Switching from %s to %s mode...",
               (current_mode == ESPMODEM_MODE_CATM1) ? "CAT-M1" : "NB-IoT",
               (mode == ESPMODEM_MODE_CATM1) ? "CAT-M1" : "NB-IoT");
    
    char response[256];
    esp_err_t esp_err;
    
    // Set CFUN=0 before changing mode
    atomic_store(&g_espmodem.command_in_progress, true);
    esp_err = esp_modem_at(g_espmodem.dce, "AT+CFUN=0", response, 10000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (esp_err != ESP_OK) {
        __log_error("Failed to set CFUN=0");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Set new mode: 1 = CAT-M1, 2 = NB-IoT
    int raw_mode = (mode == ESPMODEM_MODE_CATM1) ? 1 : 2;
    char mode_cmd[64];
    snprintf(mode_cmd, sizeof(mode_cmd), "AT+SQNMODEACTIVE=%d", raw_mode);
    
    atomic_store(&g_espmodem.command_in_progress, true);
    esp_err = esp_modem_at(g_espmodem.dce, mode_cmd, response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (esp_err != ESP_OK) {
        __log_error("Failed to set mode");
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    __log_info("Mode command sent, resetting modem...");
    
    // Reset modem to apply mode change
    return espmodem_reset();
}

espmodem_err_t espmodem_get_network_info(espmodem_network_info_t* info)
{
    if (!info) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    memset(info, 0, sizeof(espmodem_network_info_t));
    info->connected = g_espmodem.status.ppp_connected;
    
    // If not connected or no netif, return with connected=false
    if (!g_espmodem.status.ppp_connected || !g_espmodem.esp_netif) {
        return ESPMODEM_OK;
    }
    
    // Get IP information from the PPP netif
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(g_espmodem.esp_netif, &ip_info) == ESP_OK) {
        // Convert IP address to string
        snprintf(info->ip, sizeof(info->ip), IPSTR, IP2STR(&ip_info.ip));
        snprintf(info->netmask, sizeof(info->netmask), IPSTR, IP2STR(&ip_info.netmask));
        snprintf(info->gateway, sizeof(info->gateway), IPSTR, IP2STR(&ip_info.gw));
    }
    
    // Get DNS information
    esp_netif_dns_info_t dns_info;
    if (esp_netif_get_dns_info(g_espmodem.esp_netif, ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
        snprintf(info->dns1, sizeof(info->dns1), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
    }
    if (esp_netif_get_dns_info(g_espmodem.esp_netif, ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
        snprintf(info->dns2, sizeof(info->dns2), IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
    }
    
    return ESPMODEM_OK;
}

espmodem_err_t espmodem_get_imei(char* imei, size_t imei_size) { 
    if (!g_espmodem.initialized || !g_espmodem.dce || !imei) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    char response[64];
    atomic_store(&g_espmodem.command_in_progress, true);
    esp_err_t err = esp_modem_at(g_espmodem.dce, "AT+CGSN=1", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err != ESP_OK) {
        __log_error("Failed to get IMEI: %s", esp_err_to_name(err));
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    // Parse IMEI from response (format: +CGSN: "015770001956391")
    char* start = strstr(response, "+CGSN:");
    if (start) {
        start += 6; // Skip "+CGSN:"
        while (*start && (*start == ' ' || *start == '\t')) start++;
        if (*start == '"') {
            start++; // Skip opening quote
            char* end = strchr(start, '"');
            if (end) {
                size_t len = end - start;
                if (len >= imei_size) {
                    return ESPMODEM_ERR_NO_MEM;
                }
                strncpy(imei, start, len);
                imei[len] = '\0';
                return ESPMODEM_OK;
            }
        }
    }
    
    // Fallback: try to parse as plain number
    start = response;
    while (*start && (*start == '\r' || *start == '\n' || *start == ' ')) start++;
    
    size_t len = strlen(start);
    while (len > 0 && (start[len-1] == '\r' || start[len-1] == '\n' || start[len-1] == ' ')) {
        start[--len] = '\0';
    }
    
    if (len >= imei_size) {
        return ESPMODEM_ERR_NO_MEM;
    }
    
    strcpy(imei, start);
    return ESPMODEM_OK; 
}

espmodem_err_t espmodem_get_iccid(char* iccid, size_t iccid_size) { 
    if (!g_espmodem.initialized || !g_espmodem.dce || !iccid) {
        return ESPMODEM_ERR_INVALID_ARG;
    }
    
    char response[256];
    esp_err_t err;
    
    // Check SIM status first using consolidated function
    espmodem_err_t sim_err = espmodem_check_sim_present();
    if (sim_err != ESPMODEM_OK) {
        return sim_err;  // Error already logged
    }
    
    // Now get the ICCID
    atomic_store(&g_espmodem.command_in_progress, true);
    err = esp_modem_at(g_espmodem.dce, "AT+SQNCCID?", response, 5000);
    atomic_store(&g_espmodem.command_in_progress, false);
    if (err != ESP_OK) {
        __log_error("Failed to get ICCID: %s", esp_err_to_name(err));
        return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
    }
    
    __log_info("ICCID raw response: '%s'", response);
    
    // Parse ICCID from response (format: +SQNCCID: "89882280666025303041")
    char* start = strstr(response, "+SQNCCID:");
    if (start) {
        start += 9; // Skip "+SQNCCID:"
        while (*start && (*start == ' ' || *start == '\t')) start++;
        if (*start == '"') {
            start++; // Skip opening quote
            char* end = strchr(start, '"');
            if (end) {
                size_t len = end - start;
                if (len >= iccid_size) {
                    return ESPMODEM_ERR_NO_MEM;
                }
                strncpy(iccid, start, len);
                iccid[len] = '\0';
                return ESPMODEM_OK;
            }
        }
    }
    
    __log_error("Unable to parse ICCID from response: '%s'", response);
    return ESPMODEM_ERR_MODEM_NOT_RESPONDING;
}

// ============================================================================
// Unsolicited Response (URC) Handler
// ============================================================================

#ifdef CONFIG_ESP_MODEM_URC_HANDLER
/**
 * @brief Internal URC handler that forwards unsolicited responses to user callback
 * 
 * This function is called by the ESP modem library whenever it receives data
 * from the modem that wasn't explicitly requested by an AT command.
 * 
 * @param data Pointer to the received data
 * @param len Length of the received data
 * @return ESP_OK always (don't interrupt modem processing)
 */
/**
 * @brief Handle terminal errors from ESP modem library
 * 
 * This callback is invoked by the ESP modem library's uart_terminal component
 * when UART events occur. It detects:
 * 
 * - UART_BREAK: Break signals indicating modem reset, crash, or PPP drop
 * - UART_FRAME_ERR: Frame errors from baudrate mismatch or electrical issues  
 * - UART_BUFFER_FULL/UART_FIFO_OVF: Buffer overflows causing data loss
 * - UART_PARITY_ERR: Checksum errors
 * 
 * Break signals are particularly important as they indicate:
 * 1. Modem hardware reset (expected during AT^RESET, +SHUTDOWN)
 * 2. Modem firmware crash (unexpected, needs recovery)
 * 3. PPP connection drop (network failure)
 * 4. UART electrical issues (cable disconnect, noise)
 * 
 * @param err Terminal error type from ESP modem library
 */
static void modem_terminal_error_handler(esp_modem_terminal_error_t err)
{
    switch (err) {
    case ESP_MODEM_TERMINAL_UNEXPECTED_CONTROL_FLOW:
        // UART break signal or frame error detected
        // This could be:
        // 1. Expected: Modem resetting (we sent AT^RESET)
        // 2. Unexpected: Modem crash, PPP drop, cable disconnect
        g_espmodem.break_signal_count++;
        
        // Crash detection: Multiple breaks in short time window = hardware reset/crash
        uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        uint32_t time_since_last_break = now_ms - g_espmodem.last_break_timestamp_ms;
        
        // Reset counter if more than 3 seconds since last break
        if (time_since_last_break > 3000) {
            g_espmodem.breaks_in_window = 0;
        }
        
        g_espmodem.breaks_in_window++;
        g_espmodem.last_break_timestamp_ms = now_ms;
        
        __log_warn("UART break signal detected (count: %lu, window: %lu in %lums) - modem reset/crash/PPP drop",
                   (unsigned long)g_espmodem.break_signal_count,
                   (unsigned long)g_espmodem.breaks_in_window,
                   (unsigned long)time_since_last_break);
        
        // If we see 10+ breaks within 3 seconds, it's likely a hardware reset/crash
        // This matches the pattern: ioexp.lte_reset() causes 13-14 breaks
        // Set flag - independent monitoring task will detect and handle recovery
        if (g_espmodem.breaks_in_window >= 10 && !g_espmodem.recovery_in_progress) {
            __log_error("CRASH DETECTED: %lu break signals in 3 seconds - modem crashed or hardware reset!",
                       (unsigned long)g_espmodem.breaks_in_window);
            __log_info("Setting recovery flag - monitoring task will handle recovery automatically");
            
            // Dispatch MODEM_CRASH event
            if (g_event_handler) {
                espmodem_event_t event = {
                    .event_type = ESPMODEM_EVENT_MODEM_CRASH,
                    .crash = {
                        .break_count = g_espmodem.breaks_in_window,
                        .recovery_started = true
                    }
                };
                dispatch_event(&event);
            }
            
            // Set flag - independent monitoring task watches this and triggers recovery
            // The monitoring task is NOT spawned by UART task, so no deadlock
            g_espmodem.recovery_in_progress = true;
        }
        
        // Trigger error callback if registered
        if (g_espmodem.error_cb) {
            g_espmodem.error_cb(ESPMODEM_ERR_MODEM_NOT_RESPONDING,
                              "UART break signal - modem may have crashed or reset");
        }
        break;
        
    case ESP_MODEM_TERMINAL_BUFFER_OVERFLOW:
        g_espmodem.buffer_overflow_count++;
        __log_error("UART buffer overflow (count: %lu) - data loss occurred",
                   (unsigned long)g_espmodem.buffer_overflow_count);
        
        if (g_espmodem.error_cb) {
            g_espmodem.error_cb(ESPMODEM_ERR_MODEM_NOT_RESPONDING,
                              "UART buffer overflow - modem sending data too fast");
        }
        break;
        
    case ESP_MODEM_TERMINAL_CHECKSUM_ERROR:
        g_espmodem.frame_error_count++;
        __log_error("UART frame/parity error (count: %lu) - electrical issue or baudrate mismatch",
                   (unsigned long)g_espmodem.frame_error_count);
        
        if (g_espmodem.error_cb) {
            g_espmodem.error_cb(ESPMODEM_ERR_MODEM_NOT_RESPONDING,
                              "UART checksum error - check connections and baudrate");
        }
        break;
        
    case ESP_MODEM_TERMINAL_DEVICE_GONE:
        __log_error("Modem device disappeared - hardware disconnected or driver issue");
        
        if (g_espmodem.error_cb) {
            g_espmodem.error_cb(ESPMODEM_ERR_POWER_FAILED,
                              "Modem device gone - hardware failure");
        }
        break;
        
    default:
        __log_warn("Unknown terminal error: %d", err);
        break;
    }
}

// ============================================================================
// +CEREG URC Parser
// ============================================================================

/**
 * Parse +CEREG URC and extract registration details
 * 
 * Supports all formats from n=1 to n=5:
 * - +CEREG: <stat>
 * - +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>]]
 * - +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>][,<cause_type>,<reject_cause>]]
 * - +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>][,,[,[<Active-Time>],[<Periodic-TAU>]]]]
 * - +CEREG: <stat>[,[<tac>],[<ci>],[<AcT>][,[<cause_type>],[<reject_cause>][,[<Active-Time>],[<Periodic-TAU>]]]]
 */
static bool parse_cereg_urc(const char* urc, espmodem_event_t* event)
{
    if (!urc || !event || strncmp(urc, "+CEREG:", 7) != 0) {
        return false;
    }
    
    // Initialize event structure
    event->event_type = ESPMODEM_EVENT_REGISTRATION_STATUS;
    event->registration.stat = -1;
    event->registration.n = -1;  // Unknown reporting level from URC
    event->registration.tac = NULL;
    event->registration.ci = NULL;
    event->registration.act = -1;
    event->registration.cause_type = -1;
    event->registration.reject_cause = -1;
    event->registration.active_time = NULL;
    event->registration.periodic_tau = NULL;
    
    // Skip "+CEREG: " prefix
    const char* p = urc + 7;
    while (*p == ' ') p++;  // Skip spaces
    
    // Parse stat (required)
    if (*p < '0' || *p > '9') {
        __log_warn("Invalid +CEREG format: missing stat");
        return false;
    }
    
    event->registration.stat = atoi(p);
    
    // Skip to next field or end
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') {
        // Simple format: +CEREG: <stat>
        __log_debug("Parsed +CEREG: stat=%d", event->registration.stat);
        return true;
    }
    p++;  // Skip comma
    
    // Parse optional tac
    if (*p == '"') {
        p++;  // Skip opening quote
        event->registration.tac = p;
        while (*p && *p != '"') p++;
        if (*p == '"') {
            // Null-terminate TAC (modifying the buffer - caller must handle this)
            *(char*)p = '\0';
            p++;
        }
    }
    
    // Skip to next field
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') goto done;
    p++;
    
    // Parse optional ci
    if (*p == '"') {
        p++;  // Skip opening quote
        event->registration.ci = p;
        while (*p && *p != '"') p++;
        if (*p == '"') {
            *(char*)p = '\0';
            p++;
        }
    }
    
    // Skip to next field
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') goto done;
    p++;
    
    // Parse optional AcT
    if (*p >= '0' && *p <= '9') {
        event->registration.act = atoi(p);
    }
    
    // Skip to next field
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') goto done;
    p++;
    
    // Parse optional cause_type
    if (*p >= '0' && *p <= '9') {
        event->registration.cause_type = atoi(p);
    }
    
    // Skip to next field
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') goto done;
    p++;
    
    // Parse optional reject_cause
    if (*p >= '0' && *p <= '9') {
        event->registration.reject_cause = atoi(p);
    }
    
    // Skip to next field
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') goto done;
    p++;
    
    // Parse optional Active-Time
    if (*p == '"') {
        p++;
        event->registration.active_time = p;
        while (*p && *p != '"') p++;
        if (*p == '"') {
            *(char*)p = '\0';
            p++;
        }
    }
    
    // Skip to next field
    while (*p && *p != ',' && *p != '\r' && *p != '\n') p++;
    if (*p != ',') goto done;
    p++;
    
    // Parse optional Periodic-TAU
    if (*p == '"') {
        p++;
        event->registration.periodic_tau = p;
        while (*p && *p != '"') p++;
        if (*p == '"') {
            *(char*)p = '\0';
            p++;
        }
    }
    
done:
    __log_debug("Parsed +CEREG: stat=%d, tac=%s, ci=%s, act=%d",
               event->registration.stat,
               event->registration.tac ? event->registration.tac : "NULL",
               event->registration.ci ? event->registration.ci : "NULL",
               event->registration.act);
    return true;
}

static esp_err_t unsolicited_handler(uint8_t *data, size_t len)
{
    // Early return if no data
    if (!data || len == 0) {
        return ESP_OK;
    }
    
    // Detect ESP modem library buffer accumulation
    // The library keeps URCs in an internal buffer and only clears during AT commands
    // If we see the exact same length twice, it means no new data - skip processing
    if (len == g_last_processed_len && len > 0) {
        __log_debug("Buffer unchanged (len=%u) - skipping reprocess", (unsigned)len);
        return ESP_OK;
    }
    
    // If buffer grew, we only want to process the NEW data at the end
    // But this is difficult because the buffer may reorganize. For now, process all
    // and rely on the library eventually clearing via next AT command.
    // Log the growth for debugging
    if (g_last_processed_len > 0 && len != g_last_processed_len) {
        __log_debug("Buffer size changed: %u -> %u bytes", (unsigned)g_last_processed_len, (unsigned)len);
    }
    
    // Debug logging
    __log_debug("unsolicited_handler called: len=%u", (unsigned)len);
    if (len < 200) {
        char debug_buf[256];
        size_t debug_len = (len < sizeof(debug_buf) - 1) ? len : sizeof(debug_buf) - 1;
        memcpy(debug_buf, data, debug_len);
        debug_buf[debug_len] = '\0';
        __log_debug("Raw URC data: \"%s\"", debug_buf);
    }
    
    // Determine if we should suppress user callbacks (during AT command execution)
    bool suppress_user_callback = g_espmodem.command_in_progress;
    if (suppress_user_callback) {
        __log_debug("URC during command execution - processing but suppressing user callback");
    }
    
    // Process the entire buffer - ESP modem library will only call us when new data arrives
    // We don't need to track what we've seen before because by processing and returning ESP_OK,
    // we tell the library we've consumed this data
    char temp_buf[512];
    size_t copy_len = (len < sizeof(temp_buf) - 1) ? len : sizeof(temp_buf) - 1;
    memcpy(temp_buf, data, copy_len);
    temp_buf[copy_len] = '\0';
    
    // Split multi-line URCs and process each line separately
    char* line_start = temp_buf;
    char* line_end;
    
    while (line_start && *line_start) {
        // Find the end of this line
        line_end = line_start;
        while (*line_end && *line_end != '\r' && *line_end != '\n') {
            line_end++;
        }
        
        // Temporarily null-terminate this line
        char saved_char = *line_end;
        *line_end = '\0';
        
        // Strip leading whitespace
        const char* line = line_start;
        while (*line && (*line == ' ' || *line == '\t')) {
            line++;
        }
        
        // Process non-empty lines
        size_t line_len = strlen(line);
        if (line_len > 0) {
            // Filter out solicited responses (OK/ERROR)
            if (strcmp(line, "OK") != 0 && strcmp(line, "ERROR") != 0) {
                // Validate UTF-8 (filter binary data during resets)
                bool valid_utf8 = true;
                for (size_t i = 0; i < line_len; i++) {
                    unsigned char c = (unsigned char)line[i];
                    // Check for control characters (except CR, LF, TAB)
                    if (c < 0x20 && c != '\r' && c != '\n' && c != '\t') {
                        valid_utf8 = false;
                        break;
                    }
                    // Check for invalid UTF-8 sequences
                    if ((c >= 0x80 && c <= 0xBF) || c == 0xC0 || c == 0xC1 || c >= 0xF5) {
                        valid_utf8 = false;
                        break;
                    }
                }
                
                if (!valid_utf8) {
                    __log_debug("URC contains invalid UTF-8 - dropping");
                } else {
                    __log_debug("URC received (%u bytes)", (unsigned)line_len);
                    
                    // Special handling for +SHUTDOWN - ALWAYS set flag
                    if (strstr(line, "+SHUTDOWN")) {
                        g_espmodem.shutdown_received = true;
                        __log_info("+SHUTDOWN URC detected");
                    }
                    
                    // Forward to callbacks if not suppressed
                    if (!suppress_user_callback) {
                        // Parse +CEREG URCs into structured events
                        if (strncmp(line, "+CEREG:", 7) == 0) {
                            char cereg_copy[256];
                            strncpy(cereg_copy, line, sizeof(cereg_copy) - 1);
                            cereg_copy[sizeof(cereg_copy) - 1] = '\0';
                            
                            espmodem_event_t cereg_event;
                            if (parse_cereg_urc(cereg_copy, &cereg_event) && g_event_handler) {
                                dispatch_event(&cereg_event);
                            }
                        }
                        
                        // Dispatch generic URC event
                        if (g_event_handler) {
                            espmodem_event_t event = {
                                .event_type = ESPMODEM_EVENT_URC,
                                .urc = {
                                    .data = line,
                                    .length = line_len
                                }
                            };
                            dispatch_event(&event);
                        }
                        
                        // Legacy callback support
                        if (g_espmodem.unsolicited_cb) {
                            g_espmodem.unsolicited_cb(line, line_len);
                        }
                    } else {
                        __log_debug("URC suppressed: %s", line);
                    }
                }
            } else {
                __log_debug("Ignoring solicited response (OK/ERROR)");
            }
        }
        
        // Restore character and move to next line
        *line_end = saved_char;
        if (*line_end == '\r') line_end++;
        if (*line_end == '\n') line_end++;
        line_start = line_end;
    }
    
    // Force the ESP modem library to clear its internal URC buffer by sending a dummy AT command
    // The library accumulates URCs in an internal buffer that's only cleared during AT command execution
    // By sending "AT" here, we force an immediate buffer flush to prevent accumulation
    // REMOVED: This causes recursion - the OK response triggers unsolicited_handler again!
    //
    // Instead, we detect buffer growth and skip reprocessing old data:
    // - If len <= g_last_processed_len: we've seen this buffer before, skip it
    // - If len > g_last_processed_len: new data arrived, process from the growth point
    //
    // Update: Track buffer length to detect accumulation
    g_last_processed_len = len;
    
    // Return ESP_OK to tell the library we've consumed this data
    return ESP_OK;
}
#endif

espmodem_err_t espmodem_register_unsolicited_callback(espmodem_unsolicited_cb_t callback)
{
    // Update callback pointer - the URC handler is always registered during init
    // This just controls whether URCs are forwarded to Python or not
    g_espmodem.unsolicited_cb = callback;
    
#ifdef CONFIG_ESP_MODEM_URC_HANDLER
    if (callback) {
        __log_info("User URC callback registered - URCs will be forwarded to Python");
    } else {
        __log_info("User URC callback cleared - URCs will only be used for internal tracking");
    }
    
    // Note: URC handler is always active (registered during init)
    // This ensures +SHUTDOWN detection works even without user callback
#else
    __log_warn("URC handler support not compiled (CONFIG_ESP_MODEM_URC_HANDLER not set)");
    __log_warn("Unsolicited responses will not be captured");
#endif
    
    return ESPMODEM_OK;
}

// ============================================================================
// Unified Event Handler (stub implementation - events not yet dispatched)
// ============================================================================

static espmodem_event_cb_t g_event_handler = NULL;
static uint32_t g_event_mask = 0;
static void* g_event_user_ctx = NULL;

espmodem_err_t espmodem_register_event_handler(espmodem_event_cb_t callback, 
                                               uint32_t event_mask,
                                               void* user_ctx)
{
    g_event_handler = callback;
    g_event_mask = event_mask;
    g_event_user_ctx = user_ctx;
    
    if (callback) {
        __log_info("Unified event handler registered with mask 0x%04x", event_mask);
        
        // Also register URC callback if URC events are subscribed
        if (event_mask & ESPMODEM_EVENT_URC) {
            __log_debug("URC events subscribed - enabling URC forwarding");
            // The actual URC forwarding will be done by dispatch_event() when implemented
        }
    } else {
        __log_info("Unified event handler unregistered");
    }
    
    return ESPMODEM_OK;
}

// Internal helper to dispatch events to the unified event handler
// This will be called from various places in espmodem.c to send events
static void dispatch_event(const espmodem_event_t* event)
{
    if (g_event_handler && event && (event->event_type & g_event_mask)) {
        __log_debug("Dispatching event type 0x%04x to handler", event->event_type);
        g_event_handler(event, g_event_user_ctx);
    }
}