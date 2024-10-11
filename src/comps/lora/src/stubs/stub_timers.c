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
 * @brief   This file implements an adaptation layer between the lora stack and
 *          the underlying connected port layer.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#define __log_subsystem  lora
#define __log_component  stub_timers
#include "log_lib.h"
#include "lora_port.h"
#include "stub_timers.h"

#include "system/timer.h"   // -- timers definitions of LoRaMac
#include "adt_list.h"

/** -------------------------------------------------------------------------- *
 * stub initialization
 * --------------------------------------------------------------------------- *
 */
static lora_port_timer_init_t       * p_timer_init;
static lora_port_timer_delete_t     * p_timer_delete;
static lora_port_timer_start_t      * p_timer_start;
static lora_port_timer_stop_t       * p_timer_stop;
static lora_port_timer_set_period_t * p_timer_set_period;
static lora_port_get_timestamp_ms_t * p_get_timestamp_msec;

void lora_stub_timers_init(void* p_init_params)
{
    __log_info("ctor() -> intialize lora timers stub");
    lora_port_params_t * ptr = p_init_params;

    p_timer_init    = ptr->timer_init;
    p_timer_delete  = ptr->timer_delete;
    p_timer_start   = ptr->timer_start;
    p_timer_stop    = ptr->timer_stop;
    p_timer_set_period = ptr->timer_set_period;
    p_get_timestamp_msec = ptr->get_timestamp_msec;
}

