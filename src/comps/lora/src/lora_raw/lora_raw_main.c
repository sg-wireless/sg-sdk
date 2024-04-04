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
 * @brief   lora-raw mode main sub-component. it defines the lora mode APIs
 *          for lora-raw mode handling.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdlib.h>

#define __log_subsystem  lora
#define __log_component  raw_api
#include "log_lib.h"

#include "lora.h"
#include "lora_raw_process.h"
#include "lora_raw_radio_if.h"
#include "lora_mode.h"

/** -------------------------------------------------------------------------- *
 * APIs implementations
 * --------------------------------------------------------------------------- *
 */
static lora_error_t lora_raw_ctor(void)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora raw ctor()");
    lora_raw_radio_ctor();
    lora_raw_process_ctor();

    return ret;
}

static lora_error_t lora_raw_dtor(void)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora raw ~dtor()");
    lora_raw_process_dtor();
    lora_raw_radio_dtor();

    return ret;
}

static lora_error_t lora_raw_stats(void)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora raw stats()");
    lora_raw_radio_stats();

    return ret;
}

static lora_error_t lora_raw_tx(lora_tx_params_t * p_tx_params)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora raw send()");

    if(p_tx_params->len && p_tx_params->buf) {
        lora_raw_process_event_payload_t tx_msg = {
            .type = __PROCESS_MSG_PAYLOAD_TX_REQ,
            .tx_payload = {
                .buf = p_tx_params->buf,
                .len = p_tx_params->len,
                .timeout = p_tx_params->timeout
            },
        };
        lora_raw_process_event(__LORA_RAW_PROCESS_TX_REQUEST, &tx_msg,
            p_tx_params->sync);
    }

    return ret;
}

static lora_error_t lora_raw_rx(lora_rx_params_t * p_rx_params)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora raw recv()");

    lora_raw_process_event_payload_t rx_msg = {
        .type = __PROCESS_MSG_PAYLOAD_RX_REQ,
        .rx_payload = {
            .buf = p_rx_params->buf,
            .buf_len = *p_rx_params->p_len,
            .p_len = p_rx_params->p_len,
            .timeout = p_rx_params->timeout
        },
    };

    /* in some cases particularly for sync recv request with timeout:
     * if the timeout expires the requestor will check the received data using
     * the returned length at *p_len, hence, set it to zero now.
     */
    *p_rx_params->p_len = 0;
    lora_raw_process_event(__LORA_RAW_PROCESS_RX_REQUEST, &rx_msg,
        p_rx_params->sync);
    if(p_rx_params->sync) {
        __log_debug("wakeup -> received len: %d", *p_rx_params->p_len);
    }

    return ret;
}

static lora_error_t lora_raw_ioctl(uint32_t ioctl, void* arg)
{
    lora_error_t ret = __LORA_OK;

    if( ioctl == __LORA_IOCTL_SET_CALLBACK )
    {
        __log_info("ioctl -> set-callback");
        lora_raw_process_register_callback(((lora_callback_t*)arg)->callback);
    }
    else if( ioctl == __LORA_IOCTL_RX_CONT_START )
    {
        __log_info("ioctl -> rx-continuous-start");
        lora_raw_process_event(__LORA_RAW_PROCESS_RX_CONT_START, NULL, false);
    }
    else if( ioctl == __LORA_IOCTL_RX_CONT_STOP )
    {
        __log_info("ioctl -> rx-continuous-stop");
        lora_raw_process_event(__LORA_RAW_PROCESS_RX_CONT_STOP, NULL, true);
    }
    else if( ioctl == __LORA_IOCTL_TX_CONT_WAVE_START )
    {
        lora_raw_tx_cont_wave_params_t* params = arg;
        __log_info("ioctl -> rx-continuous-wave-start "
                "[ freq:%d MHz, power: %+d dBm, time:%d sec ]",
                __to_mhz(params->freq), params->power, params->time/1000 );
        lora_raw_process_event_payload_t msg = {
            .type = __PROCESS_MSG_PAYLOAD_TX_CONT_WAVE,
            .tx_cont_wave = {
                .timeout = params->time,
                .freq = params->freq,
                .power = params->power
                }};
        lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE,
            &msg, false);

    }
    else if( ioctl == __LORA_IOCTL_TX_CONT_WAVE_STOP )
    {
        __log_info("ioctl -> rx-continuous-wave-stop");
        lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE_END, NULL,
            false);
    }
    else if( ioctl == __LORA_IOCTL_SET_PARAM )
    {
        __log_info("ioctl -> set-radio-param");
        ret = lora_raw_radio_set_param(arg);
    }
    else if( ioctl == __LORA_IOCTL_GET_PARAM )
    {
        __log_info("ioctl -> get-radio-param");
        ret = lora_raw_radio_get_param(arg);
    }
    else if( ioctl == __LORA_IOCTL_GET_DEFAULT_REGION_PARAM )
    {
        __log_info("ioctl -> get default region radio param");
        ret = lora_raw_radio_get_default_region_param(arg);
    }
    else if( ioctl == __LORA_IOCTL_RECONFIG_RADIO )
    {
        __log_info("ioctl -> reconfigure-radio-device");
        lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_CONFIG, NULL, true);
    }
    else if( ioctl == __LORA_IOCTL_RESET_RADIO_PARAMS )
    {
        __log_info("ioctl -> reset-radio-params");
        lora_raw_radio_reset_params();
        lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_CONFIG, NULL, true);
    }
    else if( ioctl == __LORA_IOCTL_VERIFY_PARAM )
    {
        __log_info("ioctl -> verify-radio-params");
        ret = lora_raw_radio_verify_param(arg);
    }
    else
    {
        __log_info("ioctl -> "__red__"unknown lora raw ioctl"__default__);
    }

    return ret;
}

static lora_mode_interface_t lora_raw_instance = {
    .mode_ctor  = lora_raw_ctor,
    .mode_dtor  = lora_raw_dtor,
    .mode_stats = lora_raw_stats,
    .mode_tx    = lora_raw_tx,
    .mode_rx    = lora_raw_rx,
    .mode_ioctl = lora_raw_ioctl
};

lora_mode_interface_t* lora_raw_get_interface(void)
{
    return & lora_raw_instance;
}

/* --- end of file ---------------------------------------------------------- */
