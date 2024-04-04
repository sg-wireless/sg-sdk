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
 * @brief   lora-raw mode processing sub-component
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdbool.h>

#define __log_subsystem  lora
#define __log_component  raw_process
#include "log_lib.h"

#include "lora_event_handler.h"
#include "lora_sync_obj.h"
#include "radio_ext.h"
#include "stub_system.h"
#include "stub_timers.h"
#include "system/timer.h"
#include "lora_raw_process.h"
#include "lora_raw_radio_if.h"
#include "lora_raw_state_machine.h"
#include "adt_list.h"

/** -------------------------------------------------------------------------- *
 * process orders management
 * --------------------------------------------------------------------------- *
 */
static const char* lora_raw_get_cmd_str(lora_raw_process_event_t cmd);

typedef struct _order_s {
    adt_list_t                          links;
    lora_raw_process_event_t            req_type;
    bool                                sync;
    sync_obj_t                          sync_obj;

    enum {
        __FREE,
        __REQUEST,
        __RESPOND
    } order_state;

    union {
        lora_raw_process_event_payload_t    request_payload;
        struct{
            lora_event_t    event;
            union {
                lora_raw_rx_event_data_t rx_event_data;
            } event_data;
        } respond_payload;
    };
} order_t;

#define __max_orders_count  10
static order_t s_orders_array[__max_orders_count];

static void* s_order_mutex_handle = NULL;
#define __order_access_ctor()                               \
    do {                                                    \
        if(s_order_mutex_handle == NULL)                    \
            s_order_mutex_handle = lora_stub_mutex_new();   \
    } while (0)
#define __order_access_lock()   lora_stub_mutex_lock(s_order_mutex_handle)
#define __order_access_unlock() lora_stub_mutex_unlock(s_order_mutex_handle)


static order_t * s_p_respnded_orders = NULL;

static order_t* order_request(
    lora_raw_process_event_t    req_type,
    bool                        sync,
    sync_obj_t                  sync_obj,
    lora_raw_process_event_payload_t* req_payload
    )
{
    int i;
    __order_access_lock();
    for(i = 0; i < __max_orders_count; ++i)
    {
        if( s_orders_array[i].order_state == __FREE )
        {
            __log_info("==> order "__blue__"request "__default__" , "
                __cyan__"%-13s"__default__" , %ssync"__default__
                , lora_raw_get_cmd_str(req_type)
                , sync ? __green__"" : __red__"non-");
            order_t* p_order = & s_orders_array[i];
            p_order->order_state = __REQUEST;
            p_order->req_type = req_type;
            p_order->sync = sync;
            p_order->sync_obj = sync_obj;
            if(req_payload)
                p_order->request_payload = *req_payload;
            __order_access_unlock();
            return p_order;
        }
    }
    __order_access_unlock();
    return NULL;
}
#define __no_callback   (0xFF)
static void order_respond(
    order_t*        p_order,
    lora_event_t    event,
    void*           event_data,
    int             event_data_size
    )
{
    __order_access_lock();
    p_order->order_state = __RESPOND;
    p_order->respond_payload.event = event;

    if(event_data)
        memcpy(&p_order->respond_payload.event_data,
            event_data, event_data_size);
    
    __adt_list_push(s_p_respnded_orders, p_order);

    __order_access_unlock();
}

static order_t* order_get_responded(void)
{
    order_t* p_order = NULL;
    __order_access_lock();
    if(s_p_respnded_orders)
    {
        p_order = __adt_list_unshift(s_p_respnded_orders);
        // p_order->order_state = __FREE;
        __log_info("==> order "__green__"respond "__default__" , "
            __cyan__"%-13s"__default__" , %ssync"__default__" , "
            "response event: "__cyan__"%s"
            , lora_raw_get_cmd_str(p_order->req_type)
            , p_order->sync ? __green__"" : __red__"non-"
            , p_order->respond_payload.event == __no_callback ? "no-callback" :
                lora_get_event_str( p_order->respond_payload.event ));
    }
    __order_access_unlock();
    return p_order;
}

static void order_free(order_t* p_order)
{
    __order_access_lock();
    p_order->order_state = __FREE;
    __order_access_unlock();
}

