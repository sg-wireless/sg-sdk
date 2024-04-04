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
 * @brief   lora-wan processing sub-component
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

#define __log_subsystem     lora
#define __log_component     wan_process
#include "log_lib.h"

#include "lora_event_handler.h"
#include "lora_wan_process.h"
#include "radio.h"
#include "stub_timers.h"
#include "stub_system.h"
#include "state_machine.h"
#include "lora_mac_handler.h"
#include "lora_mac_utils.h"
#include "lora_sync_obj.h"
#include "lora_wan_port.h"
#include "lora_wan_duty.h"
#include "lora_wan_state_machine.h"

/** -------------------------------------------------------------------------- *
 * all declarations
 * --------------------------------------------------------------------------- *
 */

static volatile bool s_is_req_class_pending = false;

static input_id_t get_state_machine_input(lora_wan_process_request_t req);

static void msg_timeout_timer_callback(void* arg);
static void msg_timeout_timer_ctor(void);
static void msg_timeout_timer_dtor(void);
static void msg_timeout_timer_start(uint32_t msec);
static void msg_timeout_timer_stop(void);

static const char* lora_wan_get_req_str(lora_wan_process_request_t req);
static void lora_wan_process_handler(void* data);

static void trx_start_processing(void);
static void trx_process_timeout(void);
static void trx_cancel_ongoing_processing(void);

static void trx_post_msg_ind(int ind);

/** -------------------------------------------------------------------------- *
 * state-machine definition
 * --------------------------------------------------------------------------- *
 */
/* ############################ Not-Joined State ############################ */
__sm_trans(lora_wan, not_joined,    join_req,   start_join,     not_joined  )
__sm_trans(lora_wan, not_joined,    mac_req,    process_mac,    not_joined  )
__sm_trans(lora_wan, not_joined,    radio_evt,  process_radio,  not_joined  )
__sm_trans(lora_wan, not_joined,    join_done,  switch_slass,   chg_class   )
__sm_trans(lora_wan, not_joined,    join_fail,  restart_join,   not_joined  )
__sm_trans(lora_wan, not_joined,    commission, commission,     not_joined  )

/* ############################## Joined State ############################## */
__sm_trans(lora_wan, joined,        duty_cycle, start_trx,      trx         )
__sm_trans(lora_wan, joined,        mac_req,    process_mac,    joined      )
__sm_trans(lora_wan, joined,        radio_evt,  process_radio,  joined      )
__sm_trans(lora_wan, joined,        join_req,   start_join,     not_joined  )
__sm_trans(lora_wan, joined,        commission, commission,     not_joined  )
__sm_trans(lora_wan, joined,        req_class,  switch_slass,   chg_class   )

/* ################################ TRX State ############################### */
__sm_trans(lora_wan, trx,           duty_cycle, start_trx,      trx         )
__sm_trans(lora_wan, trx,           mac_req,    process_mac,    trx         )
__sm_trans(lora_wan, trx,           radio_evt,  process_radio,  trx         )
__sm_trans(lora_wan, trx,           join_req,   start_join,     not_joined  )
__sm_trans(lora_wan, trx,           commission, commission,     not_joined  )
__sm_trans(lora_wan, trx,           timeout,    trx_timeout,    joined      )
__sm_trans(lora_wan, trx,           req_class,  switch_slass,   trx         )

/* ################################ CLASS CHG ############################### */
__sm_trans(lora_wan, chg_class,     req_class,  switch_slass,   chg_class   )
__sm_trans(lora_wan, chg_class,     class_chg,  ind_class,      joined      )
__sm_trans(lora_wan, chg_class,     mac_req,    process_mac,    chg_class   )
__sm_trans(lora_wan, chg_class,     duty_cycle, do_nothing,     chg_class   )
__sm_trans(lora_wan, chg_class,     radio_evt,  process_radio,  chg_class   )
__sm_trans(lora_wan, chg_class,     join_req,   start_join,     not_joined  )
__sm_trans(lora_wan, chg_class,     commission, commission,     not_joined  )
__sm_trans(lora_wan, chg_class,     timeout,    trx_timeout,    chg_class   )

