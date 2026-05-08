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
 * @brief   MicroPython Hook Implementation for UART Resource Guard
 * --------------------------------------------------------------------------- *
 */

#ifdef CONFIG_SDK_MPY_HOOK_MACHINE_UART_INIT_ENABLE

#include <stddef.h>
#include <stdbool.h>
#include "uart_resource_guard.h"

// Forward declarations (avoiding board_hooks.h to prevent MicroPython type dependencies)
void hook_mpy_machine_uart_init(int uart_num, uint8_t context_id,
                                bool *p_uart_reserved, const char **p_owner_name);
void hook_mpy_machine_uart_deinit(int uart_num, uint8_t context_id);

void hook_mpy_machine_uart_init(int uart_num, uint8_t context_id,
                                bool *p_uart_reserved, const char **p_owner_name)
{
    if (p_uart_reserved && p_owner_name) {
        // Choose owner name based on context:
        // context 0 = MicroPython user code
        // context 1 = LTE.py (Python implementation)
        // context 2 = import lte (C implementation)
        const char *reserve_name;
        if (context_id == 1) {
            reserve_name = "LTE modem (LTE.py)";
        } else if (context_id == 2) {
            reserve_name = "LTE modem (import lte)";
        } else {
            reserve_name = "MicroPython user";
        }
        
        // First check if already reserved by another context
        uart_owner_t current_owner = UART_OWNER_NONE;
        const char *current_owner_name = NULL;
        
        if (uart_is_reserved(uart_num, &current_owner, &current_owner_name)) {
            // UART is reserved - try to reserve with our context
            // This will succeed if same context (allows reconfiguration)
            if (uart_reserve(uart_num, UART_OWNER_USER, reserve_name, context_id)) {
                *p_uart_reserved = false;  // Success - same context, can proceed
            } else {
                // Different context - blocked
                *p_uart_reserved = true;
                // Return the actual owner name from the reservation
                *p_owner_name = current_owner_name;
            }
        } else {
            // Not reserved, so reserve it for MicroPython user with this context
            if (uart_reserve(uart_num, UART_OWNER_USER, reserve_name, context_id)) {
                *p_uart_reserved = false;
            } else {
                // Race condition - someone else reserved it between check and reserve
                // Fetch the actual owner name
                uart_is_reserved(uart_num, NULL, p_owner_name);
                *p_uart_reserved = true;
            }
        }
    }
}

void hook_mpy_machine_uart_deinit(int uart_num, uint8_t context_id)
{
    // Release the UART if it was reserved by MicroPython user in this context
    uart_release(uart_num, UART_OWNER_USER, context_id);
}

#endif /* CONFIG_SDK_MPY_HOOK_MACHINE_UART_INIT_ENABLE */

