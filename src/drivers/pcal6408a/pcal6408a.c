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
 * @brief   This file represents the driver implementation of NXP PCAL6408A IO
 *          Expander
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "pcal6408a.h"

#define __log_subsystem     drivers
#define __log_component     pcal6408a
#include "log_lib.h"
__log_component_def(drivers,    pcal6408a,      green,      1, 0)
#include "utils_bitwise.h"

/* --------------------------------------------------------------------------- *
 * local configurations
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_PCAL6408A_REGISTERS_CACHE_ENABLE
#define __enable_pcal6408a_cache                (1)
#else
#define __enable_pcal6408a_cache                (0)
#endif

#ifdef CONFIG_PCAL6408A_REGISTERS_WRITE_VALIDATE_ENABLE
#define __enable_pcal6408a_check_after_write    (1)
#else
#define __enable_pcal6408a_check_after_write    (0)
#endif

/* --------------------------------------------------------------------------- *
 * definitions and macros
 * --------------------------------------------------------------------------- *
 */
#define __ioexp_port_pins_counts                (8)

#define __ioexp_reg_reset_output_port                   (0xff)
#define __ioexp_reg_reset_polarity_inversion            (0x00)
#define __ioexp_reg_reset_configuration                 (0xff)
#define __ioexp_reg_reset_output_drive_strength_0       (0xff)
#define __ioexp_reg_reset_output_drive_strength_1       (0xff)
#define __ioexp_reg_reset_input_latch                   (0x00)
#define __ioexp_reg_reset_pull_up_down_enable           (0x00)
#define __ioexp_reg_reset_pull_up_down_selection        (0xff)
#define __ioexp_reg_reset_interrupt_mask                (0xff)
#define __ioexp_reg_reset_output_port_config            (0x00)

#define __pin_r(_reg_name, _pin_num) \
    ioexp_read_pin_val(__concat(__ioexp_reg_, _reg_name), _pin_num)
#define __pin_w(_reg_name, _pin_num, _bool_val) \
    ioexp_write_pin_val(__concat(__ioexp_reg_, _reg_name), _pin_num, _bool_val)

/* --------------------------------------------------------------------------- *
 * types
 * --------------------------------------------------------------------------- *
 */
typedef enum _ioexp_reg_addr_e {
    // register                             command     r/w bytes   PUP default
    __ioexp_reg_input_port                  = 0x00, //  r   byte    xxxx xxxx
    __ioexp_reg_output_port                 = 0x01, //  rw  byte    1111 1111
    __ioexp_reg_polarity_inversion          = 0x02, //  rw  byte    0000 0000
    __ioexp_reg_configuration               = 0x03, //  rw  byte    1111 1111
    __ioexp_reg_output_drive_strength_0     = 0x40, //  rw  byte    1111 1111
    __ioexp_reg_output_drive_strength_1     = 0x41, //  rw  byte    1111 1111
    __ioexp_reg_input_latch                 = 0x42, //  rw  byte    0000 0000
    __ioexp_reg_pull_up_down_enable         = 0x43, //  rw  byte    0000 0000
    __ioexp_reg_pull_up_down_selection      = 0x44, //  rw  byte    1111 1111
    __ioexp_reg_interrupt_mask              = 0x45, //  rw  byte    1111 1111
    __ioexp_reg_interrupt_status            = 0x46, //  r   byte    0000 0000
    __ioexp_reg_output_port_config          = 0x4F, //  rw  byte    0000 0000
} ioexp_reg_addr_et;

static struct _ioexp_port_pin_config_s {
    uint32_t io_selection   : 1;    //  0: select as output pin
                                    //  1: select as high impedance input

    // polarity inversion works only with pin configured as input
    uint32_t polarity_inv   : 1;    // 0: non-invered input  1: inverted input

    uint32_t drv_strength   : 2;    // 00 : 0.25 x drive capability of the I/O
                                    // 01 : 0.50 x drive capability of the I/O
                                    // 10 : 0.75 x drive capability of the I/O
                                    // 11 : 1.00 x drive capability of the I/O
    
