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
 * @author  Ahmed Sabry (SG Wireless)
 * @maintainer  Christian Ehlers (SG Wireless)
 * 
 * @brief   This file contains MicroPython-based IO Expander implementation
 *          using the I2C bridge library (simplified version for testing)
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// Set log level to INFO for this component
#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ioexp.h"
#include "pcal6408a.h"
#include "mp_i2c_bridge.h"

// For console output in stats function
#include "py/mpprint.h"

/* --- Ahmed's logging framework ------------------------------------------ */
#define __log_subsystem     F1
#define __log_component     ioexp_mp
#include "log_lib.h"
__log_component_def(F1, ioexp_mp, cyan, 1, 0)

/* --- function declarations ------------------------------------------------ */
static void ioexp_enable_interrupt_task(void);

/* --- local variables ------------------------------------------------------ */

static bool s_ioexp_initialized = false;
static pcal6408a_init_config_t s_pcal6408a_config;

// Pin mappings (from original ioexp.c)
#define __ioexp_pin__lora_power     (pcal6408a_port_pin_0)
#define __ioexp_pin__lte_power      (pcal6408a_port_pin_1)
#define __ioexp_pin__lora_reset_n   (pcal6408a_port_pin_2)
#define __ioexp_pin__lte_reset_n    (pcal6408a_port_pin_3)
#define __ioexp_pin__secure_en      (pcal6408a_port_pin_4)
#define __ioexp_pin__lora_int       (pcal6408a_port_pin_5)
#define __ioexp_pin__lora_free      (pcal6408a_port_pin_6)
#define __ioexp_pin__lte_ring       (pcal6408a_port_pin_7)

// Pin handles (like original)
static pcal6408a_port_pin_handle ioexp_drv_handle__lora_power  = NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__lte_power   = NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__lora_reset_n= NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__lte_reset_n = NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__secure_en   = NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__lora_int    = NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__lora_free   = NULL;
static pcal6408a_port_pin_handle ioexp_drv_handle__lte_ring    = NULL;

// State cache for power pins (to remember state when pins are deactivated)
static bool s_lora_power_state = false;  // false = OFF, true = ON
static bool s_lte_power_state = false;   // false = OFF, true = ON

// LoRa interrupt callback storage (for LoRaWAN stack integration)
static volatile ioexp_callback_t s_lora_int_callback = NULL;
static volatile ioexp_callback_t s_lora_busy_callback = NULL;

// Interrupt polling task handle (for PCAL6408A interrupt processing)
static TaskHandle_t s_ioexp_task_handle = NULL;

/* --- Interrupt polling task implementation ------------------------------- */

static void mp_ioexp_task(void * arg)
{
    __log_info("ioexp interrupt polling task started");
    
    while(1) {
        // Wait for a longer delay initially to allow system to fully initialize
        vTaskDelay(pdMS_TO_TICKS(100));  // 100ms polling interval to be safer
        
        // Only call interrupt trigger if system is fully initialized
        if (s_ioexp_initialized) {
            // Always trigger interrupt processing since pins are configured before task starts
            __log_debug("triggering interrupt handling");
            pcal6408a_interrupt_trigger_port();
        }
    }
}

static void mp_ioexp_task_init(void)
{
    if (s_ioexp_task_handle != NULL) {
        __log_info("ioexp task already initialized");
        return;
    }
    
    BaseType_t result = xTaskCreate(
        mp_ioexp_task,           // task function
        "ioexp-ta",              // task name (shortened to match logs)
        4 * 1024,                // stack size (4KB)
        NULL,                    // parameter to task function
        configMAX_PRIORITIES - 1, // high priority (but not highest)
        &s_ioexp_task_handle     // handle to the task
    );
    
    if (result != pdPASS || s_ioexp_task_handle == NULL) {
        __log_error("Failed to create ioexp interrupt polling task");
        s_ioexp_task_handle = NULL;
    } else {
        __log_info("ioexp interrupt polling task created successfully");
    }
}

/* --- LoRa interrupt handlers for PCAL6408A integration ------------------- */

