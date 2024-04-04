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
 * @brief   This file defines some used test probing functions used while
 *          bring-up the Semtech chip sx1262
 *          it was created at the early development of this lora-stack
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "log_lib.h"
#include "ioexp.h"
#include "sx126x-board.h"
#include "sx126x_defs.h"

#include <string.h>

#if 0
/* --- test functions ------------------------------------------------------- */
#define __sizeof_arr(__arr) (sizeof(__arr)/sizeof(__arr[0]))
void test_case_0(void)
{
    uint8_t tx_buf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
    uint8_t rx_buf[sizeof(tx_buf)] = {0};
    SX126xWriteRegisters(__sx126x_reg_freq_r0(0), tx_buf, __sizeof_arr(tx_buf));
    SX126xReadRegisters(__sx126x_reg_freq_r0(0), rx_buf, __sizeof_arr(rx_buf));

    int res = memcmp(tx_buf,rx_buf, sizeof(tx_buf));

    __log_test(0, res == 0, "read/write registers", " __reg_freq_r0");
}
void test_case_1(void)
{
    uint8_t tx_buf[] = {5,6,7,8};
    uint8_t rx_buf[sizeof(tx_buf)] = {0};
    SX126xWriteRegisters(__sx126x_reg_freq_r1(0), tx_buf, __sizeof_arr(tx_buf));
    SX126xReadRegisters(__sx126x_reg_freq_r1(0), rx_buf, __sizeof_arr(rx_buf));

    int res = memcmp(tx_buf,rx_buf, sizeof(tx_buf));

    __log_test(1, res == 0, "read/write registers", " __reg_freq_r1");
}

void test_case_2(void)
{
    uint8_t tx_buf[] = {0,1,2,3,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
        21,22,23,24,25,26,27,28,29,30,31,32,33};
    uint8_t rx_buf[sizeof(tx_buf)] = {0};
    SX126xWriteBuffer(0, tx_buf, __sizeof_arr(tx_buf));
    SX126xReadBuffer(0, rx_buf, __sizeof_arr(rx_buf));

    int res = memcmp(tx_buf,rx_buf, sizeof(tx_buf));

    __log_test(2, res == 0, "read/write buffers", " @offset 0 : size: 21");
}

void test_case_3(void)
{
    uint16_t rx_buf[3] = {0};
    uint8_t status = SX126xReadCommand(__sx126x_cmd_get_stats, (uint8_t*)rx_buf, 6);

    __log_test(3, 1, "get stats", "status: %02x -- [%04x  %04x  %04x]",
        status, rx_buf[0], rx_buf[1], rx_buf[2]);
}
void test_case_4(void)
{
    uint8_t type_in = 2;
    uint8_t type_out = 2;
    SX126xWriteCommand(__sx126x_cmd_set_packet_type, &type_in, 1);
    uint8_t status = SX126xReadCommand(__sx126x_cmd_get_packet_type, &type_out, 1);

    __log_test(4, type_in == type_out, "packet type", " status = %02d", status);
}
static void lora_chip_irq_cb(void* context) {
    __log_debug("callback");
}
void test_case_5(void)
{
    __log_info("testing init irrq and dio1");
    SX126xIoIrqInit(lora_chip_irq_cb);
}
void test_case_6(void)
{
    __log_info("test init RF switch");
    SX126xIoRfSwitchInit();
}
void test_case_7(void)
{
    __log_info("test init TCXO");
    SX126xIoTcxoInit();
}

void lora_port_test_run(void)
{
    test_case_0();
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    test_case_6();
    test_case_7();

    SX126xReset();

    SX126xWakeup();
}

#endif

/* --- end of file ---------------------------------------------------------- */
