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
 * @brief   ESP Modem Middle Layer for Sequans Monarch 2 GM02S
 *          Provides interface to ESP modem library with specific handling for
 *          Sequans modems including smart baudrate detection, CMUX support,
 *          I2C power management, and robust initialization.
 * --------------------------------------------------------------------------- *
 */

#ifndef __ESPMODEM_H__
#define __ESPMODEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_modem_api.h"
#include "esp_netif.h"

// Hardware configuration for SG-Wireless F1 platform
#define ESPMODEM_UART_PORT      1
#define ESPMODEM_UART_TX_PIN    47
#define ESPMODEM_UART_RX_PIN    48
#define ESPMODEM_UART_RTS_PIN   33
#define ESPMODEM_UART_CTS_PIN   6

// I2C IO Expander configuration for LTE power control
#define ESPMODEM_I2C_PORT       I2C_NUM_0
#define ESPMODEM_I2C_SCL_PIN    7
#define ESPMODEM_I2C_SDA_PIN    8
#define ESPMODEM_I2C_FREQ       400000
#define ESPMODEM_I2C_ADDR       0x21
#define ESPMODEM_LTE_POWER_BIT  (1 << 1)

// Modem operation modes
typedef enum {
    ESPMODEM_MODE_CATM1 = 0,
    ESPMODEM_MODE_NBIOT = 1
} espmodem_mode_t;

// Error codes
typedef enum {
    ESPMODEM_OK = 0,
    ESPMODEM_ERR_INVALID_ARG = -1,
    ESPMODEM_ERR_NO_MEM = -2,
    ESPMODEM_ERR_TIMEOUT = -3,
    ESPMODEM_ERR_MODEM_NOT_RESPONDING = -4,
    ESPMODEM_ERR_SIM_NOT_READY = -5,
    ESPMODEM_ERR_NETWORK_FAILED = -6,
    ESPMODEM_ERR_PPP_FAILED = -7,
    ESPMODEM_ERR_CMUX_FAILED = -8,
    ESPMODEM_ERR_POWER_FAILED = -9,
    ESPMODEM_ERR_BAUDRATE_FAILED = -10
    // Note: WiFi/LTE coexistence now uses compatibility mode by default
    // Resource conflicts eliminated through reduced buffer allocation
} espmodem_err_t;

// Network information structure
typedef struct {
    char ip[16];
    char netmask[16]; 
    char gateway[16];
    char dns1[16];
    char dns2[16];
    bool connected;
} espmodem_network_info_t;

// Modem status structure
typedef struct {
    bool powered;
    bool sim_ready;
    bool network_attached;
    bool ppp_connected;
    bool cmux_active;
    uint32_t current_baudrate;
    int signal_rssi;
    int signal_ber;
    char operator_name[32];
    char imei[16];
    char iccid[21];
} espmodem_status_t;

// UART error statistics structure
typedef struct {
    uint32_t break_signal_count;      // UART break signals (modem reset/crash/PPP drop)
    uint32_t frame_error_count;       // Frame/parity errors (baudrate mismatch, electrical issues)
    uint32_t buffer_overflow_count;   // Buffer overflows (data loss)
} espmodem_error_stats_t;

// ============================================================================
// Core Initialization and Power Management
// ============================================================================

/**
 * Initialize the ESP modem middle layer
 * This includes I2C setup, power management, and modem detection
 */
/**
 * @brief Initialize the ESP modem middleware (idempotent)
 * 
 * This function is idempotent - safe to call multiple times.
 * If already initialized, returns ESPMODEM_OK immediately.
 * 
 * Handles three possible modem power states:
 * 1. Powered off - Powers on and initializes
 * 2. Normal AT mode (crash recovery) - Uses current state
 * 3. CMUX mode (soft reset) - Cleans up and reinitializes
 * 
 * @param carrier Carrier conformance mode for AT+SQNBANDSEL (e.g., "standard", "verizon", "att")
 *                Pass NULL to use default "standard"
 * 
 * @return ESPMODEM_OK on success, error code on failure
 */
espmodem_err_t espmodem_init(const char* carrier);

/**
 * Deinitialize the ESP modem and power off
 */
espmodem_err_t espmodem_deinit(bool power_off);

/**
 * Power on the modem hardware
 * @return ESPMODEM_OK on success, error code on failure
 */
espmodem_err_t espmodem_power_on(void);

