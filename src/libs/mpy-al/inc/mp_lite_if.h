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
 * @author  Ahmed Sabry (Pycom, SG Wireless)
 * 
 * @brief   This file implements the uPython lite interface for c-modules
 * --------------------------------------------------------------------------- *
 */
#ifndef __MP_LITE_IF_H__
#define __MP_LITE_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include "utils_misc.h"
#include "py/runtime.h"

#ifdef MAIN_SDK_CONFIG_FILE
#include MAIN_SDK_CONFIG_FILE
#endif

/* --- mp lite interface introduction --------------------------------------- */

/***************************************************************************//**
 * @code{.c}
 *      // ex_mod.c
 * 
 *      //======= [ Defining module initialisation ] ===========================
 *      __mp_mod_init(ex)(void) {
 *          // this is the initialisation function of the module
 *      }
 *      //======= [ Defining All Module Classes ] ==============================
 *      typedef struct _class_x_data {
 *          mp_obj_base_t   base;
 *          int data;
 *      } ClassX_Data_t;
 *      __mp_mod_class_new(ex, ClassX)(
 *          const mp_obj_type_t *type,
 *          size_t n_args,
 *          size_t n_kw,
 *          const mp_obj_t *args)
 *      {
 *          ClassX_Data_t *self = m_new_obj(ClassX_Data_t);
 *          mp_obj_get_int_maybe(args[0], &self->data);
 *          mp_obj_get_int_maybe(args[1], &self->j);
 *          self->base.type = type;
 *          return MP_OBJ_FROM_PTR(self);
 *      }
 *      __mp_mod_class_print(ex, ClassX)(
 *          const mp_print_t *print,
 *          mp_obj_t obj,
 *          mp_print_kind_t kind)
 *      {
 *          ClassX_Data_t* self = MP_OBJ_TO_PTR(obj);
 *          mp_printf(print, "Module Class X data %d", self->data);
 *      }
 *      __mp_mod_class_method_0(ex, ClassX, foo)(mp_obj_t __self)
 *      {
 *          __log_printf("-- Class X , method 'foo'\n");
 *          return mp_const_none;
 *      }
 *      //======= [ Defining All Module functions ] ============================
 *      // this is an example of module development
 *      __mp_mod_fun_0(ex, f0)(void) {
 *          // This is a module method with no args
 *      }
 *      __mp_mod_fun_1(ex, f1)(mp_obj_t __arg_1) {
 *          // This is a module method with one args
 *      }
 *      __mp_mod_fun_2(ex, f2)(mp_obj_t __arg_1, mp_obj_t __arg_2) {
 *          // This is a module method with two args
 *      }
 *      __mp_mod_fun_3(ex, f3)(
 *              mp_obj_t __arg_1, mp_obj_t __arg_2, mp_obj_t  __arg_3) {
 *          // This is a module method with three args
 *      }
 *      __mp_mod_fun_var(ex, fvar1, 1)(size_t __arg_n, mp_obj_t * __arg_v) {
 *          // This is a module method with variable number of args
 *          // minimum number of args is 1
 *      }
 *      __mp_mod_fun_var_between(ex, fvar_2_5, 2, 5)(
 *              size_t __arg_n, mp_obj_t * __arg_v) {
 *          // This is a module method with variable number of args
 *          // minimum number of args is 2
 *          // maximum number of args is 5
 *      }
 * 
 *      //======= [ module registration ] ======================================
 *      __mp_mod_name(ex, Example);
 *      #include __mp_mod_binding_file(ex)
 * 
 * @endcode
 ******************************************************************************/

/* === [ Micropython Lite Interface ] ======================================= */

/* ~~~ [ Module Identification ] ~~~ */
#define __mp_mod_obj(__mod)             ___mp_mod_obj_id(__mod)
#define __mp_declare_mod_obj(__mod)     \
                                extern const mp_obj_module_t __mp_mod_obj(__mod)
#define __mp_mod_name(mod, name)
#define __mp_mod_init(__mod)            __mp_mod_fun_0(__mod,init)

/* ~~~ [ Module compilation headers and preprocessor switches ] ~~~ */
#define __mp_mod_include(mod, header)
#define __mp_mod_ifdef(mod, def)
#define __mp_mod_fun_ifdef(mod, fun, def)
#define __mp_mod_class_ifdef(mod, class, def)
#define __mp_mod_class_method_ifdef(mod, class, fun, def)

