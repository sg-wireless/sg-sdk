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
 * @brief   MicroPython I2C Bridge header for ESP-IDF v5.4 compatibility
 * --------------------------------------------------------------------------- *
 */

#ifndef MP_I2C_BRIDGE_H
#define MP_I2C_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Initialize the MicroPython I2C bridge
 * @param scl_pin SCL pin number
 * @param sda_pin SDA pin number
 * @param freq I2C frequency in Hz
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t mp_i2c_bridge_init(int scl_pin, int sda_pin, uint32_t freq);

/**
 * @brief Deinitialize the MicroPython I2C bridge
 */
void mp_i2c_bridge_deinit(void);

/**
 * @brief Write data to I2C device
 * @param addr I2C device address (7-bit)
 * @param data Data buffer to write
 * @param len Number of bytes to write
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t mp_i2c_bridge_write(uint8_t addr, const uint8_t *data, size_t len);

/**
 * @brief Read data from I2C device
 * @param addr I2C device address (7-bit)
 * @param data Buffer to store read data
 * @param len Number of bytes to read
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t mp_i2c_bridge_read(uint8_t addr, uint8_t *data, size_t len);

/**
 * @brief Write then read from I2C device (typical register read operation)
 * @param addr I2C device address (7-bit)
 * @param write_data Data to write (typically register address)
 * @param write_len Number of bytes to write
 * @param read_data Buffer to store read data
 * @param read_len Number of bytes to read
 * @return ESP_OK on success, ESP_FAIL on failure
 */
esp_err_t mp_i2c_bridge_write_read(uint8_t addr, const uint8_t *write_data, size_t write_len, uint8_t *read_data, size_t read_len);

/**
 * @brief Scan I2C bus for devices
 * @param devices Array to store found device addresses
 * @param max_devices Maximum number of devices to find
 * @return Number of devices found
 */
int mp_i2c_bridge_scan(uint8_t *devices, int max_devices);

#endif // MP_I2C_BRIDGE_H