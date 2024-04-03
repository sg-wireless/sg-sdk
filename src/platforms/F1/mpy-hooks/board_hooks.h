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
 * @brief   This file contains platform specific hooks on micropython library.
 * --------------------------------------------------------------------------- *
 */
#ifndef __BOARD_HOOKS_H__
#define __BOARD_HOOKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * @def __hook_mpy_uart_stdout_access_lock
 * @def __hook_mpy_uart_stdout_access_unlock
 * 
 * @details using these hooks, the other firmware component can stall the access
 *          of standard output of micropython to do something else.
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_MPY_HOOK_UART_STDOUT_MUTUAL_ACCESS_ENABLE
extern void hook_mpy_uart_stdout_access_lock(void);
#define __hook_mpy_uart_stdout_access_lock() \
    hook_mpy_uart_stdout_access_lock()

extern void hook_mpy_uart_stdout_access_unlock(void);
#define __hook_mpy_uart_stdout_access_unlock() \
    hook_mpy_uart_stdout_access_unlock()
#endif /* CONFIG_SDK_MPY_HOOK_UART_STDOUT_MUTUAL_ACCESS_ENABLE */

/** -------------------------------------------------------------------------- *
 * @def __hook_mpy_machine_hw_i2c_init
 * 
 * @details A hook over machine_hw_i2c_init() function.
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_IOEXP_ENABLED
#ifdef CONFIG_SDK_MPY_HOOK_MACHINE_I2C_INIT_ENABLE
extern void hook_mpy_machine_hw_i2c_init(int port, int scl, int sda,
    uint32_t freq, uint32_t timeout_ms,
    bool *p_i2c_initialized, bool *p_i2c_config_error);
#define __hook_mpy_machine_hw_i2c_init( \
    _port, _scl, _sda, _freq, _tout, _p_init, _p_err) \
    hook_mpy_machine_hw_i2c_init(_port, _scl, _sda, _freq, _tout, \
    _p_init, _p_err)
#endif /* CONFIG_SDK_MPY_HOOK_MACHINE_I2C_INIT_ENABLE */
#endif /* CONFIG_IOEXP_ENABLED */

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __BOARD_HOOKS_H__ */