static void reset_process_states_saved_orders(void);
static void order_cancel_all(void)
{
    __order_access_lock();

    int i;
    for(i = 0; i < __max_orders_count; ++i)
    {
        if(s_orders_array[i].order_state != __FREE) {
            if(s_orders_array[i].sync)
                sync_obj_release(s_orders_array[i].sync_obj);
            s_orders_array[i].order_state = __FREE;
        }
    }

    s_p_respnded_orders = NULL;

    reset_process_states_saved_orders();

    __order_access_unlock();
}

/** -------------------------------------------------------------------------- *
 * rx buffer manipulation
 * --------------------------------------------------------------------------- *
 */
#define __rx_buffer_max_size        (256u)
static uint8_t s_rx_buffer[__rx_buffer_max_size];
static uint32_t s_rx_buffer_len = 0;

#define __rx_buffer_ptr() s_rx_buffer
#define __rx_buffer_len() s_rx_buffer_len

static void rx_buffer_copy_from(uint8_t * buf, uint8_t len)
{
    __log_assert(buf, "copying from a null buffer pointer");

    memcpy(s_rx_buffer, buf, len);
    s_rx_buffer_len = len;
}

static void rx_buffer_copy_to(uint8_t * buf, uint8_t buf_len, uint8_t* p_len)
{
    uint8_t copy_len = s_rx_buffer_len;
    __log_assert(buf, "copying to a null buffer pointer");
    if(buf_len < s_rx_buffer_len) {
        __log_error("rx data length (%d) is > the user buffer len (%d)",
            s_rx_buffer_len, buf_len);
        copy_len = buf_len;
    }
    memcpy(buf, s_rx_buffer, copy_len);
    *p_len = copy_len;
}

/** -------------------------------------------------------------------------- *
 * time on air timer
 * --------------------------------------------------------------------------- *
 */
static TimerEvent_t s_on_air_timer;

static void time_on_air_callback(void* data);

static void time_on_air_ctor(void)
{
    __log_info("ctor() -> time on air timer");
    TimerInit(&s_on_air_timer, time_on_air_callback);
}
static void time_on_air_dtor(void)
{
    __log_info("~dtor() -> time on air timer");
    TimerStop(&s_on_air_timer);
}

static void time_on_air_callback(void* data)
{
    __log_info("expire -> time on air timer");
    lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_TOA_EXPIRE, NULL,
        false);
}
static void time_on_air_start(void)
{
    __log_info("start  -> time on air timer");
    TimerSetValue(&s_on_air_timer, lora_raw_radio_get_time_on_air());
    TimerStart(&s_on_air_timer);
}
static void time_on_air_stop(void)
{
    __log_info("stop   -> time on air timer");
    TimerStop(&s_on_air_timer);
}
/** -------------------------------------------------------------------------- *
 * operation fulfillment deadline timer
 * --------------------------------------------------------------------------- *
 */
static TimerEvent_t s_operation_deadline_timer;

static void operation_deadline_timer_callback(void* data);

static void operation_deadline_timer_ctor(void)
{
    __log_info("ctor() -> operation deadline timer");
    TimerInit(&s_operation_deadline_timer, operation_deadline_timer_callback);
}
static void operation_deadline_timer_dtor(void)
{
    __log_info("~dtor() -> operation deadline timer");
    TimerStop(&s_operation_deadline_timer);
}

static void operation_deadline_timer_callback(void* data)
{
    __log_info("expire -> operation deadline timer");
    lora_raw_process_event(__LORA_RAW_PROCESS_OPERATION_TIMEOUT, NULL, false);
}
static void operation_deadline_timer_start(uint32_t ms)
{
    __log_info("start  -> operation deadline timer (%d msec)", ms);
    TimerSetValue(&s_operation_deadline_timer, ms);
    TimerStart(&s_operation_deadline_timer);
}
static void operation_deadline_timer_stop(void)
{
    __log_info("stop   -> operation deadline timer");
    TimerStop(&s_operation_deadline_timer);
}

/** -------------------------------------------------------------------------- *
 * process state machine
 * --------------------------------------------------------------------------- *
 */
#define __log_process_event(_evt, _sync)                                    \
    __log_info("=>> evt: "__cyan__"%-12s"__default__" , %4ssync"__default__ \
        lora_raw_get_cmd_str(_evt), _sync ? __green__"" : __red__"non-")

