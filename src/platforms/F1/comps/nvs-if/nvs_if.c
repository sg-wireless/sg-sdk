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
 * @brief   nvs interface implementation to the ESP nvs library
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include "esp_flash.h"
#include "spiram.h"
#include "esp_heap_caps.h"
#include "esp_partition.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "log_lib.h"
#include "utils_units.h"
#include "nvs_if.h"

#include "esp_log.h"

static const char *TAG = "nvs_if";

/* --- macros --------------------------------------------------------------- */

#define __total_w   100

/* --- APIs ----------------------------------------------------------------- */

static const char* s_default_nvs_part = "nvs";

static const char* get_nvs_type_name(uint32_t type)
{
    switch(type) {
        case NVS_TYPE_U8:       return __yellow__"U8"__default__;
        case NVS_TYPE_I8:       return __yellow__"I8"__default__;
        case NVS_TYPE_U16:      return __yellow__"U16"__default__;
        case NVS_TYPE_I16:      return __yellow__"I16"__default__;
        case NVS_TYPE_U32:      return __yellow__"U32"__default__;
        case NVS_TYPE_I32:      return __yellow__"I32"__default__;
        case NVS_TYPE_U64:      return __yellow__"U64"__default__;
        case NVS_TYPE_I64:      return __yellow__"I64"__default__;
        case NVS_TYPE_STR:      return __green__"STR"__default__;
        case NVS_TYPE_BLOB:     return __purple__"BLOB"__default__;
        case NVS_TYPE_ANY:      return "ANY";
    }
    return __red__"UNKNOWN"__default__;
}
#define __esp_call(__api, args...)                                  \
    do {                                                            \
        esp_err_t e = __api(args);                                  \
        if(e != ESP_OK) {                                           \
            ESP_ERROR_CHECK_WITHOUT_ABORT(e);                       \
            __log_output(__red__"err "#__api": %d"__default__, e);  \
        }                                                           \
    } while(0)

static void init_nvs_partitions(void)
{
    //esp_log_level_set(TAG, ESP_LOG_VERBOSE);
    static bool initialized = false;

    if(initialized)
    {
        ESP_LOGD(TAG, "Already initialized!");
        return;
    }

#ifdef CONFIG_NVS_ENCRYPTION
    const esp_partition_t *key_part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS,
        "nvs_keys");
    nvs_sec_cfg_t sec_cfg;
    __esp_call(nvs_flash_read_security_cfg, key_part, &sec_cfg);
    ESP_LOGD(TAG, "Dumping sec_cfg.eky");
    ESP_LOG_BUFFER_HEXDUMP(TAG, sec_cfg.eky, NVS_KEY_SIZE, ESP_LOG_DEBUG);
    ESP_LOGD(TAG, "Dumping sec_cfg.tky");
    ESP_LOG_BUFFER_HEXDUMP(TAG, sec_cfg.tky, NVS_KEY_SIZE, ESP_LOG_DEBUG);
#endif    

    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    while(it)
    {
        const esp_partition_t* part = esp_partition_get(it);

#ifdef CONFIG_NVS_ENCRYPTION
        __esp_call(nvs_flash_secure_init_partition, part->label, &sec_cfg);
#else
        __esp_call(nvs_flash_init_partition_ptr, part);
#endif
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    initialized = true;
}

static bool is_valid_nvs_partition(const char* part_name)
{
    init_nvs_partitions();

    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);

    while(it)
    {
        const esp_partition_t* part = esp_partition_get(it);
        if(strcmp(part->label, part_name) == 0)
        {
            esp_partition_iterator_release(it);
            return true;
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);
    return false;
}

#define __w_key         17
#define __w_type         8
#define __w_namespace   12
#define __w_val         63  // 100 - 17 - 8 - 12

static void nvs_if_stat_partition(
    const char* part_name,
    const char* namespace,
    bool disp_blob_data)
{
    nvs_iterator_t it;
    it = nvs_entry_find(part_name, NULL, NVS_TYPE_ANY);

    __log_output("\n---( "__yellow__"%s"__default__, part_name);
    __log_output_field(" )",
        __total_w - strlen(part_name) - 5, '-', __left__, true);

    nvs_entry_info_t info;
    nvs_handle_t handle;

    while (it != NULL)
    {
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);

        if(namespace)
        {
            if(strcmp(namespace, info.namespace_name) != 0)
            {
                continue;
            }
        }

        __log_col_str_val_l(namespace, info.namespace_name);
        __log_col_str_val_color_l(key, __cyan__, info.key);
        __log_col_str_val_l(type, get_nvs_type_name(info.type));

        __esp_call(nvs_open_from_partition,
            part_name, info.namespace_name, NVS_READONLY, &handle);

        if(info.type == NVS_TYPE_STR)
        {
            size_t len = 0;
            __esp_call(nvs_get_str, handle, info.key, NULL, &len);
            if(len) {
                char* str = malloc(len + 1);
                __esp_call(nvs_get_str, handle, info.key, str, &len);
                char* p = str;
                len = strlen(str);
                while(len > 0) {
                    int allowed_len = len > __w_val ? __w_val : len;
                    __log_col_str_val_l(val, p);
                    p += allowed_len;
                    len -= allowed_len;
                    if(len) {
                        __log_output("\n");
                        __log_col_str_val_l(namespace, "");
                        __log_col_str_val_l(key, "");
                        __log_col_str_val_l(type, "");
                    }
                }
                free(str);
            }
        }
        else if(info.type == NVS_TYPE_BLOB)
        {
            size_t len = 0;
            __esp_call(nvs_get_blob, handle, info.key, NULL, &len);
            __log_output("size: %d", len);
            if(len && disp_blob_data)
            {
                uint8_t* data = malloc(len);
                __esp_call(nvs_get_blob, handle, info.key, data, &len);
                uint8_t* p = data;
                int capacity = __w_val / 3;
                while(len > 0) {
                    __log_output("\n");
                    __log_col_str_val_l(namespace, "");
                    __log_col_str_val_l(key, "");
                    __log_col_str_val_l(type, "");
                    int allowed_len = len > capacity ? capacity : len;
                    __log_output_hex_lower(p, allowed_len);
                    p += allowed_len;
                    len -= allowed_len;
                }
                free(data);
            }
        }
        else if(info.type == NVS_TYPE_U8) {
            uint8_t val;
            nvs_get_u8(handle, info.key, &val);
            __log_col_int_val(val, 0, val);
        }
        else if(info.type == NVS_TYPE_I8) {
            int8_t val;
            nvs_get_i8(handle, info.key, &val);
            __log_col_int_val(val, 0, val);
        }
        else if(info.type == NVS_TYPE_U16) {
            uint16_t val;
            nvs_get_u16(handle, info.key, &val);
            __log_col_int_val(val, 0, val);
        }
        else if(info.type == NVS_TYPE_I16) {
            int16_t val;
            nvs_get_i16(handle, info.key, &val);
            __log_col_int_val(val, 0, val);
        }
        else if(info.type == NVS_TYPE_U32) {
            uint32_t val;
            nvs_get_u32(handle, info.key, &val);
            __log_col_int_val(val, 0, val);
        }
        else if(info.type == NVS_TYPE_I32) {
            int32_t val;
            nvs_get_i32(handle, info.key, &val);
            __log_col_int_val(val, 0, val);
        }

        nvs_close(handle);

        __log_output("\n");
    }

    nvs_release_iterator(it);
}

