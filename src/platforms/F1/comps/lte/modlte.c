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
 * @brief   MicroPython LTE Module - C Implementation using mp_lite_if
 *          Provides MicroPython bindings for the ESP modem middle layer,
 *          enabling "import lte" functionality with robust C implementation
 *          and CMUX support.
 * --------------------------------------------------------------------------- *
 */

#include "py/runtime.h"
#include "py/obj.h"
#include "py/nlr.h"
#include "py/mperrno.h"

#include <string.h>
#include <stdio.h>

#include "mp_lite_if.h"
#include "espmodem.h"
#include "uart_resource_guard.h"

// Use SG-SDK structured logging system
#define __log_subsystem     lte
#define __log_component     modlte
#include "log_lib.h"

// Module name registration
__mp_mod_name(lte, LTE_ROBUST_C);

// ============================================================================
// Global State for Callbacks
// ============================================================================

// Store the Python callback for events
static mp_obj_t g_event_callback = mp_const_none;
static uint32_t g_event_mask = 0;  // Which events the user subscribed to
static bool g_event_handler_locked = false;  // Lock to prevent handler override

// ============================================================================
// Callback Handlers
// ============================================================================

/**
 * @brief Scheduled callback wrapper for LTE events
 * 
 * This function is called by the MicroPython scheduler in the main task context.
 * It's safe to call Python functions from here.
 * 
 * The arg parameter contains a pointer to malloc'd event data that must be freed.
 * Format: [espmodem_event_t structure][optional string data]
 */
static mp_obj_t scheduled_event_callback(mp_obj_t arg)
{
    // Extract the malloc'd buffer pointer from the argument
    uint8_t* buffer = (uint8_t*)MP_OBJ_TO_PTR(arg);
    
    // Safety check
    if (!buffer) {
        __log_error("Scheduled callback called with NULL buffer!");
        return mp_const_none;
    }
    
    // Extract event structure from buffer
    espmodem_event_t* event = (espmodem_event_t*)buffer;
    
    __log_debug("Scheduled event callback executing in MicroPython task");
    __log_debug("Event type: 0x%04x", event->event_type);
    
    // Handle new unified event callback
    if (g_event_callback != mp_const_none && mp_obj_is_callable(g_event_callback)) {
        __log_debug("Event callback is valid and callable, invoking Python function");
        
        // Build Python dict with event data
        mp_obj_t event_dict = mp_obj_new_dict(0);
        
        // Add event type
        mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_type), 
                         mp_obj_new_int(event->event_type));
        
        // Add event-specific data based on type
        switch (event->event_type) {
            case ESPMODEM_EVENT_URC:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_data),
                                mp_obj_new_str(event->urc.data, event->urc.length));
                break;
                
            case ESPMODEM_EVENT_REGISTRATION_STATUS:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_stat),
                                mp_obj_new_int(event->registration.stat));
                if (event->registration.tac) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_tac),
                                    mp_obj_new_str(event->registration.tac, strlen(event->registration.tac)));
                }
                if (event->registration.ci) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_ci),
                                    mp_obj_new_str(event->registration.ci, strlen(event->registration.ci)));
                }
                if (event->registration.act >= 0) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_act),
                                    mp_obj_new_int(event->registration.act));
                }
                if (event->registration.cause_type >= 0) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_cause_type),
                                    mp_obj_new_int(event->registration.cause_type));
                }
                if (event->registration.reject_cause >= 0) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_reject_cause),
                                    mp_obj_new_int(event->registration.reject_cause));
                }
                if (event->registration.active_time) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_active_time),
                                    mp_obj_new_str(event->registration.active_time, strlen(event->registration.active_time)));
                }
                if (event->registration.periodic_tau) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_periodic_tau),
                                    mp_obj_new_str(event->registration.periodic_tau, strlen(event->registration.periodic_tau)));
                }
                break;
                
            case ESPMODEM_EVENT_PPP_CONNECTED:
            case ESPMODEM_EVENT_PPP_DISCONNECTED:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_connected),
                                mp_obj_new_bool(event->ppp.connected));
                if (event->ppp.ip) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_ip),
                                    mp_obj_new_str(event->ppp.ip, strlen(event->ppp.ip)));
                }
                if (event->ppp.netmask) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_netmask),
                                    mp_obj_new_str(event->ppp.netmask, strlen(event->ppp.netmask)));
                }
                if (event->ppp.gateway) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_gateway),
                                    mp_obj_new_str(event->ppp.gateway, strlen(event->ppp.gateway)));
                }
                if (event->ppp.dns1) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_dns1),
                                    mp_obj_new_str(event->ppp.dns1, strlen(event->ppp.dns1)));
                }
                if (event->ppp.dns2) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_dns2),
                                    mp_obj_new_str(event->ppp.dns2, strlen(event->ppp.dns2)));
                }
                break;
                
            case ESPMODEM_EVENT_MODEM_CRASH:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_break_count),
                                mp_obj_new_int(event->crash.break_count));
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_recovery_started),
                                mp_obj_new_bool(event->crash.recovery_started));
                break;
                
            case ESPMODEM_EVENT_MODEM_RESET:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_user_initiated),
                                mp_obj_new_bool(event->reset.user_initiated));
                if (event->reset.reason) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_reason),
                                    mp_obj_new_str(event->reset.reason, strlen(event->reset.reason)));
                }
                break;
                
            case ESPMODEM_EVENT_SIGNAL_QUALITY:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_rssi),
                                mp_obj_new_int(event->signal.rssi));
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_rssi_dbm),
                                mp_obj_new_int(event->signal.rssi_dbm));
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_ber),
                                mp_obj_new_int(event->signal.ber));
                break;
                
            case ESPMODEM_EVENT_ERROR:
                mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_error_code),
                                mp_obj_new_int(event->error.error_code));
                if (event->error.message) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_message),
                                    mp_obj_new_str(event->error.message, strlen(event->error.message)));
                }
                if (event->error.operation) {
                    mp_obj_dict_store(event_dict, MP_OBJ_NEW_QSTR(MP_QSTR_operation),
                                    mp_obj_new_str(event->error.operation, strlen(event->error.operation)));
                }
                break;
                
            default:
                __log_warn("Unknown event type: 0x%04x", event->event_type);
                break;
        }
        
        // Call the Python callback with the event dict
        mp_call_function_1(g_event_callback, event_dict);
        
        __log_debug("Python event callback invoked successfully");
    }
    
    // Free the entire buffer
    free(buffer);
    
    return mp_const_none;
}

