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
 * @brief   UART Resource Guard - Implementation
 * --------------------------------------------------------------------------- *
 */

#include "uart_resource_guard.h"
#include <string.h>

// Use SG-SDK structured logging
#define __log_subsystem     F1
#define __log_component     uart_guard
#include "log_lib.h"

// Register the uart_guard component with cyan color
__log_component_def(F1, uart_guard, cyan, 1, 0)

/** -------------------------------------------------------------------------- *
 * Internal State
 * --------------------------------------------------------------------------- *
 */

typedef struct {
    uart_owner_t owner;
    char owner_name[64];
    uint8_t context_id;  // Context for namespace separation
} uart_reservation_t;

static uart_reservation_t s_uart_reservations[UART_GUARD_MAX_PORTS] = {0};

/** -------------------------------------------------------------------------- *
 * Helper Functions
 * --------------------------------------------------------------------------- *
 */

static const char* owner_type_to_string(uart_owner_t owner) {
    switch (owner) {
        case UART_OWNER_NONE:        return "none";
        case UART_OWNER_LTE_ESPMODEM: return "import lte";
        case UART_OWNER_USER:        return "user code";
        default:                     return "unknown";
    }
}

/** -------------------------------------------------------------------------- *
 * Public API Implementation
 * --------------------------------------------------------------------------- *
 */

bool uart_reserve(uint8_t uart_num, uart_owner_t owner, const char* owner_name, uint8_t context_id) {
    if (uart_num >= UART_GUARD_MAX_PORTS) {
        __log_error("Invalid UART number: %d", uart_num);
        return false;
    }

    // Allow re-reservation from the same context (enables reconfiguration)
    if (s_uart_reservations[uart_num].owner != UART_OWNER_NONE) {
        if (s_uart_reservations[uart_num].context_id == context_id) {
            __log_debug("UART%d re-reserved by same context %d (%s)", 
                uart_num, context_id, owner_name ? owner_name : "unknown");
            // Update owner name if provided
            if (owner_name) {
                strncpy(s_uart_reservations[uart_num].owner_name, owner_name, 
                        sizeof(s_uart_reservations[uart_num].owner_name) - 1);
                s_uart_reservations[uart_num].owner_name[sizeof(s_uart_reservations[uart_num].owner_name) - 1] = '\0';
            }
            return true;
        }
        
        __log_warn("UART%d reservation failed: owned by context %d (%s, %s), requested by context %d (%s, %s)",
            uart_num,
            s_uart_reservations[uart_num].context_id,
            s_uart_reservations[uart_num].owner_name,
            owner_type_to_string(s_uart_reservations[uart_num].owner),
            context_id,
            owner_name ? owner_name : "unknown",
            owner_type_to_string(owner));
        return false;
    }

    s_uart_reservations[uart_num].owner = owner;
    s_uart_reservations[uart_num].context_id = context_id;
    if (owner_name) {
        strncpy(s_uart_reservations[uart_num].owner_name, owner_name, 
                sizeof(s_uart_reservations[uart_num].owner_name) - 1);
        s_uart_reservations[uart_num].owner_name[sizeof(s_uart_reservations[uart_num].owner_name) - 1] = '\0';
    } else {
        strncpy(s_uart_reservations[uart_num].owner_name, owner_type_to_string(owner),
                sizeof(s_uart_reservations[uart_num].owner_name) - 1);
    }

    __log_info("UART%d reserved by context %d (%s, %s)", 
        uart_num,
        context_id,
        s_uart_reservations[uart_num].owner_name,
        owner_type_to_string(owner));
    
    return true;
}

bool uart_release(uint8_t uart_num, uart_owner_t owner, uint8_t context_id) {
    if (uart_num >= UART_GUARD_MAX_PORTS) {
        __log_error("Invalid UART number: %d", uart_num);
        return false;
    }

    if (s_uart_reservations[uart_num].owner != owner || 
        s_uart_reservations[uart_num].context_id != context_id) {
        __log_warn("UART%d release failed: owned by context %d (%s, %s), release requested by context %d (%s)",
            uart_num,
            s_uart_reservations[uart_num].context_id,
            s_uart_reservations[uart_num].owner_name,
            owner_type_to_string(s_uart_reservations[uart_num].owner),
            context_id,
            owner_type_to_string(owner));
        return false;
    }

    __log_info("UART%d released by context %d (%s, %s)",
        uart_num,
        context_id,
        s_uart_reservations[uart_num].owner_name,
        owner_type_to_string(owner));

    s_uart_reservations[uart_num].owner = UART_OWNER_NONE;
    s_uart_reservations[uart_num].context_id = 0;
    s_uart_reservations[uart_num].owner_name[0] = '\0';
    
    return true;
}

bool uart_is_reserved(uint8_t uart_num, uart_owner_t* current_owner, const char** owner_name) {
    if (uart_num >= UART_GUARD_MAX_PORTS) {
        return false;
    }

    if (current_owner) {
        *current_owner = s_uart_reservations[uart_num].owner;
    }

    if (owner_name) {
        if (s_uart_reservations[uart_num].owner != UART_OWNER_NONE) {
            *owner_name = s_uart_reservations[uart_num].owner_name;
        } else {
            *owner_name = NULL;
        }
    }

    return s_uart_reservations[uart_num].owner != UART_OWNER_NONE;
}

const char* uart_get_owner_description(uint8_t uart_num) {
    if (uart_num >= UART_GUARD_MAX_PORTS) {
        return NULL;
    }

    if (s_uart_reservations[uart_num].owner == UART_OWNER_NONE) {
        return NULL;
    }

    return s_uart_reservations[uart_num].owner_name;
}

void uart_release_all(void) {
    for (uint8_t i = 0; i < UART_GUARD_MAX_PORTS; i++) {
        if (s_uart_reservations[i].owner != UART_OWNER_NONE) {
            __log_info("Releasing UART%d (was owned by context %d: %s)",
                i, s_uart_reservations[i].context_id, s_uart_reservations[i].owner_name);
            s_uart_reservations[i].owner = UART_OWNER_NONE;
            s_uart_reservations[i].context_id = 0;
            s_uart_reservations[i].owner_name[0] = '\0';
        }
    }
}