void nvs_if_stat(
    bool disp_blob_data,
    const char* part_name,
    const char* namespace)
{
    init_nvs_partitions();
    bool started = false;

    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);
    while(it)
    {
        if(!started)
        {
            __log_output_fill(__total_w, '=', true);

            __log_col_header_l(namespace);
            __log_col_header_l(key);
            __log_col_header_l(type);
            __log_col_header_l(val);
            __log_output("\n");
            __log_output_fill(__total_w, '=', true);
            started = true;
        }
        const esp_partition_t* part = esp_partition_get(it);
        if(part_name)
        {
            if(strcmp(part_name, part->label) == 0)
            {
                nvs_if_stat_partition(part->label, namespace, disp_blob_data);
                break;
            }
        }
        else
        {
            nvs_if_stat_partition(part->label, namespace, disp_blob_data);
        }
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    if(started)
    {
        __log_output_fill(__total_w, '=', true);
    }
}

bool nvs_if_exists(
    const char* part_name,
    const char* namespace,
    const char* key)
{
    if(part_name == NULL)
    {
        part_name = s_default_nvs_part;
    }
    if(!is_valid_nvs_partition(part_name))
    {
        __log_output(__red__"invalid nvs partition '"
            __yellow__"%s"__red__"'"__default__"\n", part_name);
        return false;
    }
    nvs_entry_info_t info;
    nvs_iterator_t it = nvs_entry_find(part_name, namespace, NVS_TYPE_ANY);

    bool found = false;

    while(it)
    {
        nvs_entry_info(it, &info);
        if( strcmp(namespace, info.namespace_name) == 0 )
        {
            if(key == NULL)
            {
                found = true;
                break;
            }
            else if( strcmp(key, info.key) == 0 )
            {
                found = true;
                break;
            }
        }
        it = nvs_entry_next(it);
    }

    nvs_release_iterator(it);
    return found;
}

static nvs_iterator_t get_nvs_entry(
    const char* part, const char* namespace, const char* key,
    nvs_entry_info_t *p_info)
{
    nvs_iterator_t it = nvs_entry_find(part, namespace, NVS_TYPE_ANY);

    while(it)
    {
        nvs_entry_info(it, p_info);
        if(strcmp(p_info->key, key) == 0)
        {
            break;
        }
        it = nvs_entry_next(it);
    }
    return it;
}

