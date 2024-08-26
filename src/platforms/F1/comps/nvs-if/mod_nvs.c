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
 *          APIs of NVS.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <string.h>
#include <stdbool.h>
#include "mp_lite_if.h"
#include "py/objstr.h"

#include "log_lib.h"
#include "nvs_if.h"

/* --- module functions definitions ----------------------------------------- */

__mp_mod_name(nvs_if, NvS_IF);

__mp_mod_fun_kw(nvs_if, stat, 0)(
        size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_dump_blobs, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false}},
        { MP_QSTR_partition, MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj=mp_const_none}},
        { MP_QSTR_namespace, MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj=mp_const_none}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_dump_blobs_bool     args[0].u_bool
    #define __arg_partition_obj       args[1].u_obj
    #define __arg_namespace_obj       args[2].u_obj

    const char* partition = NULL;
    const char* namespace = NULL;

    if(__arg_partition_obj != mp_const_none)
    {
        if( !mp_obj_is_str(__arg_partition_obj) )
        {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("partition must be string object"));
        }
        partition = mp_get_string(__arg_partition_obj);
    }
    if(__arg_namespace_obj != mp_const_none)
    {
        if( !mp_obj_is_str(__arg_namespace_obj) )
        {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("namespace must be string object"));
        }
        namespace = mp_get_string(__arg_namespace_obj);
    }

    nvs_if_stat(__arg_dump_blobs_bool, partition, namespace);

    return mp_const_none;

    #undef __arg_dump_blobs_bool
    #undef __arg_partition_obj
    #undef __arg_namespace_obj
}

__mp_mod_fun_kw(nvs_if, exists, 0)(
        size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_partition, MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj=mp_const_none}},
        { MP_QSTR_namespace, MP_ARG_KW_ONLY|MP_ARG_REQUIRED|MP_ARG_OBJ,
            {.u_obj=mp_const_none}},
        { MP_QSTR_key, MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj=mp_const_none}},
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_partition_obj         args[0].u_obj
    #define __arg_namespace_obj         args[1].u_obj
    #define __arg_key_obj               args[2].u_obj

    const char* partition = NULL;
    const char* namespace = NULL;
    const char* key = NULL;

    if(__arg_partition_obj != mp_const_none)
    {
        if( !mp_obj_is_str(__arg_partition_obj) )
        {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("partition must be string object"));
        }
        partition = mp_get_string(__arg_partition_obj);
    }

    if(__arg_namespace_obj != mp_const_none)
    {
        if( !mp_obj_is_str(__arg_namespace_obj) )
        {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("namespace must be string object"));
        }
        namespace = mp_get_string(__arg_namespace_obj);
    }

    if(__arg_key_obj != mp_const_none)
    {
        if( !mp_obj_is_str(__arg_namespace_obj) )
        {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("key must be string object"));
        }
        key = mp_get_string(__arg_key_obj);
    }

    return nvs_if_exists(partition, namespace, key)
        ? mp_const_true
        : mp_const_false;

    #undef __arg_partition_obj
    #undef __arg_namespace_obj
    #undef __arg_key_obj
}

__mp_mod_fun_var_between(nvs_if, set, 4, 4)
    (size_t __arg_n, const mp_obj_t * __arg_v)
{
    if( ! mp_obj_is_str(__arg_v[0]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("first arg must be valid nvs partition name"));
    }

    if( ! mp_obj_is_str(__arg_v[1]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("second arg must be valid nvs namespace name"));
    }

    if( ! mp_obj_is_str(__arg_v[2]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("third arg must be valid nvs key name"));
    }

    const char* part = mp_get_string(__arg_v[0]);
    const char* ns = mp_get_string(__arg_v[1]);
    const char* key = mp_get_string(__arg_v[2]);

    const mp_obj_type_t * o_type = mp_obj_get_type(__arg_v[3]);

    bool ret = false;

    if(o_type == &mp_type_str)
    {
        const char* str = mp_get_string(__arg_v[3]);
        ret = nvs_if_set(part, ns, key, str, strlen(str) + 1);
    }
    else if(o_type == &mp_type_int)
    {
        mp_int_t i_val = mp_obj_get_int(__arg_v[3]);
        ret = nvs_if_set(part, ns, key, &i_val, sizeof(mp_int_t));
    }
    else if(o_type == &mp_type_bytes)
    {
        mp_buffer_info_t buf;
        mp_get_buffer_raise(__arg_v[3], &buf, MP_BUFFER_READ);
        ret = nvs_if_set(part, ns, key, buf.buf, buf.len);
    }

    return ret ? mp_const_true : mp_const_false;
}