/** -------------------------------------------------------------------------- *
 * stub middle-timers
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    adt_list_t      links;
    const char*     name;
    void(* p_callback)(void*);
    void*           handle;
    void*           arg;
} stub_timer_t;

static stub_timer_t * stub_timers_list = NULL;

#define __max_stub_timers_resources     20
static stub_timer_t stub_timers_resources[__max_stub_timers_resources] = {0};

static stub_timer_t* get_timer(void* handle)
{
    stub_timer_t* p_timer;
    __adt_list_foreach(stub_timers_list, p_timer)
    {
        if(handle == p_timer->handle)
        {
            return p_timer;
        }
    }
    return NULL;
}

static void stub_timer_callback(void* handle)
{
    stub_timer_t* p_timer = get_timer(handle);
    if( p_timer )
    {
        p_timer->p_callback(p_timer->arg);
        return;
    }
    __log_error("unknown callback timer");
}

void* lora_stub_timer_init(const char* name, void(*cb)(void*), void* arg)
{
    // -- look if it is init before
    stub_timer_t* p_timer;
    __adt_list_foreach(stub_timers_list, p_timer)
    {
        if( p_timer->p_callback == cb && p_timer->arg == arg )
        {
            __log_warn("timer exists --> handle:%p, cb:%p, arg:%p",
                p_timer->handle, cb, arg);
            return p_timer->handle;
        }
    }

    // -- find free resource location
    int i;
    p_timer = NULL;
    for(i = 0; i < __max_stub_timers_resources; ++i)
    {
        if(stub_timers_resources[i].handle == NULL)
        {
            p_timer = & stub_timers_resources[i];
            break;
        }
    }

    if( p_timer == NULL )
    {
        __log_error("no enough resource entry for new timer init");
        return NULL;
    }

    __adt_list_push(stub_timers_list, p_timer);

    // -- init a new timer
    void* handle = p_timer_init(name, NULL, stub_timer_callback);
    p_timer->handle = handle;
    p_timer->p_callback = cb;
    p_timer->arg = arg;
    p_timer->name = name;

    __log_debug(__blue__"timer new init --> handle:%p, cb:%p, arg:%p",
                handle, cb, arg);
    return handle;
}

void lora_stub_timer_delete(void* handle)
{
    #define __disable_delete    (1)
    stub_timer_t * p_timer = get_timer(handle);
    if( p_timer )
    {
        #if __disable_delete
        __log_debug(__blue__"timer delete(stop) --> handle:%p, cb:%p, arg:%p",
                handle, p_timer->p_callback, p_timer->arg);
        p_timer_stop(handle);
        #else
        __log_debug(__blue__"timer delete --> handle:%p, cb:%p, arg:%p",
            handle, p_timer->p_callback, p_timer->arg);
        __adt_list_del(stub_timers_list, p_timer);
        memset(p_timer, 0, sizeof(stub_timer_t));
        p_timer_delete(handle);
        #endif
    }
    else
        __log_error("timer delete not found timer --> handle:%p", handle);
}

void lora_stub_timer_start(void* handle, uint32_t period_ms)
{
    uint32_t ts_1 = p_get_timestamp_msec();
    stub_timer_t* p_timer = get_timer(handle);
    if( p_timer )
    {
        if(period_ms) {
            uint32_t offset = p_get_timestamp_msec() - ts_1;
            if( period_ms > offset )
                p_timer_set_period(handle, period_ms - offset);
        }
        __log_debug(__blue__"timer start --> handle:%p, cb:%p, arg:%p, msec:%d",
                handle, p_timer->p_callback, p_timer->arg, period_ms);
        p_timer_start(handle);
        return;
    }
    __log_error("start unknown timer handle:%p", handle);
}

void lora_stub_timer_set_period(void* handle, uint32_t period_ms)
{
    stub_timer_t* p_timer = get_timer(handle);
    if( p_timer )
    {
        __log_debug(__blue__"timer set period --> handle:%p, cb:%p, arg:%p, "
            "msec:%d", handle, p_timer->p_callback, p_timer->arg, period_ms);
        p_timer_set_period(handle, period_ms);
        return;
    }
    __log_error("set period for unknown timer handle:%p", handle);
}

void lora_stub_timer_stop( void* handle )
{
    stub_timer_t* p_timer = get_timer(handle);
    if( p_timer )
    {
        __log_debug(__blue__"timer stop --> handle:%p, cb:%p, arg:%p",
                handle, p_timer->p_callback, p_timer->arg);
        p_timer_stop(handle);
        return;
    }
    __log_error("stop for unknown timer handle:%p", handle);
}

void lora_stub_timers_stop_all(void)
{
    __log_info("stop all lora timers");

    stub_timer_t * p_timer;
    __adt_list_foreach(stub_timers_list, p_timer)
    {
        p_timer_stop(p_timer->handle);
    }
}

/** -------------------------------------------------------------------------- *
 * lora-mac timers stubs
 * --------------------------------------------------------------------------- *
 */
static const char* s_timer_name = "lora-mac-timer";

void TimerInit( TimerEvent_t *obj, void (*callback)(void*) )
{
    obj->Context = lora_stub_timer_init(s_timer_name, callback, NULL);
}
void TimerStart( TimerEvent_t *obj )
{
    lora_stub_timer_start(obj->Context, 0);
}
void TimerStartWithPeriod( TimerEvent_t *obj, uint32_t value )
{
    lora_stub_timer_start(obj->Context, value);
}
void TimerStop( TimerEvent_t *obj )
{
    lora_stub_timer_stop(obj->Context);
}
void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    lora_stub_timer_set_period(obj->Context, value);
}

#if 0
typedef struct _stub_timer_node_s {
    struct _stub_timer_node_s * next;
    void* handle;
    void* cb;
} stub_timer_node_t;
#define __stub_timers_max  20
static stub_timer_node_t stub_timers_arr[__stub_timers_max] = {0};
static stub_timer_node_t * p_stub_timers_list = NULL;
#define __stub_timers_reset()                                   \
    do{                                                         \
        memset(stub_timers_arr, 0, sizeof(stub_timers_arr));    \
        p_stub_timers_list = NULL;                              \
    } while(0)
#define __stub_timers_foreach(it)   \
    for( it = p_stub_timers_list; it != NULL; it = it->next )