static const char* lora_raw_get_cmd_str(lora_raw_process_event_t cmd) {
    static const char* names [] = {
        [__LORA_RAW_PROCESS_TX_REQUEST] = "tx-request",
        [__LORA_RAW_PROCESS_TX_DONE] = "tx-done",
        [__LORA_RAW_PROCESS_TX_TIMEOUT] = "tx-timeout",
        [__LORA_RAW_PROCESS_RX_REQUEST] = "rx-request",
        [__LORA_RAW_PROCESS_RX_DONE] = "rx-done",
        [__LORA_RAW_PROCESS_RX_TIMEOUT] = "rx-timeout",
        [__LORA_RAW_PROCESS_RX_ERROR] = "rx-error",
        [__LORA_RAW_PROCESS_RX_CONT_START] = "rx-cont-start",
        [__LORA_RAW_PROCESS_RX_CONT_STOP] = "rx-cont-stop",
        [__LORA_RAW_PROCESS_CAD_DONE] = "cad-done",
        [__LORA_RAW_PROCESS_RADIO_IRQ] = "radio-irq",
        [__LORA_RAW_PROCESS_RADIO_CONFIG] = "radio-config",
        [__LORA_RAW_PROCESS_RADIO_TOA_EXPIRE] = "ToA-expired",
        [__LORA_RAW_PROCESS_OPERATION_TIMEOUT] = "opr-timeout",
        [__LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE] = "start-tx_cont-wave",
        [__LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE_END] = "end-tx_cont-wave"
    };
    return names[cmd];
}

static lora_event_callback_t * s_registered_callback;

static order_t* p_rx_order = NULL;
static order_t* p_tx_order = NULL;

static void reset_process_states_saved_orders(void)
{
    p_rx_order = p_tx_order = NULL;
}

/* ############################# IDLE State ################################# */
__sm_trans(lora_raw, idle,      req_tx,         start_tx,           tx         )
__sm_trans(lora_raw, idle,      req_rx,         start_rx,           rx         )
__sm_trans(lora_raw, idle,      radio_irq,      process_irq,        idle       )
__sm_trans(lora_raw, idle,      req_rx_cont,    start_rx,           rx_cont    )
/* ############################# TX   State ################################# */
__sm_trans(lora_raw, tx,        tx_done,        handle_tx_done,     idle       )
__sm_trans(lora_raw, tx,        tx_timeout,     handle_tx_timeout,  idle       )
__sm_trans(lora_raw, tx,        opr_timeout,    handle_tx_timeout,  idle       )
__sm_trans(lora_raw, tx,        radio_irq,      process_irq,        tx         )
/* ############################# RX   State ################################# */
__sm_trans(lora_raw, rx,        radio_irq,      process_irq,        rx         )
__sm_trans(lora_raw, rx,        rx_done,        handle_rx_done,     idle       )
__sm_trans(lora_raw, rx,        rx_timeout,     handle_rx_timeout,  idle       )
__sm_trans(lora_raw, rx,        opr_timeout,    handle_rx_timeout,  idle       )
__sm_trans(lora_raw, rx,        rx_fail,        handle_rx_fail,     idle       )
__sm_trans(lora_raw, rx,        req_tx,         start_tx,           tx         )
/* ########################## Time_on_Air  State ############################ */
__sm_trans(lora_raw, toa,       toa_expire,     back_to_rx,         rx         )
__sm_trans(lora_raw, toa,       opr_timeout,    handle_rx_timeout,  idle       )
__sm_trans(lora_raw, toa,       radio_irq,      postpone,           toa        )
__sm_trans(lora_raw, toa,       req_tx,         start_tx,           tx         )
/* ############################# RX_Cont  State ############################# */
__sm_trans(lora_raw, rx_cont,   end_rx_cont,    stop_rx_cont,       idle       )
__sm_trans(lora_raw, rx_cont,   radio_irq,      process_irq,        rx_cont    )
__sm_trans(lora_raw, rx_cont,   rx_done,        handle_rx_done,     rx_cont    )
__sm_trans(lora_raw, rx_cont,   req_tx,         start_tx,           tx_temp    )