/**
 * Power off the modem hardware
 * @param force If true, power off immediately without graceful shutdown
 * @return ESPMODEM_OK on success, error code on failure
 */
espmodem_err_t espmodem_power_off(bool force);

/**
 * Check if modem is powered on
 * @return true if modem is powered, false otherwise
 */
bool espmodem_is_powered(void);

/**
 * Check if modem is initialized
 * @return true if modem is initialized, false otherwise
 */
bool espmodem_is_initialized(void);

// ============================================================================
// Network Operations
// ============================================================================

/**
 * Attach to the cellular network (NON-BLOCKING)
 * Configures the modem with APN, enables radio, and initiates network search.
 * Returns immediately - use espmodem_is_attached() to poll for registration status.
 * 
 * Note: NB-IoT initial attachment may take several minutes.
 * 
 * @param apn Access Point Name (e.g., "iot.1nce.net")
 * @param type PDP type (e.g., "IP", "IPV6", "IPV4V6")
 * @param cid Context Identifier (typically 1)
 * @param band Frequency band (0 for auto, specific band number for manual, -1 if using bands string)
 * @param bands Comma-separated list of bands (e.g., "1,2,3,") or NULL if using single band
 * @return ESPMODEM_OK if attachment initiated successfully
 */
espmodem_err_t espmodem_attach(const char* apn, const char* type, int cid, int band, const char* bands);

/**
 * Connect to data session using CMUX mode (NON-BLOCKING)
 * Activates CMUX multiplexing and starts PPP negotiation in background.
 * Returns immediately - use espmodem_is_connected() to poll for PPP status.
 * 
 * Prerequisites: Network must be attached (espmodem_is_attached() returns true)
 * 
 * @return ESPMODEM_OK if CMUX activated successfully
 */
espmodem_err_t espmodem_connect(void);

/**
 * Connect to data session with network parameters (atomic operation)
 * Performs network attachment and CMUX+PPP setup in one operation
 */
espmodem_err_t espmodem_connect_with_params(const char* apn, int band, espmodem_mode_t mode);

/**
 * Disconnect from data session but keep network attachment
 */
espmodem_err_t espmodem_disconnect(void);

/**
 * Detach from cellular network
 */
espmodem_err_t espmodem_detach(void);

/**
 * Reset the modem using AT^RESET command
 */
espmodem_err_t espmodem_reset(void);

/**
 * Change carrier conformance mode at 921600 baud (after initialization)
 * Handles the modem reset automatically when conformance mode changes
 * @param carrier New carrier mode (e.g., "standard", "verizon", "att")
 * @return ESPMODEM_OK on success, error code otherwise
 */
espmodem_err_t espmodem_change_conformance_mode(const char* carrier);

/**
 * Waits for +SHUTDOWN and +SYSSTART, re-detects baudrate
 */
espmodem_err_t espmodem_wait_reset(bool wait_shutdown);

// Note: espmodem_recover_from_crash() is internal and called automatically
// when crash is detected (10+ breaks in 3 seconds). No user API needed.

// ============================================================================
// Status and Information
// ============================================================================

/**
 * Check if modem is attached to network
 * Queries modem with AT+CEREG? to get current registration status
 * @return true if registered (home or roaming), false otherwise
 */
bool espmodem_is_attached(void);

/**
 * Check if PPP data connection is active
 * @return true if PPP has obtained an IP address, false otherwise
 */
bool espmodem_is_connected(void);

/**
 * Check if LTE modem is currently active (for WiFi conflict detection)
 */
bool espmodem_is_active(void);

/**
 * Wait for crash recovery to complete (if in progress)
 * This should be called before any AT command operation
 * 
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return ESPMODEM_OK if recovery complete or not in progress, ESPMODEM_ERR_TIMEOUT if timeout
 */
espmodem_err_t espmodem_wait_for_recovery(uint32_t timeout_ms);

/**
 * Get comprehensive modem status
 */
espmodem_err_t espmodem_get_status(espmodem_status_t* status);

/**
 * Get UART error statistics (break signals, frame errors, overflows)
 * 
 * Break signals indicate:
 * - Expected: Modem reset (AT^RESET, conformance mode change)
 * - Unexpected: Modem crash, PPP connection drop, cable disconnect
 * 
 * @param stats Pointer to structure to receive error statistics
 * @return ESPMODEM_OK on success, ESPMODEM_ERR_INVALID_ARG if stats is NULL
 */
