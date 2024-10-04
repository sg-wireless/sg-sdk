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
 * @brief   lora-stack manager sub-component.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

#define __log_subsystem  lora
#define __log_component  mgr_api
#include "log_lib.h"
#include "lora.h"
#include "lora_wan_nvm.h"
#include "lora_event_handler.h"
#include "lora_mac_utils.h"
#include "lora_sync_obj.h"
#include "lora_nvm.h"
#include "lora_mode.h"
#include "lora_common_utils.h"
#include "lora_wan_port.h"
#include "lora_port.h"

/** -------------------------------------------------------------------------- *
 * manager nvm data handling
 * --------------------------------------------------------------------------- *
 */
static struct {
    lora_mode_t mode;
    lora_nvm_record_tail_t record_tail; // -- needed by the lora_nvm.h
} s_mgr_settings;

static const char* s_lora_mgr_nvm_key = "lora-mgr";

static void lora_mgr_nvm_load_defaults_callback(void* ptr, uint32_t size)
{
    (void) ptr; (void) size;
    s_mgr_settings.mode = __LORA_MODE_WAN;
}
static void lora_mgr_handle_nvm(void)
{
    lora_nvm_handle_change(
        s_lora_mgr_nvm_key,
        &s_mgr_settings,
        sizeof(s_mgr_settings),
        lora_mgr_nvm_load_defaults_callback
        );
}

static const char* lora_mode_get_str(lora_mode_t mode)
{
    if( mode == __LORA_MODE_RAW )
        return "raw";
    else if( mode == __LORA_MODE_WAN )
        return "wan";
    
    return __red__"unknown-lora-mode"__default__;
}

const char* lora_get_event_str(lora_event_t event)
{
    switch (event)
    {
        case __LORA_EVENT_TX_DONE:      return "TX_DONE";
        case __LORA_EVENT_TX_TIMEOUT:   return "TX_TIMEOUT";
        case __LORA_EVENT_TX_FAIL:      return "TX_FAIL";
        case __LORA_EVENT_TX_CONFIRM:   return "TX_CONFIRM";
        case __LORA_EVENT_RX_DONE:      return "RX_DONE";
        case __LORA_EVENT_RX_TIMEOUT:   return "RX_TIMEOUT";
        case __LORA_EVENT_RX_FAIL:      return "RX_FAIL";
        case __LORA_EVENT_INDICATION:   return "INDICATION";
        case __LORA_EVENT_NONE:         return "NONE";
    }

    return __red__"unknown"__default__;
}

const char* lora_get_region_str(lora_region_t region)
{
    return get_lora_region_string(region);
}

/** -------------------------------------------------------------------------- *
 * LoRa Manager Interface Implementation
 * --------------------------------------------------------------------------- *
 */

static lora_mode_interface_t* s_modes_ifs[] = {
    [__LORA_MODE_RAW] = NULL,
    [__LORA_MODE_WAN] = NULL
};

static bool is_lora_on = false;

#define __current_mode()    s_modes_ifs[s_mgr_settings.mode]

lora_error_t lora_ctor(void)
{
    if(is_lora_on)
        return __LORA_OK;

    __log_info("lora-mgr -> ctor()");

    lora_board_ctor();

    s_modes_ifs[__LORA_MODE_RAW] = lora_raw_get_interface();
    s_modes_ifs[__LORA_MODE_WAN] = lora_wan_get_interface();

    __log_info("init lora mgr");
    lora_event_handler_init();
    sync_obj_init();

    lora_mgr_handle_nvm();

    __current_mode()->mode_ctor();

    is_lora_on = true;

    return __LORA_OK;
}

lora_error_t lora_dtor(void)
{
    if(is_lora_on == false)
        return __LORA_OK;

    is_lora_on = false;

    __log_info("lora-mgr -> ~dtor()");

    __current_mode()->mode_dtor();

    lora_board_dtor();

    return __LORA_OK;
}

lora_error_t lora_get_mode(lora_mode_t * p_mode)
{
    *p_mode = s_mgr_settings.mode;

    return __LORA_OK;
}

lora_error_t lora_change_mode(lora_mode_t mode)
{
    lora_error_t ret = __LORA_OK;

    if( ( mode ^ s_mgr_settings.mode ) == 0 )
    {
        __log_warn("lora-mgr -> set same mode '"__blue__"%s"__default__"'",
            lora_mode_get_str(mode));
        ret = __LORA_ERROR;
    }
    else
    {
        __log_info("lora-mgr -> change mode from "__red__"%s"__default__
            " to "__green__"%s"__default__,
            lora_mode_get_str(s_mgr_settings.mode), lora_mode_get_str(mode));

        if(is_lora_on)
        {
            __current_mode()->mode_dtor();
            s_mgr_settings.mode = mode;
            __current_mode()->mode_ctor();
        }
        else
            s_mgr_settings.mode = mode;

        lora_mgr_handle_nvm();
    }

    return ret;
}

lora_error_t lora_tx(lora_tx_params_t * p_tx_params)
{
    __log_info("lora-mgr -> tx()");
    if(is_lora_on)
        return __current_mode()->mode_tx(p_tx_params);
    else
        return __LORA_POWERED_OFF;
}