static void lora_int_pin_handler(bool pin_value)
{
    // This gets called by PCAL6408A driver when DIO_1 pin changes (TX_DONE, RX_DONE, etc.)
    __log_info("LoRa INT pin handler called: pin_value=%d", pin_value);
    if (s_lora_int_callback) {
        // Call the LoRaWAN stack callback with current timestamp
        uint32_t timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
        __log_info("calling LoRa INT callback with timestamp=%u", timestamp);
        s_lora_int_callback(timestamp);
    } else {
        __log_warn("LoRa INT callback is NULL");
    }
}

static void lora_busy_pin_handler(bool pin_value)
{
    // This gets called by PCAL6408A driver when busy pin changes
    __log_info("LoRa BUSY pin handler called: pin_value=%d", pin_value);
    if (s_lora_busy_callback) {
        // Call the LoRaWAN stack callback with current timestamp
        uint32_t timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
        __log_info("calling LoRa BUSY callback with timestamp=%u", timestamp);
        s_lora_busy_callback(timestamp);
    } else {
        __log_warn("LoRa BUSY callback is NULL");
    }
}

/* --- PCAL6408A I2C interface using MP I2C bridge ------------------------ */

static void pcal6408a_mp_i2c_read(uint8_t dev_addr, uint8_t* buff, uint32_t cbytes) {
    esp_err_t ret = mp_i2c_bridge_read(dev_addr, buff, cbytes);
    if (ret != ESP_OK) {
        __log_error("I2C read failed: %s", esp_err_to_name(ret));
    }
}

static void pcal6408a_mp_i2c_write(uint8_t dev_addr, uint8_t* buff, uint32_t cbytes) {
    esp_err_t ret = mp_i2c_bridge_write(dev_addr, buff, cbytes);
    if (ret != ESP_OK) {
        __log_error("I2C write failed: %s", esp_err_to_name(ret));
    }
}

/* --- API implementation --------------------------------------------------- */