    /**
     * the following config is effective only when the input pin
     * [ non-latched input ]:
     *    - a state change in the corresponding input pin generates an interrupt
     *    - a read of the input port register clears the interrupt
     *    - if the input goes back to its intial logic state before the input
     *      port register is read, then the interrupt is cleared
     * [ latched input ]:
     *    - a change of state of the input generates an interrupt and the input
     *      logic value is loaded into the corresponding bit of the input reg
     *    - a read of the input port register clears the interrupt
     *    - If the input pin returns to its initial logic state before the input
     *      port register is read, then the interrupt is not cleared and the
     *      corresponding bit of the input port register keeps the logic value
     *      that initiated the interrupt.
     */
    uint32_t latched        : 1;    // 0: input state is not latched
                                    // 1: input state is latched

    uint32_t pull_enable    : 1;    // 0: pull resistor disabled
                                    // 1: pull resistor enabled

    uint32_t pull_selection : 1;    // 0: select pull down 100 Kohm resistor
                                    // 1: select pull up 100 Kohm resistor

    uint32_t interrupt_en   : 1;    // 0: interrupt enabled for this pin
                                    // 1: interrupt disabled for this pin
    
    uint32_t is_configured  : 1;    // to mark this pin as already configured

    uint32_t is_active      : 1;    // to mark this pin is in active mode

    pcal6408a_pin_interrupt_handler_t  p_interrupt_handler;

    const char*  name;              // user name for debugging purposes

} s_ioexp_port_pins_config[__ioexp_port_pins_counts];

/* --------------------------------------------------------------------------- *
 * local data structs
 * --------------------------------------------------------------------------- *
 */
static bool s_is_output_open_drain_enabled = false;

static pcal6408a_i2c_read_port_t s_p_i2c_read;
static pcal6408a_i2c_write_port_t s_p_i2c_write;

static uint8_t s_addr_pin_logic_level = 0;
#define __ioexp_address     (uint8_t)( 0x20u | (s_addr_pin_logic_level) )

/* --------------------------------------------------------------------------- *
 * local functions declarations
 * --------------------------------------------------------------------------- *
 */
static struct _ioexp_port_pin_config_s* get_pin_config_from_handle(
                pcal6408a_port_pin_handle handle );
static uint8_t ioexp_read_reg(ioexp_reg_addr_et reg);
static void ioexp_write_reg(ioexp_reg_addr_et reg, uint8_t value);
static bool ioexp_read_pin_val(ioexp_reg_addr_et reg, uint8_t pin);
static void ioexp_write_pin_val(ioexp_reg_addr_et reg, uint8_t pin, bool value);
static uint8_t drive_strength_read(uint8_t pin_num);
static void drive_strength_write(uint8_t pin_num, uint8_t val);
static bool pcal6408a_is_Active(struct _ioexp_port_pin_config_s * p_cfg,
    uint8_t pin);

/* --------------------------------------------------------------------------- *
 * public API definitions
 * --------------------------------------------------------------------------- *
 */
pcal6408a_error_t pcal6408a_init(pcal6408a_init_config_t * p_init_struct)
{
    struct _ioexp_port_pin_config_s * p_cfg;

    // store the open drain output pin types
    s_is_output_open_drain_enabled = p_init_struct->is_open_drain_output;

    // store the i2c r/w methods
    s_p_i2c_read = p_init_struct->i2c_read;
    s_p_i2c_write = p_init_struct->i2c_write;

    // set the logic level of the addr pin
    s_addr_pin_logic_level = p_init_struct->is_addr_pin_connected_to_vdd;

    // load the current values from the PCAL6408A registers
    #define __read_reg(_r) \
        uint8_t _r =  ioexp_read_reg(__concat(__ioexp_reg_, _r))
    __read_reg(polarity_inversion);
    __read_reg(configuration);
    __read_reg(output_drive_strength_0);
    __read_reg(output_drive_strength_1);
    __read_reg(input_latch);
    __read_reg(pull_up_down_enable);
    __read_reg(pull_up_down_selection);
    __read_reg(interrupt_mask);
    #undef __read_reg

    uint16_t drv_strength = ((uint16_t)output_drive_strength_1 << 8) |
        (uint16_t)output_drive_strength_0;

    int i;
    p_cfg = s_ioexp_port_pins_config;
    for(i = 0; i < __ioexp_port_pins_counts; ++i)
    {
        p_cfg->io_selection   = __bitwise_bit_get(8, configuration, i);
        p_cfg->drv_strength   = __bitwise_bits_read(16, drv_strength, 3u, i<<1);
        p_cfg->interrupt_en   = __bitwise_bit_get(8, interrupt_mask, i);
        p_cfg->latched        = __bitwise_bit_get(8, input_latch, i);
        p_cfg->polarity_inv   = __bitwise_bit_get(8, polarity_inversion, i);
        p_cfg->pull_enable    = __bitwise_bit_get(8, pull_up_down_enable, i);
        p_cfg->pull_selection = __bitwise_bit_get(8, pull_up_down_selection, i);

        p_cfg->is_active     = 0;
        p_cfg->p_interrupt_handler = NULL;
        ++ p_cfg;
    }

    // configure device output pins open drain selection
    ioexp_write_reg(
        __ioexp_reg_output_port_config,
        s_is_output_open_drain_enabled);
    
    return __pcal6408a_ok;
}