__sm_trans(lora_raw, toa_temp,  end_rx_cont,    stop_rx_cont,       idle       )
__sm_trans(lora_raw, toa_temp,  toa_expire,     back_to_rx,         rx_cont    )
__sm_trans(lora_raw, toa_temp,  radio_irq,      postpone,           toa_temp   )
__sm_trans(lora_raw, toa_temp,  req_tx,         start_tx,           tx_temp    )

__sm_trans(lora_raw, tx_temp,   end_rx_cont,    do_nothing,         tx         )
__sm_trans(lora_raw, tx_temp,   tx_done,        handle_tx_done,     rx_cont    )
__sm_trans(lora_raw, tx_temp,   tx_timeout,     handle_tx_timeout,  rx_cont    )
__sm_trans(lora_raw, tx_temp,   opr_timeout,    handle_tx_timeout,  rx_cont    )
__sm_trans(lora_raw, tx_temp,   radio_irq,      process_irq,        tx_temp    )
/* ############################# TX_Cont  State ############################# */
__sm_trans(lora_raw, tx_cont,   tx_timeout,     radio_sleep,        idle       )
__sm_trans(lora_raw, tx_cont,   end_tx_cont,    radio_sleep,        idle       )

/** -------------------------------------------------------------------------- *
 * actions definitions
 * --------------------------------------------------------------------------- * 
 */
static void start_tx_timer(void* data)
{
    order_t* p_order = data;
    if(__sm_present_state_id(lora_raw) != __sm_state_id(lora_raw, tx_temp)) {
        operation_deadline_timer_stop();
        uint32_t timeout = p_order->request_payload.tx_payload.timeout;
        if( timeout == 0) {
            lora_raw_param_t param = {.type = __LORA_RAW_PARAM_TX_TIMEOUT};
            lora_raw_radio_get_param( & param );
            timeout = param.param.tx_timeout;
        }
        operation_deadline_timer_start(timeout);
    }
}

__sm_state_enter(lora_raw, idle)(void* data)
{
    operation_deadline_timer_stop();
}
__sm_state_enter(lora_raw, tx)(void* data)
{
    start_tx_timer(data);
}
__sm_state_enter(lora_raw, tx_temp)(void* data)
{
    start_tx_timer(data);
}
__sm_state_enter(lora_raw, rx_cont)(void* data)
{
    operation_deadline_timer_stop();
}
__sm_state_enter(lora_raw, rx)(void* data)
{
    order_t* p_order = data;
    if(__sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, idle)) {
        uint32_t timeout = p_order->request_payload.rx_payload.timeout;
        if( timeout == 0) {
            lora_raw_param_t param = {.type = __LORA_RAW_PARAM_RX_TIMEOUT};
            lora_raw_radio_get_param( & param );
            timeout = param.param.rx_timeout;
        }
        operation_deadline_timer_start(timeout);
    }
}
__sm_state_leave(lora_raw, toa)(void* data)
{
    time_on_air_stop();
}
__sm_state_leave(lora_raw, toa_temp)(void* data)
{
    time_on_air_stop();
}

__sm_state_default_action(lora_raw, idle)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, tx)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, toa)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, rx)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, rx_cont)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, tx_temp)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, toa_temp)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_state_default_action(lora_raw, tx_cont)(void* data)
{
    order_t* p_order = data;
    order_respond(p_order, __no_callback, NULL, 0);
}

__sm_action(lora_raw, radio_sleep)(void* data)/* ---------------- radio_sleep */
{
    order_t* p_order = data;
    lora_raw_radio_sleep();
    order_respond(p_order, __no_callback, NULL, 0);
}
__sm_action(lora_raw, start_tx)(void* data)/* ---------------------- start_tx */
{
    order_t* p_order = data;

    if( p_rx_order )
    {
        order_respond(p_rx_order, __no_callback, NULL, 0);
        p_rx_order = NULL;
    }

    lora_raw_radio_send(
        p_order->request_payload.tx_payload.buf,
        p_order->request_payload.tx_payload.len );
    if(p_order->sync)
        p_tx_order = p_order;
    else
        order_respond(p_order, __no_callback, NULL, 0);
}

