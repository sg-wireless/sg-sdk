/** ----------------------------------------------------------------------------
 *  @file       fuel_gauge_mp.c
 *  @author     Ahmed Sabry (SG Wireless)
 *  @maintainer Christian Ehlers (SG Wireless)
 *  @date       April 13, 2024
 *  @brief      Fuel Gauge Interface component implementation using MicroPython I2C
 *              bridge instead of direct ESP-IDF I2C calls for driver coordination.
 *  @details    This implementation provides the same fuel gauge API but routes
 *              I2C communication through MicroPython bridge for consistency.
 *
 *  @copyright  Copyright (c) 2024-2026 SG Wireless - All Rights Reserved
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to 
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 * --------------------------------------------------------------------------- */

/* --- includes ------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"

/* log header */
#define __log_subsystem     F1
#define __log_component     fuel_gauge_mp
#include "log_lib.h"
__log_component_def(F1, fuel_gauge_mp, purple, 1, 0)

/* SDK headers */
#include "fuel_gauge.h"
#include "mp_i2c_bridge.h"

/* BQ27421 driver headers */
#include "bq27421.h"
#include "bq27421_stub.h"

/* --- macros --------------------------------------------------------------- */

/** I2C SCL GPIO pin */
#ifdef CONFIG_FUEL_GAUGE_I2C_SCL_GPIO
#define __mp_i2c_scl_gpio      CONFIG_FUEL_GAUGE_I2C_SCL_GPIO
#else
#define __mp_i2c_scl_gpio      8
#endif

/** I2C SDA GPIO pin */  
#ifdef CONFIG_FUEL_GAUGE_I2C_SDA_GPIO
#define __mp_i2c_sda_gpio      CONFIG_FUEL_GAUGE_I2C_SDA_GPIO
#else
#define __mp_i2c_sda_gpio      9
#endif

/** I2C frequency */
#define __mp_i2c_freq               100000  // 100 kHz

/* --- global variables ----------------------------------------------------- */

static bool s_fuel_gauge_is_init = false;

/* --- BQ27421 HAL port functions ------------------------------------------- */

static void bq27421_i2c_read_port(uint8_t dev_addr, uint8_t* buff, uint32_t cbytes)
{
    __log_debug("i2c [r] [addr: %02x] [len: %d]", dev_addr, cbytes);

    esp_err_t ret = mp_i2c_bridge_write_read(dev_addr, NULL, 0, buff, cbytes);
    if (ret != ESP_OK) {
        __log_error("MicroPython I2C read failed: %s", esp_err_to_name(ret));
        // Note: BQ27421 driver expects void return, so we can't indicate error here
        // The driver will detect communication errors through timeouts or invalid responses
    }
}

static void bq27421_i2c_write_port(uint8_t dev_addr, uint8_t* buff, uint32_t cbytes)
{
    __log_debug("i2c [w] [addr: %02x] [len: %d] [byte1: %02x]",
        dev_addr, cbytes, buff[0]);

    esp_err_t ret = mp_i2c_bridge_write(dev_addr, buff, cbytes);
    if (ret != ESP_OK) {
        __log_error("MicroPython I2C write failed: %s", esp_err_to_name(ret));
        // Note: BQ27421 driver expects void return, so we can't indicate error here
        // The driver will detect communication errors through timeouts or invalid responses
    }
}

/* --- local functions ------------------------------------------------------ */

static bool mp_fuel_gauge_i2c_init(void)
{
    __log_info("Initializing fuel gauge I2C using MicroPython bridge");
    
    esp_err_t ret = mp_i2c_bridge_init(__mp_i2c_scl_gpio, __mp_i2c_sda_gpio, __mp_i2c_freq);
    if (ret != ESP_OK) {
        __log_error("Failed to initialize MicroPython I2C bridge: %s", esp_err_to_name(ret));
        return false;
    }
    
    __log_info("MicroPython I2C bridge initialized for fuel gauge");
    
    /* Scan for BQ27421 device */
    uint8_t devices[16];
    int num_found = mp_i2c_bridge_scan(devices, sizeof(devices));
    if (num_found > 0) {
        __log_info("I2C scan found %d devices:", num_found);
        for (int i = 0; i < num_found; i++) {
            __log_info("  Device at address 0x%02X", devices[i]);
            if (devices[i] == (BQ27421_I2C_ADDRESS >> 1)) {  // Convert from 8-bit to 7-bit address
                __log_info("  -> BQ27421 fuel gauge detected!");
            }
        }
    }
    
    return true;
}

static void mp_fuel_gauge_i2c_deinit(void)
{
    __log_info("Deinitializing fuel gauge I2C bridge");
    mp_i2c_bridge_deinit();
}

/* --- API functions -------------------------------------------------------- */

bool fuel_gauge_init(
    uint16_t design_capacity_mAh,
    uint16_t terminate_voltage_mV,
    uint16_t taper_current_mA)
{
    if(s_fuel_gauge_is_init) {
        __log_error("fuel gauge already initialized");
        return false;
    }

    __log_info("fuel gauge init");

    /* Initialize MicroPython I2C bridge */
    if (!mp_fuel_gauge_i2c_init()) {
        __log_error("Failed to initialize I2C bridge for fuel gauge");
        return false;
    }

    /* Setup the HAL porting interface using stub */
    bq27421_stub_init(bq27421_i2c_read_port, bq27421_i2c_write_port);

    /* Perform the initial configuration of the fuel gauge */
    s_fuel_gauge_is_init = bq27421_init(design_capacity_mAh, terminate_voltage_mV, taper_current_mA);

    if(!s_fuel_gauge_is_init) {
        __log_error("bq27421 init failed");
        mp_fuel_gauge_i2c_deinit();
        bq27421_stub_init(NULL, NULL);
        return false;
    }

    __log_info("fuel gauge init -- done");

    return true;
}

void fuel_gauge_deinit(void)
{
    if(!s_fuel_gauge_is_init) {
        __log_error("fuel gauge is not initialized");
        return;
    }

    __log_info("fuel gauge deinit");

    mp_fuel_gauge_i2c_deinit();
    bq27421_stub_init(NULL, NULL);
    s_fuel_gauge_is_init = false;

    __log_info("fuel gauge deinit -- done");
}

bool fuel_gauge_read_info(fuel_gauge_info_t * info)
{
    if(!s_fuel_gauge_is_init) {
        __log_error("fuel gauge is not initialized");
        return false;
    }

    if (info == NULL) {
        __log_error("info pointer is null");
        return false;
    }

    /* Read battery info using BQ27421 update function */
    if( ! bq27421_update(info) ) {
        __log_error("failed to read battery info");
        return false;
    }

    __log_debug("battery info - voltage: %d mV, current: %d mA, soc: %d%%", 
                info->voltage_mV, info->current_mA, info->soc_percent);

    return true;
}

/* --- end of file ---------------------------------------------------------- */