// Define the function object for the scheduler
static MP_DEFINE_CONST_FUN_OBJ_1(scheduled_event_callback_obj, scheduled_event_callback);

/**
 * @brief C callback handler for LTE events from espmodem layer
 * 
 * This function is called from the espmodem layer (potentially from different task contexts)
 * when events occur. It allocates memory, copies the event data, and schedules the Python
 * callback to run safely in the main task.
 * 
 * IMPORTANT: This may run in a different FreeRTOS task, so we CANNOT directly call
 * Python functions or create Python objects. We use malloc() to copy the data and
 * mp_sched_schedule() to run in the main task. The scheduled callback will free().
 * 
 * @param event Event structure containing event type and data
 * @param user_ctx User context (unused, we use global callback storage)
 */
static void event_callback_handler(const espmodem_event_t* event, void* user_ctx)
{
    (void)user_ctx;  // Unused - we use global callback storage
    
    if (!event) {
        __log_error("Event callback called with NULL event!");
        return;
    }
    
    __log_debug("Event handler called from task context: type=0x%04x", event->event_type);
    
    // Check if event is subscribed to
    if ((event->event_type & g_event_mask) == 0) {
        __log_debug("Event not subscribed (mask=0x%04x), ignoring", g_event_mask);
        return;
    }
    
    if (g_event_callback == mp_const_none) {
        __log_debug("No callback registered, ignoring event");
        return;
    }
    
    // Calculate buffer size needed (event struct + any string data)
    size_t buffer_size = sizeof(espmodem_event_t);
    size_t string_data_size = 0;
    
    // Add space for string data that needs to be copied
    switch (event->event_type) {
        case ESPMODEM_EVENT_URC:
            string_data_size = event->urc.length + 1;  // +1 for null terminator
            break;
        case ESPMODEM_EVENT_REGISTRATION_STATUS:
            if (event->registration.tac) string_data_size += strlen(event->registration.tac) + 1;
            if (event->registration.ci) string_data_size += strlen(event->registration.ci) + 1;
            if (event->registration.active_time) string_data_size += strlen(event->registration.active_time) + 1;
            if (event->registration.periodic_tau) string_data_size += strlen(event->registration.periodic_tau) + 1;
            break;
        case ESPMODEM_EVENT_PPP_CONNECTED:
        case ESPMODEM_EVENT_PPP_DISCONNECTED:
            if (event->ppp.ip) string_data_size += strlen(event->ppp.ip) + 1;
            if (event->ppp.netmask) string_data_size += strlen(event->ppp.netmask) + 1;
            if (event->ppp.gateway) string_data_size += strlen(event->ppp.gateway) + 1;
            if (event->ppp.dns1) string_data_size += strlen(event->ppp.dns1) + 1;
            if (event->ppp.dns2) string_data_size += strlen(event->ppp.dns2) + 1;
            break;
        case ESPMODEM_EVENT_MODEM_RESET:
            if (event->reset.reason) string_data_size = strlen(event->reset.reason) + 1;
            break;
        case ESPMODEM_EVENT_ERROR:
            if (event->error.message) string_data_size += strlen(event->error.message) + 1;
            if (event->error.operation) string_data_size += strlen(event->error.operation) + 1;
            break;
        default:
            break;
    }
    
    buffer_size += string_data_size;
    
    // Allocate memory for event structure + string data
    uint8_t* buffer = malloc(buffer_size);
    if (!buffer) {
        __log_error("Out of memory for event buffer (%zu bytes) - dropping event", buffer_size);
        return;
    }
    
    // Copy event structure
    memcpy(buffer, event, sizeof(espmodem_event_t));
    espmodem_event_t* event_copy = (espmodem_event_t*)buffer;
    
    // Copy string data and update pointers
    char* string_buffer = (char*)(buffer + sizeof(espmodem_event_t));
    
    switch (event->event_type) {
        case ESPMODEM_EVENT_URC:
            memcpy(string_buffer, event->urc.data, event->urc.length);
            string_buffer[event->urc.length] = '\0';
            event_copy->urc.data = string_buffer;
            break;
            
        case ESPMODEM_EVENT_REGISTRATION_STATUS:
            if (event->registration.tac) {
                strcpy(string_buffer, event->registration.tac);
                event_copy->registration.tac = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->registration.ci) {
                strcpy(string_buffer, event->registration.ci);
                event_copy->registration.ci = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->registration.active_time) {
                strcpy(string_buffer, event->registration.active_time);
                event_copy->registration.active_time = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->registration.periodic_tau) {
                strcpy(string_buffer, event->registration.periodic_tau);
                event_copy->registration.periodic_tau = string_buffer;
            }
            break;
            
        case ESPMODEM_EVENT_PPP_CONNECTED:
        case ESPMODEM_EVENT_PPP_DISCONNECTED:
            if (event->ppp.ip) {
                strcpy(string_buffer, event->ppp.ip);
                event_copy->ppp.ip = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->ppp.netmask) {
                strcpy(string_buffer, event->ppp.netmask);
                event_copy->ppp.netmask = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->ppp.gateway) {
                strcpy(string_buffer, event->ppp.gateway);
                event_copy->ppp.gateway = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->ppp.dns1) {
                strcpy(string_buffer, event->ppp.dns1);
                event_copy->ppp.dns1 = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->ppp.dns2) {
                strcpy(string_buffer, event->ppp.dns2);
                event_copy->ppp.dns2 = string_buffer;
            }
            break;
            
        case ESPMODEM_EVENT_MODEM_RESET:
            if (event->reset.reason) {
                strcpy(string_buffer, event->reset.reason);
                event_copy->reset.reason = string_buffer;
            }
            break;
            
        case ESPMODEM_EVENT_ERROR:
            if (event->error.message) {
                strcpy(string_buffer, event->error.message);
                event_copy->error.message = string_buffer;
                string_buffer += strlen(string_buffer) + 1;
            }
            if (event->error.operation) {
                strcpy(string_buffer, event->error.operation);
                event_copy->error.operation = string_buffer;
            }
            break;
            
        default:
            break;
    }
    
    __log_debug("Event data copied to malloc'd buffer at %p (%zu bytes)", buffer, buffer_size);
    
    // Verify pointer is word-aligned (LSB must be 0 for MicroPython)
    if (((uint32_t)buffer & 1u) != 0) {
        __log_error("Non-aligned allocation detected - aborting to prevent corruption");
        free(buffer);
        return;
    }
    
    __log_debug("Pointer alignment verified, scheduling callback");
    
    // Schedule the callback
    if (!mp_sched_schedule(MP_OBJ_FROM_PTR(&scheduled_event_callback_obj), MP_OBJ_FROM_PTR(buffer))) {
        __log_warn("Scheduler queue full - dropping event");
        free(buffer);
    } else {
        __log_debug("Event successfully scheduled for processing");
    }
}

