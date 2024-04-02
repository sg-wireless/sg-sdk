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
 * @brief   This file represents a uPython test module for chained buffer
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_ADT_BUFFERS_CHAIN_DEMO_EXAMPLE_ENABLE

#include "mp_lite_if.h"
#include "buffers_chain.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"


/** -------------------------------------------------------------------------- *
 * demo implementation
 * --------------------------------------------------------------------------- *
 */

static SemaphoreHandle_t access_mutex;
static SemaphoreHandle_t rx_sync_sem;
static SemaphoreHandle_t tx_sync_sem;
static uint8_t read_buf[256];


static void buf_chain_access_lock(void){
    xSemaphoreTake(access_mutex, portMAX_DELAY);
}
static void buf_chain_access_unlock(void){
    xSemaphoreGive(access_mutex);
}

static void buf_chain_rx_sync_wait(void* obj){
    xSemaphoreTake(rx_sync_sem, portMAX_DELAY);
}
static void buf_chain_rx_sync_signal(void* obj){
    xSemaphoreGive(rx_sync_sem);
}

static void buf_chain_tx_sync_wait(void* obj){
    xSemaphoreTake(tx_sync_sem, portMAX_DELAY);
}
static void buf_chain_tx_sync_signal(void* obj){
    xSemaphoreGive(tx_sync_sem);
}

__buf_chain_def(rx_chain, 0, buf_chain_rx_sync_wait, buf_chain_rx_sync_signal);
__buf_chain_def(tx_chain, 0, buf_chain_tx_sync_wait, buf_chain_tx_sync_signal);

__buf_chain_mem_def(test_buf_mem, 128, 4, buf_chain_access_lock,
    buf_chain_access_unlock);

__mp_mod_init(buf_chain)(void) {

    access_mutex = xSemaphoreCreateMutex();
    rx_sync_sem = xSemaphoreCreateBinary();
    tx_sync_sem = xSemaphoreCreateBinary();

    __buf_chain_connect(test_buf_mem, rx_chain);
    __buf_chain_connect(test_buf_mem, tx_chain);

    return mp_const_none;
}

__mp_mod_ifdef(buf_chain, CONFIG_SDK_ADT_BUFFERS_CHAIN_DEMO_EXAMPLE_ENABLE);

__mp_mod_fun_1(buf_chain, rx_write)(mp_obj_t obj) {

    mp_buffer_info_t buf = {0};

    mp_get_buffer_raise(obj, &buf, MP_BUFFER_READ);

    __buf_chain_write(rx_chain, buf.buf, buf.len, false);

    return mp_const_none;
}
__mp_mod_fun_0(buf_chain, rx_read)(void) {

    uint32_t len;
    __buf_chain_read(rx_chain, read_buf, &len, false);

    return mp_obj_new_bytearray( len, read_buf );
}

__mp_mod_fun_1(buf_chain, tx_write)(mp_obj_t obj) {

    mp_buffer_info_t buf = {0};

    mp_get_buffer_raise(obj, &buf, MP_BUFFER_READ);

    __buf_chain_write(tx_chain, buf.buf, buf.len, false);

    return mp_const_none;
}
__mp_mod_fun_0(buf_chain, tx_read)(void) {

    uint32_t len;
    __buf_chain_read(tx_chain, read_buf, &len, false);

    return mp_obj_new_bytearray( len, read_buf );
}

/* --- end of file ---------------------------------------------------------- */
#endif /* CONFIG_SDK_ADT_BUFFERS_CHAIN_DEMO_EXAMPLE_ENABLE */