void ioexp_init(void)
{
    esp_err_t ret;
    
    __log_info("ioexp_init() called");
    
    if (s_ioexp_initialized) {
        __log_info("ioexp already initialized");
        return;
    }
    
    // Initialize I2C bridge
    ret = mp_i2c_bridge_init(7, 8, 100000);  // SCL=7, SDA=8, 100kHz
    if (ret != ESP_OK) {
        __log_error("Failed to initialize I2C bridge: %s", esp_err_to_name(ret));
        return;
    }
    
    // Configure PCAL6408A driver to use MicroPython I2C bridge
    s_pcal6408a_config.is_addr_pin_connected_to_vdd = true;
    s_pcal6408a_config.is_open_drain_output = false;
    s_pcal6408a_config.i2c_read = pcal6408a_mp_i2c_read;
    s_pcal6408a_config.i2c_write = pcal6408a_mp_i2c_write;
    
    pcal6408a_error_t pcal_ret = pcal6408a_init(&s_pcal6408a_config);
    if (pcal_ret != __pcal6408a_ok) {
        __log_error("Failed to initialize PCAL6408A: %d", pcal_ret);
        mp_i2c_bridge_deinit();
        return;
    }
    
    // Configure all pins like the original implementation
    ioexp_drv_handle__lora_power = pcal6408a_configure_output_port_pin(
        __ioexp_pin__lora_power, pcal6408a_pull_resistor_none, 
        pcal6408a_drive_capability_level_0, "lora_power");
    
    ioexp_drv_handle__lte_power = pcal6408a_configure_output_port_pin(
        __ioexp_pin__lte_power, pcal6408a_pull_resistor_none, 
        pcal6408a_drive_capability_level_0, "lte_power");
    
    ioexp_drv_handle__lora_reset_n = pcal6408a_configure_output_port_pin(
        __ioexp_pin__lora_reset_n, pcal6408a_pull_resistor_none, 
        pcal6408a_drive_capability_level_0, "lora_reset_n");
    
    ioexp_drv_handle__lte_reset_n = pcal6408a_configure_output_port_pin(
        __ioexp_pin__lte_reset_n, pcal6408a_pull_resistor_none, 
        pcal6408a_drive_capability_level_0, "lte_reset_n");
    
    ioexp_drv_handle__secure_en = pcal6408a_configure_output_port_pin(
        __ioexp_pin__secure_en, pcal6408a_pull_resistor_none, 
        pcal6408a_drive_capability_level_0, "secure_en");

    // Configure LoRa interrupt pins as inputs (for SX1262 DIO_1 and busy signals)
    ioexp_drv_handle__lora_int = pcal6408a_configure_input_port_pin(
        __ioexp_pin__lora_int, pcal6408a_pull_resistor_down, false, false, 
        lora_int_pin_handler, "lora_int");
    
    ioexp_drv_handle__lora_free = pcal6408a_configure_input_port_pin(
        __ioexp_pin__lora_free, pcal6408a_pull_resistor_down, false, false, 
        lora_busy_pin_handler, "lora_free");
    
    // Set initial power states to OFF (low) for power saving
    // Activate pins first, set them low, then deactivate for power saving
    if (ioexp_drv_handle__lora_power) {
        __log_info("Activating LoRa power pin, setting LOW, then deactivating");
        pcal6408a_activate_pin(ioexp_drv_handle__lora_power);
        pcal6408a_write(ioexp_drv_handle__lora_power, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__lora_power);
        s_lora_power_state = false; // Cache initial state
    }
    
    if (ioexp_drv_handle__lte_power) {
        __log_info("Activating LTE power pin, setting LOW, then deactivating");
        pcal6408a_activate_pin(ioexp_drv_handle__lte_power);
        pcal6408a_write(ioexp_drv_handle__lte_power, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__lte_power);
        s_lte_power_state = false; // Cache initial state
    }
    
    // Set reset pins to inactive state
    if (ioexp_drv_handle__lora_reset_n) {
        __log_info("Activating LoRa reset pin, setting LOW, then deactivating");
        pcal6408a_activate_pin(ioexp_drv_handle__lora_reset_n);
        pcal6408a_write(ioexp_drv_handle__lora_reset_n, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__lora_reset_n);
    }
    
    if (ioexp_drv_handle__lte_reset_n) {
        __log_info("Activating LTE reset pin, setting LOW, then deactivating");
        pcal6408a_activate_pin(ioexp_drv_handle__lte_reset_n);
        pcal6408a_write(ioexp_drv_handle__lte_reset_n, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__lte_reset_n);
    }
    
    // Set secure element to disabled state
    if (ioexp_drv_handle__secure_en) {
        __log_info("Activating secure element pin, setting LOW, then deactivating");
        pcal6408a_activate_pin(ioexp_drv_handle__secure_en);
        pcal6408a_write(ioexp_drv_handle__secure_en, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__secure_en);
    }
    
    s_ioexp_initialized = true;
    __log_info("IO expander initialized successfully using MicroPython I2C bridge");
    
    // Now that all pins are configured, we can safely enable the interrupt task
    __log_info("About to enable interrupt task");
    ioexp_enable_interrupt_task();
    __log_info("ioexp_init() completed");
}

void ioexp_enable_interrupt_task(void)
{
    if (s_ioexp_initialized && s_ioexp_task_handle == NULL) {
        __log_info("Creating ioexp interrupt polling task");
        mp_ioexp_task_init();
    }
}

void ioexp_reset(void)
{
    if (s_ioexp_initialized) {
        // Clean up interrupt polling task
        if (s_ioexp_task_handle != NULL) {
            vTaskDelete(s_ioexp_task_handle);
            s_ioexp_task_handle = NULL;
            __log_info("ioexp interrupt polling task deleted");
        }
        
        mp_i2c_bridge_deinit();
        s_ioexp_initialized = false;
        __log_info("IO expander reset");
    }
}