// ============================================================================
// Utility Functions
// ============================================================================

static void check_powered(void)
{
    if (!espmodem_is_powered()) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Modem is not powered on!"));
    }
}

static void check_init(void)
{
    if (!espmodem_is_initialized()) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Modem is not initialized!"));
    }
    check_powered();
    
    // Wait for crash recovery to complete if in progress
    // Use 10 second timeout (recovery typically takes 3-5 seconds)
    espmodem_err_t err = espmodem_wait_for_recovery(10000);
    if (err == ESPMODEM_ERR_TIMEOUT) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Timeout waiting for modem crash recovery"));
    } else if (err != ESPMODEM_OK) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to wait for modem recovery"));
    }
}

static void handle_espmodem_error(espmodem_err_t err, const char* operation)
{
    switch (err) {
        case ESPMODEM_OK:
            return; // No error
        case ESPMODEM_ERR_INVALID_ARG:
            // Check if this is a UART reservation failure
            if (strcmp(operation, "init") == 0) {
                // During init, INVALID_ARG could mean:
                // 1. UART is already reserved by another component
                // 2. Failed to change conformance mode (automatic on reinit)
                const char* owner = uart_get_owner_description(1);
                if (owner && strstr(owner, "LTE") == NULL) {
                    // UART reserved by non-LTE component
                    mp_raise_msg_varg(&mp_type_OSError,
                        MP_ERROR_TEXT("Cannot initialize LTE: UART(1) is in use by %s"),
                        owner);
                } else {
                    // Conformance mode change failed (check logs for details)
                    mp_raise_ValueError(MP_ERROR_TEXT("Failed to change conformance mode (see logs for details)"));
                }
            }
            mp_raise_ValueError(MP_ERROR_TEXT("Invalid argument"));
            break;
        case ESPMODEM_ERR_NO_MEM:
            mp_raise_OSError(MP_ENOMEM);
            break;
        case ESPMODEM_ERR_TIMEOUT:
            mp_raise_OSError(MP_ETIMEDOUT);
            break;
        case ESPMODEM_ERR_MODEM_NOT_RESPONDING:
            mp_raise_OSError(MP_ENODEV);
            break;
        case ESPMODEM_ERR_SIM_NOT_READY:
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("SIM card not present or PIN protected"));
            //mp_raise_OSError(MP_EIO);
            break;
        case ESPMODEM_ERR_NETWORK_FAILED:
            mp_raise_OSError(MP_ECONNREFUSED);
            break;
        case ESPMODEM_ERR_PPP_FAILED:
            mp_raise_OSError(MP_ECONNABORTED);
            break;
        case ESPMODEM_ERR_CMUX_FAILED:
            mp_raise_OSError(MP_ECONNRESET);
            break;
        case ESPMODEM_ERR_POWER_FAILED:
            mp_raise_OSError(MP_EIO);
            break;
        case ESPMODEM_ERR_BAUDRATE_FAILED:
            mp_raise_OSError(MP_EIO);
            break;
        default:
            mp_raise_OSError(MP_EIO);
            break;
    }
}

