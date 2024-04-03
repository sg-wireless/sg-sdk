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
 * @brief   This file represents the driver interface of NXP PCAL6408A IO
 *          Expander
 * --------------------------------------------------------------------------- *
 */
#ifndef __PCAL8408A_H__
#define __PCAL8408A_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include <stdint.h>

/* --- typedefs ------------------------------------------------------------- */

/**
 * @enum    pcal6408a_error_t
 * @brief   contains the error return type of the API functions of this driver
 */
typedef enum {
    __pcal6408a_ok,
    __pcal6408a_bad_args,
    __pcal6408a_bad_handle,
    __pcal6408a_non_initialized_handle,
    __pcal6408a_write_to_input_pin,
    __pcal6408a_pin_inactive,
    __pcal6408a_error
} pcal6408a_error_t;

/**
 * @typedef pcal6408a_i2c_read_port_t
 * @brief   driver i2c read port, when porting this driver, it should be
 *          implemented at platform side
 * 
 * @param dev_addr  the connected pcal6408a i2c address
 * @param buff      a buffer of bytes to be transfered over the i2c bus
 * @param cbytes    the bytes count in the buff
 */
typedef void (* pcal6408a_i2c_read_port_t )
    (uint8_t dev_addr, uint8_t* buff, uint32_t cbytes);

/**
 * @typedef pcal6408a_i2c_write_port_t
 * @brief   driver i2c write port, when porting this driver, it should be
 *          implemented at platform side
 * 
 * @param dev_addr  the connected pcal6408a i2c address
 * @param buff      a buffer of bytes to be transfered over the i2c bus
 * @param cbytes    the bytes count in the buff
 */
typedef void (* pcal6408a_i2c_write_port_t )
    (uint8_t dev_addr, uint8_t* buff, uint32_t cbytes);

/**
 * @typedef pcal6408a_pin_interrupt_handler_t
 * @brief   the callback function prototype that will be called if an interrupt
 *          happened due to the corresponding io exp pin
 */
typedef void (* pcal6408a_pin_interrupt_handler_t)(bool bin_value);

/**
 * @struct  pcal6408a_init_config_t
 * @brief   initialization configurations of driver
 */
typedef struct {

    // attributes
    bool is_addr_pin_connected_to_vdd;      /**< addr pin connection */
    bool is_open_drain_output;              /**< output open-drain enable */

    // plugins
    pcal6408a_i2c_read_port_t   i2c_read;   /**< i2c read port */
    pcal6408a_i2c_write_port_t  i2c_write;  /**< i2c write port */

} pcal6408a_init_config_t;

/**
 * @typedef pcal6408a_port_pin_handle
 * @brief   the handle type of the configured pin
 */
typedef void* pcal6408a_port_pin_handle;

typedef enum {
    pcal6408a_port_pin_0,
    pcal6408a_port_pin_1,
    pcal6408a_port_pin_2,
    pcal6408a_port_pin_3,
    pcal6408a_port_pin_4,
    pcal6408a_port_pin_5,
    pcal6408a_port_pin_6,
    pcal6408a_port_pin_7
} pcal6408a_port_pin_t;

typedef enum {
    pcal6408a_pull_resistor_none,
    pcal6408a_pull_resistor_up,
    pcal6408a_pull_resistor_down
} pcal6408a_pull_resistor_et;

typedef enum {
    pcal6408a_drive_capability_level_0,
    pcal6408a_drive_capability_level_1,
    pcal6408a_drive_capability_level_2,
    pcal6408a_drive_capability_level_3,
} pcal6408a_drive_capability_et;

/* --- api declarations ----------------------------------------------------- */

/**
 * @brief   initializes the driver with a given configuration
 * 
 * @param p_init_struct a pointer to configuration parameters
 * @return  error type
 */
pcal6408a_error_t pcal6408a_init(pcal6408a_init_config_t * p_init_struct);

/**
 * @brief   resets the device to its default power-up defaults to be used
 *          for system sleep preparation
 */
void pcal6408a_reset(void);