/* ~~~ [ Module Methods Definitions ] ~~~ */
#define __mp_mod_fun_0(mod, fun)
#define __mp_mod_fun_1(mod, fun)
#define __mp_mod_fun_2(mod, fun)
#define __mp_mod_fun_3(mod, fun)
#define __mp_mod_fun_var(mod, fun, n_min_args)
#define __mp_mod_fun_var_between(mod, fun, n_min_args, n_max_args)
#define __mp_mod_fun_kw(mod, fun, n_min_args)

/* ~~~ [ Module Constant Definitions ] ~~~ */
#define __mp_mod_const(mod, const, value)

/* ~~~ [ Module Class Definition ] ~~~ */
#define __mp_class_obj(__mod, __class)  ___mp_class_obj_id(__mod, __class)
#define __mp_mod_class_print(mod, class)
#define __mp_mod_class_new(mod, class)
#define __mp_mod_class_const(mod, class, const, value)

#define __mp_mod_class_method_0(mod, class, fun)
#define __mp_mod_class_method_1(mod, class, fun)
#define __mp_mod_class_method_2(mod, class, fun)
#define __mp_mod_class_method_3(mod, class, fun)
#define __mp_mod_class_method_var(mod, class, fun, n_min_args)
#define __mp_mod_class_method_var_between(mod, class, fun, n_min, n_max)
#define __mp_mod_class_method_kw(mod, class, fun, n_min_args)

/* ~~~ [ Module binding include file ] ~~~ */
#define __mp_mod_binding_file(mod)

/* === [ Micropython Lite Interface Implementation ] ======================== */
#define ___mp_mod_fun_id(mod, fun)      __tricat(mod, __, fun)
#define ___mp_mod_fun_obj_id(mod, fun) __concat(___mp_mod_fun_id(mod,fun),_obj)
#define ___mp_mod_class_fun_id(mod, class, fun) \
    ___mp_mod_fun_id(__tricat(mod, _, class), fun)
#define ___mp_mod_class_fun_obj_id(mod, class, fun) \
    ___mp_mod_fun_obj_id(__tricat(mod, _, class), fun)

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Module functions and Class methods implementations
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#ifdef __mp_mod_fun_0
    #undef __mp_mod_fun_0
    #define __mp_mod_fun_0(__mod, __fun) ___mp_mod_method(0, __mod, __fun)
#endif
#ifdef __mp_mod_fun_1
    #undef __mp_mod_fun_1
    #define __mp_mod_fun_1(__mod, __fun) ___mp_mod_method(1, __mod, __fun)
#endif
#ifdef __mp_mod_fun_2
    #undef __mp_mod_fun_2
    #define __mp_mod_fun_2(__mod, __fun) ___mp_mod_method(2, __mod, __fun)
#endif
#ifdef __mp_mod_fun_3
    #undef __mp_mod_fun_3
    #define __mp_mod_fun_3(__mod, __fun) ___mp_mod_method(3, __mod, __fun)
#endif
#ifdef __mp_mod_fun_var
    #undef __mp_mod_fun_var
    #define __mp_mod_fun_var(__mod, __fun, __n_min_args) \
        ___mp_mod_method(VAR, __mod, __fun, __n_min_args, )
#endif
#ifdef __mp_mod_fun_var_between
    #undef __mp_mod_fun_var_between
    #define __mp_mod_fun_var_between(__mod, __fun, n_min_args, n_max_args) \
        ___mp_mod_method(VAR_BETWEEN, __mod, __fun, n_min_args, n_max_args, )
#endif
#ifdef __mp_mod_fun_kw
    #undef __mp_mod_fun_kw
    #define __mp_mod_fun_kw(__mod, __fun, n_min_args) \
        ___mp_mod_method(KW, __mod, __fun, n_min_args, )
#endif

#ifdef __mp_mod_class_method_0
    #undef __mp_mod_class_method_0
    #define __mp_mod_class_method_0(__mod, __class, __fun)          \
        __mp_mod_fun_1(__tricat(__mod,_,__class), __fun)
#endif
#ifdef __mp_mod_class_method_1
    #undef __mp_mod_class_method_1
    #define __mp_mod_class_method_1(__mod, __class, __fun)          \
        __mp_mod_fun_2(__tricat(__mod,_,__class), __fun)
#endif
#ifdef __mp_mod_class_method_2
    #undef __mp_mod_class_method_2
    #define __mp_mod_class_method_2(__mod, __class, __fun)          \
        __mp_mod_fun_3(__tricat(__mod,_,__class), __fun)
