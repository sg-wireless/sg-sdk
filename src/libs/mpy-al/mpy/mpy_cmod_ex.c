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
 * --------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2022, Pycom Limited.
 *
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * @author  Ahmed Sabry (Pycom)
 * 
 * @brief   This file represents two examples of using the mp-lite interface to
 *          create uPython bindable modules
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include "mp_lite_if.h"
#include "log_lib.h"

#ifdef CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_ENABLE

/** -------------------------------------------------------------------------- *
 * example-1
 * --------------------------------------------------------------------------- *
 */

static const char* valid_str[] = {"not-valid int", "valid int"};

#ifdef CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO
__mp_mod_ifdef(mod_foo, CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_ENABLE)
__mp_mod_ifdef(mod_foo, CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO)
__mp_mod_init(mod_foo)(void) {
    __log_output("-- test module init\n");
    return mp_const_none;
}

__mp_mod_fun_0(mod_foo, help)(void)
{
    __log_output("-- available function are:\n"
        "   mod_foo.f0()       -- method without arguments\n"
        "   mod_foo.f1()       -- method with one argument\n"
        "   mod_foo.f2()       -- method with two arguments\n"
        "   mod_foo.f3()       -- method with three arguments\n"
        "   mod_foo.fvar0()    -- method with var args [min = 0]\n"
        "   mod_foo.fvar1()    -- method with var args [min = 1]\n"
        "   mod_foo.fvar_2_5() -- method with var args [min = 2, max = 5]\n"
        );
    return mp_const_none;
}
__mp_mod_const(mod_foo, CONST_1, 234)
__mp_mod_const(mod_foo, CONST_2, 0)
__mp_mod_const(mod_foo, CONST_3, 10000)
__mp_mod_fun_0(mod_foo, f0)(void)
{
    __log_output("-- test method f0 -- no args\n");
    return mp_const_none;
}

__mp_mod_fun_1(mod_foo, f1)(mp_obj_t __arg_1)
{
    mp_int_t i = 0;
    bool is_valid = mp_obj_get_int_maybe(__arg_1, &i);
    __log_output("-- one arg [%s -- %d]\n",
        valid_str[is_valid], i);
    return mp_const_none;
}

__mp_mod_fun_2(mod_foo, f2)(mp_obj_t __arg_1, mp_obj_t __arg_2)
{
    mp_int_t i = 0 , j = 0;
    bool is_valid_i = mp_obj_get_int_maybe(__arg_1, &i);
    bool is_valid_j = mp_obj_get_int_maybe(__arg_2, &j);
    __log_output("-- two args"
                 " [%s -- %d][%s -- %d]\n",
        valid_str[is_valid_i], i,
        valid_str[is_valid_j], j);
    return mp_const_none;
}

__mp_mod_fun_3(mod_foo, f3)(mp_obj_t __arg_1, mp_obj_t __arg_2,mp_obj_t __arg_3)
{
    mp_int_t i = 0, j = 0, k = 0;
    bool is_valid_i = mp_obj_get_int_maybe(__arg_1, &i);
    bool is_valid_j = mp_obj_get_int_maybe(__arg_2, &j);
    bool is_valid_k = mp_obj_get_int_maybe(__arg_3, &k);
    __log_output(" -- three args"
                 " [%s -- %d][%s -- %d][%s -- %d]\n",
        valid_str[is_valid_i], i,
        valid_str[is_valid_j], j,
        valid_str[is_valid_k], k
        );
    return mp_const_none;
}

__mp_mod_fun_var(mod_foo, fvar0, 0)(size_t __arg_n, const mp_obj_t * __arg_v)
{
    int j = 1;
    while ( __arg_n-- )
    {
        mp_int_t i = 0;
        bool is_valid_i = mp_obj_get_int_maybe(*__arg_v, &i);
        __log_output(" -- var arg[%2d] [%s -- %d]\n", j++,
            valid_str[is_valid_i], i
            );
        __arg_v ++;
    }
    return mp_const_none;
}

__mp_mod_fun_var(mod_foo, fvar1, 1)(size_t __arg_n, const mp_obj_t * __arg_v)
{
    int j = 1;
    while ( __arg_n-- )
    {
        mp_int_t i = 0;
        bool is_valid_i = mp_obj_get_int_maybe(*__arg_v, &i);
        __log_printf(" -- var arg[%2d] [%s -- %d]\n", j++,
            valid_str[is_valid_i], i
            );
        __arg_v ++;
    }
    return mp_const_none;
}

__mp_mod_fun_var_between(mod_foo, fvar_2_5, 2, 5)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    int j = 1;
    while ( __arg_n-- )
    {
        mp_int_t i = 0;
        bool is_valid_i = mp_obj_get_int_maybe(*__arg_v, &i);
        __log_output(" -- var arg[%2d] [%s -- %d]\n", j++,
            valid_str[is_valid_i], i
            );
        __arg_v ++;
    }
    return mp_const_none;
}

#ifdef CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_A
typedef struct _class_a_data {
    mp_obj_base_t   base;
    int i;
    int j;
} ClassA_Data_t;

__mp_mod_class_ifdef(mod_foo, ClassA,
    CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_A)
__mp_mod_class_new(mod_foo, ClassA)(
    const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *args)
{
    ClassA_Data_t *self = m_new_obj(ClassA_Data_t);

    if(n_args >= 2)
    {
        mp_obj_get_int_maybe(args[0], &self->i);
        mp_obj_get_int_maybe(args[1], &self->j);
    }
    else
    {
        self->i = self->j = 0;
    }
    self->base.type = type;

    return MP_OBJ_FROM_PTR(self);
}