__mp_mod_fun_var_between(nvs_if, get, 3, 3)
    (size_t __arg_n, const mp_obj_t * __arg_v)
{
    if( ! mp_obj_is_str(__arg_v[0]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("first arg must be valid nvs partition name"));
    }

    if( ! mp_obj_is_str(__arg_v[1]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("second arg must be valid nvs namespace name"));
    }

    if( ! mp_obj_is_str(__arg_v[2]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("third arg must be valid nvs key name"));
    }

    const char* part = mp_get_string(__arg_v[0]);
    const char* ns = mp_get_string(__arg_v[1]);
    const char* key = mp_get_string(__arg_v[2]);

    nvs_if_value_type_t v_type;
    uint32_t v_len;
    bool ret = nvs_if_get_value_type(part, ns, key, &v_type, &v_len);

    if(ret)
    {
        if(v_type == __NVS_VALUE_STRING) {
            char* buf = malloc(v_len + 1);
            nvs_if_get_value(part, ns, key, v_type, buf, v_len);
            mp_obj_t o = mp_obj_new_str(buf, strlen(buf));
            free(buf);
            return o;
        } else if (v_type == __NVS_VALUE_BLOB) {
            uint8_t * buf = malloc(v_len);
            nvs_if_get_value(part, ns, key, v_type, buf, v_len);
            mp_obj_t o = mp_obj_new_bytearray(v_len, buf);
            free(buf);
            return o;
        } else {
            if( v_type == __NVS_VALUE_INT8 ||
                v_type == __NVS_VALUE_INT16 ||
                v_type == __NVS_VALUE_INT32)
            {
                mp_int_t val = 0;
                nvs_if_get_value(part, ns, key, v_type, &val, v_len);
                return mp_obj_new_int(val);
            }

            if( v_type == __NVS_VALUE_UINT8 ||
                v_type == __NVS_VALUE_UINT16 ||
                v_type == __NVS_VALUE_UINT32)
            {
                mp_uint_t val = 0;
                nvs_if_get_value(part, ns, key, v_type, &val, v_len);
                return mp_obj_new_int_from_uint(val);
            }
        }
    }

    return mp_const_none;
}

__mp_mod_fun_var_between(nvs_if, add, 4, 4)
    (size_t __arg_n, const mp_obj_t * __arg_v)
{
    if( ! mp_obj_is_str(__arg_v[0]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("first arg must be valid nvs partition name"));
    }

    if( ! mp_obj_is_str(__arg_v[1]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("second arg must be valid nvs namespace name"));
    }

    if( ! mp_obj_is_str(__arg_v[2]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("third arg must be valid nvs key name"));
    }

    const char* part = mp_get_string(__arg_v[0]);
    const char* ns = mp_get_string(__arg_v[1]);
    const char* key = mp_get_string(__arg_v[2]);

    const mp_obj_type_t * o_type = mp_obj_get_type(__arg_v[3]);

    bool ret = false;

    if(o_type == &mp_type_str)
    {
        const char* str = mp_get_string(__arg_v[3]);
        ret = nvs_if_add(part, ns, key, __NVS_VALUE_STRING, str, strlen(str)+1);
    }
    else if(o_type == &mp_type_int)
    {
        mp_int_t i_val = mp_obj_get_int(__arg_v[3]);
        ret = nvs_if_add(part, ns, key, __NVS_VALUE_INT32,
            &i_val, sizeof(mp_int_t));
    }
    else if(o_type == &mp_type_bytes)
    {
        mp_buffer_info_t buf;
        mp_get_buffer_raise(__arg_v[3], &buf, MP_BUFFER_READ);
        ret = nvs_if_add(part, ns, key, __NVS_VALUE_BLOB, buf.buf, buf.len);
    }

    return ret ? mp_const_true : mp_const_false;
}

__mp_mod_fun_var_between(nvs_if, delete, 3, 3)
    (size_t __arg_n, const mp_obj_t * __arg_v)
{
    if( ! mp_obj_is_str(__arg_v[0]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("first arg must be valid nvs partition name"));
    }

    if( ! mp_obj_is_str(__arg_v[1]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("second arg must be valid nvs namespace name"));
    }

    if( ! mp_obj_is_str(__arg_v[2]) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("third arg must be valid nvs key name"));
    }

    const char* part = mp_get_string(__arg_v[0]);
    const char* ns = mp_get_string(__arg_v[1]);
    const char* key = mp_get_string(__arg_v[2]);

    bool ret = nvs_if_del(part, ns, key);

    return ret ? mp_const_true : mp_const_false;
}

/* --- end of file ---------------------------------------------------------- */