// ============================================================================
// Module Functions (lte.xxx)
// ============================================================================

// lte.init(carrier='standard') - Initialize the LTE module (idempotent - safe to call multiple times)
__mp_mod_fun_kw(lte, init, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_carrier, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    // Get carrier conformance mode
    // Pass "None" string if mp_const_none to preserve the distinction between:
    //   LTE() or LTE(carrier=None) → "None" → keep current unless non-standard
    //   LTE(carrier='standard')    → "standard" → explicitly switch to standard
    const char* carrier = "None";
    if (args[0].u_obj != mp_const_none) {
        carrier = mp_obj_str_get_str(args[0].u_obj);
    }
    
    __log_info("lte.init(carrier='%s')", carrier);
    
    // Always call the C layer init - it handles idempotency correctly
    espmodem_err_t err = espmodem_init(carrier);
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "init");
    }
    
    __log_info("LTE initialization complete");
    return mp_const_none;
}

// lte.attach(apn=None, type='IP', cid=None, band=None, bands=None)
__mp_mod_fun_kw(lte, attach, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    check_init();
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_apn, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_type, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_cid, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_band, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_bands, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    // Get APN
    const char* apn = ""; // Default APN
    if (args[0].u_obj != mp_const_none) {
        apn = mp_obj_str_get_str(args[0].u_obj);
    }
    
    // Get type (IP, IPV4V6)
    const char* type = "IP"; // Default to IP
    if (args[1].u_obj != mp_const_none) {
        type = mp_obj_str_get_str(args[1].u_obj);
    }
    
    // Get CID (Context Identifier)
    int cid = 1; // Default to 1
    if (args[2].u_obj != mp_const_none) {
        cid = mp_obj_get_int(args[2].u_obj);
    }
    
    __log_info("lte.attach(apn='%s', type='%s', cid=%d)", apn, type, cid);
    
    // Get band or bands
    int band = 0; // Auto-select
    static char bands_buf[128] = {0}; // Static to persist beyond function scope
    const char* bands_str = NULL;
    
    // Check if both band and bands are specified (not allowed)
    if (args[3].u_obj != mp_const_none && args[4].u_obj != mp_const_none) {
        mp_raise_ValueError(MP_ERROR_TEXT("Cannot specify both band and bands"));
    }
    
    if (args[3].u_obj != mp_const_none) {
        // Single band
        band = mp_obj_get_int(args[3].u_obj);
    } else if (args[4].u_obj != mp_const_none) {
        // Multiple bands - build comma-separated string
        mp_obj_t bands_list = args[4].u_obj;
        size_t bands_len;
        mp_obj_t *bands_items;
        mp_obj_get_array(bands_list, &bands_len, &bands_items);
        
        // Build bands string (e.g., "1,2,3,")
        memset(bands_buf, 0, sizeof(bands_buf));
        char* p = bands_buf;
        for (size_t i = 0; i < bands_len && (p - bands_buf) < sizeof(bands_buf) - 10; i++) {
            int b = mp_obj_get_int(bands_items[i]);
            p += snprintf(p, sizeof(bands_buf) - (p - bands_buf), "%d,", b);
        }
        bands_str = bands_buf;
    }
    
    espmodem_err_t err = espmodem_attach(apn, type, cid, band, bands_str);
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "attach");
    }
    
    return mp_const_none;
}

