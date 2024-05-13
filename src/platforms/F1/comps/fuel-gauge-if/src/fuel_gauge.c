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
 * @brief   Fuel Gauge Interface component implementation
 * --------------------------------------------------------------------------- *
 */

/* --------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdio.h>
#include <stdbool.h>


#define __log_subsystem     F1
#define __log_component     fuelgauge
#include "log_lib.h"
__log_component_def(F1, fuelgauge, purple, 1, 0)

#include "driver/i2c.h"         // for i2c transfer
#include "driver/gpio.h"        // for interrupt and reset pin
#include "hal/i2c_ll.h"

#include "bq27421.h"
#include "bq27421_stub.h"
#include "fuel_gauge.h"

/* --------------------------------------------------------------------------- *
 * i2c handling
 * --------------------------------------------------------------------------- *
 */
#define __esp32_i2c_instance    I2C_NUM_1
#define __esp32_i2c_freq        100000
#define __esp32_i2c_timeout   __time_ms(10)

#ifdef CONFIG_FUEL_GAUGE_I2C_SCL_GPIO
#define __esp32_i2c_scl_gpio    CONFIG_FUEL_GAUGE_I2C_SCL_GPIO
#else
#define __esp32_i2c_scl_gpio    GPIO_NUM_16
#endif

#ifdef CONFIG_FUEL_GAUGE_I2C_SDA_GPIO
#define __esp32_i2c_sda_gpio    CONFIG_FUEL_GAUGE_I2C_SDA_GPIO
#else
#define __esp32_i2c_sda_gpio    GPIO_NUM_15
#endif

#define __esp_api_call(__api_call, __err_msg, __ret)        \
    do {                                                    \
        esp_err_t err = __api_call;                         \
        if( err != ESP_OK ) {                               \
            __log_error("(err_code:%d)" __err_msg, err);    \
            return __ret;                                   \
        }                                                   \
    } while (0)

static void bq27421_i2c_read_port(
    uint8_t dev_addr, uint8_t* buff, uint32_t cbytes)
{
    __log_debug("i2c [r] [addr: %02x] [len: %d] [byte1: %02x]"
        , dev_addr, cbytes, buff[0]);

    __esp_api_call( i2c_master_read_from_device(__esp32_i2c_instance,
        dev_addr, buff, cbytes, portMAX_DELAY), "i2c master read error", );
}

static void bq27421_i2c_write_port
    (uint8_t dev_addr, uint8_t* buff, uint32_t cbytes)
{
    __log_debug("i2c [w] [addr: %02x] [len: %d] [byte1: %02x]",
        dev_addr, cbytes, buff[0]);

    __esp_api_call( i2c_master_write_to_device(__esp32_i2c_instance,
        dev_addr, buff, cbytes, portMAX_DELAY), "i2c master write error", );
}

static void esp32_fuel_gauge_i2c_ctor(void)
{
    i2c_config_t i2c_cfg = {
        .scl_io_num = __esp32_i2c_scl_gpio,
        .sda_io_num = __esp32_i2c_sda_gpio,
        .mode = I2C_MODE_MASTER,
        /**
         * there is already an external pull-up resistors.
         */
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = __esp32_i2c_freq
    };

    __esp_api_call(i2c_param_config(__esp32_i2c_instance, &i2c_cfg),
        "i2c param config error", );

    int timeout = __time2cycles( __esp32_i2c_timeout, I2C_APB_CLK_FREQ );
    __log_debug("i2c timeout : %d cycles", timeout);

    timeout = timeout > I2C_LL_MAX_TIMEOUT ? I2C_LL_MAX_TIMEOUT : timeout;

    __esp_api_call(i2c_set_timeout(__esp32_i2c_instance, timeout),
        "i2c set timeout error", );

    __esp_api_call( i2c_driver_install(__esp32_i2c_instance, I2C_MODE_MASTER,
        0, 0, 0), "i2c driver install error", );
}

static void esp32_fuel_gauge_i2c_dtor(void)
{
    __log_info("dtor() -> i2c pins");

    __esp_api_call( i2c_driver_delete(__esp32_i2c_instance),
        "i2c driver delete error", );

    __esp_api_call(gpio_reset_pin(__esp32_i2c_scl_gpio),
        "gpio reset pin error", );
    __esp_api_call(gpio_reset_pin(__esp32_i2c_sda_gpio),
        "gpio reset pin error", );
}

/* --------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
static bool s_initialized = false;

bool fuel_gauge_init(
    uint16_t design_capacity_mAh,
    uint16_t terminate_voltage_mV,
    uint16_t taper_current_mA)
{
    if(s_initialized) {
        __log_output(
            __red__"fuel-gauge is already initialized,"
            __cyan__" deinit() first, and init() again\n"
            __default__);
        return false;
    }

    esp32_fuel_gauge_i2c_ctor();

    bq27421_stub_init(bq27421_i2c_read_port, bq27421_i2c_write_port);

    s_initialized = bq27421_init(
        design_capacity_mAh,
        terminate_voltage_mV,
        taper_current_mA);

    if(!s_initialized) {
        __log_output(__red__"Fuel-Gauge init error\n"__default__);
        esp32_fuel_gauge_i2c_dtor();
        bq27421_stub_init(NULL, NULL);
        s_initialized = false;
    }

    return s_initialized;
}

void fuel_gauge_deinit(void)
{
    if(!s_initialized) {
        return;
    }

    esp32_fuel_gauge_i2c_dtor();

    bq27421_stub_init(NULL, NULL);

    s_initialized = false;
}

bool fuel_gauge_read_info(fuel_gauge_info_t * info)
{
    return s_initialized ? bq27421_update(info) : false;
}

/* --- end of file ---------------------------------------------------------- */