#endif
#ifdef __mp_mod_class_method_3
    #undef __mp_mod_class_method_3
    #define __mp_mod_class_method_3(__mod, __class, __fun)          \
        __mp_mod_fun_var_between(__tricat(__mod,_,__class), __fun, 4, 4)
#endif
#ifdef __mp_mod_class_method_var
    #undef __mp_mod_class_method_var
    #define __mp_mod_class_method_var(__mod, __class, __fun,        \
            __min_args)                                             \
        __mp_mod_fun_var(__tricat(__mod,_,__class), __fun,          \
            (__min_args + 1))
#endif
#ifdef __mp_mod_class_method_var_between
    #undef __mp_mod_class_method_var_between
    #define __mp_mod_class_method_var_between(__mod, __class,       \
            __fun, __min_args, __max_args)                          \
        __mp_mod_fun_var_between(__tricat(__mod,_,__class),         \
            __fun, (__min_args+1), (__max_args+1))
#endif
#ifdef __mp_mod_class_method_kw
    #undef __mp_mod_class_method_kw
    #define __mp_mod_class_method_kw(__mod, __class, __fun,         \
            __min_args)                                             \
        __mp_mod_fun_kw(__tricat(__mod,_,__class), __fun,           \
            (__min_args+1))
#endif
#ifdef __mp_mod_class_print
    #undef __mp_mod_class_print
    #define __mp_mod_class_print(__mod, __class)                    \
        void ___mp_mod_class_fun_id(__mod, __class, _print)
#endif
#ifdef __mp_mod_class_new
    #undef __mp_mod_class_new
    #define __mp_mod_class_new(__mod, __class)                      \
        mp_obj_t ___mp_mod_class_fun_id(__mod, __class, _new)
#endif

    #define __args_0__  (void)
    #define __args_1__  (mp_obj_t __arg_1)
    #define __args_2__  (mp_obj_t __arg_1, mp_obj_t __arg_2)
    #define __args_3__  (mp_obj_t __arg_1, mp_obj_t __arg_2,mp_obj_t __arg_3)
    #define __args_VAR__           (size_t __arg_n, const mp_obj_t * __arg_v)
    #define __args_VAR_BETWEEN__   (size_t __arg_n, const mp_obj_t * __arg_v)
    #define __args_KW__ (size_t __arg_n,const mp_obj_t*__arg_v,mp_map_t*__arg_m)
    #define ___mp_mod_method(__arg_x, __mod, __fun, __min_max...)   \
        static mp_obj_t ___mp_mod_fun_id(__mod, __fun)              \
            __tricat(__args_, __arg_x, __);                         \
        __concat(MP_DEFINE_CONST_FUN_OBJ_, __arg_x) (               \
            ___mp_mod_fun_obj_id(__mod, __fun),                     \
            __min_max                                               \
            ___mp_mod_fun_id(__mod, __fun)                          \
            );                                                      \
        static mp_obj_t ___mp_mod_fun_id(__mod, __fun)

    #define __mp_declare_mod_fun_obj(__mod, __fun, __arg_types)     \
        __concat(MP_DECLARE_CONST_FUN_OBJ_,__arg_types)             \
            ( ___mp_mod_fun_obj_id(__mod, __fun) )
    #define __mp_declare_mod_class_meth_obj(                        \
                __mod, _cls, __fun, __arg_types)                    \
        __mp_declare_mod_fun_obj( __tricat(__mod,_,_cls),           \
                __fun, __arg_types)

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Module Table Mcros implementation
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define ___mp_mod_dict_table_id(__mod)      __concat(__mod, _mod_globals_table)
#define ___mp_mod_dict_obj_id(__mod)        __concat(__mod, _mod_dict_obj)
#define ___mp_mod_obj_id(__mod)             __concat(__mod, _mod_obj)
#define ___mp_mod_class_dict_table_id(__mod,__class) \
    ___mp_mod_dict_table_id(__tricat(__mod,_,__class))
#define ___mp_mod_class_dict_obj_id(__mod,__class) \
    ___mp_mod_dict_obj_id(__tricat(__mod,_,__class))
#define ___mp_class_obj_id(__mod, __class)  \
    __concat(__tricat(__mod, _, __class), _class_obj)

#define __mp_mod_class_dict_table(__mod,__class)                            \
    static const mp_rom_map_elem_t                                          \
        ___mp_mod_class_dict_table_id(__mod, __class)[] =
