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
 * @brief   UART Resource Guard - Prevents conflicts between LTE and UART usage
 *          
 *          This guard prevents users from accidentally using UART1 for other
 *          purposes when it's reserved for the LTE modem.
 *          
 *          Two ways to use LTE on UART1:
 *          1. LTE.py (legacy) - Uses MicroPython UART(1), registered as UART_OWNER_USER
 *          2. import lte (new) - Uses ESP modem library, registered as UART_OWNER_LTE_ESPMODEM
 *          
 *          The guard automatically prevents conflicts through MicroPython UART hooks
 *          that reserve/release the UART during init/deinit.
 * --------------------------------------------------------------------------- */

#ifndef __UART_RESOURCE_GUARD_H__
#define __UART_RESOURCE_GUARD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * UART Resource Ownership Types
 * --------------------------------------------------------------------------- *
 */

typedef enum {
    UART_OWNER_NONE = 0,        ///< UART is available
    UART_OWNER_LTE_ESPMODEM,    ///< UART reserved by import lte (espmodem)
    UART_OWNER_USER             ///< UART reserved by user code (including LTE.py)
} uart_owner_t;

/** -------------------------------------------------------------------------- *
 * Core API
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief Attempt to reserve a UART for exclusive use
 * 
 * @param uart_num UART port number (0, 1, 2, etc.)
 * @param owner Who is requesting the UART
 * @param owner_name Human-readable name for error messages (e.g., "LTE modem", "user code")
 * @param context_id Context identifier (default 0 for normal user code, use different values for separate namespaces)
 * 
 * @return true if UART was successfully reserved, false if already in use by different context
 * 
 * @note Re-reserving from the same context is allowed (enables reconfiguration)
 */
bool uart_reserve(uint8_t uart_num, uart_owner_t owner, const char* owner_name, uint8_t context_id);

/**
 * @brief Release a UART reservation
 * 
 * @param uart_num UART port number
 * @param owner Who is releasing the UART (must match the current owner)
 * @param context_id Context identifier (must match the reservation context)
 * 
 * @return true if successfully released, false if not owned by this owner/context
 */
bool uart_release(uint8_t uart_num, uart_owner_t owner, uint8_t context_id);

/**
 * @brief Release all UART reservations (for cleanup on soft reset)
 * 
 * Unconditionally releases all UART reservations, regardless of owner or context.
 * Use this for cleanup during soft reset to ensure a clean state.
 */
void uart_release_all(void);

/**
 * @brief Check if a UART is reserved
 * 
 * @param uart_num UART port number
 * @param[out] current_owner Optional pointer to receive current owner
 * @param[out] owner_name Optional pointer to receive owner name
 * 
 * @return true if UART is reserved, false if available
 */
bool uart_is_reserved(uint8_t uart_num, uart_owner_t* current_owner, const char** owner_name);

/**
 * @brief Get human-readable description of current UART owner
 * 
 * @param uart_num UART port number
 * 
 * @return String describing the owner, or NULL if not reserved
 */
const char* uart_get_owner_description(uint8_t uart_num);

/** -------------------------------------------------------------------------- *
 * Platform-Specific Configuration
 * --------------------------------------------------------------------------- *
 */

// Maximum number of UARTs to track (ESP32-S3 has 3 UARTs)
#define UART_GUARD_MAX_PORTS 3

#ifdef __cplusplus
}
#endif

#endif /* __UART_RESOURCE_GUARD_H__ */