/** -------------------------------------------------------------------------- *
 * state-machine actions
 * --------------------------------------------------------------------------- *
 */
void lora_wan_state_machine_ctor(void)
{
    __log_info("ctor() -> lora wan state-machine");
    if( lm_is_joined() )
    {
        __log_info("reset state-machine state to "__green__"joined");
        __sm_ch_state(lora_wan, joined);
    }
    else
    {
        __log_info("reset state-machine state to "__red__"not-joined");
        __sm_ch_state(lora_wan, not_joined);
    }
}

void lora_wan_state_machine_dtor(void)
{
    __log_info("~dtor() -> lora wan state-machine");
}

__sm_state_enter(lora_wan, joined)(void* data)
{
    lora_wan_duty_resume();
}

__sm_action(lora_wan, start_join)(void* data)
{
    trx_cancel_ongoing_processing();

    if( lm_get_active_status() == __lm_status_active_abp )
    {
        __sm_ch_state(lora_wan, joined);
    }
    else
    {
        lmh_join();
    }
}
__sm_action(lora_wan, restart_join)(void* data)
{
    trx_cancel_ongoing_processing();

    lmh_reset(false);

    if( lm_get_active_status() == __lm_status_active_abp )
    {
        __sm_ch_state(lora_wan, joined);
    }
    else
    {
        lmh_join();
    }
}
__sm_action(lora_wan, commission)(void* data)
{
    trx_cancel_ongoing_processing();

    lmh_reset(true);

    /* in case of ABP the lora-wan is supposed to be joined automatically */
    if( lm_get_active_status() == __lm_status_active_abp )
    {
        __sm_ch_state(lora_wan, joined);
    }
}
__sm_action(lora_wan, process_mac)(void* data)
{
    lmh_process();
}
__sm_action(lora_wan, process_radio)(void* data)
{
    if( Radio.IrqProcess != NULL ) {
        Radio.IrqProcess( );
    }
    lmh_process();
}
__sm_action(lora_wan, start_trx)(void* data)
{
    if( lmh_is_busy() )
    {
        lmh_process();
    }
    else
    {
        trx_start_processing();
    }
}
__sm_action(lora_wan, trx_timeout)(void* data)
{
    trx_process_timeout();
}
__sm_action(lora_wan, switch_slass)(void* data)
{
    /**
     * class change request can come in any state:
     * 
     * not-joined:
     *      ignore because the class change will be an automatic operation
     *      after join success
     * 
     * joined:
     *      initiate the class change operation and move to chg-class state
     * 
     * trx:
     *      raise a flag that a class change request is pending and when
     *      the current message processing operations ends, it will check
     *      the class change request flag and initiate its operation accordingly
     *      then move manually to the chg-class state
     * 
     * chg-class
     *      - here a class change operation is in progress, so cancel the
     *        current operation that in progress
     *      - initialte the new class change operation
     *      - when the class change indication come:
     *          - if there is a pending messages to be processed
     *              move manually to the trx state
     *          - else
     *              move to the joined state
     */
    if(  __sm_present_state_id(lora_wan) == __sm_state_id(lora_wan, trx) )
    {
        __log_info("in trx state, wait until trx finish");
        s_is_req_class_pending = true;
    }
    else if ( __sm_present_state_id(lora_wan) ==
            __sm_state_id(lora_wan, chg_class) )
    {
        __log_info("previous class change in progress, wait until it finish");
        s_is_req_class_pending = true;
    }
    else if(lmh_is_busy())
    {
        __log_info("lora-mac is busy, wait until it become free");
        s_is_req_class_pending = true;
    }
    else
    {
        __log_info("process class change ..");
        s_is_req_class_pending = false;
        lmh_change_class();
    }
}

__sm_action(lora_wan, ind_class)(void* data)
{
    if(s_is_req_class_pending)
    {
        __log_info("class change request during current class change");
        s_is_req_class_pending = false;
        lmh_change_class();
    }
}