__sm_action(lora_raw, start_rx)(void* data)/* ---------------------- start_rx */
{
    order_t* p_order = data;
    lora_raw_radio_recv();
    if(p_order->sync)
        p_rx_order = p_order;
    else
        order_respond(p_order, __no_callback, NULL, 0);
}

__sm_action(lora_raw, process_irq)(void* data)/* ---------------- process_irq */
{
    order_t* p_order = data;

    if(__sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, rx) ||
        __sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, rx_cont) )
    {
        uint32_t events = lora_radio_ext_get_irqs();
        bool rx_done = (events & __LORA_RADIO_IRQ_RX_DONE) != 0;
        bool valid_header = (events & __LORA_RADIO_IRQ_HEADER_VALID) != 0;
        if( valid_header & ! rx_done )
        {
            time_on_air_start();
            if(__sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, rx))
                __sm_ch_state(lora_raw, toa);
            else
                __sm_ch_state(lora_raw, toa_temp);

            order_respond(p_order, __no_callback, NULL, 0);
            return;
        }
    }
    lora_raw_radio_process_irqs();

    order_respond(p_order, __no_callback, NULL, 0);
}

__sm_action(lora_raw, handle_tx_done)(void* data)/* ---------- handle_tx_done */
{
    order_t* p_order = data;

    lora_raw_radio_sleep();

    if(__sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, tx_temp))
    {
        lora_raw_radio_recv();
    }

    if(p_tx_order) {
        order_respond(p_tx_order, __LORA_EVENT_TX_DONE, NULL, 0);
        p_tx_order = NULL;
        order_respond(p_order, __no_callback, NULL, 0);
    } else {
        order_respond(p_order, __LORA_EVENT_TX_DONE, NULL, 0);
    }
}

__sm_action(lora_raw, handle_tx_timeout)(void* data)/* ---- handle_tx_timeout */
{
    order_t* p_order = data;
    operation_deadline_timer_stop();
    lora_raw_radio_sleep();
    if(__sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, tx_temp))
    {
        lora_raw_radio_recv();
    }

    if(p_tx_order)
    {
        if(p_tx_order->sync)
            order_respond(p_tx_order, __LORA_EVENT_TX_TIMEOUT, NULL, 0);
        else
            order_respond(p_tx_order, __no_callback, NULL, 0);
        p_tx_order = NULL;
        order_respond(p_order, __no_callback, NULL, 0);
    } else {
        order_respond(p_order, __LORA_EVENT_TX_TIMEOUT, NULL, 0);
    }
}

__sm_action(lora_raw, handle_rx_done)(void* data)/* ---------- handle_rx_done */
{
    order_t* p_order = data;

    if(__sm_present_state_id(lora_raw) != __sm_state_id(lora_raw, rx_cont))
        lora_raw_radio_sleep();

    rx_buffer_copy_from(
        p_order->request_payload.rx_done_payload.buf,
        p_order->request_payload.rx_done_payload.len);

    __log_debug("[radio:: buf=%p, len=%d] ==> [process:: buf=%p, len=%d]",
            p_order->request_payload.rx_done_payload.buf,
            p_order->request_payload.rx_done_payload.len,
            __rx_buffer_ptr(), __rx_buffer_len());

    lora_raw_rx_event_data_t evt_data = {
        .buf = __rx_buffer_ptr(),
        .len = __rx_buffer_len(),
        .rssi = p_order->request_payload.rx_done_payload.rssi,
        .snr = p_order->request_payload.rx_done_payload.snr,
    };

    if( p_rx_order && p_rx_order->sync )
    {
        rx_buffer_copy_to(
            p_rx_order->request_payload.rx_payload.buf,
            p_rx_order->request_payload.rx_payload.buf_len,
            p_rx_order->request_payload.rx_payload.p_len);

        __log_debug("[process:: buf=%p, len=%d] ==> [caller:: buf=%p, len=%d]",
                __rx_buffer_ptr(), __rx_buffer_len(),
                p_rx_order->request_payload.rx_payload.buf,
                * p_rx_order->request_payload.rx_payload.p_len);

        order_respond(p_rx_order, __LORA_EVENT_RX_DONE, &evt_data,
            sizeof(lora_raw_rx_event_data_t));
        p_rx_order = NULL;
        order_respond(p_order, __no_callback, NULL, 0);
    }
    else
    {
        if(p_rx_order) {
            order_respond(p_rx_order, __no_callback, NULL, 0);
            p_rx_order = NULL;
        }
        order_respond(p_order, __LORA_EVENT_RX_DONE, &evt_data,
            sizeof(lora_raw_rx_event_data_t));
    }
}

