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
 * @brief   This file handle and organizes the micropython callbacks for the
 *          lora-stack.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#define __log_subsystem  lora
#define __log_component  mpy_lora
#include "log_lib.h"
#include "mp_lite_if.h"
// #include "mpirq.h"
#include "lora.h"
#include "lora_wan_duty.h"
#include "mphalport.h"

#include "mpy_lora_cb_if.h"

/** -------------------------------------------------------------------------- *
 * Populating the callback events as a uPython mudule constants:
 * --------------------------------------------------------------------------- *
 */
__mp_mod_include(lora, "mpy_lora_cb_if.h")
__mp_mod_class_const(lora, _event, EVENT_TX_DONE,   __MPY_LORA_CB_ON_TX_DONE)
__mp_mod_class_const(lora, _event, EVENT_TX_TIMEOUT,__MPY_LORA_CB_ON_TX_TIMEOUT)
__mp_mod_class_const(lora, _event, EVENT_TX_FAILED, __MPY_LORA_CB_ON_TX_FAIL)
__mp_mod_class_const(lora, _event, EVENT_TX_CONFIRM,__MPY_LORA_CB_ON_TX_CONFIRM)
__mp_mod_class_const(lora, _event, EVENT_RX_DONE,   __MPY_LORA_CB_ON_RX_DONE)
__mp_mod_class_const(lora, _event, EVENT_RX_TIMEOUT,__MPY_LORA_CB_ON_RX_TIMEOUT)
__mp_mod_class_const(lora, _event, EVENT_RX_FAIL,   __MPY_LORA_CB_ON_RX_FAIL)
__mp_mod_class_const(lora, _event, EVENT_ANY,       0xFF)

/** -------------------------------------------------------------------------- *
 * The following callback strategy is followed:
 * 
 * - The LoRa system supports only one user callback to be registered. Through
 *   this callback the user can know which event has occurred and its attached
 *   data if available.
 * 
 * - The uPython module here support the callback event ditribution.
 *   -> The user can register one callback for all LoRa events
 *   -> The user can specialize a callback for any opened port.
 *   -> The user can specialize a callback for any occerred event.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * Callbacks internal data structure for callbacks distribution
 * --------------------------------------------------------------------------- *
 */

typedef struct {
    uint8_t     port_num;
    mp_obj_t    cb_on_event[__mpy_lora_max_callback_events_count];
} mpy_lora_callbacks_t;

static struct {
    mpy_lora_callbacks_t   any;
    mpy_lora_callbacks_t   port_special_cb[__max_supported_simultaneous_ports];
} mpy_callbacks_main;

static mpy_lora_callbacks_t* acquire_port_callbacks_resource(int port);
static bool release_port_callbacks_resource(int port);
static void lora_stack_callback(lora_event_t event, void* event_data);

static void init_callback_struct(mpy_lora_callbacks_t* p_cb_struct)
{
    int i;
    for(i = 0; i < __mpy_lora_max_callback_events_count; ++i)
    {
        p_cb_struct->cb_on_event[i] = mp_const_none;
    }
}

void mpy_lora_callback_init(void)
{
    init_callback_struct( & mpy_callbacks_main.any );

    int i;
    for(i = 0; i < __max_supported_simultaneous_ports; ++i)
    {
        mpy_callbacks_main.port_special_cb[i].port_num = 0;
        init_callback_struct( & mpy_callbacks_main.port_special_cb[i] );
    }

    /* attach the main callback to the lora stack */
    lora_callback_t cb_args = {
        .port = __port_any,
        .callback = lora_stack_callback
    };
    lora_ioctl(__LORA_IOCTL_SET_CALLBACK, &cb_args);

}

void mpy_lora_callback_set(
    int port,
    uint32_t on_events,
    mp_obj_t cb_fun_obj)
{
    mpy_lora_callbacks_t * p_cbs = NULL;

    if( port == __port_any ) {
        p_cbs = & mpy_callbacks_main.any;
    } else {
        p_cbs = acquire_port_callbacks_resource(port);
        if( p_cbs == NULL )
        {
            mp_raise_TypeError(
                MP_ERROR_TEXT("no more ports callbacks resources"));
        }
    }

    if( p_cbs ) {
        int i;
        for(i = 0; i < __mpy_lora_max_callback_events_count; ++i)
        {
            if( on_events & (1u << i) )
            {
                p_cbs->cb_on_event[i] = cb_fun_obj;
            }
        }
    }
}