// Helper function to safely read pin status, handling deactivated pins
static pcal6408a_error_t safe_pin_read(pcal6408a_port_pin_handle handle, bool *status)
{
    if (!handle) {
        return __pcal6408a_bad_handle;
    }
    
    pcal6408a_error_t err = pcal6408a_read(handle, status);
    __log_info("Initial read result: err=%d, status=%s", err, *status ? "TRUE" : "FALSE");
    
    if (err == __pcal6408a_pin_inactive) {
        __log_info("Pin inactive, temporarily reactivating for read");
        // Pin was deactivated (power saving mode), reactivate temporarily to read
        err = pcal6408a_activate_pin(handle);
        if (err == __pcal6408a_ok) {
            err = pcal6408a_read(handle, status);
            __log_info("After reactivation: err=%d, status=%s", err, *status ? "TRUE" : "FALSE");
            // Deactivate again to preserve power saving
            pcal6408a_deactivate_pin(handle);
            __log_info("Pin deactivated again for power saving");
        } else {
            __log_error("Failed to reactivate pin: %d", err);
        }
    }
    
    return err;
}

void ioexp_stats(void)
{
    if (!s_ioexp_initialized) {
        mp_printf(&mp_plat_print, "IO expander not initialized\n");
        return;
    }
    
    mp_printf(&mp_plat_print, "IO expander stats:\n");
    mp_printf(&mp_plat_print, "  - Initialized: %s\n", s_ioexp_initialized ? "Yes" : "No");
    mp_printf(&mp_plat_print, "  - Using MicroPython I2C bridge\n");
    
    // Check and display status of each configured pin
    bool pin_status;
    pcal6408a_error_t err;
    
    // LoRa power status (using cached state for accuracy)
    mp_printf(&mp_plat_print, "  - LoRa Power: %s\n", s_lora_power_state ? "ON" : "OFF");
    
    // LoRa reset status
    if (ioexp_drv_handle__lora_reset_n) {
        err = safe_pin_read(ioexp_drv_handle__lora_reset_n, &pin_status);
        if (err == __pcal6408a_ok) {
            mp_printf(&mp_plat_print, "  - LoRa Reset: %s\n", pin_status ? "RELEASED" : "ACTIVE");
        } else {
            mp_printf(&mp_plat_print, "  - LoRa Reset: Read error (%d)\n", err);
        }
    } else {
        mp_printf(&mp_plat_print, "  - LoRa Reset: Not configured\n");
    }
    
    // LTE power status (using cached state for accuracy)
    mp_printf(&mp_plat_print, "  - LTE Power: %s\n", s_lte_power_state ? "ON" : "OFF");
    
    // LTE reset status
    if (ioexp_drv_handle__lte_reset_n) {
        err = safe_pin_read(ioexp_drv_handle__lte_reset_n, &pin_status);
        if (err == __pcal6408a_ok) {
            mp_printf(&mp_plat_print, "  - LTE Reset: %s\n", pin_status ? "RELEASED" : "ACTIVE");
        } else {
            mp_printf(&mp_plat_print, "  - LTE Reset: Read error (%d)\n", err);
        }
    } else {
        mp_printf(&mp_plat_print, "  - LTE Reset: Not configured\n");
    }
    
    // Secure element status
    if (ioexp_drv_handle__secure_en) {
        err = safe_pin_read(ioexp_drv_handle__secure_en, &pin_status);
        if (err == __pcal6408a_ok) {
            mp_printf(&mp_plat_print, "  - Secure Element: %s\n", pin_status ? "ENABLED" : "DISABLED");
        } else {
            mp_printf(&mp_plat_print, "  - Secure Element: Read error (%d)\n", err);
        }
    } else {
        mp_printf(&mp_plat_print, "  - Secure Element: Not configured\n");
    }
}

bool ioexp_micropython_req_i2c_init(int port, int scl, int sda, uint32_t freq, uint32_t timeout_ms)
{
    // Validate parameters against our expected configuration
    if (port != 0 || scl != 7 || sda != 8) {
        __log_error("Invalid I2C configuration: port=%d, scl=%d, sda=%d", port, scl, sda);
        return false;
    }
    
    if (freq != 0 && freq != 100000) {
        __log_error("Invalid I2C frequency: %lu (expected 100000)", freq);
        return false;
    }
    
    __log_info("MicroPython I2C configuration validated");
    return true;
}