void pcal6408a_reset(void)
{
    __log_info("reset pcal6408a internal registers to default power-up values");
    __log_warn("all previous configs are cleared and need to be init again");

    memset(s_ioexp_port_pins_config, 0, sizeof(s_ioexp_port_pins_config));

    s_is_output_open_drain_enabled = false;

    #define __reset_reg(_reg)                           \
        ioexp_write_reg(__concat(__ioexp_reg_, _reg),   \
            __concat(__ioexp_reg_reset_, _reg))
    
    __reset_reg(interrupt_mask);        // disable all incoming interrupts first
    __reset_reg(pull_up_down_enable);   // disconnect all pull resistors
    __reset_reg(configuration);         // configure all as inputs
    __reset_reg(output_port);
    __reset_reg(polarity_inversion);
    __reset_reg(output_drive_strength_0);
    __reset_reg(output_drive_strength_1);
    __reset_reg(input_latch);
    __reset_reg(pull_up_down_selection);
    __reset_reg(output_port_config);

    #undef __reset_reg
}

void pcal6408a_interrupt_trigger_port(void)
{
    __log_debug("interrupt triggered");
    // -- read the interrupt status register
    uint8_t int_status = ioexp_read_reg( __ioexp_reg_interrupt_status );

    // -- read the input register
    uint8_t input_values = ioexp_read_reg( __ioexp_reg_input_port );

    // -- § loop for all initialized port pins pin
    //    § call the configured handler to the responsible compoent
    int pins_count = __ioexp_port_pins_counts;
    uint8_t mask = 1u;
    struct _ioexp_port_pin_config_s *p_pin_config = s_ioexp_port_pins_config;
    while( pins_count -- )
    {
        if(mask & int_status)
        {
            __log_assert(p_pin_config->is_configured,
                "non-configured ioexp pin interrupt");
            __log_assert(p_pin_config->io_selection,
                "non-configured ioexp output pin");
            __log_assert(p_pin_config->is_active,
                "non-active ioexp output pin");
            __log_debug("signal interrupt [ %-20s ] [sig-handler-addr: %p]",
                p_pin_config->name ? p_pin_config->name : "",
                p_pin_config->p_interrupt_handler);

            if( p_pin_config->p_interrupt_handler )
            {
                // __log_enforce("- h irq");
                p_pin_config->p_interrupt_handler( (input_values & mask) != 0 );
            }
        }
        mask <<= 1u;
        ++ p_pin_config;
    }
    __log_debug("interrupt triggered -- DONE!!");
}

static pcal6408a_port_pin_handle pcal6408a_configure_io_pin(
    bool                        is_input,
    pcal6408a_port_pin_t        pin_num,  // 0 ~ 7
    pcal6408a_pull_resistor_et  pull_resistor,
    bool                        is_logic_inverted,
    bool                        is_latched,
    pcal6408a_drive_capability_et drive_capability,
    pcal6408a_pin_interrupt_handler_t p_interrupt_handler,
    const char* name
){
    struct _ioexp_port_pin_config_s * p_pin_config;

    p_pin_config = & s_ioexp_port_pins_config[pin_num];

    // store the user required configuration
    p_pin_config->io_selection  = is_input;
    p_pin_config->latched       = is_latched;
    p_pin_config->polarity_inv  = is_logic_inverted;
    p_pin_config->name          = name != NULL ? name : "anonymous-pin";

    if(p_interrupt_handler != NULL) {
        p_pin_config->p_interrupt_handler = p_interrupt_handler;
        p_pin_config->interrupt_en = 0;
    }

    p_pin_config->pull_enable = pull_resistor != pcal6408a_pull_resistor_none;
    if( p_pin_config->pull_enable ) {
        p_pin_config->pull_selection =
            pull_resistor == pcal6408a_pull_resistor_up;
    }
    p_pin_config->drv_strength = drive_capability;

    p_pin_config->is_configured = true;

    p_pin_config->is_active = pcal6408a_is_Active(p_pin_config, pin_num);

    return (pcal6408a_port_pin_handle) p_pin_config;
}

