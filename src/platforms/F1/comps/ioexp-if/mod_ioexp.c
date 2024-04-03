/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   This file represents a uPython c-module interface to the exported
 *          APIs of the IO Expander interface.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "mp_lite_if.h"
#include "ioexp.h"

/* --- module functions definitions ----------------------------------------- */

__mp_mod_name(ioexp, IO-EXP);

__mp_mod_fun_0(ioexp, init) (void) {
    ioexp_init();
    return mp_const_none;
}

__mp_mod_fun_0(ioexp, reset) (void) {
    ioexp_reset();
    return mp_const_none;
}

#ifdef __feature_lora
__mp_mod_fun_ifdef(ioexp, lora_power_on, __feature_lora)
__mp_mod_fun_0(ioexp, lora_power_on) (void) {
    ioexp_lora_chip_power_on();
    return mp_const_none;
}

__mp_mod_fun_ifdef(ioexp, lora_power_off, __feature_lora)
__mp_mod_fun_0(ioexp, lora_power_off) (void) {
    ioexp_lora_chip_power_off();
    return mp_const_none;
}

__mp_mod_fun_ifdef(ioexp, lora_power, __feature_lora)
__mp_mod_fun_var_between(ioexp, lora_power, 0, 1) (
    size_t __arg_n, const mp_obj_t * __arg_v) {

    if( __arg_n == 1 )
    {
        mp_obj_t obj = __arg_v[0];
        if(! mp_obj_is_bool(obj) )
        {
            mp_raise_TypeError(MP_ERROR_TEXT("passing non bool parameter"));
            return mp_const_none;
        }

        if(obj == mp_const_true)
            ioexp_lora_chip_power_on();
        else
            ioexp_lora_chip_power_off();
        return mp_const_none;
    }
    else
    {
        if( ioexp_lora_chip_power_status() == true )
            return mp_const_true;
        else
            return mp_const_false;
    }
}

__mp_mod_fun_ifdef(ioexp, lora_reset, __feature_lora)
__mp_mod_fun_0(ioexp, lora_reset) (void) {
    ioexp_lora_chip_reset();
    return mp_const_none;
}
#endif /* __feature_lora */

#ifdef __feature_lte
__mp_mod_fun_ifdef(ioexp, lte_power_on, __feature_lte)
__mp_mod_fun_0(ioexp, lte_power_on) (void) {
    ioexp_lte_chip_power_on();
    return mp_const_none;
}

__mp_mod_fun_ifdef(ioexp, lte_power_off, __feature_lte)
__mp_mod_fun_0(ioexp, lte_power_off) (void) {
    ioexp_lte_chip_power_off();
    return mp_const_none;
}

__mp_mod_fun_ifdef(ioexp, lte_power, __feature_lte)
__mp_mod_fun_var_between(ioexp, lte_power, 0, 1) (
    size_t __arg_n, const mp_obj_t * __arg_v) {

    if( __arg_n == 1 )
    {
        mp_obj_t obj = __arg_v[0];
        if(! mp_obj_is_bool(obj) )
        {
            mp_raise_TypeError(MP_ERROR_TEXT("passing non bool parameter"));
            return mp_const_none;
        }

        if(obj == mp_const_true)
            ioexp_lte_chip_power_on();
        else
            ioexp_lte_chip_power_off();
        return mp_const_none;
    }
    else
    {
        if( ioexp_lte_chip_power_status() == true )
            return mp_const_true;
        else
            return mp_const_false;
    }
}

__mp_mod_fun_ifdef(ioexp, lte_reset, __feature_lte)
__mp_mod_fun_0(ioexp, lte_reset) (void) {
    ioexp_lte_chip_reset();
    return mp_const_none;
}
#endif /* __feature_lte */

#ifdef __feature_secure_element
__mp_mod_fun_ifdef(ioexp, secure_chip_on, __feature_secure_element)
__mp_mod_fun_0(ioexp, secure_chip_on) (void) {
    ioexp_secure_chip_enable();
    return mp_const_none;
}

__mp_mod_fun_ifdef(ioexp, secure_chip_off, __feature_secure_element)
__mp_mod_fun_0(ioexp, secure_chip_off) (void) {
    ioexp_secure_chip_disable();
    return mp_const_none;
}

__mp_mod_fun_ifdef(ioexp, secure_chip_power, __feature_secure_element)
__mp_mod_fun_var_between(ioexp, secure_chip_power, 0, 1) (
    size_t __arg_n, const mp_obj_t * __arg_v) {

    if( __arg_n == 1 )
    {
        mp_obj_t obj = __arg_v[0];
        if(! mp_obj_is_bool(obj) )
        {
            mp_raise_TypeError(MP_ERROR_TEXT("passing non bool parameter"));
            return mp_const_none;
        }

        if(obj == mp_const_true)
            ioexp_secure_chip_enable();
        else
            ioexp_secure_chip_disable();
        return mp_const_none;
    }
    else
    {
        if( ioexp_secure_chip_status() == true )
            return mp_const_true;
        else
            return mp_const_false;
    }
}
#endif /* __feature_secure_element */

__mp_mod_fun_1(ioexp, open_drain) (mp_obj_t obj) {

    if(mp_obj_is_bool(obj))
    {
        void pcal6408a_config_open_drain_output(bool);
        pcal6408a_config_open_drain_output( mp_obj_is_true(obj) );
    }
    else
    {
        mp_raise_TypeError(MP_ERROR_TEXT("expected bool argument"));
    }
    return mp_const_none;
}

__mp_mod_fun_0(ioexp, stats) (void) {
    ioexp_stats();
    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