void ioexp_micropython_req_i2c_deinit(void)
{
    __log_info("MicroPython I2C deinit request - no action needed");
}

/* --- LoRa chip controls --------------------------------------------------- */

void ioexp_lora_chip_power_on(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__lora_power && ioexp_drv_handle__lora_reset_n) {
        // Activate pins
        pcal6408a_activate_pin(ioexp_drv_handle__lora_power);
        pcal6408a_activate_pin(ioexp_drv_handle__lora_reset_n);
        
        // Set reset high first, then power on
        pcal6408a_write(ioexp_drv_handle__lora_reset_n, true);
        pcal6408a_write(ioexp_drv_handle__lora_power, true);
        
        __log_info("LoRa chip power ON");
    } else {
        __log_error("Failed to configure LoRa power pin");
    }
}

void ioexp_lora_chip_power_off(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__lora_power) {
        // Set power pin low and deactivate to save power
        pcal6408a_write(ioexp_drv_handle__lora_power, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__lora_power);
        
        // Also turn off reset pin and deactivate to save power
        if (ioexp_drv_handle__lora_reset_n) {
            pcal6408a_write(ioexp_drv_handle__lora_reset_n, false);
            pcal6408a_deactivate_pin(ioexp_drv_handle__lora_reset_n);
        }
        
        __log_info("LoRa chip power OFF");
    } else {
        __log_error("Failed to configure LoRa power pin");
    }
}

bool ioexp_lora_chip_power_status(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return false;
    }
    
    if (!ioexp_drv_handle__lora_power) {
        __log_error("LoRa power pin not configured");
        return false;
    }
    
    bool pin_status = false;
    pcal6408a_error_t err = safe_pin_read(ioexp_drv_handle__lora_power, &pin_status);
    
    if (err != __pcal6408a_ok) {
        __log_error("Failed to read LoRa power pin status: %d", err);
        return false;
    }
    
    return pin_status;
}

void ioexp_lora_chip_reset(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__lora_reset_n) {
        // Toggle reset pin (active low)
        pcal6408a_write(ioexp_drv_handle__lora_reset_n, false);
        vTaskDelay(pdMS_TO_TICKS(10));
        pcal6408a_write(ioexp_drv_handle__lora_reset_n, true);
        
        __log_info("LoRa chip reset");
    }
}

/* --- LTE chip controls ---------------------------------------------------- */

void ioexp_lte_chip_power_on(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__lte_power && ioexp_drv_handle__lte_reset_n) {
        __log_info("LTE power ON: Activating and setting pins");
        // Activate pins
        pcal6408a_activate_pin(ioexp_drv_handle__lte_power);
        pcal6408a_activate_pin(ioexp_drv_handle__lte_reset_n);
        
        // Set reset high first, then power on
        pcal6408a_write(ioexp_drv_handle__lte_reset_n, true);
        pcal6408a_write(ioexp_drv_handle__lte_power, true);
        
        // Update cached state
        s_lte_power_state = true;
        
        __log_info("LTE chip power ON");
    } else {
        __log_error("Failed to configure LTE power pin");
    }
}

void ioexp_lte_chip_power_off(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__lte_power) {
        __log_info("LTE power OFF: Setting pin low and deactivating");
        // Set power pin low and deactivate to save power
        pcal6408a_write(ioexp_drv_handle__lte_power, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__lte_power);
        
        // Update cached state
        s_lte_power_state = false;
        
        // Also turn off reset pin and deactivate to save power
        if (ioexp_drv_handle__lte_reset_n) {
            pcal6408a_write(ioexp_drv_handle__lte_reset_n, false);
            pcal6408a_deactivate_pin(ioexp_drv_handle__lte_reset_n);
        }
        
        __log_info("LTE chip power OFF");
    } else {
        __log_error("Failed to configure LTE power pin");
    }
}