lora_error_t lora_rx(lora_rx_params_t * p_rx_params)
{
    __log_info("lora-mgr -> rx()");
    if(is_lora_on)
        return __current_mode()->mode_rx(p_rx_params);
    else
        return __LORA_POWERED_OFF;
}

lora_error_t lora_stats(void)
{
    __log_info("lora-mgr -> stats()");
    if(is_lora_on)
        return __current_mode()->mode_stats();
    else
        return __LORA_POWERED_OFF;
}

lora_error_t lora_ioctl(uint32_t ioctl, void* arg)
{
    __log_info("lora-mgr -> ioctl()");
    if(is_lora_on)
        return __current_mode()->mode_ioctl(ioctl, arg);
    else
        return __LORA_POWERED_OFF;
}

static void lora_callback_stub(lora_event_t event, void* event_data);

lora_error_t lora_connect_callback_stub(void)
{
    __log_info("connect internal lora-stack callbacks stub");
    /**
     * set the mode callback stub, if the user didn't provide a dedicated
     * callback, this stub will consume the incoming indications and events
     * particularly from LoRaWAN mode
     */
    lora_callback_t cb_struct = {
        .port = __port_any,
        .callback = lora_callback_stub
    };
    __current_mode()->mode_ioctl(__LORA_IOCTL_SET_CALLBACK, (void*)&cb_struct);

    return __LORA_OK;
}

/**
 * This function acts as a stub for any running lora mode.
 * It is important particularly for LoRaWAN ports that does not have user
 * callback to continuously flush incoming indications or to debug incoming
 * stuff without having callback connected 
 */
static void lora_callback_stub(lora_event_t event, void* event_data)
{
    __log_output("lora: "__yellow__"%s "__default__, lora_get_event_str(event));

    #define __rx_buff_len   0xff
    static uint8_t buff[__rx_buff_len];

    if(event == __LORA_EVENT_INDICATION)
    {
        /**
         * --- LoRaWAN indications
         */
        lora_wan_ind_params_t ind_param;
        do {

            ind_param.buf = buff;
            ind_param.len = __rx_buff_len;

            lora_wan_port_get_ind_params(&ind_param);

            if(ind_param.event != __LORA_EVENT_NONE)
            {
                __log_output("evt: "__cyan__"%s"__default__
                    " app-id: "__green__"%d"__default__,
                    lora_get_event_str(ind_param.event),
                    ind_param.tx.msg_app_id);
            }

            if(ind_param.event == __LORA_EVENT_TX_DONE ||
                ind_param.event == __LORA_EVENT_TX_CONFIRM)
            {
                __log_output(" ul_counter:"__cyan__"%d, "__default__
                        " tx_power: "__yellow__"%d"__default__", "
                        " data-rate: "__green__"%d"__default__,
                    ind_param.tx.ul_frame_counter,
                    ind_param.tx.tx_power, ind_param.tx.data_rate);
            }
            else if(ind_param.event == __LORA_EVENT_RX_DONE)
            {
                __log_output(
                        " dl_counter:"  __cyan__    "%d"__default__
                        " rssi: "       __yellow__  "%d"__default__
                        " snr: "        __yellow__  "%d"__default__
                        " data-rate: "  __green__   "%d"__default__
                        " data-len: "   __cyan__    "%d"__default__
                        "\n",
                    ind_param.rx.dl_frame_counter,
                    ind_param.rx.rssi, ind_param.rx.snr,
                    ind_param.rx.data_rate, ind_param.len);
                __log_output_field(" rx data ", 82, '-', __center__, true);
                __log_output_dump(ind_param.buf, ind_param.len, 20,
                    __log_dump_flag_hide_address|__log_dump_flag_hide_offset|
                    __log_dump_flag_disp_char|__log_dump_flag_disp_char_on_rhs,
                    __word_len_8);
                __log_output("\n");
                __log_output_fill(82, '-', true);
            }

            if(ind_param.event != __LORA_EVENT_NONE)
            {
                __log_output("\n");
            }
        } while(ind_param.event != __LORA_EVENT_NONE);
    }
    else
    {
        /**
         * --- LoRaRAW events
         */
        if(event == __LORA_EVENT_RX_DONE)
        {
            lora_raw_rx_event_data_t* p_rx_data = event_data;

            __log_output(
                " rssi: "       __yellow__  "%d"__default__
                " snr: "        __yellow__  "%d"__default__
                " data-len: "   __cyan__    "%d"__default__
                "\n",
                p_rx_data->rssi, p_rx_data->snr, p_rx_data->len);
            __log_output_field(" rx data ", 82, '-', __center__, true);
            __log_output_dump(p_rx_data->buf, p_rx_data->len, 20,
                __log_dump_flag_hide_address|__log_dump_flag_hide_offset|
                __log_dump_flag_disp_char|__log_dump_flag_disp_char_on_rhs,
                __word_len_8);
            __log_output("\n");
            __log_output_fill(82, '-', true);
        }
        __log_output("\n");
    }
}

/* --- end of file ---------------------------------------------------------- */