pcal6408a_port_pin_handle pcal6408a_configure_input_port_pin(
    pcal6408a_port_pin_t    pin_num,  // 0 ~ 7
    pcal6408a_pull_resistor_et pull_resistor,
    bool        is_logic_inverted,
    bool        is_latched,
    pcal6408a_pin_interrupt_handler_t p_interrupt_handler,
    const char* name
) {
    __log_debug("config pin [ %-15s ] [input ] [pull: %s] "
               "[latched: %d] [inverted: %d]",
        name ? name : "",
        pull_resistor == pcal6408a_pull_resistor_up ?       " up " :
            pull_resistor == pcal6408a_pull_resistor_down ? "down" : "none",
        is_latched,
        is_logic_inverted
        );
    return pcal6408a_configure_io_pin(
        true, pin_num, pull_resistor, is_logic_inverted, is_latched,
        pcal6408a_drive_capability_level_0, p_interrupt_handler, name
    );
}

pcal6408a_port_pin_handle pcal6408a_configure_output_port_pin(
    pcal6408a_port_pin_t        pin_num,
    pcal6408a_pull_resistor_et  pull_resistor,
    pcal6408a_drive_capability_et   drive_capability,
    const char* name
) {
    __log_debug("config pin [ %-15s ] [output] [pull: %s] [out drv level: %d]",
        name ? name : "",
        pull_resistor == pcal6408a_pull_resistor_up ?       " up " :
            pull_resistor == pcal6408a_pull_resistor_down ? "down" : "none",
        drive_capability
        );
    return pcal6408a_configure_io_pin(
        false, pin_num, pull_resistor, false, false,
        drive_capability, NULL, name
    );
}

static bool pcal6408a_is_Active(
    struct _ioexp_port_pin_config_s * p_cfg,
    uint8_t pin)
{
    // -- check if all pin configuration is applied or not
    uint8_t config = __pin_r(configuration, pin);

    if(p_cfg->io_selection && config)
    {
        // -- input pin config apply check
        return 
            __pin_r(interrupt_mask, pin) == p_cfg->interrupt_en &&
            __pin_r(input_latch, pin) == p_cfg->latched &&
            __pin_r(polarity_inversion, pin) == p_cfg->polarity_inv &&
            __pin_r(pull_up_down_enable, pin) == p_cfg->pull_enable &&
            __pin_r(pull_up_down_selection, pin) == p_cfg->pull_selection;
    }
    else if(!p_cfg->io_selection && !config)
    {
        // -- output pin config apply check
        return (
            // -- for open-drain output, the pull resistors are disconnected
            s_is_output_open_drain_enabled ||
            // -- for push-pull output, the resistor configs shall match
            (
                __pin_r(pull_up_down_enable, pin) == p_cfg->pull_enable &&
                __pin_r(pull_up_down_selection, pin) == p_cfg->pull_selection &&
                drive_strength_read(pin) == p_cfg->drv_strength
            )
        );
    }

    return false;
}