void mpy_lora_callback_unset(int port)
{
    release_port_callbacks_resource(port);
}

static mpy_lora_callbacks_t* acquire_port_callbacks_resource(int port)
{
    if(port == 0) {
        __log_error("port 0 not allowed for applications");
        return NULL;
    }

    int i;
    mpy_lora_callbacks_t * p_cbs, *p_avail = NULL;
    bool found_avail_resource = false;

    for(i = 0; i < __max_supported_simultaneous_ports; ++i)
    {
        p_cbs = & mpy_callbacks_main.port_special_cb[i];

        if( p_cbs->port_num == port )
        {
            /* this resource if acquired before for this port */
            return p_cbs;
        }
        else if( !found_avail_resource && p_cbs->port_num == 0 )
        {
            /* save this available resource entry in case it is first time
               to acquire a resource for this port */
            p_avail = p_cbs;
            found_avail_resource = true;
        }
    }

    if( p_avail != NULL )
    {
        /* initialize the first available resource and reserve is */
        p_cbs = p_avail;
        p_cbs->port_num = port;
        init_callback_struct(p_cbs);
        return p_cbs;
    }

    __log_error("no enough callbacks resources for port %d", port);
    return NULL;
}

static bool release_port_callbacks_resource(int port)
{
    if(port == 0) {
        __log_error("port 0 not allowed for applications");
        return false;
    }

    int i;
    for(i = 0; i < __max_supported_simultaneous_ports; ++i)
    {
        if(mpy_callbacks_main.port_special_cb[i].port_num == port)
        {
            mpy_callbacks_main.port_special_cb[i].port_num = 0;
            return true;
        }
    }

    __log_error("port %d callback resource not found", port);
    return false;
}

static mp_obj_t get_mpy_callback_func(
    int target_port,
    mpy_lora_callback_type_t cb_evt)
{
    if(cb_evt == __MPY_LORA_CB_ON_ANY)
    {
        __log_debug("-- end here --");
        return mp_const_none;
    }

    mpy_lora_callbacks_t * p_any_cb = & mpy_callbacks_main.any;
    mpy_lora_callbacks_t * p_port_cb = NULL;

    if(target_port != __port_any)
    {
        /* find if specialized port callback */
        int i;
        for(i = 0; i < __max_supported_simultaneous_ports; ++i)
        {
            int port = mpy_callbacks_main.port_special_cb[i].port_num;
            if(port && port == target_port)
            {
                __log_info("-- found special port %d callback", target_port);
                p_port_cb = &mpy_callbacks_main.port_special_cb[i];
                break;
            }
        }
    }

    if(p_port_cb == NULL) {
        __log_info("-- no special port %d callback", target_port);
    }

    /* find if specialized event callback */
    int idx = 0;
    uint32_t temp = cb_evt;
    while( (temp & 1) == 0 ) {
        ++idx;
        temp >>= 1;
    }

    mp_obj_t cb_fun_obj = mp_const_none;

    if(p_port_cb)
        cb_fun_obj = p_port_cb->cb_on_event[idx];

    if(cb_fun_obj == mp_const_none)
        cb_fun_obj = p_any_cb->cb_on_event[idx];

    return cb_fun_obj;
}

/** -------------------------------------------------------------------------- *
 * Pycom LoRa-Stack callback conjunction
 * --------------------------------------------------------------------------- *
 */
#define __rx_buf_len  0xff
static uint8_t s_rx_buffer[__rx_buf_len];
static uint8_t s_rx_len;

static void mpy_schedule_user_callback(mp_obj_t func_obj, mp_obj_t arg_obj)
{
    if(func_obj != mp_const_none)
    {
        mp_sched_schedule(func_obj, arg_obj);
    }
}

