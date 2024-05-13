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
 * @author  Ahmed Sabry (SG Wireless)
 *
 * @brief   Fuel-Gauge Interface component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __FUEL_GAUGE_H__
#define __FUEL_GAUGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bq27421.h"

/** -------------------------------------------------------------------------- *
 * typedes
 * --------------------------------------------------------------------------- *
 */
typedef bq27421_info fuel_gauge_info_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief   initialize the Fuel Gauge.
 * 
 * @param   design_capacity_mAh the capacity of the battary in mAh
 * @param   terminate_voltage_mV an optional system minimum operating voltage in
 *              mV. Provide zero if not specified.
 * @param   taper_current_mA the taper current detection threshold of the
 *              charger (including charger tolerances). The charger will stop
 *              charging below this value. Provide zero, if not specified.
 * 
 * @return  true if init successfully
 *          false if not initiliazed successfully
 */
bool fuel_gauge_init(
    uint16_t design_capacity_mAh,
    uint16_t terminate_voltage_mV,
    uint16_t taper_current_mA);

/**
 * @brief   deinit Fuel-Gauge
 * 
 */
void fuel_gauge_deinit(void);

/**
 * @brief   read Fuel-Gauge info
 * 
 * @param info pointer to a info structure which is the same as defined struct
 *          in the driver code bq27421.h
 * 
 * @return true if read successfully
 *          false otherwise
 */
bool fuel_gauge_read_info(fuel_gauge_info_t * info);


/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __FUEL_GAUGE_H__ */