pcal6408a_error_t pcal6408a_activate_pin(pcal6408a_port_pin_handle handle)
{
    struct _ioexp_port_pin_config_s * p_pin_config = 
        get_pin_config_from_handle(handle);
    if( ! p_pin_config )
        return __pcal6408a_bad_handle;
    if( ! p_pin_config->is_configured )
        return __pcal6408a_non_initialized_handle;
    
    uint8_t pin_num = p_pin_config - s_ioexp_port_pins_config;

    if(p_pin_config->is_active)
    {
        __log_info("pin %d (%s) is already activated",
            pin_num, p_pin_config->name);
        return __pcal6408a_ok;
    }

    __log_info("activate pin (%s) %d", p_pin_config->name, pin_num);

    __pin_w(configuration, pin_num, p_pin_config->io_selection);

    if(p_pin_config->io_selection) {
        // -- latch config
        __pin_w(input_latch, pin_num, p_pin_config->latched);

        // -- polarity inversion configuration
        __pin_w(polarity_inversion, pin_num, p_pin_config->polarity_inv);
    }

    /**
     * because the pull resistors are disconnected in case of open drain
     * output, it will be configured in case of open drain output is disabled
     */
    if( p_pin_config->io_selection ||  !s_is_output_open_drain_enabled ) {
        // -- configure pull resistor
        if(p_pin_config->pull_enable) {
            __pin_w(pull_up_down_selection, pin_num,
                p_pin_config->pull_selection);
        }

        __pin_w(pull_up_down_enable, pin_num, p_pin_config->pull_enable);
    }

    /** configuring drive strength for output pins */
    if( !p_pin_config->io_selection ) {
        // configure drive strength
        drive_strength_write(pin_num, p_pin_config->drv_strength);
    }

    // -- configure interrupt
    __pin_w(interrupt_mask, pin_num, p_pin_config->interrupt_en);
    
    p_pin_config->is_active = true;

    return __pcal6408a_ok;
}

pcal6408a_error_t pcal6408a_deactivate_pin(pcal6408a_port_pin_handle handle)
{
    struct _ioexp_port_pin_config_s * p_pin_config = 
        get_pin_config_from_handle(handle);
    if( ! p_pin_config )
        return __pcal6408a_bad_handle;
    if( ! p_pin_config->is_configured )
        return __pcal6408a_non_initialized_handle;
    
    uint8_t pin_num = p_pin_config - s_ioexp_port_pins_config;

    if( !p_pin_config->is_active )
    {
        __log_info("pin %d (%s) is already deactivated",
            pin_num, p_pin_config->name);
        return __pcal6408a_ok;
    }

    __log_info("deactivate pin (%s) %d", p_pin_config->name, pin_num);

    #define __reset_pin(_reg)                                           \
        ioexp_write_pin_val(__concat(__ioexp_reg_, _reg), pin_num,      \
            (__concat(__ioexp_reg_reset_, _reg) & (1u << pin_num)) != 0)

    __reset_pin(interrupt_mask);
    __reset_pin(pull_up_down_enable);
    __reset_pin(configuration);
    __reset_pin(output_port);
    __reset_pin(polarity_inversion);
    __reset_pin(input_latch);
    __reset_pin(pull_up_down_selection);

    if( !p_pin_config->io_selection ) {
        // reset drive strength
        uint16_t reset_value =
            ((uint16_t)__ioexp_reg_reset_output_drive_strength_1 << 8) |
            (uint16_t)__ioexp_reg_reset_output_drive_strength_0;
        
        drive_strength_write(pin_num,
            __bitwise_bits_read(16, reset_value, 3u, pin_num << 1) );
    }

    #undef __reset_pin

    p_pin_config->is_active = false;

    return __pcal6408a_ok;
}

pcal6408a_error_t pcal6408a_read(
    pcal6408a_port_pin_handle   handle,
    bool * p_read_logic_level
    )
{
    struct _ioexp_port_pin_config_s * p_pin_config = 
        get_pin_config_from_handle(handle);
    if( ! p_pin_config )
        return __pcal6408a_bad_handle;
    if( ! p_pin_config->is_configured )
        return __pcal6408a_non_initialized_handle;
    if( ! p_pin_config->is_active )
        return __pcal6408a_pin_inactive;

    uint8_t pin_num = p_pin_config - s_ioexp_port_pins_config;

    *p_read_logic_level = ioexp_read_pin_val(
        p_pin_config->io_selection
            ? __ioexp_reg_input_port
            : __ioexp_reg_output_port,
        pin_num);

    __log_debug("pcal6408a sig [ %-15s ] [ R: %d ]",
        p_pin_config->name ? p_pin_config->name : "",
        *p_read_logic_level);

    return __pcal6408a_ok;
}