static mp_obj_t prepare_callback_arg_tuple(
    lora_mode_t mode,
    mpy_lora_callback_type_t  cb_evt,
    void*  evt_data)
{
    mp_obj_t tuple_obj = mp_const_none;

    if(mode == __LORA_MODE_RAW) {
        if(cb_evt == __MPY_LORA_CB_ON_RX_DONE)
        {
            tuple_obj = mp_obj_new_dict(4);
            lora_raw_rx_event_data_t* rx_info = evt_data;
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_event),
                MP_OBJ_NEW_SMALL_INT(__MPY_LORA_CB_ON_RX_DONE));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_data),
                mp_obj_new_bytearray(rx_info->len, rx_info->buf));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_RSSI),
                MP_OBJ_NEW_SMALL_INT(rx_info->rssi));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_SNR),
                MP_OBJ_NEW_SMALL_INT(rx_info->snr));
        }
        else
        {
            tuple_obj = mp_obj_new_dict(1);
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_event),
                MP_OBJ_NEW_SMALL_INT(cb_evt));

        }
    }
    else if( mode == __LORA_MODE_WAN )
    {
        if( cb_evt == __MPY_LORA_CB_ON_RX_DONE )
        {
            tuple_obj = mp_obj_new_dict(7);
            lora_wan_ind_params_t* ind_info = evt_data;
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_event),
                MP_OBJ_NEW_SMALL_INT(cb_evt));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_data),
                mp_obj_new_bytearray(ind_info->len, ind_info->buf));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_RSSI),
                MP_OBJ_NEW_SMALL_INT(ind_info->rx.rssi));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_SNR),
                MP_OBJ_NEW_SMALL_INT(ind_info->rx.snr));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_port),
                MP_OBJ_NEW_SMALL_INT(ind_info->port_num));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_DR),
                MP_OBJ_NEW_SMALL_INT(ind_info->rx.data_rate));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_dl_frame_counter),
                MP_OBJ_NEW_SMALL_INT(ind_info->rx.dl_frame_counter));
        }
        else if (evt_data != NULL)
        {
            tuple_obj = mp_obj_new_dict(2);
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_event),
                MP_OBJ_NEW_SMALL_INT(cb_evt));
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_msg_id),
                MP_OBJ_NEW_SMALL_INT((uint32_t)evt_data));
        }
        else
        {
            tuple_obj = mp_obj_new_dict(1);
            mp_obj_dict_store(tuple_obj, MP_OBJ_NEW_QSTR(MP_QSTR_event),
                MP_OBJ_NEW_SMALL_INT(cb_evt));
        }
    }

    return tuple_obj;
}

static mpy_lora_callback_type_t get_event_callback_type(lora_event_t event)
{
    static struct {uint16_t evt; uint16_t cb_evt;} m [] = {
        { __LORA_EVENT_TX_DONE     ,__MPY_LORA_CB_ON_TX_DONE },
        { __LORA_EVENT_TX_TIMEOUT  ,__MPY_LORA_CB_ON_TX_TIMEOUT },
        { __LORA_EVENT_TX_FAIL     ,__MPY_LORA_CB_ON_TX_FAIL },
        { __LORA_EVENT_TX_CONFIRM  ,__MPY_LORA_CB_ON_TX_CONFIRM },
        { __LORA_EVENT_RX_DONE     ,__MPY_LORA_CB_ON_RX_DONE },
        { __LORA_EVENT_RX_TIMEOUT  ,__MPY_LORA_CB_ON_RX_TIMEOUT },
        { __LORA_EVENT_RX_FAIL     ,__MPY_LORA_CB_ON_RX_FAIL }
    };

    int i;
    for(i = 0; i < sizeof(m)/sizeof(m[0]); i++)
    {
        if( event == m[i].evt ) {
            return m[i].cb_evt;
        }
    }
    return __MPY_LORA_CB_ON_ANY;
}

