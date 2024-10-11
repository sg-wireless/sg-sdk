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
 * @brief   This file is an interface to the IO Expander chip PCAL6408A on
 *          SGW3501 Board
 * --------------------------------------------------------------------------- *
 */

#ifndef __IO_EXP_H__
#define __IO_EXP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include "stdint.h"

/* --- typedefs ------------------------------------------------------------- */

/**
 * @typedef ioexp_callback_t
 * @brief   a callback function prototype. the component that has an io expander
 *          input port pin handler can define a callback function of this type
 *          and the ioexpander will invoke it if the respective signal changed
 * @param timestamp the precise timestamp of the signal change trigger.
 */
typedef void (* ioexp_callback_t)(uint32_t timestamp);

/* --- api declarations ----------------------------------------------------- */

/**
 * @brief   To initialize the io-expander interfacing component to the esp32 mcu
 *          it initializes the required system resources.
 * 
 * @note    hardware resources are initialized on demend through the exported
 *          physical signals controls APIs
 */
void ioexp_init(void);

/**
 * @brief   To reset the io-expander driver
 * 
 * @note    it causes all driver intialized signals to be lost, hence it should
 *          be used for debugging purposes only
 */
void ioexp_reset(void);

/**
 * @brief   To stats the connected signals on the io-expander.
 */
void ioexp_stats(void);

/**
 * @brief   To support micropython i2c request to initialize the I2C bus
 *          on which the ioexp is connected.
 * @param port the i2c hardware instance.
 * @param scl  the i2c SCL gpio number.
 * @param sda  the i2c SDA gpio number.
 * @param freq the i2c operating frequency. Set to Zero, to skip its checking
 * @param timeout_ms the i2c bus timeout. Set to Zero, to skip its checking
 * 
 * @return  true    if all given gonfiguration parameters conforms with IO-EXP
 *          false   if one of the given parameter is not conforming.
 * 
 * @note    This function is needed for micropython fusion only.
 */
bool ioexp_micropython_req_i2c_init(int port, int scl, int sda,
    uint32_t freq, uint32_t timeout_ms);

/**
 * @brief   To support micropython i2c request to de-initialize the I2C bus
 *          on which the ioexp is connected.
 * @note    This function is needed for micropython fusion only.
 */
void ioexp_micropython_req_i2c_deinit(void);

/**
 * lora chip ioexp controls
 */
void ioexp_lora_chip_power_on(void);    /**< sets lora_power pin high */
void ioexp_lora_chip_power_off(void);   /**< sets lora_power pin low */
bool ioexp_lora_chip_power_status(void); /**< check if it is on or off */
void ioexp_lora_chip_reset(void);       /**< send reset pulse on lora_reset_n */
void ioexp_lora_chip_set_int_signal_callback( ioexp_callback_t cb );
bool ioexp_lora_chip_read_int_pin( void );
void ioexp_lora_chip_set_busy_signal_callback( ioexp_callback_t cb );
bool ioexp_lora_chip_is_busy(void);

/**
 * lte chip ioexp controls
 */
void ioexp_lte_chip_power_on(void);     /**< sets lte_power pin high */
void ioexp_lte_chip_power_off(void);    /**< sets lte_power pin low */
bool ioexp_lte_chip_power_status(void); /**< check if it is on or off */
void ioexp_lte_chip_reset(void);        /**< send reset pulse on lte_reset_n */
void ioexp_lte_chip_set_ring_signal_callback( ioexp_callback_t cb );

/**
 * secure chip ioexp controls
 */
void ioexp_secure_chip_enable(void);    /**< sets secure_en pin high */
void ioexp_secure_chip_disable(void);   /**< sets secure_en pin low */
bool ioexp_secure_chip_status(void);    /**< check if it is on or off */

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __IO_EXP_H__ */