// lte.connect(cid=None) - Start data session
__mp_mod_fun_kw(lte, connect, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    check_init();
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cid, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    // Get CID (Context Identifier) if specified
    // Note: The CID should have been configured in attach(). If a different CID is
    // specified here, it would require re-running attach() with the new CID first.
    // LTE.py stores the CID but doesn't use it differently in connect() vs attach().
    // For now, we accept but don't use it - the CID from attach() is already stored.
    if (args[0].u_obj != mp_const_none) {
        int cid = mp_obj_get_int(args[0].u_obj);
        // TODO: If we want to support changing CID in connect(), we would need to
        // call attach() again or add a parameter to espmodem_connect()
        (void)cid; // Suppress unused warning
    }
    
    espmodem_err_t err = espmodem_connect();
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "connect");
    }
    
    return mp_const_none;
}

// lte.disconnect()
__mp_mod_fun_0(lte, disconnect)(void)
{
    check_init();

    espmodem_err_t err = espmodem_disconnect();
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "disconnect");
    }
    return mp_const_none;
}

// lte.reset()
__mp_mod_fun_0(lte, reset)(void)
{
    check_init();
    
    espmodem_err_t err = espmodem_reset();
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "reset");
    }
    return mp_const_none;
}

// lte.isattached()
__mp_mod_fun_0(lte, isattached)(void)
{
    check_init();
    bool attached = espmodem_is_attached();
    __log_info("lte.isattached() -> %s", attached ? "True" : "False");
    return mp_obj_new_bool(attached);
}

// lte.isconnected()
__mp_mod_fun_0(lte, isconnected)(void)
{
    check_init();
    bool connected = espmodem_is_connected();
    __log_info("lte.isconnected() -> %s", connected ? "True" : "False");
    return mp_obj_new_bool(connected);
}

// lte.is_attached() - alias for isattached()
__mp_mod_fun_0(lte, is_attached)(void)
{
    check_init();
    return mp_obj_new_bool(espmodem_is_attached());
}

// lte.is_connected() - alias for isconnected()
__mp_mod_fun_0(lte, is_connected)(void)
{
    check_init();
    return mp_obj_new_bool(espmodem_is_connected());
}