static input_id_t get_state_machine_input(lora_wan_process_request_t req)
{
    if( req == __LORA_WAN_PROCESS_COMMISSION )
        return __sm_input_id(lora_wan, commission);
    if( req == __LORA_WAN_PROCESS_JOIN_REQUEST )
        return __sm_input_id(lora_wan, join_req);
    if( req == __LORA_WAN_PROCESS_JOIN_DONE )
        return __sm_input_id(lora_wan, join_done);
    if( req == __LORA_WAN_PROCESS_JOIN_FAIL )
        return __sm_input_id(lora_wan, join_fail);
    if( req == __LORA_WAN_PROCESS_PROCESS_MAC )
        return __sm_input_id(lora_wan, mac_req);
    if( req == __LORA_WAN_PROCESS_RADIO_EVENT )
        return __sm_input_id(lora_wan, radio_evt);
    if( req == __LORA_WAN_PROCESS_TRX_DUTY_CYCLE )
        return __sm_input_id(lora_wan, duty_cycle);
    if( req == __LORA_WAN_PROCESS_MSG_TIMEOUT )
        return __sm_input_id(lora_wan, timeout);
    if( req == __LORA_WAN_PROCESS_REQ_CLASS )
        return __sm_input_id(lora_wan, req_class);
    if( req == __LORA_WAN_PROCESS_CLASS_CHANGED )
        return __sm_input_id(lora_wan, class_chg);
    
    __log_error("-- unknown lora wan input for process req: %d --", req);
    return 0xFF;
}

/** -------------------------------------------------------------------------- *
 * trx processing
 * --------------------------------------------------------------------------- *
 */
static volatile bool s_enable_rx_litening = false;

static void* msg_timeout_timer = NULL;

static const char* msg_timeout_timer_name = "wan-process-timeout-timer";

static void msg_timeout_timer_callback(void* arg)
{
    __log_timer_expire(msg_timeout_timer_name);
    lora_wan_process_request(__LORA_WAN_PROCESS_MSG_TIMEOUT, NULL);
}

static void msg_timeout_timer_ctor(void)
{
    __log_timer_ctor(msg_timeout_timer_name);
    msg_timeout_timer = lora_stub_timer_init(msg_timeout_timer_name,
        msg_timeout_timer_callback, NULL);
}

static void msg_timeout_timer_dtor(void)
{
    __log_timer_dtor(msg_timeout_timer_name);
    lora_stub_timer_delete(msg_timeout_timer);
}

static void msg_timeout_timer_start(uint32_t msec)
{
    __log_timer_start(msg_timeout_timer_name, msec);
    lora_stub_timer_start(msg_timeout_timer, msec);
}

static void msg_timeout_timer_stop(void)
{
    __log_timer_stop(msg_timeout_timer_name);
    lora_stub_timer_stop(msg_timeout_timer);
}

/** -------------------------------------------------------------------------- *
 * transmission processing
 * --------------------------------------------------------------------------- *
 */
static struct {
    lora_wan_port_tx_msg_t  msg_header;
    uint8_t msg_payload[256];
} tx_msg;

static lmh_tx_status_params_t tx_status_params;

static volatile bool is_msg_processing = false;
static volatile bool is_msg_retry = false;

static lora_wan_port_tx_msg_t * p_msg_header = (void*)&tx_msg;
static uint8_t tx_msg_payload_len;

#define __log_tx_msg()                                                      \
    __log_info("ul-msg[port:%d seq:%d , app_id:%d, len:%d] "                \
        "sync: %s, confirm:%s, timeout:%s", p_msg_header->port_num,         \
        p_msg_header->msg_seq_num, p_msg_header->msg_app_id,                \
        tx_msg_payload_len, g_yes_no[p_msg_header->sync],                   \
        g_yes_no[p_msg_header->confirm],                                    \
        g_yes_no[p_msg_header->has_timeout ? 1 : 0]);                       \
    __log_dump(tx_msg.msg_payload, tx_msg_payload_len, 16,                  \
        __log_dump_flag_disp_char_on_rhs|__log_dump_flag_disp_char|         \
        __log_dump_flag_hide_address, __word_len_8)

