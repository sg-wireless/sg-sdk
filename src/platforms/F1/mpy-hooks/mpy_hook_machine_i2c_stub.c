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
 * @brief   Stub implementation for MicroPython machine I2C hook
 * @details This provides a minimal implementation of the hook_mpy_machine_hw_i2c_init
 *          function to satisfy the MicroPython machine_i2c.c requirements while
 *          allowing the I2C bridge to handle coordination.
 * --------------------------------------------------------------------------- *
 */

#include <stdint.h>
#include <stdbool.h>

// Forward declaration to avoid full board_hooks.h include with MicroPython types
#ifdef CONFIG_IOEXP_ENABLED
#ifdef CONFIG_SDK_MPY_HOOK_MACHINE_I2C_INIT_ENABLE
extern void hook_mpy_machine_hw_i2c_init(int port, int scl, int sda,
    uint32_t freq, uint32_t timeout_ms,
    bool *p_i2c_initialized, bool *p_i2c_config_error);
#endif
#endif

#ifdef CONFIG_IOEXP_ENABLED
#ifdef CONFIG_SDK_MPY_HOOK_MACHINE_I2C_INIT_ENABLE

/**
 * @brief Stub implementation of I2C initialization hook
 * 
 * This is a minimal stub that allows normal I2C initialization to proceed
 * without any special board-specific coordination. The I2C bridge library
 * now handles any necessary coordination between different I2C users.
 * 
 * @param port I2C port number
 * @param scl SCL pin number  
 * @param sda SDA pin number
 * @param freq I2C frequency in Hz
 * @param timeout_ms Timeout in milliseconds
 * @param p_i2c_initialized Pointer to flag indicating if I2C already initialized
 * @param p_i2c_config_error Pointer to flag indicating configuration error
 */
void hook_mpy_machine_hw_i2c_init(int port, int scl, int sda,
    uint32_t freq, uint32_t timeout_ms,
    bool *p_i2c_initialized, bool *p_i2c_config_error)
{
    // Simple stub implementation - allow normal initialization to proceed
    // The I2C bridge now handles coordination between users
    *p_i2c_initialized = false;  // Let MicroPython proceed with normal init
    *p_i2c_config_error = false; // No configuration errors
}

#endif /* CONFIG_SDK_MPY_HOOK_MACHINE_I2C_INIT_ENABLE */
#endif /* CONFIG_IOEXP_ENABLED */