pcal6408a_error_t pcal6408a_write(
    pcal6408a_port_pin_handle   handle,
    bool logic_level
    )
{
    struct _ioexp_port_pin_config_s * p_pin_config = 
        get_pin_config_from_handle(handle);
    if( ! p_pin_config )
        return __pcal6408a_bad_handle;
    if( ! p_pin_config->is_configured )
        return __pcal6408a_non_initialized_handle;
    if( ! p_pin_config->is_active )
        return __pcal6408a_pin_inactive;
    if( p_pin_config->io_selection )
        return __pcal6408a_write_to_input_pin;

    uint8_t pin_num = p_pin_config - s_ioexp_port_pins_config;

    ioexp_write_pin_val( __ioexp_reg_output_port, pin_num, logic_level );

    #if __enable_pcal6408a_check_after_write
    bool check = ioexp_read_pin_val( __ioexp_reg_output_port, pin_num);

    __log_debug("pcal6408a sig [ %-15s ] [ W: %d ] [ V: %s ]",
        p_pin_config->name ? p_pin_config->name : "",
        logic_level,
        check == logic_level
            ? __green__ "PASS" __default__
            : __red__   "FAIL" __default__ );
    #endif

    return __pcal6408a_ok;
}

void pcal6408a_config_open_drain_output(bool enable)
{
    s_is_output_open_drain_enabled = enable;

    // -- update the output port configuration
    ioexp_write_reg(__ioexp_reg_output_port_config, enable);
}

/* --------------------------------------------------------------------------- *
 * private methods definitions
 * --------------------------------------------------------------------------- *
 */
static struct _ioexp_port_pin_config_s* get_pin_config_from_handle(
    pcal6408a_port_pin_handle handle )
{
    struct _ioexp_port_pin_config_s* p_pin_config = handle;
    if( p_pin_config < s_ioexp_port_pins_config
        || p_pin_config >= s_ioexp_port_pins_config + __ioexp_port_pins_counts)
    {
        __log_ptr(p_pin_config);
        __log_ptr(s_ioexp_port_pins_config);
        return NULL;
    }
    return p_pin_config;
}

static uint8_t drive_strength_read(uint8_t pin_num)
{
    uint8_t addr = __ioexp_reg_output_drive_strength_0 + (pin_num >= 4);
    uint8_t idx = pin_num & 3u;
    uint8_t val = ioexp_read_reg(addr);
    return __bitwise_bits_read(8, val, 3u, idx << 1);
}

static void drive_strength_write(uint8_t pin_num, uint8_t val)
{
    uint8_t addr = __ioexp_reg_output_drive_strength_0 + (pin_num >= 4);
    uint8_t idx = pin_num & 3u;
    uint8_t reg_val = ioexp_read_reg(addr);
    __bitwise_bits_write(8, reg_val, 3u, idx << 1, val);
    ioexp_write_reg(addr, reg_val);
}

/* --------------------------------------------------------------------------- *
 * registers caching and read/write methods
 * --------------------------------------------------------------------------- *
 */
#if __enable_pcal6408a_cache
/**
 * § This array represents a cache for the ioexpander registers to save i2c
 *   read and write cycle for configuration registers
 * § the register __ioexp_reg_input_port and __ioexp_reg_interrupt_status
 *   are excluded from the cache and always read when requested.
 */
static struct _reg_cache_s {
    const char* reg_name;
    uint8_t addr;
    uint8_t valid;
    uint8_t cache_value;
} s_reg_cache[] = {
    {"output_port",             __ioexp_reg_output_port,             false, 0},
    {"polarity_inversion",      __ioexp_reg_polarity_inversion,      false, 0},
    {"configuration",           __ioexp_reg_configuration,           false, 0},
    {"output_drive_strength_0", __ioexp_reg_output_drive_strength_0, false, 0},
    {"output_drive_strength_1", __ioexp_reg_output_drive_strength_1, false, 0},
    {"input_latch",             __ioexp_reg_input_latch,             false, 0},
    {"pull_up_down_enable",     __ioexp_reg_pull_up_down_enable,     false, 0},
    {"pull_up_down_selection",  __ioexp_reg_pull_up_down_selection,  false, 0},
    {"interrupt_mask",          __ioexp_reg_interrupt_mask,          false, 0},
    {"output_port_config",      __ioexp_reg_output_port_config,      false, 0},
};
#endif