#define __mp_mod_class_dict_table_entry_method(__mod,__class,__fun)         \
    { MP_ROM_QSTR(__concat(MP_QSTR_,__fun)),                                \
      MP_ROM_PTR(&___mp_mod_class_fun_obj_id(__mod,__class,__fun)) }
#define __mp_mod_class_dict_table_entry_const_int(_mod, _class, _name, _val)\
    { MP_ROM_QSTR(__concat(MP_QSTR_,_name)), MP_ROM_INT(_val) }

#define __mp_mod_dict_table(__mod)                                          \
    static const mp_rom_map_elem_t ___mp_mod_dict_table_id(__mod)[] =
#define __mp_dict_table_entry_modinit(__mod)                                \
    { MP_ROM_QSTR(MP_QSTR___init__), MP_ROM_PTR(&__concat(__mod,__init_obj))}
#define __mp_dict_table_entry_name(__mod,__name)                            \
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(__concat(MP_QSTR_,__name)) }
#define __mp_dict_table_entry_method(__mod,__fun)                           \
    { MP_ROM_QSTR(__concat(MP_QSTR_,__fun)),                                \
      MP_ROM_PTR(&___mp_mod_fun_obj_id(__mod,__fun)) }
#define __mp_dict_table_entry_const_int(__mod, __const_name, __const_val)   \
    { MP_ROM_QSTR(__concat(MP_QSTR_,__const_name)), MP_ROM_INT(__const_val) }
#define __mp_dict_table_entry_class(__mod, __class)                         \
    { MP_ROM_QSTR(__concat(MP_QSTR_,__class)),                              \
      MP_ROM_PTR(&___mp_class_obj_id(__mod, __class)) }

#define __mp_mod_bind(__mod)                                                \
    static MP_DEFINE_CONST_DICT(                                            \
        ___mp_mod_dict_obj_id(__mod),                                       \
        ___mp_mod_dict_table_id(__mod));                                    \
    const mp_obj_module_t ___mp_mod_obj_id(__mod) = {                       \
        .base = { &mp_type_module },                                        \
        .globals = (mp_obj_dict_t *)&___mp_mod_dict_obj_id(__mod) }

#define __mp_mod_register(__mod)                                            \
    __mp_mod_bind(__mod);                                                   \
    MP_REGISTER_MODULE(__concat(MP_QSTR_,__mod), ___mp_mod_obj_id(__mod))

#ifdef __mp_mod_binding_file
#undef __mp_mod_binding_file
#define __mp_mod_binding_file(__mod) __stringify(__concat(__mp_mod_, __mod).h)
#endif

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Calling module functions on C level implementation
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#define __mp_args_list(args...) args,
#define __mp_args_list_empty()
#define __mp_kw_args_list(args...) args
#define __mp_kw_arg_pair_int(__kw_qstr, __int)                              \
    MP_OBJ_NEW_QSTR( __kw_qstr ), MP_OBJ_NEW_SMALL_INT( __int )
#define __mp_kw_arg_pair(__kw_qstr, __arg)                                  \
    MP_OBJ_NEW_QSTR( __kw_qstr ), __arg

#define __mp_create_new_obj(__obj, __type_obj,                              \
    n_args, args_list, n_kw_args, kw_args_list)                             \
    do {                                                                    \
        mp_obj_t args[n_args + (n_kw_args << 1)] = {                        \
            args_list kw_args_list                                          \
        };                                                                  \
        __obj = MP_OBJ_TYPE_GET_SLOT(&__type_obj, make_new)                 \
                    (&__type_obj, n_args, n_kw_args, args);                 \
    } while(0)

#define __mp_call_obj_method(__ret, __obj, __method_qstr,                   \
    __n_args, __args_list, __n_kw_args, __kw_args_list)                     \
    do {                                                                    \
        (void)__ret;                                                        \
        mp_obj_t args[ 2 + __n_args + (__n_kw_args << 1) ] = {              \
            NULL, __obj, __args_list  __kw_args_list                        \
        };                                                                  \
        mp_load_method(__obj, __method_qstr, args);                         \
        if( args[0] != NULL ) {                                             \
            __ret = mp_call_method_n_kw(__n_args, __n_kw_args, args);       \
        } else {                                                            \
            __ret = MP_ROM_NONE;                                            \
        }                                                                   \
    } while(0)

/* --- public api ----------------------------------------------------------- */

const char* mp_get_string(mp_obj_t obj);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __MP_LITE_IF_H__ */
