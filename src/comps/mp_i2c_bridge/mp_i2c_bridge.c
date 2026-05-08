/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @author  Ahmed Sabry (SG Wireless)
 * @maintainer  Christian Ehlers (SG Wireless)
 * 
 * @brief   MicroPython I2C Bridge implementation for ESP-IDF v5.4 compatibility
 * --------------------------------------------------------------------------- *
 */

#include "mp_i2c_bridge.h"
#include "driver/i2c.h"
#include "esp_log.h"

static const char *TAG = "mp_i2c_bridge";

// I2C port to use
#define I2C_PORT I2C_NUM_0

// Global I2C configuration
static bool s_i2c_initialized = false;

esp_err_t mp_i2c_bridge_init(int scl_pin, int sda_pin, uint32_t freq) {
    if (s_i2c_initialized) {
        ESP_LOGI(TAG, "I2C bridge already initialized");
        return ESP_OK;
    }
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = freq,
    };
    
    esp_err_t ret = i2c_param_config(I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure I2C parameters: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    s_i2c_initialized = true;
    ESP_LOGI(TAG, "I2C bridge initialized: SCL=%d, SDA=%d, freq=%lu", scl_pin, sda_pin, freq);
    return ESP_OK;
}

void mp_i2c_bridge_deinit(void) {
    if (s_i2c_initialized) {
        i2c_driver_delete(I2C_PORT);
        s_i2c_initialized = false;
        ESP_LOGI(TAG, "I2C bridge deinitialized");
    }
}

esp_err_t mp_i2c_bridge_write(uint8_t addr, const uint8_t *data, size_t len) {
    if (!s_i2c_initialized) {
        ESP_LOGE(TAG, "I2C bridge not initialized");
        return ESP_FAIL;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t mp_i2c_bridge_read(uint8_t addr, uint8_t *data, size_t len) {
    if (!s_i2c_initialized) {
        ESP_LOGE(TAG, "I2C bridge not initialized");
        return ESP_FAIL;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t mp_i2c_bridge_write_read(uint8_t addr, const uint8_t *write_data, size_t write_len, uint8_t *read_data, size_t read_len) {
    if (!s_i2c_initialized) {
        ESP_LOGE(TAG, "I2C bridge not initialized");
        return ESP_FAIL;
    }
    
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    
    // Write phase
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, write_data, write_len, true);
    
    // Read phase with repeated start
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, read_data, read_len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write_read failed: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

int mp_i2c_bridge_scan(uint8_t *devices, int max_devices) {
    if (!s_i2c_initialized) {
        ESP_LOGE(TAG, "I2C bridge not initialized");
        return 0;
    }
    
    int found = 0;
    
    // Scan addresses 0x08 to 0x77 (standard I2C range)
    for (uint8_t addr = 0x08; addr <= 0x77 && found < max_devices; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        
        esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        
        if (ret == ESP_OK) {
            devices[found++] = addr;
        }
    }
    
    ESP_LOGI(TAG, "I2C scan found %d devices", found);
    return found;
}