static void trx_start_processing(void)
{
    __log_info("-- start trx processing");
    __log_info("-- get new tx message to send");
    uint8_t len;

    if(is_msg_processing)
    {
        __log_info("-- prev msg still in processing ..");
        if(is_msg_retry)
        {
            __log_info("-- retry sending the msg again ..");
            lmh_send(tx_msg.msg_payload, tx_msg_payload_len,
                    p_msg_header->port_num, p_msg_header->confirm);
        }
        else
        {
            __log_warn("-- start processing while ongoing message, ignore");
        }
    }
    else
    {
        pick_up_new_tx_msg:
        __log_info("-- pick-up a new tx msg ..");
        if( lora_wan_get_tx_data( (void*)&tx_msg, &len ) == __PORT_OK )
        {
            __log_info("-- new msg tx request ..");

            tx_msg_payload_len = tx_msg.msg_header.len;

            __log_tx_msg();

            if(tx_msg.msg_header.has_timeout)
            {
                uint32_t ts = lora_stub_get_timestamp_ms();
                if(tx_msg.msg_header.expire_timestamp > ts) {
                    uint32_t period = tx_msg.msg_header.expire_timestamp - ts;
                    __log_info("-- start timeout timer: %d msec", period);
                    msg_timeout_timer_start( period );

                    lmh_send(tx_msg.msg_payload, tx_msg_payload_len,
                        p_msg_header->port_num, p_msg_header->confirm);
                    is_msg_processing = true;
                } else {
                    __log_info("-- msg already timedout, drop it ..");
                    trx_post_msg_ind(__IND_TX_TIMEOUT);
                    goto pick_up_new_tx_msg;
                    // __sm_ch_state(lora_wan, joined);
                }
            }
            else
            {
                lmh_send(tx_msg.msg_payload, tx_msg_payload_len,
                        p_msg_header->port_num, p_msg_header->confirm);
                is_msg_processing = true;
            }
            is_msg_retry = false;
        }
        else
        {
            if( s_enable_rx_litening )
            {
                __log_info("-- no pending tx requests, start cycle for rx ..");
                lmh_send(NULL, 0, 0, false);
            }
            else
            {
                __log_info("-- no pending tx requests, do nothing ..");
                lmh_process();
            }
        }
    }
}

static void trx_after_processing(void)
{
    if( s_is_req_class_pending )
    {
        __log_info("class change is pending, so start class switching");
        s_is_req_class_pending = false;
        lmh_change_class();
        __sm_ch_state(lora_wan, chg_class);
    }
    else
    {
        __log_info("no class change pending, switch to JOINED state");
        __sm_ch_state(lora_wan, joined);
    }
}

static void trx_cancel_ongoing_processing(void)
{
    if(is_msg_processing)
    {
        __log_info("-- cancel ongoing message processing");
        is_msg_processing = false;
        is_msg_retry = false;
        trx_post_msg_ind(__IND_TX_FAIL);
    }
}

static void trx_post_msg_ind(int ind)
{
    if(tx_msg.msg_header.sync)
    {
        sync_obj_release(tx_msg.msg_header.sync_obj);
    }
    lora_wan_port_ind_msg_t ind_msg = {
        .type = ind,
        .ind_params.tx = {
            .msg_app_id = tx_msg.msg_header.msg_app_id,
            .msg_seq_num = tx_msg.msg_header.msg_seq_num,
        }
    };
    if(ind == __IND_TX_CONFIRM || ind == __IND_TX_DONE)
    {
        ind_msg.ind_params.tx.tx_power = tx_status_params.tx_power;
        ind_msg.ind_params.tx.ul_frame_counter = tx_status_params.ul_counter;
        ind_msg.ind_params.tx.data_rate = tx_status_params.data_rate;
    }
    lora_wan_port_indication(tx_msg.msg_header.port_num, &ind_msg);
}

static void trx_process_timeout(void)
{
    __log_info("-- handle trx timeout");
    trx_post_msg_ind(__IND_TX_TIMEOUT);
    is_msg_processing = false;
    is_msg_retry = false;
}