#define __stub_timers_add(h, cb)  stub_timers_add(h, cb)
static void stub_timers_add(void* handle, void *cb )
{
    stub_timer_node_t* node = stub_timers_arr;
    int i;
    for(i = 0; i < __stub_timers_max; ++i) {
        if( node->handle == NULL ) {
            node->handle = handle;
            node->cb  = cb;
            node->next = p_stub_timers_list;
            p_stub_timers_list = node;
            return;
        }
        ++ node;
    }
    __log_error("no enough entry for new timer init");
}
#define __stub_timers_get(cb) stub_timers_get( cb )
static void* stub_timers_get( void*cb )
{
    stub_timer_node_t * it;
    __stub_timers_foreach(it) {
        if( cb == it->cb ) {
            return it->handle;
        }
    }
    return NULL;
}


void lora_stub_timers_stop_all(void)
{
    stub_timer_node_t * it;
    __log_info("stop all lora timers");
    __stub_timers_foreach(it) {
        p_timer_stop(it->handle);
    }
}

void* lora_stub_timer_init(const char* name, void(*callback)(void*), void* arg)
{
    if(p_timer_init)
        return p_timer_init(name, arg, callback);
    else
        return NULL;
}
void lora_stub_timer_delete(void* handle)
{
    if(p_timer_delete)
        p_timer_delete(handle);
}
void lora_stub_timer_start(void* handle, uint32_t period_ms)
{
    if(p_timer_set_period)
        p_timer_set_period(handle, period_ms);
}
void lora_stub_timer_stop(void* handle)
{
    if(p_timer_stop)
        p_timer_stop(handle);
}

void TimerInit( TimerEvent_t *obj, void ( *callback )( void *context ) )
{
    if(p_timer_init) {
        void* handle = __stub_timers_get(callback);
        if( handle == NULL ) {
            handle = p_timer_init(s_timer_name, NULL, callback);
            __stub_timers_add( handle, callback );
            __log_debug(__purple__"init new timer [obj:%p] [handle:%p]",
                obj, handle);
        } else {
            __log_debug(__blue__"timer already exist [obj:%p] [handle:%p]",
                obj, handle);
        }
        obj->Context = handle;
        obj->Callback = callback;
    } else
        __log_warn("port "__red__"timer_init"__default__" disconnected");
}

void TimerStart( TimerEvent_t *obj )
{
    if(p_timer_start) {
        void* handle = __stub_timers_get(obj->Callback);
        if(handle == NULL) {
            __log_error("invalid timer [obj:%p] [handle:%p]",
                obj, handle);
            return;
        }
        if(handle == obj->Context) {
            p_timer_start(obj->Context);
        } else {
            __log_error("fatal -> start non-init timer");
        }
    }
    else
        __log_warn("port "__red__"timer_start"__default__" disconnected");
}

void TimerStop( TimerEvent_t *obj )
{
    if(p_timer_stop) {
        void* handle = __stub_timers_get(obj->Callback);
        if(handle == NULL) {
            __log_error("invalid timer [obj:%p] [handle:%p]",
                obj, handle);
            return;
        }
        if(handle == obj->Context) {
            p_timer_stop(obj->Context);
        } else {
            __log_error("fatal -> stop non-init timer");
        }
    }
    else
        __log_warn("port "__red__"timer_stop"__default__" disconnected");
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    if(p_timer_set_period) {
        void* handle = __stub_timers_get(obj->Callback);
        if(handle == NULL) {
            __log_error("invalid timer [obj:%p] [handle:%p]",
                obj, handle);
            return;
        }
        if(handle == obj->Context) {
            p_timer_set_period(obj->Context, value);
        } else {
            __log_error("fatal -> set-period non-init timer");
        }
    }
    else
        __log_warn("port "__red__"timer_set_period"__default__" disconnected");
}
#endif

/* --- end of file ---------------------------------------------------------- */