espmodem_err_t espmodem_get_error_stats(espmodem_error_stats_t* stats);

/**
 * Get network information (IP, gateway, DNS, etc.)
 */
espmodem_err_t espmodem_get_network_info(espmodem_network_info_t* info);

/**
 * Get signal strength
 * @param rssi Raw RSSI value (0-31, 99=unknown)
 * @param rssi_dbm RSSI in dBm (-113 to -51, -999=unknown)
 * @param ber Bit Error Rate (0-7, 99=unknown)
 */
espmodem_err_t espmodem_get_signal_strength(int* rssi, int* rssi_dbm, int* ber);

/**
 * Check if SIM card is present and ready
 * Retries up to 5 times with 250ms delays between attempts (matches LTE.py behavior)
 * First checks if radio is enabled (CFUN=1), if not, sets CFUN=4 to read SIM
 * @return ESPMODEM_OK if SIM is ready, ESPMODEM_ERR_SIM_NOT_READY if not
 */
espmodem_err_t espmodem_check_sim_present(void);

// ============================================================================
// Modem Configuration
// ============================================================================

/**
 * Get current network mode (CAT-M1 or NB-IoT)
 */
espmodem_err_t espmodem_get_mode(espmodem_mode_t* mode);

/**
 * Set network mode (CAT-M1 or NB-IoT)
 * This will cause a modem reset
 */
espmodem_err_t espmodem_set_mode(espmodem_mode_t mode);

/**
 * Get IMEI number
 */
espmodem_err_t espmodem_get_imei(char* imei, size_t imei_size);

/**
 * Get ICCID (SIM card ID)
 */
espmodem_err_t espmodem_get_iccid(char* iccid, size_t iccid_size);

// ============================================================================
// Advanced Operations
// ============================================================================

/**
 * Send raw AT command to the modem
 * Only works when not in PPP mode or when CMUX is active
 */
espmodem_err_t espmodem_send_at_command(const char* command, char* response, 
                                       size_t response_size, uint32_t timeout_ms);

/**
 * Enable/disable debug logging
 */
void espmodem_set_debug(bool enable);

/**
 * Get the ESP modem DCE handle for advanced operations
 * Use with caution - for expert users only
 */
esp_modem_dce_t* espmodem_get_dce_handle(void);

/**
 * Get the ESP netif handle for advanced network operations
 */
esp_netif_t* espmodem_get_netif_handle(void);

// ============================================================================
// Event System (Unified Event Callbacks)
// ============================================================================

/**
 * LTE Event Types
 * 
 * Applications can register a single callback to receive all events, or use
 * event masks to filter specific events of interest.
 */
typedef enum {
    ESPMODEM_EVENT_NONE                = 0x0000,
    ESPMODEM_EVENT_URC                 = 0x0001,  // Unsolicited response code received
    ESPMODEM_EVENT_REGISTRATION_STATUS = 0x0002,  // Network registration status changed (+CEREG)
    ESPMODEM_EVENT_PPP_CONNECTED       = 0x0008,  // PPP connection established
    ESPMODEM_EVENT_PPP_DISCONNECTED    = 0x0010,  // PPP connection lost
    ESPMODEM_EVENT_MODEM_CRASH         = 0x0020,  // Modem crash detected
    ESPMODEM_EVENT_MODEM_RESET         = 0x0040,  // Modem reset initiated
    ESPMODEM_EVENT_SIGNAL_QUALITY      = 0x0080,  // Signal strength changed
    ESPMODEM_EVENT_ERROR               = 0x0100,  // Error occurred
    ESPMODEM_EVENT_ALL                 = 0xFFFF   // Subscribe to all events
} espmodem_event_type_t;

/**
 * Event data structure
 * 
 * Contains event-specific data. Check event_type to determine which
 * union member is valid.
 */
