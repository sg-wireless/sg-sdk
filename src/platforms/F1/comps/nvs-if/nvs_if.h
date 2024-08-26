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
 * @brief   This file represents the ESP nvs interface header
 * --------------------------------------------------------------------------- *
 */

#ifndef __NVS_IF_H__
#define __NVS_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -- APIs ------------------------------------------------------------------ */

/**
 * @brief list stats of all NVS memory contents
 * 
 * @param disp_blob_data to echo the blob hex values
 * @param part_name list status for a specific partition. passing NULL will stat
 *          all nvs partitions.
 * @param namespace list status of a specific namespace. passing NULL will stat
 *          all namespaces.
 */
void nvs_if_stat(
    bool disp_blob_data,
    const char* part_name,
    const char* namespace);

/**
 * @brief check key existence
 * 
 * @param part_name looks for the namespace in the specified partition. passing
 *          NULL, will looks for it in 'nvs' partition
 * @param namespace the namespace associated with the key. must not be NULL.
 * @param key the required key to be checked. passing NULL, will check for
 *          namespace existence only irrespective on the keys.
 * 
 * @return  true if exists
 *          false if not exists
 */
bool nvs_if_exists(
    const char* part_name,
    const char* namespace,
    const char* key);

/**
 * @brief used to modify an existing nvs key/value pair.
 * 
 * @param part  the nvs partition name containing the target key/value pair.
 *              passing NULL, will use the default NVS partition 'nvs'
 * @param namespace the namespace associated with these key/value pair
 * @param key the key name
 * @param buf a buffer holding the new modified value and data structue in the
 *            incoming buffer must be conforming to the existing key value type
 *            otherwise the value context will be corrupted.
 * @param len the length of the incoming value buffer
 * 
 * @return  true if the set successfully
 *          false if not set
 */
bool nvs_if_set(
    const char* part,       /**< the nvs partition name */
    const char* namespace,
    const char* key,
    const void* buf,
    size_t len);

typedef enum _nvs_if_value_type_e {
    __NVS_VALUE_STRING,
    __NVS_VALUE_UINT8,
    __NVS_VALUE_INT8,
    __NVS_VALUE_UINT16,
    __NVS_VALUE_INT16,
    __NVS_VALUE_UINT32,
    __NVS_VALUE_INT32,
    __NVS_VALUE_BLOB,
} nvs_if_value_type_t;

/**
 * @brief to retrieve the existing key value data type
 * 
 * @param part  the nvs partition name containing the target key/value pair.
 *              passing NULL, will use the default NVS partition 'nvs'
 * @param namespace the namespace associated with these key/value pair
 * @param key the key name
 * @param[out] p_type a pointer at which the retrieved data type will be written
 * @param[out] p_len a pointer at which the required data length will be written
 * 
 * @return  true if the retrieved successfully
 *          false if not retrieved
 */
bool nvs_if_get_value_type(
    const char* part,
    const char* namespace,
    const char* key,
    nvs_if_value_type_t* p_type,
    uint32_t *p_len);

/**
 * @brief to retrieve the existing key value data type
 * 
 * @param part  the nvs partition name containing the target key/value pair.
 *              passing NULL, will use the default NVS partition 'nvs'
 * @param namespace the namespace associated with these key/value pair
 * @param key the key name
 * @param type the required data type and it must conform to the existing data
 *             type. it must be retrieved by \ref nvs_if_get_value_type().
 * @param buf a buffer at which the value will be written
 * @param len the maximum length of the buffer. it must be retrieved by \ref
 *            nvs_if_get_value_type().
 * 
 * @return  true if the value is gotten successfully
 *          false if the value is not gotten
 */
bool nvs_if_get_value(
    const char* part,
    const char* namespace,
    const char* key,
    nvs_if_value_type_t type,
    void* buf,
    uint32_t len);

/**
 * @brief to add new key/value pair to an nvs partition
 * 
 * @param part  the nvs partition name. passing NULL, will use the default nvs
 *              partition 'nvs'
 * @param namespace the target namespace
 * @param key the key name
 * @param type the required data type of the value data.
 * @param buf a buffer containing the initial value
 * @param len the maximum length of the given buffer buffer.
 * 
 * @return  true if the key/value pair has been added successfully
 *          false if not added
 */
bool nvs_if_add(
    const char* part,
    const char* namespace,
    const char* key,
    nvs_if_value_type_t value_type,
    const void* buf,
    size_t len);

/**
 * @brief to delete an existing key and its associated value
 * 
 * @param part  the nvs partition name. passing NULL, will use the default nvs
 *              partition 'nvs'
 * @param namespace the target namespace
 * @param key the key name
 * 
 * @return  true if the key/value pair has been deleted successfully or are not
 *              exist any more
 *          false if not deleted
 */
bool nvs_if_del(
    const char* part,
    const char* namespace,
    const char* key);


/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __NVS_IF_H__ */
