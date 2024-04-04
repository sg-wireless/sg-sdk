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
 * @brief   This file declare the adaptor specification of the sx126x lora chip
 *          with respect to the lora-stack
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#define __log_subsystem  lora
#define __log_component  stub_sx126x
#include "log_lib.h"

#include "sx126x_port.h"    // -- port definition of the lora-network box
#include "sx126x-board.h"   // -- port declaration of semtch sx126x driver

#include "sx126x_defs.h"
#include "driver_inspector.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include "system/systime.h"
// #include "rx_win.h"

/** -------------------------------------------------------------------------- *
 * board adaptor implementation
 * --------------------------------------------------------------------------- *
 */

static sx126x_port_spi_trx_t *              p_spi_trx;
static sx126x_port_wait_on_busy_signal_t *  p_wait_on_busy_signal;
static sx126x_port_send_reset_pulse_t *     p_send_reset_pulse;
static sx126x_port_int_pin_state_t *        p_get_int_pin_state;

void sx126x_port_init( sx126x_port_t * p_port_params )
{
    extern void BoardCriticalSectionInit(void);
    BoardCriticalSectionInit();
    p_spi_trx = p_port_params->p_spi_trx;
    p_wait_on_busy_signal = p_port_params->p_wait_on_busy_signal;
    p_send_reset_pulse = p_port_params->p_send_reset_pulse;
    p_get_int_pin_state = p_port_params->p_get_int_pin_state;
}

static DioIrqHandler* p_sx126x_drv_irq_handler;
static void(*p_service_level_irq_handler)(void) = NULL;

void sx126x_port_irq( void )
{
    // -- register a timestamp here
    // lora_rxwin_stamp_event(__stamp_event_irq);

    // __log_enforce("- lora port");
    __log_info("lora interrupt ...");

    // -- radio processing
    p_sx126x_drv_irq_handler(NULL);

    // -- lora service app processing
    if(p_service_level_irq_handler)
        p_service_level_irq_handler();
}

void lora_port_set_service_level_irq_handler(void(*p_handler)(void))
{
    p_service_level_irq_handler = p_handler;
}

/** -------------------------------------------------------------------------- *
 * semtech driver ports implementation
 * --------------------------------------------------------------------------- *
 */
void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    lora_log_trx(false, __sx126x_cmd_write_register, address, buffer, size);
    __log_info("sx1262 --> Write regs addr:%Cc%04x%Cd (%Cb%s%Cd), size: %Cg%d%Cd",
        address, lora_port_get_reg_name(address), size);

    SX126xCheckDeviceReady( );

    sx126x_spi_trx_t trx = {
        .command = __sx126x_cmd_write_register,
        .address = address,
        .address_bits = 16,
        .tx_buffer = buffer,
        .size = size,
        .flags = __sx126x_spi_trx_has_address
    };
    p_spi_trx( &trx );

    SX126xWaitOnBusy( );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    __log_info("sx1262 <-- Read regs addr:%Cc%04x%Cd (%Cb%s%Cd), size: %Cg%d%Cd",
        address, lora_port_get_reg_name(address), size);

    SX126xCheckDeviceReady( );

    sx126x_spi_trx_t trx = {
        .command = __sx126x_cmd_read_register,
        .address = address,
        .address_bits = 16,
        .rx_buffer = buffer,
        .size = size,
        .flags = __sx126x_spi_trx_has_address |
                __sx126x_spi_trx_has_dummy_rx_byte1
    };
    p_spi_trx( &trx );
    lora_log_trx(true, __sx126x_cmd_read_register, address, buffer, size);
}

