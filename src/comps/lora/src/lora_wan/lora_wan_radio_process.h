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
 * @brief   lora-wan radio events processing sub-component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_WAN_RADIO_PROCESS_H__
#define __LORA_WAN_RADIO_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif
/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief initializes the lora radio processing
 */
void lw_radio_process_ctor(void);

/**
 * @brief sets the semteck lora-stack rx window system max time error to be
 *  considered while applying the induced rx-window calibration parameters
 *  such as time-shift and window-time extension
 * @param sys_time_err the semteck lora-stack system max time error
 */
void lw_rxwin_set_sys_time_err(uint32_t sys_time_err);

/**
 * @fn lw_rxwin_calibration_get_time_shift
 * @fn lw_rxwin_calibration_set_time_shift
 * @fn lw_rxwin_calibration_get_time_extension
 * @fn lw_rxwin_calibration_set_time_extension
 * @fn lw_rxwin_calibration_get_enablement
 * @fn lw_rxwin_calibration_set_enablement
 * 
 * @brief responsible for setting/getting the corresponding calibration
 *      configuration parameter such as rx window time-shift, time-extension
 *      and the calibration enablement
 */
int32_t lw_rxwin_calibration_get_time_shift(void);
void    lw_rxwin_calibration_set_time_shift(int32_t time_shift);
int32_t lw_rxwin_calibration_get_time_extension(void);
void    lw_rxwin_calibration_set_time_extension(int32_t time_extension);
bool    lw_rxwin_calibration_get_enablement(void);
void    lw_rxwin_calibration_set_enablement(bool enable);

/**
 * @fn lw_rxwin_set_last_tx_done_timestamp
 * @fn lm_rxwin_get_delay
 * @fn lm_rxwin_set_rx_state
 * @fn lm_rxwin_toggle_debug_verbosity
 * @fn lm_rxwin_time_ctrl_align
 * @fn lw_radio_process_rxwin_timer_expire
 * 
 * @brief these are the methods used to connect the semteck lora-stack with
 *      rxwin fine tuning control in the radio processing module.
 */
void lw_rxwin_set_last_tx_done_timestamp(uint32_t timestamp);
uint32_t lm_rxwin_get_delay(uint32_t win_act_delay);
void lm_rxwin_set_rx_state(int state);
void lm_rxwin_time_ctrl_align(void);
void lw_radio_process_rxwin_timer_expire(
    void (*p_handler)(void*), uint32_t rx_act_delay, int rx_win);

/**
 * @brief to start processing of the occurred radio events
 */
void lw_radio_process_events(void);

void lm_rxwin_toggle_debug_verbosity(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_WAN_RADIO_PROCESS_H__ */
