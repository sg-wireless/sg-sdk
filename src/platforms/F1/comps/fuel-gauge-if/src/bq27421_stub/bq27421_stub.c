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
 * @brief   Fuel Gauge driver stub which contain the platform functions required
 *          by the driver.
 * --------------------------------------------------------------------------- *
 */

/* --------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "stm32f4xx_hal.h"
#include "bq27421_stub.h"

void* hi2c2 = NULL;

static bq27421_i2c_read_port_t* s_i2c_read_method;
static bq27421_i2c_write_port_t* s_i2c_write_method;

int HAL_I2C_Master_Transmit(
    void* inst,
    uint16_t i2c_addr,
    uint8_t* data,
    int data_len,
    uint32_t timeout )
{
    if(s_i2c_write_method)
    {
        s_i2c_write_method((uint8_t)0x55, data, data_len);
    }
    return HAL_OK;
}

int HAL_I2C_Master_Receive(
    void* inst,
    uint16_t i2c_addr,
    uint8_t* data,
    int data_len,
    uint32_t timeout)
{
    if(s_i2c_read_method)
    {
        s_i2c_read_method((uint8_t)0x55, data, data_len);
    }
    return HAL_OK;
}

void HAL_Delay(uint32_t delay)
{
    vTaskDelay(delay / portTICK_PERIOD_MS);
}

void bq27421_stub_init(
    bq27421_i2c_read_port_t* i2c_read_method,
    bq27421_i2c_write_port_t* i2c_write_method)
{
    s_i2c_read_method = i2c_read_method;
    s_i2c_write_method = i2c_write_method;
}

/* --- end of file ---------------------------------------------------------- */