void SX126xWriteCommand(
    RadioCommands_t opcode, uint8_t *buffer, uint16_t size)
{
    lora_log_trx(false, opcode, 0, buffer, size);
    __log_info("sx1262 --> Write Cmd:%Cc%02x%Cd (%Cb%s%Cd), size: %Cg%d%Cd",
        opcode, lora_port_get_cmd_name(opcode), size);

    SX126xCheckDeviceReady( );

    sx126x_spi_trx_t trx = {
        .command = opcode,
        .tx_buffer = buffer,
        .size = size
    };
    p_spi_trx( &trx );

    // if( opcode == __sx126x_cmd_set_rx ) {
    //     lora_rxwin_stamp_event(__stamp_event_rx_req);
    // }

    if( opcode != __sx126x_cmd_set_sleep ) {
        SX126xWaitOnBusy( );
    } else {
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

uint8_t SX126xReadCommand(
    RadioCommands_t opcode, uint8_t *buffer, uint16_t size )
{
    __log_info("sx1262 <-- Read Cmd:%Cc%02x%Cd (%Cb%s%Cd), size: %Cg%d%Cd",
        opcode, lora_port_get_cmd_name(opcode), size);

    SX126xCheckDeviceReady( );

    sx126x_spi_trx_t trx = {
        .command = opcode,
        .rx_buffer = buffer,
        .size = size,
        .flags = __sx126x_spi_trx_has_rx_byte1
    };
    p_spi_trx( &trx );

    // if(opcode == __sx126x_cmd_get_irq_status && (buffer[1] & 1u)) {
    //     lora_rxwin_stamp_event(__stamp_event_tx_done);
    // }

    lora_log_trx(true, opcode, 0, buffer, size);
    return trx.rx_byte1;
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters(address, &value, 1);
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t value;
    SX126xReadRegisters(address, &value, 1);
    return value;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    lora_log_trx(false, __sx126x_cmd_write_buffer, offset, buffer, size);
    __log_info("sx1262 --> Write Buf offset:%Cb%02x%Cd , size: %Cg%d%Cd",
        offset, size);

    SX126xCheckDeviceReady( );

    sx126x_spi_trx_t trx = {
        .command = __sx126x_cmd_write_buffer,
        .address = offset,
        .address_bits = 8,
        .tx_buffer = buffer,
        .size = size,
        .flags = __sx126x_spi_trx_has_address
    };
    p_spi_trx( &trx );
    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    __log_info("sx1262 <-- Read Buf offset:%Cb%02x%Cd , size: %Cg%d%Cd",
        offset, size);

    SX126xCheckDeviceReady( );

    sx126x_spi_trx_t trx = {
        .command = __sx126x_cmd_read_buffer,
        .address = offset,
        .address_bits = 8,
        .rx_buffer = buffer,
        .size = size,
        .flags = __sx126x_spi_trx_has_address |
                __sx126x_spi_trx_has_dummy_rx_byte1
    };
    p_spi_trx( &trx );
    lora_log_trx(true, __sx126x_cmd_read_buffer, offset, buffer, size);
}

void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
    p_sx126x_drv_irq_handler = dioIrq;
}

void SX126xIoTcxoInit( void )
{
    #define __lora_dio3_tcxo_1_6_v  0
    #define __lora_dio3_tcxo_1_7_v  1
    #define __lora_dio3_tcxo_1_8_v  2
    #define __lora_dio3_tcxo_2_2_v  3
    #define __lora_dio3_tcxo_2_4_v  4
    #define __lora_dio3_tcxo_2_7_v  5
    #define __lora_dio3_tcxo_3_0_v  6
    #define __lora_dio3_tcxo_3_3_v  7
    // uint8_t payload [] = {__lora_dio3_tcxo_3_3_v, 0xff, 0, 0};
    uint32_t delay = (uint32_t)(5000.0f / 15.625f);
    uint8_t payload [4];
    payload[0] = __lora_dio3_tcxo_1_7_v;
    payload[1] = (delay >> 16) & 0xff;
    payload[2] = (delay >> 8) & 0xff;
    payload[3] = (delay) & 0xff;

    SX126xWriteCommand(__sx126x_cmd_set_dio_3_as_tcxo_ctrl, payload,
        sizeof(payload));
}

void SX126xIoRfSwitchInit( void )
{
    uint8_t enable = 1;
    SX126xWriteCommand(__sx126x_cmd_set_dio_2_as_rf_switch_ctrl, &enable, 1);
}

void SX126xReset( void )
{
    __log_debug("issue lora reset...");
    p_send_reset_pulse();
    SX126xWaitOnBusy();
}

void SX126xWaitOnBusy( void )
{
    p_wait_on_busy_signal();
}

void SX126xWakeup( void )
{
    __log_debug("wakeup lora chip...");
    __log_debug("sending get status...");

    uint8_t status = 0;
    sx126x_spi_trx_t trx = {
        .command = __sx126x_cmd_get_status,
        .tx_buffer = &status,
        .size = 1,
        .cs_enable_pretrans_ms = 5,
        .flags = __sx126x_spi_trx_has_cs_pretrans_ms
    };
    p_spi_trx( &trx );

    SX126xWaitOnBusy();

    SX126xSetOperatingMode( MODE_STDBY_RC );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetDeviceId( void )
{
    return SX1262;
}

void SX126xAntSwOn( void )
{
    __log_error(" --- not implemented --- ");
}

void SX126xAntSwOff( void )
{
    __log_error(" --- not implemented --- ");
}

uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return 1; // 100 usec from datasheet
}

uint32_t SX126xGetDio1PinState( void )
{
    return p_get_int_pin_state();
}

static RadioOperatingModes_t s_radio_operating_mode;

RadioOperatingModes_t SX126xGetOperatingMode( void )
{
    // __log_info(" <-- getting operating mode (%Cb%s%Cd)",
    //     lora_port_get_operating_mode_name(s_radio_operating_mode));
    return s_radio_operating_mode;
}

void SX126xSetOperatingMode( RadioOperatingModes_t mode )
{
    // __log_info(" --> setting operating mode (%Cb%s%Cd)",
    //     lora_port_get_operating_mode_name(mode));
    s_radio_operating_mode = mode;
}

void BoardGetUniqueId( uint8_t *id ) // -- to return the DevEUI
{
    // uint8_t dev_eui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xAE, 0xEC};
    // uint8_t dev_eui[] = {0x71,0x2a,0xf7,0x44,0x60,0xfa,0x2a,0x8a};

    // memcpy(id, dev_eui, 8);
}

/* --- end of file ---------------------------------------------------------- */