// lte.send_at_cmd(cmd='AT', timeout=-1, wait_ok_error=False, check_error=False, buffer_size=4096)
__mp_mod_fun_kw(lte, send_at_cmd, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    check_init();
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_cmd, MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_QSTR(MP_QSTR_AT)} },
        { MP_QSTR_timeout, MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_wait_ok_error, MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_check_error, MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_buffer_size, MP_ARG_INT, {.u_int = 4096} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    const char* cmd = mp_obj_str_get_str(args[0].u_obj);
    int timeout = args[1].u_int;
    if (timeout < 0) timeout = 5000; // Default 5 second timeout
    
    // Get buffer size from parameter (default 4096, min 1024, max 32768)
    int buffer_size = args[4].u_int;
    if (buffer_size < 1024) buffer_size = 1024;
    if (buffer_size > 32768) buffer_size = 32768;
    
    // Allocate buffer for AT response
    char* response = malloc(buffer_size);
    if (!response) {
        mp_raise_OSError(MP_ENOMEM);
    }
    
    __log_info("AT: %s", cmd);
    
    espmodem_err_t err = espmodem_send_at_command(cmd, response, buffer_size, timeout);
    
    if (err != ESPMODEM_OK) {
        free(response);
        if (args[3].u_bool) { // check_error
            handle_espmodem_error(err, "send_at_cmd");
        }
        return mp_obj_new_str("ERROR", 5);
    }
    
    // Clean up response (remove OK/ERROR lines like LTE.py does)
    char* clean_response = response;
    char* ok_pos = strstr(response, "OK");
    if (ok_pos) {
        *ok_pos = '\0';
    }
    char* error_pos = strstr(response, "ERROR");
    if (error_pos) {
        *error_pos = '\0';
    }
    
    // Create the string object before freeing the buffer
    mp_obj_t result = mp_obj_new_str(clean_response, strlen(clean_response));
    
    if (strlen(clean_response) > 0) {
        __log_info("AT response: %s", clean_response);
    } else {
        __log_info("AT response: OK");
    }
    
    free(response);
    
    return result;
}

// lte.mode(new_mode=None)
__mp_mod_fun_var_between(lte, mode, 0, 1)(size_t n_args, const mp_obj_t *args)
{
    check_init();

    if (n_args == 0) {
        // Get current mode
        espmodem_mode_t mode;
        espmodem_err_t err = espmodem_get_mode(&mode);
        if (err != ESPMODEM_OK) {
            handle_espmodem_error(err, "get_mode");
        }
        __log_info("lte.mode() -> %d (%s)", (int)mode, mode == ESPMODEM_MODE_CATM1 ? "CAT-M1" : "NB-IoT");
        return mp_obj_new_int((int)mode);
    } else {
        // Set new mode
        int new_mode = mp_obj_get_int(args[0]);
        espmodem_mode_t mode = (new_mode == 0) ? ESPMODEM_MODE_CATM1 : ESPMODEM_MODE_NBIOT;
        
        __log_info("lte.mode(%d) - requested: %s", new_mode, mode == ESPMODEM_MODE_CATM1 ? "CAT-M1" : "NB-IoT");
        
        espmodem_err_t err = espmodem_set_mode(mode);
        if (err != ESPMODEM_OK) {
            handle_espmodem_error(err, "set_mode");
        }
        return mp_const_none;
    }
}

// lte.imei()
__mp_mod_fun_0(lte, imei)(void)
{
    check_init();

    char imei[16];
    espmodem_err_t err = espmodem_get_imei(imei, sizeof(imei));
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "get_imei");
    }
    __log_info("lte.imei() -> %s", imei);
    return mp_obj_new_str(imei, strlen(imei));
}

// lte.iccid()
__mp_mod_fun_0(lte, iccid)(void)
{
    check_init();

    char iccid[24];
    espmodem_err_t err = espmodem_get_iccid(iccid, sizeof(iccid));
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "get_iccid");
    }
    __log_info("lte.iccid() -> %s", iccid);
    return mp_obj_new_str(iccid, strlen(iccid));
}

// Additional C-specific functions

// lte.get_signal_strength() - Returns (rssi, rssi_dbm, ber)
__mp_mod_fun_0(lte, get_signal_strength)(void)
{
    check_init();

    int rssi, rssi_dbm, ber;
    espmodem_err_t err = espmodem_get_signal_strength(&rssi, &rssi_dbm, &ber);
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "get_signal_strength");
    }
    
    mp_obj_t tuple[3] = {
        mp_obj_new_int(rssi),
        mp_obj_new_int(rssi_dbm),
        mp_obj_new_int(ber)
    };
    return mp_obj_new_tuple(3, tuple);
}