/**
 * @brief   trigger port to the driver.
 *          this function shall be invoked if an interrupt received from the
 *          io expander interrupt pin.
 *          the driver will make the required callbacks to the pin handler that
 *          is responsible for interrupt occurring.
 */
void pcal6408a_interrupt_trigger_port(void);

/**
 * @brief   configures an IO expander pin as input pin
 * 
 * @param   pin_num the pin number from 0 to 7
 * @param   pull_resistor the pull resistor type
 * @param   is_logic_inverted the input logic values will be inverted
 * @param   is_latched the input port value that intiated the interrupt will be
 *          kept until the interrupt status is read
 * @param   p_interrupt_handler a callback to the handler of the received
 *          interrupt over this specific pin
 * @param   name a user defined name string for the signal over this pin
 * 
 * @return  non-NULL a handle to the pin
 *          NULL     if configuration failed
 */
pcal6408a_port_pin_handle pcal6408a_configure_input_port_pin(
    pcal6408a_port_pin_t    pin_num,  // 0 ~ 7
    pcal6408a_pull_resistor_et pull_resistor,
    bool        is_logic_inverted,
    bool        is_latched,
    pcal6408a_pin_interrupt_handler_t p_interrupt_handler,
    const char* name
    );

/**
 * @brief   configures an IO expander pin as input pin
 * 
 * @param   pin_num the pin number from 0 to 7
 * @param   pull_resistor the pull resistor type
 * @param   drive_capability the output current driving capability
 * @param   name a user defined name string for the signal over this pin
 * 
 * @return  non-NULL a handle to the pin
 *          NULL     if configuration failed
 */
pcal6408a_port_pin_handle pcal6408a_configure_output_port_pin(
    pcal6408a_port_pin_t        pin_num,
    pcal6408a_pull_resistor_et  pull_resistor,
    pcal6408a_drive_capability_et   drive_capability,
    const char* name
    );

/**
 * @brief   activate the configured function of the pin
 * 
 * @param   handle a handle to the preconfigured port pin
 * @return  __pcal6408a_ok          write successful
 *          __pcal6408a_bad_handle  wrong handle passed
 *          __pcal6408a_non_initialized_handle
 */
pcal6408a_error_t pcal6408a_activate_pin(pcal6408a_port_pin_handle handle);

/**
 * @brief   deactivate the configured function of the pin
 * 
 * @param   handle a handle to the preconfigured port pin
 * @return  __pcal6408a_ok          write successful
 *          __pcal6408a_bad_handle  wrong handle passed
 *          __pcal6408a_non_initialized_handle
 */
pcal6408a_error_t pcal6408a_deactivate_pin(pcal6408a_port_pin_handle handle);

/**
 * @brief   reads the logic value of the input or output port pin
 * 
 * @param   handle a handle to the preconfigured port pin
 * @param   p_read_logic_level a pointer to which the read logic level will be
 *          written
 * @return  __pcal6408a_ok          read successful
 *          __pcal6408a_bad_handle  wrong handle passed
 *          __pcal6408a_non_initialized_handle
 *          __pcal6408a_pin_inactive
 */
pcal6408a_error_t pcal6408a_read(
    pcal6408a_port_pin_handle   handle,
    bool * p_read_logic_level
    );

/**
 * @brief   writes the logic value of the output port pin
 * 
 * @param   handle a handle to the preconfigured port pin
 * @param   logic_level the logic level to be written to the output port pin
 * @return  __pcal6408a_ok          write successful
 *          __pcal6408a_bad_handle  wrong handle passed
 *          __pcal6408a_non_initialized_handle
 *          __pcal6408a_write_to_input_pin
 *          __pcal6408a_pin_inactive
 */
pcal6408a_error_t pcal6408a_write(
    pcal6408a_port_pin_handle   handle,
    bool logic_level
    );

/**
 * @brief   reconfigure the output pins to open-drain configuration or not
 * 
 * @param   enable to enable or disable the output open-drain configuration
 */
void pcal6408a_config_open_drain_output(bool enable);

/**
 * @brief   displays the stats of the controlled ioexpander
 * 
 */
void pcal6408a_stats(void);

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __PCAL8408A_H__ */