static bool ioexp_read_pin_val(ioexp_reg_addr_et reg, uint8_t pin)
{
    uint8_t reg_val = ioexp_read_reg(reg);
    return __bitwise_bit_get(8, reg_val, pin);
}

static void ioexp_write_pin_val(ioexp_reg_addr_et reg, uint8_t pin, bool value)
{
    uint8_t reg_value = ioexp_read_reg(reg);

    // write new value if it is not already there
    if( __bitwise_bit_get(8, reg_value, pin) != value )
    {
        if( value == true )
            __bitwise_bit_set(8, reg_value, pin);
        else
            __bitwise_bit_clr(8, reg_value, pin);

        ioexp_write_reg(reg, reg_value);
    }
}

#define __ioexp_read_reg(_reg, u8_var)                                  \
    do {                                                                \
        /* perform a write cycle on the i2c bus to identify on the */   \
        /* device which register is to be read in the next cycle */     \
        u8_var = _reg;                                                  \
        s_p_i2c_write(__ioexp_address, &u8_var, 1);                     \
        /* perform a read cycle on the i2c bus to get the required */   \
        /* device reg value */                                          \
        u8_var = 0;                                                     \
        s_p_i2c_read(__ioexp_address, &u8_var, 1);                      \
    } while(0)

static uint8_t ioexp_read_reg(ioexp_reg_addr_et reg)
{
    #if __enable_pcal6408a_cache

    uint8_t reg_value;

    if(reg == __ioexp_reg_input_port || reg == __ioexp_reg_interrupt_status)
    {
        __ioexp_read_reg(reg, reg_value);
        __log_debug("-->i2c read [addr:%02x, val:%02x] "__yellow__"%s",
            reg, reg_value,
            reg == __ioexp_reg_input_port ? "input port" : "interrupt status");
        return reg_value;
    }

    struct _reg_cache_s* p_cache = s_reg_cache;
    int i;
    for(i = 0; i < sizeof(s_reg_cache)/sizeof(s_reg_cache[0]); ++i, ++p_cache)
    {
        if(p_cache->addr == reg)
        {
            // -- input port and interrupt are always invalid,
            //    shall be read every time
            if(!p_cache->valid || reg == __ioexp_reg_input_port ||
                reg == __ioexp_reg_interrupt_status)
            {
                __ioexp_read_reg(reg, reg_value);

                // update cache valid bit and value
                p_cache->valid = true;
                p_cache->cache_value = reg_value;

                __log_debug("-->i2c read [addr:%02x, val:%02x] "__yellow__"%s",
                    reg, reg_value, p_cache->reg_name);
            }
            else if(p_cache->valid)
            {
                reg_value = p_cache->cache_value;
            }

            return reg_value;
        }
    }

    __log_error("invalid ioexp reg address: %02x", reg);
    return 0;

    #else

    uint8_t reg_val;
    __ioexp_read_reg(reg, reg_val);
    return reg_val;

    #endif
}

static void ioexp_write_reg(ioexp_reg_addr_et reg, uint8_t value)
{
    uint8_t buff [] = { reg, value };

    #if __enable_pcal6408a_cache

    if(reg == __ioexp_reg_input_port || reg == __ioexp_reg_interrupt_status)
    {
        __log_debug("-->i2c write[addr:%02x, val:%02x] "__yellow__"%s",
            reg, value,
            reg == __ioexp_reg_input_port ? "input port" : "interrupt status");
        s_p_i2c_write(__ioexp_address, buff, 2);
        return;
    }

    // locate the cache reference of this register
    struct _reg_cache_s* p_cache = s_reg_cache;
    int i;
    for(i = 0; i < sizeof(s_reg_cache)/sizeof(s_reg_cache[0]); ++i, ++p_cache)
    {
        if(p_cache->addr == reg)
        {
            if( p_cache->valid && p_cache->cache_value == value )
            {
                // the current reg value equals the required value
                // hence no need to update the reg value
                return;
            }
            goto start_write;
        }
    }
    __log_error("invalid ioexp reg address: %02x", reg);
    return;

    start_write:

    __log_debug("-->i2c write[addr:%02x, val:%02x] "__yellow__"%s",
        reg, value, p_cache->reg_name);

    #endif

    s_p_i2c_write(__ioexp_address, buff, 2);

    #if __enable_pcal6408a_cache
    p_cache->cache_value = value;
    p_cache->valid = true;
    #endif
}