static void trx_retry_handler(lmh_tx_status_params_t* p_tx_info)
{
    if(tx_msg.msg_header.retries)
    {
        __log_info("-- msg retry -- left tries:%d", tx_msg.msg_header.retries);
        -- tx_msg.msg_header.retries;
        is_msg_retry = true;
    }
    else
    {
        __log_info("-- no trials left");
        is_msg_processing = false;
        is_msg_retry = false;
        trx_post_msg_ind(__IND_TX_FAIL);
        trx_after_processing();
    }
}

static void lmh_cb_on_mac_tx(lmh_tx_status_params_t* p_tx_info)
{
    msg_timeout_timer_stop();

    __log_info("callback() -> mac-tx-event:"__cyan__"%s"__default__,
        lora_utils_get_mac_event_info_status_str(p_tx_info->status));
    
    if( is_msg_processing )
    {
        __log_info("-- handle msg in processing ..");
        if(p_tx_info->status == LORAMAC_EVENT_INFO_STATUS_OK)
        {
            __log_info("-- tx mac status ok ..");
            if( p_tx_info->port == tx_msg.msg_header.port_num )
            {
                __log_info("-- tx on correct port ..");
                if( tx_msg.msg_header.confirm )
                {
                    if(p_tx_info->ack_received)
                    {
                        __log_info(__green__"-- confirmed - tx-done ..");
                        tx_status_params = *p_tx_info;
                        trx_post_msg_ind(__IND_TX_CONFIRM);
                        is_msg_processing = false;
                        is_msg_retry = false;
                        trx_after_processing();
                    }
                    else
                    {
                        __log_info(__red__"-- not confirmed ..");
                        trx_retry_handler(p_tx_info);
                    }
                }
                else
                {
                    __log_info(__green__"-- no confirm required, tx-done ..");
                    is_msg_processing = false;
                    is_msg_retry = false;
                    tx_status_params = *p_tx_info;
                    trx_post_msg_ind(__IND_TX_DONE);
                    trx_after_processing();
                }
            }
            else // -- tx status on un-expected port number
            {
                __log_warn("-- tx info received on un-expected port %d",
                    p_tx_info->port);
                trx_retry_handler(p_tx_info);
            }
        }
        else if(p_tx_info->status == LORAMAC_EVENT_INFO_STATUS_ERROR)
        {
            __log_error("-- mac tx status error");
            trx_retry_handler(p_tx_info);
        }
        else if(p_tx_info->status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT)
        {
            __log_warn("-- mac tx status timeout");
            trx_retry_handler(p_tx_info);
        }
        else
        {
            __log_warn("-- mac tx status un-handled case");
            trx_retry_handler(p_tx_info);
        }
    }
    else
    {
        __log_warn("-- mac tx no msg in processing");
    }
}
static void lmh_cb_on_mac_rx(lmh_rx_status_params_t* p_rx_info)
{
    __log_info("callback() -> mac-rx-event:"__cyan__"%s"__default__,
        lora_utils_get_mac_event_info_status_str(p_rx_info->status));
    if(p_rx_info->port)
    {
        __log_info("-- rx on port %d", p_rx_info->port);

        if(p_rx_info->len && p_rx_info->buf)
        {
            lora_wan_port_ind_msg_t ind_msg = {
                .type = __IND_RX_DONE,
                .ind_params.rx = {
                    .dl_frame_counter = p_rx_info->dl_counter,
                    .rssi = p_rx_info->rssi,
                    .snr = p_rx_info->snr,
                    .data_rate = p_rx_info->data_rate
                }
            };
            __log_info("-- rx data len: %d, transfer it to the dedicated port",
                p_rx_info->len);
            lora_wan_port_rx_indication(p_rx_info->port,
                &ind_msg, p_rx_info->buf, p_rx_info->len);
        }
        else
        {
            __log_info(__red__"-- no rx data");
        }
    }
    else
    {
        __log_info("-- rx on port %d, len: %d",
            p_rx_info->port, p_rx_info->len);
    }
    trx_after_processing();
}

