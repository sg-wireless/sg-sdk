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

    int i, j;
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
        for( j = 0; j < __mpy_lora_max_callback_events_count; ++j )
        {
            p_cbs->cb_on_event[j] = mp_const_none;
        }
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

/** -------------------------------------------------------------------------- *
 * Pycom LoRa-Stack callback conjunction
 * --------------------------------------------------------------------------- *
 */
#define __rx_buf_len  0xff
static uint8_t s_rx_buffer[__rx_buf_len];
static uint8_t s_rx_len;

static const mp_obj_fun_builtin_fixed_t mpy_callback_exec_obj;

static void lora_stack_callback(lora_event_t event, void* event_data)
{
    __log_info("callback:: event: "__yellow__"%s"__default__,
        lora_get_event_str(event));

    /**
     * This RX event is for lora-raw mode.
     * The received data shall be captured here because the thread context will
     * be switched after calling mp_sched_schedule() and the data delivered here
     * with this event will not be valid and safe to process.
     */
    if(event == __LORA_EVENT_RX_DONE) {
        __log_assert(event_data, "invalid received data");
        lora_raw_rx_event_data_t* p_rx_data = event_data;
        memcpy(s_rx_buffer, p_rx_data->buf, p_rx_data->len);
        s_rx_len = p_rx_data->len;
        __log_dump(s_rx_buffer, s_rx_len, 8, __log_dump_flag_hide_address |
            __log_dump_flag_disp_char_on_rhs | __log_dump_flag_disp_char,
            __word_len_8);
    }

    /**
     * This step will cause the uPython to schedule calling the user callback
     * function in the mp_task which is the main uPython thread of execution
     */
    mp_sched_schedule(MP_OBJ_FROM_PTR(&mpy_callback_exec_obj),
        MP_OBJ_NEW_SMALL_INT(event));
}

/** -------------------------------------------------------------------------- *
 * uPython Callback function conjunction
 * --------------------------------------------------------------------------- *
 */