static bool nvs_if_set_impl(
    const char* part,
    const char* namespace,
    const char* key,
    const void* buf,
    size_t len,
    bool  is_new,
    nvs_if_value_type_t type)
{
    if(part == NULL)
    {
        part = s_default_nvs_part;
    }
    if(!is_valid_nvs_partition(part))
    {
        __log_output(__red__"invalid nvs partition '"
            __yellow__"%s"__red__"'"__default__"\n", part);
        return false;
    }

    nvs_entry_info_t info;
    nvs_iterator_t it = get_nvs_entry(part, namespace, key, &info);
    bool set_ok = false;

    if(it == NULL)
    {
        if(is_new)
        {
            set_ok = true;
        }
    }
    else
    {
        if(!is_new)
        {
            set_ok = true;
            switch(info.type) {
            case NVS_TYPE_U8:   type = __NVS_VALUE_UINT8;   break;
            case NVS_TYPE_I8:   type = __NVS_VALUE_INT8;    break;
            case NVS_TYPE_U16:  type = __NVS_VALUE_UINT16;  break;
            case NVS_TYPE_I16:  type = __NVS_VALUE_INT16;   break;
            case NVS_TYPE_U32:  type = __NVS_VALUE_UINT32;  break;
            case NVS_TYPE_I32:  type = __NVS_VALUE_INT32;   break;
            case NVS_TYPE_STR:  type = __NVS_VALUE_STRING;  break;
            case NVS_TYPE_BLOB: type = __NVS_VALUE_BLOB;    break;
            default:
                set_ok = false;
            }
            
        }
    }

    if(set_ok)
    {
        nvs_handle_t handle;
        esp_err_t e = nvs_open_from_partition(
            part, namespace, NVS_READWRITE, &handle);

        if( e == ESP_OK )
        {
            if(type == __NVS_VALUE_STRING)
            {
                e = nvs_set_str(handle, key, (const char *)buf);
            }
            else if(type == __NVS_VALUE_BLOB)
            {
                e = nvs_set_blob(handle, key, buf, len);
            }
            else if(type == __NVS_VALUE_UINT8) {
                e = nvs_set_u8(handle, key, *(uint8_t*)buf);
            }
            else if(type == __NVS_VALUE_INT8) {
                e = nvs_set_i8(handle, key, *(int8_t*)buf);
            }
            else if(type == __NVS_VALUE_UINT16) {
                e = nvs_set_u16(handle, key, *(uint16_t*)buf);
            }
            else if(type == __NVS_VALUE_INT16) {
                e = nvs_set_i16(handle, key, *(int16_t*)buf);
            }
            else if(type == __NVS_VALUE_UINT32) {
                e = nvs_set_u32(handle, key, *(uint32_t*)buf);
            }
            else if(type == __NVS_VALUE_INT32) {
                e = nvs_set_i32(handle, key, *(int32_t*)buf);
            }

            if(e == ESP_OK)
            {
                e = nvs_commit(handle);
                if(e == ESP_OK)
                {
                    nvs_close(handle);
                    return true;
                }
            }
        }

        nvs_close(handle);
    }

    return false;
}

bool nvs_if_set(
    const char* part,
    const char* namespace,
    const char* key,
    const void* buf,
    size_t len)
{
    return nvs_if_set_impl(part, namespace, key, buf, len, false, 0);
}

bool nvs_if_get_value_type(
    const char* part,
    const char* namespace,
    const char* key,
    nvs_if_value_type_t* p_type,
    uint32_t *p_len)
{
    if(part == NULL)
    {
        part = s_default_nvs_part;
    }
    if(!is_valid_nvs_partition(part))
    {
        __log_output(__red__"invalid nvs partition '"
            __yellow__"%s"__red__"'"__default__"\n", part);
        return false;
    }

    if(!(p_type && p_len))
    {
        return false;
    }

    nvs_entry_info_t info;
    nvs_iterator_t it = get_nvs_entry(part, namespace, key, &info);

    if(it)
    {
        nvs_handle_t handle;
        esp_err_t e = nvs_open_from_partition(
            part, namespace, NVS_READWRITE, &handle);

        if( e == ESP_OK )
        {
            if(info.type == NVS_TYPE_STR)
            {
                *p_type = __NVS_VALUE_STRING;
                size_t len;
                e = nvs_get_str(handle, info.key, NULL, &len);
                *p_len = len + 1;
            }
            else if(info.type == NVS_TYPE_BLOB)
            {
                *p_type = __NVS_VALUE_BLOB;
                size_t len;
                e = nvs_get_blob(handle, info.key, NULL, &len);
                *p_len = len;
            }
            else if(info.type == NVS_TYPE_U8) {
                *p_type = __NVS_VALUE_UINT8;
                *p_len = sizeof(uint8_t);
            }
            else if(info.type == NVS_TYPE_I8) {
                *p_type = __NVS_VALUE_INT8;
                *p_len = sizeof(int8_t);
            }
            else if(info.type == NVS_TYPE_U16) {
                *p_type = __NVS_VALUE_UINT16;
                *p_len = sizeof(uint16_t);
            }
            else if(info.type == NVS_TYPE_I16) {
                *p_type = __NVS_VALUE_INT16;
                *p_len = sizeof(int16_t);
            }
            else if(info.type == NVS_TYPE_U32) {
                *p_type = __NVS_VALUE_UINT32;
                *p_len = sizeof(uint32_t);
            }
            else if(info.type == NVS_TYPE_I32) {
                *p_type = __NVS_VALUE_INT32;
                *p_len = sizeof(int32_t);
            }

            if(e == ESP_OK)
            {
                nvs_close(handle);
                return true;
            }
        }

        nvs_close(handle);
    }

    return false;
}

