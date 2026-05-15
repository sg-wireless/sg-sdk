/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
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
 *
 * @author  Christian Ehlers (SG Wireless)
 *
 * @brief   This is the storage interface for the configuration stored in the
 * NVS
 * --------------------------------------------------------------------------- *
 */

#include "esp_log.h"
#include "string.h"

static const char *TAG = "nvs_config";

#include "nvs.h"
#include "nvs_flash.h"

bool nvs_config_init0(nvs_handle_t *ro_handle, nvs_handle_t *rw_handle,
                      const char *name) {
    ESP_LOGD(TAG, "Initializing nvs config for module %s", name);
    esp_err_t err;
    
    // Open main NVS partition with read-write access
    err = nvs_open(name, NVS_READWRITE, rw_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS namespace %s", esp_err_to_name(err), name);
        return false;
    }
    
    // For backwards compatibility, set ro_handle to the same as rw_handle
    *ro_handle = *rw_handle;
    
    return true;
}

bool nvs_read_blob(nvs_handle_t ro_handle, nvs_handle_t rw_handle,
                   const char *name, uint8_t *data, size_t size) {
    // Only use rw_handle since both handles point to the same NVS
    esp_err_t err = nvs_get_blob(rw_handle, name, data, &size);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Setting %s not found", name);
        return false;
    }
    return true;
}

bool nvs_write_blob(nvs_handle_t rw_handle, const char *name, uint8_t *data,
                    uint8_t size) {
    esp_err_t err = nvs_set_blob(rw_handle, name, data, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving setting: %s ", esp_err_to_name(err));
        return false;
    }
    return (nvs_commit(rw_handle) == ESP_OK);
}

bool nvs_read_str(nvs_handle_t ro_handle, nvs_handle_t rw_handle,
                  const char *name, char *data) {
    size_t required_size;
    // Only use rw_handle since both handles point to the same NVS
    esp_err_t err = nvs_get_str(rw_handle, name, NULL, &required_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Setting %s not found", name);
        return false;
    }
    if (err == ESP_OK) {
        char *ret_str = malloc(required_size);
        nvs_get_str(rw_handle, name, ret_str, &required_size);
        memcpy(data, ret_str, required_size);
        free(ret_str);
        return true;
    }
    return false;
}

bool nvs_write_str(nvs_handle_t rw_handle, const char *name,
                   const char *value) {
    esp_err_t err = nvs_set_str(rw_handle, name, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving setting %s: %s ", name,
                 esp_err_to_name(err));
        return false;
    }
    return (nvs_commit(rw_handle) == ESP_OK);
}

bool nvs_read_u8(nvs_handle_t ro_handle, nvs_handle_t rw_handle,
                 const char *name, uint8_t *data) {
    // Only use rw_handle since both handles point to the same NVS
    esp_err_t err = nvs_get_u8(rw_handle, name, data);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Setting %s not found", name);
        return false;
    }
    return true;
}

bool nvs_write_u8(nvs_handle_t rw_handle, const char *name, uint8_t data) {
    esp_err_t err = nvs_set_u8(rw_handle, name, data);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving setting: %s ", esp_err_to_name(err));
        return false;
    }
    return (nvs_commit(rw_handle) == ESP_OK);
}

bool nvs_read_u32(nvs_handle_t ro_handle, nvs_handle_t rw_handle,
                  const char *name, uint32_t *data) {
    // Only use rw_handle since both handles point to the same NVS
    esp_err_t err = nvs_get_u32(rw_handle, name, data);
    if (err != ESP_OK) {
        ESP_LOGD(TAG, "Setting %s not found", name);
        return false;
    }
    return true;
}

bool nvs_write_u32(nvs_handle_t rw_handle, const char *name, uint32_t data) {
    esp_err_t err = nvs_set_u32(rw_handle, name, data);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error saving setting: %s ", esp_err_to_name(err));
        return false;
    }
    return (nvs_commit(rw_handle) == ESP_OK);
}

/* -- end of file ----------------------------------------------------------- */