static void mpy_callback_caller(
    int target_port,
    mpy_lora_callback_type_t cb_type,
    mp_obj_t event_data_obj)
{
    if(cb_type == __MPY_LORA_CB_ON_ANY)
    {
        __log_debug("-- end here --");
        return;
    }

    mp_obj_t event_obj = MP_OBJ_NEW_SMALL_INT(cb_type);

    int i;
    mpy_lora_callbacks_t * p_any_cb = & mpy_callbacks_main.any;
    mpy_lora_callbacks_t * p_port_cb = NULL;
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

    if(p_port_cb == NULL) {
        __log_info("-- no special port %d callback", target_port);
    }

    mp_obj_t cb_fun_obj = mp_const_none;

    int idx = 0;
    /* start counting zeros until the first one */
    int temp = cb_type;
    while( (temp & 1) == 0 ) {
        ++idx;
        temp >>= 1;
    }

    if(p_port_cb)
        cb_fun_obj = p_port_cb->cb_on_event[idx];

    if(cb_fun_obj == mp_const_none)
        cb_fun_obj = p_any_cb->cb_on_event[idx];

    if(cb_fun_obj != mp_const_none) {
        __log_info("-- call uPython callback");
        mp_call_function_2(cb_fun_obj, event_obj, event_data_obj);
    }
}
#ifdef __unittest
extern void lora_unittest_stub_get_indication(lora_wan_ind_params_t* ind_params);
#endif
static mp_obj_t mpy_callback_exec(mp_obj_t arg)
{
    lora_event_t event =  MP_OBJ_SMALL_INT_VALUE(arg);
    mpy_lora_callback_type_t cb_type;
    mp_obj_t event_data_obj;
    __log_debug("-- mod_lora_cb_if callback handler --");

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
        do{
            #ifdef __unittest
            lora_unittest_stub_get_indication(& ind_params);
            #else
            lora_ioctl(__LORA_IOCTL_PORT_GET_IND_PARAM, & ind_params);
            #endif

            event_data_obj = mp_const_none;

            switch(ind_params.event)
            {
            case __LORA_EVENT_TX_DONE:
                __log_info("-- tx done indication");
                cb_type = __MPY_LORA_CB_ON_TX_DONE;
                event_data_obj = MP_OBJ_NEW_SMALL_INT(ind_params.tx.msg_app_id);
                break;
            case __LORA_EVENT_TX_CONFIRM:
                __log_info("-- tx confirm indication");
                cb_type = __MPY_LORA_CB_ON_TX_CONFIRM;
                event_data_obj = MP_OBJ_NEW_SMALL_INT(ind_params.tx.msg_app_id);
                break;
            case __LORA_EVENT_TX_TIMEOUT:
                __log_info("-- tx timeout indication");
                cb_type = __MPY_LORA_CB_ON_TX_TIMEOUT;
                event_data_obj = MP_OBJ_NEW_SMALL_INT(ind_params.tx.msg_app_id);
                break;
            case __LORA_EVENT_TX_FAIL:
                __log_info("-- tx fail indication");
                cb_type = __MPY_LORA_CB_ON_TX_FAIL;
                event_data_obj = MP_OBJ_NEW_SMALL_INT(ind_params.tx.msg_app_id);
                break;
            case __LORA_EVENT_RX_DONE:
                __log_info("-- rx data indication -> len:%d", ind_params.len);
                cb_type = __MPY_LORA_CB_ON_RX_DONE;
                event_data_obj = mp_obj_new_bytearray(
                    ind_params.len, ind_params.buf );
                break;
            case __LORA_EVENT_RX_TIMEOUT:
                __log_info("-- rx timeout indication");
                cb_type = __MPY_LORA_CB_ON_RX_TIMEOUT;
                break;
            case __LORA_EVENT_RX_FAIL:
                __log_info("-- rx fail indication");
                cb_type = __MPY_LORA_CB_ON_RX_FAIL;
                break;
            default:
                __log_info("-- unhandled indication");
                cb_type = __MPY_LORA_CB_ON_ANY;
            }

            mpy_callback_caller(ind_params.port_num, cb_type, event_data_obj);

        } while( ind_params.event != __LORA_EVENT_NONE );
    }
    else
    {
        /** ------------------------------------------------------ *
         * lora-raw callbacks
         * ------------------------------------------------------- *
         */
        event_data_obj = mp_const_none;

        __log_debug("--> event %d", event);

        if( event == __LORA_EVENT_RX_DONE )
        {
            __log_debug("rx event , len: %d, buf = %s", s_rx_len, s_rx_buffer);
            event_data_obj = mp_obj_new_bytearray( s_rx_len, s_rx_buffer );
            cb_type = __MPY_LORA_CB_ON_RX_DONE;
        }
        else if(event == __LORA_EVENT_TX_DONE )
        {
            cb_type = __MPY_LORA_CB_ON_TX_DONE;
        }
        else if(event == __LORA_EVENT_TX_CONFIRM )
        {
            cb_type = __MPY_LORA_CB_ON_TX_CONFIRM;
        }
        else if (event == __LORA_EVENT_TX_FAIL)
        {
            cb_type = __MPY_LORA_CB_ON_TX_FAIL;
        }
        else if(event == __LORA_EVENT_TX_TIMEOUT)
        {
            cb_type = __MPY_LORA_CB_ON_TX_TIMEOUT;
        }
        else if(event == __LORA_EVENT_RX_FAIL)
        {
            cb_type = __MPY_LORA_CB_ON_RX_FAIL;
        }
        else if(event == __LORA_EVENT_RX_TIMEOUT)
        {
            cb_type = __MPY_LORA_CB_ON_RX_TIMEOUT;
        }
        else
        {
            cb_type = __MPY_LORA_CB_ON_ANY;
        }

        mpy_callback_caller(__port_any, cb_type, event_data_obj);
    }

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(mpy_callback_exec_obj, mpy_callback_exec);

#ifdef __unittest
void lora_unittest_lora_stack_callback(lora_event_t event, void* event_data)
{
    lora_stack_callback(event, event_data);
}
#endif

/* --- end of file ---------------------------------------------------------- */