/** -------------------------------------------------------------------------- *
 * lora-wan process management
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    lora_wan_process_request_t request_type;
    void * trigger_data;
} lora_wan_process_queue_item_t;

static void lora_port_service_level_irq_handler(void)
{
    __log_info("notify from irq handler");
    lora_wan_process_request(__LORA_WAN_PROCESS_RADIO_EVENT, NULL);
}

void lora_wan_process_ctor(void)
{
    __log_info("ctor()   -> lora_wan_process");

    is_msg_processing = false;
    is_msg_retry = false;

    lora_event_handler_register(__lora_evt_loramac_handler_process_notify,
        lora_wan_process_handler);

    extern void lora_port_set_service_level_irq_handler(void(*p_handler)(void));
    lora_port_set_service_level_irq_handler(
        lora_port_service_level_irq_handler);
    
    msg_timeout_timer_ctor();

    lmh_callbacks_t cbs = {lmh_cb_on_mac_tx, lmh_cb_on_mac_rx};
    lmh_callbacks(& cbs);
    lora_wan_state_machine_ctor();
}

void lora_wan_process_dtor(void)
{
    __log_info("~dtor()  -> lora_wan_process");
    msg_timeout_timer_dtor();
    lora_event_handler_deregister(__lora_evt_loramac_handler_process_notify);
    lora_wan_state_machine_dtor();
    is_msg_processing = false;
    is_msg_retry = false;
}

void lora_wan_process_request(
    lora_wan_process_request_t request_type,
    void* trigger_data)
{
    lora_wan_process_queue_item_t queue_item = {
        .request_type = request_type,
        .trigger_data = trigger_data
    };
    lora_event_handler_issue(__lora_evt_loramac_handler_process_notify,
        &queue_item, sizeof(queue_item));
}

bool lora_wan_process_busy(void)
{
    return is_msg_processing;
}

static const char* lora_wan_get_req_str(lora_wan_process_request_t req)
{
    switch(req) {
    case __LORA_WAN_PROCESS_COMMISSION:     return "commission";
    case __LORA_WAN_PROCESS_JOIN_REQUEST:   return "join-request";
    case __LORA_WAN_PROCESS_JOIN_DONE:      return "join-done";
    case __LORA_WAN_PROCESS_JOIN_FAIL:      return "join-fail";
    case __LORA_WAN_PROCESS_JOIN_STATUS_REQ:return "join-status-req";
    case __LORA_WAN_PROCESS_PROCESS_MAC:    return "process-mac";
    case __LORA_WAN_PROCESS_RADIO_EVENT:    return "radio-event";
    case __LORA_WAN_PROCESS_TRX_DUTY_CYCLE: return "trx-duty-cycle";
    case __LORA_WAN_PROCESS_MSG_TIMEOUT:    return "msg-timeout";
    case __LORA_WAN_PROCESS_REQ_CLASS:      return "req-class";
    case __LORA_WAN_PROCESS_CLASS_CHANGED:  return "ind-class";
    }
    return __red__"unknown-process-request"__default__;
}
static void lora_wan_process_handler(void* data)
{
    __log_info(__purple__"-- lora wan processing cycle --");

    lora_wan_process_queue_item_t* req = (lora_wan_process_queue_item_t*)data;

    __log_info(">> lora-wan-process [trigger: "__green__"%s"__default__"]",
        lora_wan_get_req_str(req->request_type));

    if(req->request_type == __LORA_WAN_PROCESS_JOIN_STATUS_REQ)
    {
        lora_wan_join_status_req_t* status_req = req->trigger_data;
        status_req->is_joined = lm_is_joined();
        __log_enforce("-- is_joined : %d", status_req->is_joined);
        sync_obj_signal(status_req->sync_obj);
    }
    else
    {
        __sm_run(lora_wan, get_state_machine_input(req->request_type),
            req->trigger_data);
    }
}

void lora_wan_enable_rx_listening(void)
{
    s_enable_rx_litening = true;
}

void lora_wan_disable_rx_listening(void)
{
    s_enable_rx_litening = false;
}

/* --- end of file ---------------------------------------------------------- */