// lte.get_status() - Returns comprehensive status dict
__mp_mod_fun_0(lte, get_status)(void)
{
    check_init();

    espmodem_status_t status;
    espmodem_err_t err = espmodem_get_status(&status);
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "get_status");
    }
    
    mp_obj_t dict = mp_obj_new_dict(8);
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_powered), mp_obj_new_bool(status.powered));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_sim_ready), mp_obj_new_bool(status.sim_ready));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_network_attached), mp_obj_new_bool(status.network_attached));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_ppp_connected), mp_obj_new_bool(status.ppp_connected));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_cmux_active), mp_obj_new_bool(status.cmux_active));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_baudrate), mp_obj_new_int(status.current_baudrate));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_rssi), mp_obj_new_int(status.signal_rssi));
    mp_obj_dict_store(dict, MP_OBJ_NEW_QSTR(MP_QSTR_ber), mp_obj_new_int(status.signal_ber));
    
    return dict;
}

// lte.deinit(detach=True, power_off=True) - Deinitialize modem with optional detach and power off
__mp_mod_fun_kw(lte, deinit, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    check_init();

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_detach, MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_power_off, MP_ARG_BOOL, {.u_bool = true} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    bool detach = args[0].u_bool;
    bool power_off = args[1].u_bool;
    
    __log_info("lte.deinit(detach=%s, power_off=%s)", detach ? "True" : "False", power_off ? "True" : "False");
    
    // Disconnect if connected
    if (espmodem_is_connected()) {
        __log_info("Disconnecting from network...");
        espmodem_disconnect();
    }
    
    // Detach from network if requested
    if (detach) {
        __log_info("Detaching from network...");
        espmodem_detach();
    } else {
        __log_info("Keeping network attachment (detach=False)");
    }
    
    // Deinitialize (with optional power off)
    // Note: When power_off=False, modem stays powered and can use PSM/eDRX for low power
    // The modem can wake the ESP32 via RING signal for incoming messages
    if (power_off) {
        __log_info("Powering off modem...");
    }

    // Release event handler lock
    if (g_event_handler_locked) {
        __log_info("Releasing event handler lock");
        g_event_handler_locked = false;
    }
    g_event_callback = mp_const_none;
    g_event_mask = 0;

    espmodem_err_t err = espmodem_deinit(power_off);
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "deinit");
    }
    
    __log_info("LTE deinitialization complete");
    return mp_const_none;
}

// lte.power_on(wait_ok=True)
__mp_mod_fun_kw(lte, power_on, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    check_init();

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_wait_ok, MP_ARG_BOOL, {.u_bool = true} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    espmodem_err_t err = espmodem_power_on();
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "power_on");
    }
    
    // If wait_ok is true, wait for modem to be ready
    if (args[0].u_bool) {
        // Give modem time to start up
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Try a simple AT command to verify modem is responding
        char response[64];
        err = espmodem_send_at_command("AT", response, sizeof(response), 5000);
        if (err != ESPMODEM_OK) {
            handle_espmodem_error(err, "power_on");
        }
    }
    
    return mp_const_none;
}

// lte.power_off(force=False)
__mp_mod_fun_kw(lte, power_off, 0)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_force, MP_ARG_BOOL, {.u_bool = false} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    espmodem_err_t err = espmodem_power_off(args[0].u_bool);
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "power_off");
    }
    
    return mp_const_none;
}

// lte.is_powered()
__mp_mod_fun_0(lte, is_powered)(void)
{
    return mp_obj_new_bool(espmodem_is_powered());
}

// lte.detach() - Detach from network but keep modem powered
__mp_mod_fun_0(lte, detach)(void)
{
    check_init();
    
    espmodem_err_t err = espmodem_detach();
    if (err != ESPMODEM_OK) {
        handle_espmodem_error(err, "detach");
    }
    return mp_const_none;
}