__mp_mod_class_print(mod_foo, ClassA)(
    const mp_print_t *print,
    mp_obj_t obj,
    mp_print_kind_t kind)
{
    ClassA_Data_t* self = MP_OBJ_TO_PTR(obj);
    mp_printf(print, "Module Class A (i: %d, j: %d)", self->i, self->j);
}

__mp_mod_class_method_0(mod_foo, ClassA, fooA)(mp_obj_t __arg_1)
{
    __log_output("-- Class A , method 'foo'\n");
    return mp_const_none;
}

__mp_mod_class_method_ifdef(mod_foo, ClassA, barA,
    CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_A_METHOD_BAR_A)
__mp_mod_class_method_var(mod_foo, ClassA, barA, 2)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    __log_output("-- Class A , method 'bar'\n");
    return mp_const_none;
}
#endif /* CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_A */

#ifdef CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_B
typedef struct _class_b_data {
    mp_obj_base_t   base;
    int i;
    int j;
} ClassB_Data_t;

__mp_mod_class_ifdef(mod_foo, ClassB,
    CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_B)
__mp_mod_class_new(mod_foo, ClassB)(
    const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *args)
{
    ClassB_Data_t *self = m_new_obj(ClassB_Data_t);

    if(n_args >= 2)
    {
        mp_obj_get_int_maybe(args[0], &self->i);
        mp_obj_get_int_maybe(args[1], &self->j);
    }
    else
    {
        self->i = self->j = 0;
    }
    self->base.type = type;

    return MP_OBJ_FROM_PTR(self);
}

__mp_mod_class_print(mod_foo, ClassB)(
    const mp_print_t *print,
    mp_obj_t obj,
    mp_print_kind_t kind)
{
    ClassB_Data_t* self = MP_OBJ_TO_PTR(obj);
    mp_printf(print, "Module Class B (i: %d, j: %d)", self->i, self->j);
}

__mp_mod_class_method_0(mod_foo, ClassB, fooB)(mp_obj_t __self)
{
    __log_output("-- Class B , method 'foo'\n");
    return mp_const_none;
}

__mp_mod_class_method_var(mod_foo, ClassB, barB, 2)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    __log_output("-- Class A , method 'bar'\n");
    return mp_const_none;
}

__mp_mod_class_method_var(mod_foo, ClassB, testB, 2)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    __log_output("-- Class A , method 'bar'\n");
    return mp_const_none;
}
#endif /* CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO_CLASS_B */

__mp_mod_name(mod_foo, MOD_FOO);
#endif /* CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_FOO */
/** -------------------------------------------------------------------------- *
 * mod_bar
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_BAR
__mp_mod_ifdef(mod_bar, CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_ENABLE)
__mp_mod_ifdef(mod_bar, CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_BAR)
__mp_mod_fun_var(mod_bar, fvar1, 1)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    int j = 1;
    while ( __arg_n-- )
    {
        mp_int_t i = 0;
        bool is_valid_i = mp_obj_get_int_maybe(*__arg_v, &i);
        __log_output(" -- var arg[%2d] [%s -- %d]\n", j++,
            valid_str[is_valid_i], i
            );
        __arg_v ++;
    }
    return mp_const_none;
}

#ifdef CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_BAR_FUN_FVAR_2_5
__mp_mod_fun_ifdef(mod_bar, fvar_2_5,
    CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_BAR_FUN_FVAR_2_5)
__mp_mod_fun_var_between(mod_bar, fvar_2_5, 2, 5)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    int j = 1;
    while ( __arg_n-- )
    {
        mp_int_t i = 0;
        bool is_valid_i = mp_obj_get_int_maybe(*__arg_v, &i);
        __log_output(" -- var arg[%2d] [%s -- %d]\n", j++,
            valid_str[is_valid_i], i
            );
        __arg_v ++;
    }
    return mp_const_none;
}
#endif /* CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_BAR_FUN_FVAR_2_5 */

typedef struct _class_x_data {
    mp_obj_base_t   base;
    int i;
    int j;
} ClassX_Data_t;

__mp_mod_class_new(mod_bar, ClassX)(
    const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *args)
{
    ClassX_Data_t *self = m_new_obj(ClassX_Data_t);

    if(n_args >= 2)
    {
        mp_obj_get_int_maybe(args[0], &self->i);
        mp_obj_get_int_maybe(args[1], &self->j);
    }
    else
    {
        self->i = self->j = 0;
    }
    self->base.type = type;

    return MP_OBJ_FROM_PTR(self);
}

__mp_mod_class_method_0(mod_bar, ClassX, fooX)(mp_obj_t __self)
{
    __log_output("-- Class X , method 'foo'\n");
    return mp_const_none;
}
__mp_mod_const(mod_bar, START, 34)
__mp_mod_const(mod_bar, COUNT, 90)
__mp_mod_class_method_var(mod_bar, ClassX, barX, 2)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    __log_output("-- Class X , method 'bar'\n");
    return mp_const_none;
}

__mp_mod_name(mod_bar, MOD_BAR);
#endif /* CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_MOD_BAR */
/* --- end of file ---------------------------------------------------------- */
#endif /* CONFIG_SDK_MPY_AL_CMOD_EXAMPLES_ENABLE */
