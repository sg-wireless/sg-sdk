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
 * @brief   This file declare the adaptor specification of the lora-stack
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_PORT_H__
#define __LORA_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- includes ------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>

/* --- Pycom LoRa Stack System Port Definition ------------------------------ */

// -- non-volatile memory (nvm) ports types
typedef bool lora_port_nvm_check_t(const char* key);
typedef int lora_port_nvm_load_t(const char* key, uint8_t* buf, uint32_t size);
typedef int lora_port_nvm_store_t(const char* key, uint8_t* buf, uint32_t size);
typedef int lora_port_nvm_sync_t(void);
typedef int lora_port_nvm_clear_t(const char* key);

// -- system timers ports types
typedef void  lora_port_timer_callback_t(void* arg);
typedef void* lora_port_timer_init_t(const char* name, void* arg,
    lora_port_timer_callback_t *p_callback);
typedef void  lora_port_timer_delete_t(void* handle);
typedef void  lora_port_timer_start_t(void* handle);
typedef void  lora_port_timer_stop_t(void* handle);
typedef void  lora_port_timer_set_period_t(void* handle, uint32_t msec);

// -- system time handling ports types
typedef uint32_t lora_port_get_timestamp_ms_t(void);
typedef void     lora_port_delay_ms_t(uint32_t msec);

// -- critical section accessors ports types
typedef void* lora_port_mutex_new_t(void);
typedef void lora_port_mutex_lock_t(void* handle);
typedef void lora_port_mutex_unlock_t(void* handle);

// -- synchoronization semaphore ports types
typedef void* lora_port_sem_new_t(void);
typedef void lora_port_sem_wait_t(void* handle);
typedef void lora_port_sem_signal_t(void* handle);

// -- crc32 port type
typedef uint32_t lora_port_crc32_calc_t(
    uint32_t initial_crc, uint8_t * buf, uint32_t len);

typedef struct {
    // -- non-volatile memory (nvm)
    lora_port_nvm_check_t * nvm_check;
    lora_port_nvm_load_t  * nvm_load;
    lora_port_nvm_store_t * nvm_store;
    lora_port_nvm_clear_t * nvm_clear;
    lora_port_nvm_sync_t  * nvm_sync;

    // -- system timers
    lora_port_timer_init_t  * timer_init;
    lora_port_timer_delete_t  * timer_delete;
    lora_port_timer_start_t * timer_start;
    lora_port_timer_stop_t  * timer_stop;
    lora_port_timer_set_period_t * timer_set_period;

    // -- system time
    lora_port_get_timestamp_ms_t * get_timestamp_msec;
    lora_port_delay_ms_t         * delay_msec;

    // -- access mutex
    lora_port_mutex_new_t* mutex_new;
    lora_port_mutex_lock_t* mutex_lock;
    lora_port_mutex_unlock_t* mutex_unlock;

    // -- sync sem
    lora_port_sem_new_t* sem_new;
    lora_port_sem_wait_t* sem_wait;
    lora_port_sem_signal_t* sem_signal;

    // -- optional utilities ( optional )
    lora_port_crc32_calc_t * crc32_calc;

} lora_port_params_t;

void lora_port_init(lora_port_params_t * p_init_params);

/**
 * @brief   lora port constructor -- it is a board specific function
 */
void lora_board_ctor(void);

/**
 * @brief   lora port destructor -- it is a board specific function
 */
void lora_board_dtor(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif

#endif /* __LORA_PORT_H__ */