__sm_action(lora_raw, handle_rx_timeout)(void* data)/* ---- handle_rx_timeout */
{
    order_t* p_order = data;

    if(__sm_present_state_id(lora_raw) != __sm_state_id(lora_raw, rx_cont))
        lora_raw_radio_sleep();

    if( p_rx_order )
    {
        if(p_rx_order->sync)
            order_respond(p_rx_order, __LORA_EVENT_RX_TIMEOUT, NULL, 0);
        else
            order_respond(p_rx_order, __no_callback, NULL, 0);
        p_rx_order = NULL;
        order_respond(p_order, __no_callback, NULL, 0);
    } else {
        order_respond(p_order, __LORA_EVENT_RX_TIMEOUT, NULL, 0);
    }
}

__sm_action(lora_raw, handle_rx_fail)(void* data)/* ---------- handle_rx_fail */
{
    order_t* p_order = data;

    lora_raw_radio_sleep();
    if(__sm_present_state_id(lora_raw) == __sm_state_id(lora_raw, rx_cont))
        lora_raw_radio_recv();

    if( p_rx_order )
    {
        if(p_rx_order->sync)
            order_respond(p_rx_order, __LORA_EVENT_RX_FAIL, NULL, 0);
        else
            order_respond(p_rx_order, __no_callback, NULL, 0);
        p_rx_order = NULL;
        order_respond(p_order, __no_callback, NULL, 0);
    } else {
        order_respond(p_order, __LORA_EVENT_RX_FAIL, NULL, 0);
    }
}

__sm_action(lora_raw, back_to_rx)(void* data)/* ------------------ back_to_rx */
{
    order_t* p_order = data;

    lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_IRQ, NULL, false);

    order_respond(p_order, __no_callback, NULL, 0);
}

__sm_action(lora_raw, stop_rx_cont)(void* data)/* -------------- stop_rx_cont */
{
    order_t* p_order = data;

    lora_raw_radio_sleep();

    order_respond(p_order, __no_callback, NULL, 0);
}

/** -------------------------------------------------------------------------- *
 * process handler
 * --------------------------------------------------------------------------- *
 */
static input_id_t lora_raw_sm_get_input_id(lora_raw_process_event_t proc_evt)
{
    switch(proc_evt) {
    case __LORA_RAW_PROCESS_TX_REQUEST:
        return __sm_input_id(lora_raw, req_tx);
    case __LORA_RAW_PROCESS_TX_DONE:
        return __sm_input_id(lora_raw, tx_done);
    case __LORA_RAW_PROCESS_TX_TIMEOUT:
        return __sm_input_id(lora_raw, tx_timeout);
    case __LORA_RAW_PROCESS_RX_REQUEST:
        return __sm_input_id(lora_raw, req_rx);
    case __LORA_RAW_PROCESS_RX_DONE:
        return __sm_input_id(lora_raw, rx_done);
    case __LORA_RAW_PROCESS_RX_TIMEOUT:
        return __sm_input_id(lora_raw, rx_timeout);
    case __LORA_RAW_PROCESS_RX_ERROR:
        return __sm_input_id(lora_raw, rx_fail);
    case __LORA_RAW_PROCESS_RX_CONT_START:
        return __sm_input_id(lora_raw, req_rx_cont);
    case __LORA_RAW_PROCESS_RX_CONT_STOP:
        return __sm_input_id(lora_raw, end_rx_cont);
    case __LORA_RAW_PROCESS_RADIO_IRQ:
        return __sm_input_id(lora_raw, radio_irq);
    case __LORA_RAW_PROCESS_RADIO_TOA_EXPIRE:
        return __sm_input_id(lora_raw, toa_expire);
    case __LORA_RAW_PROCESS_OPERATION_TIMEOUT:
        return __sm_input_id(lora_raw, opr_timeout);
    case __LORA_RAW_PROCESS_CAD_DONE:
    case __LORA_RAW_PROCESS_RADIO_CONFIG:
    case __LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE:
        return 0;
    case __LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE_END:
        return __sm_input_id(lora_raw, end_tx_cont);
    }
    return 0;
}