bool nvs_if_get_value(
    const char* part,
    const char* namespace,
    const char* key,
    nvs_if_value_type_t type,
    void* buf,
    uint32_t len)
{
    if(part == NULL)
    {
        part = s_default_nvs_part;
    }
    if(!is_valid_nvs_partition(part))
    {
        __log_output(__red__"invalid nvs partition '"
            __yellow__"%s"__red__"'"__default__"\n", part);
        return false;
    }

    if(!buf)
    {
        return false;
    }

    nvs_entry_info_t info;
    nvs_iterator_t it = get_nvs_entry(part, namespace, key, &info);

    if(it)
    {
        nvs_handle_t handle;
        esp_err_t e = nvs_open_from_partition(
            part, namespace, NVS_READWRITE, &handle);

        if( e == ESP_OK )
        {
            if(info.type == NVS_TYPE_STR)
            {
                size_t size;
                e = nvs_get_str(handle, info.key, NULL, &size);
                if(size <= len && type == __NVS_VALUE_STRING)
                {
                    e = nvs_get_str(handle, info.key, buf, &size);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_BLOB)
            {
                size_t size;
                e = nvs_get_blob(handle, info.key, NULL, &size);
                if(size <= len && type == __NVS_VALUE_BLOB)
                {
                    e = nvs_get_blob(handle, info.key, buf, &size);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_U8) {
                if(sizeof(uint8_t) <= len && type == __NVS_VALUE_UINT8)
                {
                    e = nvs_get_u8(handle, info.key, buf);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_I8) {
                if(sizeof(int8_t) <= len && type == __NVS_VALUE_INT8)
                {
                    e = nvs_get_i8(handle, info.key, buf);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_U16) {
                if(sizeof(int8_t) <= len && type == __NVS_VALUE_UINT16)
                {
                    e = nvs_get_u16(handle, info.key, buf);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_I16) {
                if(sizeof(int16_t) <= len && type == __NVS_VALUE_INT16)
                {
                    e = nvs_get_i16(handle, info.key, buf);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_U32) {
                if(sizeof(uint32_t) <= len && type == __NVS_VALUE_UINT32)
                {
                    e = nvs_get_u32(handle, info.key, buf);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }
            else if(info.type == NVS_TYPE_I32) {
                if(sizeof(int32_t) <= len && type == __NVS_VALUE_INT32)
                {
                    e = nvs_get_i32(handle, info.key, buf);
                }
                else
                {
                    e = ! ESP_OK;
                }
            }

            if(e == ESP_OK)
            {
                nvs_close(handle);
                return true;
            }
        }

        nvs_close(handle);
    }

    return false;
}

bool nvs_if_add(
    const char* part,
    const char* namespace,
    const char* key,
    nvs_if_value_type_t value_type,
    const void* buf,
    size_t len)
{
    return nvs_if_set_impl(part, namespace, key, buf, len, true, value_type);
}

bool nvs_if_del(
    const char* part,
    const char* namespace,
    const char* key)
{
    if(part == NULL)
    {
        part = s_default_nvs_part;
    }
    if(!is_valid_nvs_partition(part))
    {
        __log_output(__red__"invalid nvs partition '"
            __yellow__"%s"__red__"'"__default__"\n", part);
        return false;
    }

    nvs_handle_t handle;
    esp_err_t e = nvs_open_from_partition(
        part, namespace, NVS_READWRITE, &handle);

    if(e == ESP_OK)
    {
        e = nvs_erase_key(handle, key);
        if(e == ESP_OK)
        {
            e = nvs_commit(handle);
        }
    }

    return e == ESP_OK ? true : false;
}

/* --- end of file ---------------------------------------------------------- */