/* --------------------------------------------------------------------------- *
 * debug and stats methods
 * --------------------------------------------------------------------------- *
 */
#if __enable_pcal6408a_cache
static void pcal6408a_cache_validate(void)
{
    struct _reg_cache_s* p_cache = s_reg_cache;
    
    #define __w_reg_name            25
    #define __w_addr                5
    #define __w_physical            9
    #define __w_cached              8
    #define __w_valid               6
    #define __w_verdict             8

    #define __log_col_header(_col)  \
        __log_output("%-"__stringify(__concat(__w_, _col))"s", #_col)
    
    #define __h_fmt_reg_name        "%-"__stringify(__w_reg_name)

    __log_col_header(reg_name);
    __log_col_header(addr);
    __log_col_header(physical);
    __log_col_header(cached);
    __log_col_header(valid);
    __log_col_header(verdict);
    __log_output("\n");

    int i;
    for(i = 0; i < sizeof(s_reg_cache)/sizeof(s_reg_cache[0]); ++i, ++p_cache)
    {
        if(p_cache->addr == __ioexp_reg_input_port ||
            p_cache->addr == __ioexp_reg_interrupt_status)
        {
            /* cache values for those two registers are ignored */
            continue;
        }
        uint8_t read_value;
        __ioexp_read_reg(p_cache->addr, read_value);

        __log_output(__blue__"%-"__stringify(__w_reg_name)"s"__default__,
            p_cache->reg_name);

        __log_output("%02x", p_cache->addr);
        __log_output_fill(__w_addr - 2 , ' ', false);

        __log_output("%02x", read_value);
        __log_output_fill(__w_physical - 2 , ' ', false);

        __log_output("%02x", p_cache->cache_value);
        __log_output_fill(__w_cached - 2 , ' ', false);

        __log_output("%d", p_cache->valid);
        __log_output_fill(__w_valid - 1 , ' ', false);

        __log_output("%s\n",
            p_cache->valid && p_cache->cache_value == read_value
                ? __green__"VALID" : __red__"INVALID");
    }
}
#endif

void pcal6408a_stats(void)
{
    struct _ioexp_port_pin_config_s* p_cfg = s_ioexp_port_pins_config;

    #define __cap_w  65
    __log_output_header("current ioexp pin configurations", __cap_w, '=');
    __log_output("pin io.  cfg. act. pinv drv  latch pull type int  signal\n");

    const char* true_false[] = {__red__"F", __green__"T"};
    const char* up_down[] = {__red__"down", __green__"up"};

    #define __drive_capability  3.3f

    int i;
    for(i = 0; i < __ioexp_port_pins_counts; ++i, ++p_cfg)
    {
        __log_output("(%d) ", i);
        __log_output("%-5s"__default__, p_cfg->io_selection
            ? __yellow__"IN" : __cyan__"OUT");
        __log_output("%-+4s "__default__, true_false[p_cfg->is_configured]);
        __log_output("%-+4s "__default__, true_false[p_cfg->is_active]);
        __log_output("%-+4s "__default__, true_false[p_cfg->polarity_inv]);
        __log_output("%-4.2fv"__default__,
            (p_cfg->drv_strength + 1) * 0.25f * __drive_capability);
        __log_output("%-+5s "__default__, true_false[p_cfg->latched]);
        __log_output("%-+4s "__default__, true_false[p_cfg->pull_enable]);
        __log_output("%-+4s "__default__, up_down[p_cfg->pull_selection]);
        __log_output("%-+4s "__default__, true_false[!p_cfg->interrupt_en]);
        __log_output("%s"__default__"\n", p_cfg->name ? p_cfg->name :
            p_cfg->is_configured ? __red__"unknown":__cyan__"-- unused --");
    }

    __log_output("output ports configuration: "__yellow__"%s\n",
        s_is_output_open_drain_enabled ? "open-drain" : "push-pull");

    #if __enable_pcal6408a_cache
    __log_output_header("current ioexp cache contents", __cap_w, '=');
    pcal6408a_cache_validate();
    #endif
    __log_output_fill(__cap_w, '=', true);
    #undef __cap_w
}

/* --- end of file ---------------------------------------------------------- */