bool ioexp_lte_chip_power_status(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return false;
    }
    
    if (!ioexp_drv_handle__lte_power) {
        __log_error("LTE power pin not configured");
        return false;
    }
    
    // Return cached state instead of reading from hardware
    // since deactivated pins lose their state for power saving
    __log_info("Returning cached LTE power state: %s", s_lte_power_state ? "ON" : "OFF");
    return s_lte_power_state;
}

void ioexp_lte_chip_reset(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__lte_reset_n) {
        // Toggle reset pin (active low)
        pcal6408a_write(ioexp_drv_handle__lte_reset_n, false);
        vTaskDelay(pdMS_TO_TICKS(10));
        pcal6408a_write(ioexp_drv_handle__lte_reset_n, true);
        
        __log_info("LTE chip reset");
    }
}

void ioexp_lte_chip_set_ring_signal_callback(ioexp_callback_t cb)
{
    __log_info("LTE ring callback set (not implemented)");
    // Implementation would involve configuring interrupt on the ring pin
}

/* --- Secure chip controls ------------------------------------------------- */

void ioexp_secure_chip_enable(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__secure_en) {
        pcal6408a_activate_pin(ioexp_drv_handle__secure_en);
        pcal6408a_write(ioexp_drv_handle__secure_en, true);
        __log_info("Secure chip enabled");
    } else {
        __log_error("Failed to configure secure enable pin");
    }
}

void ioexp_secure_chip_disable(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return;
    }
    
    if (ioexp_drv_handle__secure_en) {
        // Set pin low and deactivate to save power
        pcal6408a_write(ioexp_drv_handle__secure_en, false);
        pcal6408a_deactivate_pin(ioexp_drv_handle__secure_en);
        __log_info("Secure chip disabled");
    } else {
        __log_error("Failed to configure secure enable pin");
    }
}

bool ioexp_secure_chip_status(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return false;
    }
    
    if (!ioexp_drv_handle__secure_en) {
        __log_error("Secure element pin not configured");
        return false;
    }
    
    bool pin_status = false;
    pcal6408a_error_t err = safe_pin_read(ioexp_drv_handle__secure_en, &pin_status);
    
    if (err != __pcal6408a_ok) {
        __log_error("Failed to read secure element pin status: %d", err);
        return false;
    }
    
    return pin_status;
}

/* --- LoRa SX1262 interrupt/signal functions ------------------------------- */

bool ioexp_lora_chip_read_int_pin(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return false;
    }
    
    if (!ioexp_drv_handle__lora_int) {
        __log_error("LoRa interrupt pin not configured");
        return false;
    }
    
    bool pin_status = false;
    pcal6408a_error_t err = safe_pin_read(ioexp_drv_handle__lora_int, &pin_status);
    
    if (err != __pcal6408a_ok) {
        __log_error("Failed to read LoRa interrupt pin: %d", err);
        return false;
    }
    
    return pin_status;
}

bool ioexp_lora_chip_is_busy(void)
{
    if (!s_ioexp_initialized) {
        __log_error("IO expander not initialized");
        return false;
    }
    
    if (!ioexp_drv_handle__lora_free) {
        __log_error("LoRa busy pin not configured");
        return false;
    }
    
    bool pin_status = false;
    pcal6408a_error_t err = safe_pin_read(ioexp_drv_handle__lora_free, &pin_status);
    
    if (err != __pcal6408a_ok) {
        __log_error("Failed to read LoRa busy pin: %d", err);
        return false;
    }
    
    // Note: busy pin logic might be inverted - check if this needs to be !pin_status
    return pin_status;
}

// Note: Callback functions now properly implemented for LoRaWAN integration
void ioexp_lora_chip_set_int_signal_callback(ioexp_callback_t cb)
{
    s_lora_int_callback = cb;
    if (cb) {
        __log_info("LoRa interrupt callback registered");
    } else {
        __log_info("LoRa interrupt callback cleared");
    }
}

void ioexp_lora_chip_set_busy_signal_callback(ioexp_callback_t cb)
{
    s_lora_busy_callback = cb;
    if (cb) {
        __log_info("LoRa busy callback registered");
    } else {
        __log_info("LoRa busy callback cleared");
    }
}