typedef struct {
    espmodem_event_type_t event_type;
    
    union {
        // ESPMODEM_EVENT_URC
        struct {
            const char* data;      // URC string (null-terminated)
            size_t length;         // Length of URC string
        } urc;
        
        // ESPMODEM_EVENT_REGISTRATION_STATUS
        struct {
            int stat;              // Registration status (0-10, 80)
            int n;                 // URC reporting level (0-5)
            const char* tac;       // Tracking Area Code (hex string, may be NULL)
            const char* ci;        // Cell ID (hex string, may be NULL)
            int act;               // Access Technology (7=LTE-M, 9=NB-IoT)
            int cause_type;        // Cause type (0=EMM, 1=manufacturer, -1=not present)
            int reject_cause;      // Reject cause (-1 if not present)
            const char* active_time;    // T3324 timer (may be NULL)
            const char* periodic_tau;   // T3412 timer (may be NULL)
        } registration;
        
        // ESPMODEM_EVENT_PPP_CONNECTED / PPP_DISCONNECTED
        struct {
            bool connected;        // true if connected, false if disconnected
            const char* ip;        // IP address (NULL if disconnected)
            const char* netmask;   // Netmask (NULL if disconnected)
            const char* gateway;   // Gateway IP (NULL if disconnected)
            const char* dns1;      // Primary DNS (NULL if disconnected)
            const char* dns2;      // Secondary DNS (NULL if disconnected or not available)
        } ppp;
        
        // ESPMODEM_EVENT_MODEM_CRASH
        struct {
            uint32_t break_count;  // Number of UART breaks detected
            bool recovery_started; // true if automatic recovery initiated
        } crash;
        
        // ESPMODEM_EVENT_MODEM_RESET
        struct {
            bool user_initiated;   // true if reset via espmodem_reset()
            const char* reason;    // Reason for reset
        } reset;
        
        // ESPMODEM_EVENT_SIGNAL_QUALITY
        struct {
            int rssi;              // Raw RSSI value (0-31, 99=unknown)
            int rssi_dbm;          // RSSI in dBm (-113 to -51)
            int ber;               // Bit error rate (0-7, 99=unknown)
        } signal;
        
        // ESPMODEM_EVENT_ERROR
        struct {
            espmodem_err_t error_code;
            const char* message;   // Error message (null-terminated)
            const char* operation; // Operation that failed (may be NULL)
        } error;
    };
} espmodem_event_t;

/**
 * Unified event callback function type
 * 
 * Called when any subscribed event occurs. The application should check
 * event->event_type to determine which event occurred and access the
 * appropriate union member for event data.
 * 
 * @param event Event structure containing event type and data
 * @param user_ctx User context pointer (set during registration)
 */
typedef void (*espmodem_event_cb_t)(const espmodem_event_t* event, void* user_ctx);

/**
 * Legacy callback types (deprecated, use espmodem_event_cb_t instead)
 */
typedef void (*espmodem_network_event_cb_t)(bool connected, const espmodem_network_info_t* info);
typedef void (*espmodem_signal_event_cb_t)(int rssi, int rssi_dbm, int ber);
typedef void (*espmodem_error_event_cb_t)(espmodem_err_t error, const char* message);
typedef void (*espmodem_unsolicited_cb_t)(const char* data, size_t length);

/**
 * Register unified event callback
 * 
 * Register a single callback to receive events. Use event_mask to filter
 * which events you want to receive, or use ESPMODEM_EVENT_ALL to receive
 * all events.
 * 
 * Example - Subscribe to connection events only:
 *   espmodem_register_event_handler(my_callback, 
 *                                    ESPMODEM_EVENT_PPP_CONNECTED | 
 *                                    ESPMODEM_EVENT_PPP_DISCONNECTED,
 *                                    my_context);
 * 
 * Example - Subscribe to all events:
 *   espmodem_register_event_handler(my_callback, ESPMODEM_EVENT_ALL, NULL);
 * 
 * @param callback Function to call when events occur (NULL to unregister)
 * @param event_mask Bitmask of events to receive (OR'd espmodem_event_type_t values)
 * @param user_ctx User context pointer passed to callback (can be NULL)
 * @return ESPMODEM_OK on success, error code on failure
 */
espmodem_err_t espmodem_register_event_handler(espmodem_event_cb_t callback, 
                                               uint32_t event_mask,
                                               void* user_ctx);

/**
 * Legacy callback registration (deprecated, use espmodem_register_event_handler)
 * 
 * These functions are maintained for backward compatibility but will be
 * removed in a future version. Use the unified event handler instead.
 */
espmodem_err_t espmodem_register_network_callback(espmodem_network_event_cb_t callback);
espmodem_err_t espmodem_register_signal_callback(espmodem_signal_event_cb_t callback);
espmodem_err_t espmodem_register_error_callback(espmodem_error_event_cb_t callback);
espmodem_err_t espmodem_register_unsolicited_callback(espmodem_unsolicited_cb_t callback);

#ifdef __cplusplus
}
#endif

#endif /* __ESPMODEM_H__ */