static void handle_radio_config(order_t* p_order)
{
    __log_info("process radio config apply");
    lora_stub_timers_stop_all();
    lora_raw_radio_dtor();
    lora_raw_radio_ctor();
    __sm_ch_state(lora_raw, idle);

    order_cancel_all();
}

static void handle_radio_tx_cont_wave(order_t* p_order)
{
    __log_info("process radio config apply");
    lora_stub_timers_stop_all();
    lora_raw_radio_sleep();
    __sm_ch_state(lora_raw, tx_cont);

    lora_raw_radio_tx_cont_wave(p_order->request_payload.tx_cont_wave.freq,
        p_order->request_payload.tx_cont_wave.power,
        p_order->request_payload.tx_cont_wave.timeout);

    order_cancel_all();
}

static void lora_raw_process_handler(void* data)
{
    __log_info(__purple__"-- new processing cycle --");

    order_t* p_order = *(order_t**)data;

    if( p_order->req_type == __LORA_RAW_PROCESS_RADIO_CONFIG )
    {
        __log_info("process radio config apply");
        handle_radio_config(p_order);
    }
    else if(p_order->req_type == __LORA_RAW_PROCESS_RADIO_TX_CONT_WAVE)
    {
        handle_radio_tx_cont_wave(p_order);
    }
    else
    {
        __sm_run(lora_raw, lora_raw_sm_get_input_id(p_order->req_type),
            p_order);
    }

    while( ( p_order = order_get_responded() ) != NULL )
    {
        if( p_order->sync )
        {
            sync_obj_release(p_order->sync_obj);
        }

        if( s_registered_callback &&
                 p_order->respond_payload.event != __no_callback )
        {
            s_registered_callback(p_order->respond_payload.event,
                & p_order->respond_payload.event_data);
        }
        order_free( p_order );
    }
}

static void lora_port_service_level_irq_handler(void)
{
    __log_info("notify from irq handler");
    lora_raw_process_event(__LORA_RAW_PROCESS_RADIO_IRQ, NULL, false);
}

/** -------------------------------------------------------------------------- *
 * APIs Implementations
 * --------------------------------------------------------------------------- *
 */
void lora_raw_process_ctor(void)
{
    __log_info("ctor() -> lora raw processor");

    lora_event_handler_register(__lora_evt_raw_process_cmd,
        lora_raw_process_handler);

    extern void lora_port_set_service_level_irq_handler(void(*p_handler)(void));
    lora_port_set_service_level_irq_handler(
        lora_port_service_level_irq_handler);

    __sm_ch_state(lora_raw, idle);

    time_on_air_ctor();
    operation_deadline_timer_ctor();
    __order_access_ctor();
}

void lora_raw_process_dtor(void)
{
    __log_info("~dtor() -> lora raw process");
    time_on_air_dtor();
    operation_deadline_timer_dtor();
    lora_event_handler_deregister(__lora_evt_raw_process_cmd);
    order_cancel_all();
    lora_raw_radio_sleep();
}

void lora_raw_process_register_callback(lora_event_callback_t * p_callback)
{
    __log_info("raw process callback registered");
    s_registered_callback = p_callback;
}

void lora_raw_process_event(
    lora_raw_process_event_t event,
    lora_raw_process_event_payload_t* event_data,
    bool sync)
{
    order_t *   p_order;
    sync_obj_t  sync_obj = 0;

    do{
        p_order = order_request(event, sync, sync_obj, event_data);
    } while( ! p_order);

    if(sync)
    {
        sync_obj = sync_obj_acquire( lora_raw_get_cmd_str(event) );
        p_order->sync_obj = sync_obj;
    }

    lora_event_handler_issue(__lora_evt_raw_process_cmd,
        &p_order, sizeof(p_order));

    if( sync )
    {
        sync_obj_wait( sync_obj );
    }
}

void lora_raw_sm_print(void)
{
    __sm_disp(lora_raw);
}

/* --- end of file ---------------------------------------------------------- */
