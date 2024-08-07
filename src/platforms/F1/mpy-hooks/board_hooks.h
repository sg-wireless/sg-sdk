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
 * @def __hook_mpy_uart_irq_custom_char_handler
 * 
 * @details This is a macro though which, the received stream on the uart can
 *          be scanned and dedicated actions can be taken.
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_MPY_HOOK_UART_IRQ_CUSTOM_CHAR_HANDLE_ENABLE
extern void hook_mpy_uart_irq_custom_char_handler(char c);
#define __hook_mpy_uart_irq_custom_char_handler(_ch)    \
    hook_mpy_uart_irq_custom_char_handler(_ch)
#endif /* CONFIG_SDK_MPY_HOOK_UART_IRQ_CUSTOM_CHAR_HANDLE_ENABLE */

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

/** -------------------------------------------------------------------------- *
 * Machine virtual timers hooks
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_MPY_HOOK_MACHINE_VIRTUAL_TIMERS_ENABLE

extern mp_obj_t hook_mpy_machine_timer_virtual_new(
    const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *args);

extern void hook_mpy_machine_timer_virtual_print(
    const void *print,
    void* obj);

extern uint32_t hook_mpy_machine_timer_virtual_value(void* obj);

extern void hook_mpy_machine_timer_virtual_deinit(void* obj);

extern mp_obj_t hook_mpy_machine_timer_virtual_init(
    size_t n_args,
    const mp_obj_t *args,
    mp_map_t *kw_args);

#endif /* CONFIG_SDK_MPY_HOOK_MACHINE_VIRTUAL_TIMERS_ENABLE */

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __BOARD_HOOKS_H__ */