static void mpy_callback_exec_handler(lora_event_t event, void* event_data)
{
    mpy_lora_callback_type_t cb_evt;

    if(event == __LORA_EVENT_INDICATION)
    {
        /** ------------------------------------------------------ *
         * lora-wan callbacks
         * ------------------------------------------------------- *
         */
        __log_info("lora-wan callback indication handler");
        lora_wan_ind_params_t ind_params = {
            .buf = s_rx_buffer,
            .len = __rx_buf_len
        };

        /* will flush all indications that come until receiving NONE
           event which means no more indications from lora-wan */
        do {
            lora_ioctl(__LORA_IOCTL_PORT_GET_IND_PARAM, & ind_params);

            __log_info("callback::ind-event: "__yellow__"%s"__default__,
                lora_get_event_str(event));

            cb_evt = get_event_callback_type(ind_params.event);

            switch(ind_params.event)
            {
            case __LORA_EVENT_TX_DONE:
            case __LORA_EVENT_TX_CONFIRM:
            case __LORA_EVENT_TX_TIMEOUT:
            case __LORA_EVENT_TX_FAIL:
                mpy_schedule_user_callback(
                    get_mpy_callback_func(ind_params.port_num, cb_evt),
                    prepare_callback_arg_tuple(__LORA_MODE_WAN,
                        cb_evt, (void*)(ind_params.tx.msg_app_id)) );
                break;
            case __LORA_EVENT_RX_DONE:
                mpy_schedule_user_callback(
                    get_mpy_callback_func(ind_params.port_num, cb_evt),
                    prepare_callback_arg_tuple(__LORA_MODE_WAN,
                        cb_evt, &ind_params) );
                break;
            case __LORA_EVENT_RX_TIMEOUT:
            case __LORA_EVENT_RX_FAIL:
                mpy_schedule_user_callback(
                    get_mpy_callback_func(ind_params.port_num, cb_evt),
                    prepare_callback_arg_tuple(__LORA_MODE_WAN, cb_evt, NULL));
                break;
            default:
                __log_info("-- unhandled indication");
            }

        } while( ind_params.event != __LORA_EVENT_NONE );
    }
    else
    {
        /** ------------------------------------------------------ *
         * lora-raw callbacks
         * ------------------------------------------------------- *
         */

        __log_info("callback:: event: "__yellow__"%s"__default__,
            lora_get_event_str(event));
        if(event == __LORA_EVENT_RX_DONE)
        {
            __log_assert(event_data, "invalid received data");

            lora_raw_rx_event_data_t* rx_info = event_data;
            __log_dump(s_rx_buffer, s_rx_len, 8, __log_dump_flag_hide_address |
                __log_dump_flag_disp_char_on_rhs | __log_dump_flag_disp_char,
                __word_len_8);
            mpy_schedule_user_callback(
                get_mpy_callback_func(__port_any, __MPY_LORA_CB_ON_RX_DONE),
                prepare_callback_arg_tuple(__LORA_MODE_RAW,
                    __MPY_LORA_CB_ON_RX_DONE, rx_info));
        }
        else
        {
            cb_evt = get_event_callback_type(event);
            mpy_schedule_user_callback(
                get_mpy_callback_func(__port_any, cb_evt),
                prepare_callback_arg_tuple(__LORA_MODE_RAW, cb_evt, NULL));
        }
    }
}

static mp_obj_t mpy_callback_exec(mp_obj_t arg)
{
    if( ! MP_OBJ_IS_INT(arg) )
    {
        // -- occurs only in the lora-raw rx event callback
        void* ptr = MP_OBJ_TO_PTR(arg);
        mpy_callback_exec_handler(__LORA_EVENT_RX_DONE, ptr);
        free(ptr);
        return mp_const_none;
    }
    else
    {
        mpy_callback_exec_handler(MP_OBJ_SMALL_INT_VALUE(arg), NULL);
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mpy_callback_exec_obj, mpy_callback_exec);

static void lora_stack_callback(lora_event_t event, void* event_data)
{
    mp_obj_t arg_obj;
    if(event == __LORA_EVENT_RX_DONE)
    {
        /**
         * This RX event is for lora-raw mode.
         * The received data shall be captured here because the thread context
         * will be switched after calling mp_sched_schedule() and the data
         * delivered here with this event will not be valid and safe to process.
         */
        __log_assert(event_data, "invalid received data");
        lora_raw_rx_event_data_t* rx_info = event_data;
        /**
         * the free operation must be done after handling this event
         * in the scheduled callback executing function mpy_callback_exec()
         */
        lora_raw_rx_event_data_t* saved = 
            malloc(sizeof(lora_raw_rx_event_data_t) + rx_info->len);
        *saved = *rx_info;
        saved->buf = (uint8_t*)(saved + 1);
        memcpy(saved->buf, rx_info->buf, rx_info->len);

        __log_dump(s_rx_buffer, s_rx_len, 8, __log_dump_flag_hide_address |
            __log_dump_flag_disp_char_on_rhs | __log_dump_flag_disp_char,
            __word_len_8);

        /**
         * the allocation must be aligned to word not byte, so the first least
         * significant bit (LSB) must be zero.
         * micropython use the LSB to descriminate the integer objects.
         */
        __log_assert(((uint32_t)saved & 1u) == 0,
            "non-aligned allocation, can not continue .. panic");
        
        arg_obj = MP_OBJ_FROM_PTR(saved);
    }
    else
    {
        arg_obj = MP_OBJ_NEW_SMALL_INT(event);
    }

    mp_sched_schedule(MP_OBJ_FROM_PTR(&mpy_callback_exec_obj), arg_obj);
}

/* --- end of file ---------------------------------------------------------- */
