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
 * @brief   This file contains board specific hooks on uart functionalities
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include "utils_misc.h"
#include "mp_lite_if.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "shared/readline/readline.h"
#include "sdkconfig.h"
#include "board_hooks.h"
#include "log_lib.h"

#ifdef CONFIG_SAFEBOOT_FEATURE_ENABLE
#include "boot_if.h"
#endif

/** -------------------------------------------------------------------------- *
 * Hooks implementation
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_MPY_HOOK_UART_IRQ_CUSTOM_CHAR_HANDLE_ENABLE

#if defined(CONFIG_SAFEBOOT_FEATURE_ENABLE) && \
    defined(CONFIG_SAFEBOOT_ENABLE_SOFT_RESET)
    #define __soft_reset__  y
#else
    #define __soft_reset__  n
#endif

static bool s_mpy_ctrl_c_enable = true;
static bool s_mpy_kb_intr_enable = true;
__opt_paste(__soft_reset__, y, static bool s_mpy_ctrl_f_enable = true;)

void hook_mpy_uart_irq_custom_char_handler(char c)
{
    if (c == mp_interrupt_char && s_mpy_kb_intr_enable)
    {
        mp_sched_keyboard_interrupt();
    }
    else if (c == CHAR_CTRL_C && !s_mpy_ctrl_c_enable)
    {
        /* do nothing */
    }
    #if __opt_test(__soft_reset__, y)
    else if (c == CHAR_CTRL_F && s_mpy_ctrl_f_enable)
    {
        bootif_safeboot_soft_reset();
        // -- no code execute after this point, system will be resetting
    }
    #endif /* __soft_reset__ */
    else
    {
        // this is an inline function so will be in IRAM
        ringbuf_put(&stdin_ringbuf, c);
    }
}

static mp_obj_t change_ctrl_key_enable_switch(
    size_t n, const mp_obj_t * arg_v, bool * p_enable_switch);

__mp_mod_ifdef(mpy_hook,CONFIG_SDK_MPY_HOOK_UART_IRQ_CUSTOM_CHAR_HANDLE_ENABLE);

__mp_mod_fun_var_between(mpy_hook, ctrl_c_switch, 0, 1)(
    size_t arg_n, const mp_obj_t * arg_v)
{
    return change_ctrl_key_enable_switch(arg_n, arg_v, &s_mpy_ctrl_c_enable);
}

__mp_mod_fun_var_between(mpy_hook, keyboard_intr_switch, 0, 1)(
    size_t arg_n, const mp_obj_t * arg_v)
{
    return change_ctrl_key_enable_switch(arg_n, arg_v, &s_mpy_kb_intr_enable);
}

#if __opt_test(__soft_reset__, y)
__mp_mod_fun_ifdef(mpy_hook, ctrl_f_switch, CONFIG_SAFEBOOT_FEATURE_ENABLE);
__mp_mod_fun_ifdef(mpy_hook, ctrl_f_switch, CONFIG_SAFEBOOT_ENABLE_SOFT_RESET);
__mp_mod_fun_var_between(mpy_hook, ctrl_f_switch, 0, 1)(
    size_t arg_n, const mp_obj_t * arg_v)
{
    return change_ctrl_key_enable_switch(arg_n, arg_v, &s_mpy_ctrl_f_enable);
}
#endif /* __soft_reset__ */

static mp_obj_t change_ctrl_key_enable_switch(
    size_t n, const mp_obj_t * arg_v, bool * p_enable_switch)
{
    if( n == 1 )
    {
        mp_obj_t obj = arg_v[0];

        if( ! mp_obj_is_bool(obj) )
        {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("none bool value"));
        }

        *p_enable_switch = mp_obj_is_true(obj);

        return mp_const_none;
    }

    return *p_enable_switch ? mp_const_true : mp_const_false;
}

/* --- end of file ---------------------------------------------------------- */
#endif /* CONFIG_SDK_MPY_HOOK_UART_IRQ_CUSTOM_CHAR_HANDLE_ENABLE */