// lte.set_event_handler(callback, events=None, lock=False) - Register unified event handler
__mp_mod_fun_kw(lte, set_event_handler, 1)(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    check_init();
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_callback, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_events, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_lock, MP_ARG_BOOL, {.u_bool = false} },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    mp_obj_t callback_obj = args[0].u_obj;
    mp_obj_t events_obj = args[1].u_obj;
    bool lock = args[2].u_bool;
    
    __log_info("Setting unified event handler");
    
    // Unregister if callback is None
    if (callback_obj == mp_const_none) {
        if (g_event_handler_locked) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("event handler is locked, call lte.deinit() first"));
        }
        __log_info("Unregistering event handler");
        g_event_callback = mp_const_none;
        g_event_mask = 0;
        espmodem_register_event_handler(NULL, 0, NULL);
        return mp_const_none;
    }
    
    // Check if handler is locked by another caller
    if (g_event_handler_locked) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("event handler is locked, call lte.deinit() first"));
    }
    
    // Verify it's callable
    if (!mp_obj_is_callable(callback_obj)) {
        __log_error("Provided callback is not callable");
        mp_raise_TypeError(MP_ERROR_TEXT("Callback must be callable"));
    }
    
    // Parse event mask
    uint32_t event_mask = ESPMODEM_EVENT_ALL;  // Default: subscribe to all events
    
    if (events_obj != mp_const_none) {
        if (mp_obj_is_int(events_obj)) {
            // Single event or pre-computed mask
            event_mask = mp_obj_get_int(events_obj);
        } else if (mp_obj_is_type(events_obj, &mp_type_list) || 
                   mp_obj_is_type(events_obj, &mp_type_tuple)) {
            // List or tuple of event types
            size_t len;
            mp_obj_t *items;
            mp_obj_get_array(events_obj, &len, &items);
            
            event_mask = 0;
            for (size_t i = 0; i < len; i++) {
                event_mask |= mp_obj_get_int(items[i]);
            }
        } else {
            mp_raise_TypeError(MP_ERROR_TEXT("events must be int, list, or tuple"));
        }
    }
    
    __log_info("Event mask: 0x%04x", event_mask);
    
    // Store the callback and mask
    g_event_callback = callback_obj;
    g_event_mask = event_mask;
    
    // Register with espmodem layer
    espmodem_err_t err = espmodem_register_event_handler(event_callback_handler, event_mask, NULL);
    if (err != ESPMODEM_OK) {
        __log_error("Failed to register event handler: %d", err);
        g_event_callback = mp_const_none;
        g_event_mask = 0;
        handle_espmodem_error(err, "set_event_handler");
    }
    
    // Apply lock if requested
    if (lock) {
        g_event_handler_locked = true;
        __log_info("Event handler locked - override prevented until lte.deinit()");
    }
    
    __log_info("Event handler registered successfully");
    
    return mp_const_none;
}

// lte.check_sim_present() - Check if SIM card is present and ready
__mp_mod_fun_0(lte, check_sim_present)(void)
{
    check_init();

    // Use consolidated function that retries up to 5 times like LTE.py
    espmodem_err_t err = espmodem_check_sim_present();
    if (err == ESPMODEM_OK) {
        return mp_const_true;
    }
    
    return mp_const_false;
}

// lte.ifconfig() - Get network interface configuration
__mp_mod_fun_0(lte, ifconfig)(void)
{
    check_init();
    
    if (!espmodem_is_connected()) {
        return mp_const_none;
    }
    
    espmodem_network_info_t info;
    espmodem_err_t err = espmodem_get_network_info(&info);
    if (err != ESPMODEM_OK) {
        return mp_const_none;
    }
    
    // Return tuple: (ip, netmask, gateway, dns1, dns2)
    // dns2 may be empty string if not available
    mp_obj_t tuple[5] = {
        mp_obj_new_str(info.ip, strlen(info.ip)),
        mp_obj_new_str(info.netmask, strlen(info.netmask)),
        mp_obj_new_str(info.gateway, strlen(info.gateway)),
        mp_obj_new_str(info.dns1, strlen(info.dns1)),
        mp_obj_new_str(info.dns2, strlen(info.dns2))
    };
    return mp_obj_new_tuple(5, tuple);
}

// Constants - Mode definitions
__mp_mod_const(lte, CATM1, 0);
__mp_mod_const(lte, NBIOT, 1);

// Constants - Event types (must use literal values, not enum names, for mp_lite_if)
__mp_mod_const(lte, EVENT_URC, 0x0001);
__mp_mod_const(lte, EVENT_REGISTRATION_STATUS, 0x0002);
__mp_mod_const(lte, EVENT_PPP_CONNECTED, 0x0008);
__mp_mod_const(lte, EVENT_PPP_DISCONNECTED, 0x0010);
__mp_mod_const(lte, EVENT_MODEM_CRASH, 0x0020);
__mp_mod_const(lte, EVENT_MODEM_RESET, 0x0040);
__mp_mod_const(lte, EVENT_SIGNAL_QUALITY, 0x0080);
__mp_mod_const(lte, EVENT_ERROR, 0x0100);
__mp_mod_const(lte, EVENT_ALL, 0xFFFF);

