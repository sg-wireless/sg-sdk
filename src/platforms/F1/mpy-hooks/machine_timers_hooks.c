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
 * @brief   This file contains the implementation of the machine virtual timers.
 *          The micropython machine timers interface is split based on the timer
 *          type request. If it is marked as virtual timer, all APIs are
 *          re-directed to this file.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include "utils_misc.h"
#include "mp_lite_if.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "shared/readline/readline.h"
#include "sdkconfig.h"
#include "board_hooks.h"
#include "log_lib.h"
#include "freertos/timers.h"
#include "adt_list.h"

__log_component_def(F1, hook_timers, cyan, 1, 0)

#ifdef CONFIG_SDK_MPY_HOOK_MACHINE_VIRTUAL_TIMERS_ENABLE

/** -------------------------------------------------------------------------- *
 * Hook virtual timer object definition
 * --------------------------------------------------------------------------- *
 */
typedef struct _machine_virtual_timer_obj_s {
    /**
     * The following two members "base" and "is_virtual" are constituting
     * the footprint of all machine timers objects and the second member
     * "is_virtual" differentiates between the normal machine timer object and
     * the virtual timer object. This differentiation occurs at machine_timer.c
     * level.
     */
    mp_obj_base_t base;
    bool is_virtual;

    /**
     * all incoming object members are specific members to the virtual timer
     * object and they are different than the normal machine timer object in
     * micropython.
     */

    adt_list_t links; /** links to all initialised virtual timers */
        /** linked list accessors helpers */
        #define __links_offset() \
            ((uint32_t)&(((machine_virtual_timer_obj_t*)0)->links))
        #define __obj_to_list_item(_obj)            \
            (adt_list_t*)(void*)((uint8_t*)_obj + __links_offset())
        #define __list_item_to_obj(_item)           \
            (machine_virtual_timer_obj_t*)(void*)   \
                ((uint8_t*)_item - __links_offset())

    /** virtual timer attributes */
    uint32_t period;
    mp_obj_t callback;
    mp_obj_t name;
    uint8_t  repeat;
    uint8_t  enabled;

    /** RTOS timer handler */
    TimerHandle_t handle;

} machine_virtual_timer_obj_t;

static adt_list_t* s_initialised_timers_head = NULL;

/** -------------------------------------------------------------------------- *
 * intialised virtual timers access guard
 * --------------------------------------------------------------------------- *
 */
static SemaphoreHandle_t s_access_mutex = NULL;

static void access_guard_init(void)
{
    static bool initialised = false;

    if(! initialised)
    {
        s_access_mutex = xSemaphoreCreateMutex();
        __log_assert(s_access_mutex != NULL,
            "failed to create mpy-vtimers hook access guard mutex");
        initialised = true;
    }
}

#define __access_lock()                                 \
    do {                                                \
        access_guard_init();                            \
        xSemaphoreTake(s_access_mutex, portMAX_DELAY);  \
    } while(0)

#define __access_unlock()                               \
    do {                                                \
        access_guard_init();                            \
        xSemaphoreGive(s_access_mutex);                 \
    } while(0)

/** -------------------------------------------------------------------------- *
 * Hooks helper functions
 * --------------------------------------------------------------------------- *
 */

static bool virtual_timer_find(void* obj)
{
    void* it;
    __adt_list_foreach(s_initialised_timers_head, it)
    {
        machine_virtual_timer_obj_t *o = __list_item_to_obj(it);
        if( o == obj )
        {
            return true;
        }
    }
    return false;
}

static bool virtual_timer_validate(void* obj)
{
    bool ret;
    __access_lock();
    ret = virtual_timer_find(obj);
    __access_unlock();
    return ret;
}

static bool virtual_timer_delete(void* obj)
{
    bool ret = false;
    __access_lock();
    if(virtual_timer_find(obj))
    {
        __adt_list_del(s_initialised_timers_head, __obj_to_list_item(obj));
        ret = true;
    }
    __access_unlock();
    return ret;
}

static bool virtual_timer_add(void* obj)
{
    bool ret = false;
    __access_lock();
    if( ! virtual_timer_find(obj) )
    {
        __adt_list_push(s_initialised_timers_head, __obj_to_list_item(obj));
        ret = true;
    }
    __access_unlock();
    return ret;
}

static const char* get_timer_name(machine_virtual_timer_obj_t* self)
{
    const char* name = "anonymous-timer";
    if( self->name != mp_const_none )
    {
        name = mp_get_string(self->name);
    }
    return name;
}

/** -------------------------------------------------------------------------- *
 * Hooks implementations
 * --------------------------------------------------------------------------- *
 */
static void virtual_timer_generic_callback(void* arg)
{
    machine_virtual_timer_obj_t *self = pvTimerGetTimerID( arg );

    __log_info("callback() -> %08x ( %s )", arg, get_timer_name( self ));

    if(!self->repeat)
    {
        self->enabled = false;
    }

    if(self->callback != mp_const_none)
    {
        mp_sched_schedule(self->callback, self);
    }
    else
    {
        __log_error(
            "timer [ "__yellow__"%s"__default__" ] expiry without callback",
            get_timer_name( self ) );
    }
}

static void virtual_timer_init_helper(
    machine_virtual_timer_obj_t *self,
    mp_uint_t n_args,
    const mp_obj_t *pos_args,
    mp_map_t *kw_args)
{
    __log_info("init_helper()");
    enum {
        ARG_mode,
        ARG_callback,
        ARG_period,
        ARG_name,
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,    MP_ARG_KW_ONLY|MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_callback,MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_period,  MP_ARG_KW_ONLY|MP_ARG_INT, {.u_int = -1U} },
        { MP_QSTR_name,    MP_ARG_KW_ONLY|MP_ARG_OBJ, {.u_obj = mp_const_none}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args,
        MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    self->repeat = args[ARG_mode].u_int;
    self->period = args[ARG_period].u_int;
    self->callback = args[ARG_callback].u_obj;
    self->name = args[ARG_name].u_obj;

    const char* name = get_timer_name( self );

    bool err = false;

    if(self->period == -1U)
    {
        __log_error("period not set for the timer: %s", name);
        err = true;
    }

    if(self->callback == mp_const_none)
    {
        __log_error("callback not set for the timer: %s", name);
        err = true;
    }

    if(err)
    {
        __log_error("init failed for the timer: %s", name);
        goto clean_and_exit;
    }

    if(self->handle)
    {
        __log_info("re-init the timer");
        if( xTimerGetPeriod(self->handle) != self->period / portTICK_PERIOD_MS )
        {
            if( xTimerChangePeriod(self->handle,
                    self->period / portTICK_PERIOD_MS, 0) != pdPASS )
            {
                __log_error("failed to set new period for the timer: %s", name);
                goto clean_and_exit;
            }
        }

        if( uxTimerGetReloadMode(self->handle) != self->repeat )
        {
            vTimerSetReloadMode(self->handle, self->repeat);
        }
    }
    else
    {
        __log_info("create OS timer ->[self: %08x] %s", self, name);
        self->handle = xTimerCreate(
            "virtual-timer", self->period / portTICK_PERIOD_MS,
            self->repeat, self, (void*)virtual_timer_generic_callback);
    }

    virtual_timer_add(self);
    return;

    clean_and_exit:
    if(self->handle)
    {
        if( xTimerDelete(self->handle, 0) != pdPASS )
        {
            __log_error( "failed to delete timer %s", name );
        }
        self->handle = NULL;
    }
    virtual_timer_delete(self);
}

mp_obj_t hook_mpy_machine_timer_virtual_new(
    const mp_obj_type_t *type,
    size_t n_args,
    size_t n_kw,
    const mp_obj_t *args)
{
    __log_info("new()");
    machine_virtual_timer_obj_t *self;

    self = mp_obj_malloc(machine_virtual_timer_obj_t, type);

    __log_info("new() -> %08x", self);

    self->is_virtual = true;
    self->callback = mp_const_none;
    self->period = -1U;
    self->repeat = 1; // periodic
    self->handle = NULL;
    self->enabled = false;
    memset(&self->links, 0, sizeof(self->links));

    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        virtual_timer_init_helper(self, n_args - 1, args + 1, &kw_args);
    }
    return self;
}

uint32_t hook_mpy_machine_timer_virtual_value(void* obj)
{
    __log_info("value()");
    machine_virtual_timer_obj_t *self = obj;

    return self->period;
}

void hook_mpy_machine_timer_virtual_deinit(void* obj)
{
    __log_info("deinit()");
    machine_virtual_timer_obj_t *self = obj;

    if( ! virtual_timer_validate(obj) )
    {
        __log_error( "deinit non initialised timer" );
        return;
    }

    const char* name = get_timer_name( self );

    if(self->handle)
    {
        if( xTimerStop(self->handle, portMAX_DELAY) != pdPASS )
        {
            __log_error( "failed to stop timer %s", name );
        }
        else
        {
            self->enabled = false;
        }

        if( xTimerDelete(self->handle, 0) != pdPASS )
        {
            __log_error( "failed to delete timer %s", name );
        }
        self->handle = NULL;
    }

    virtual_timer_delete(obj);
}

mp_obj_t hook_mpy_machine_timer_virtual_init(
    size_t n_args,
    const mp_obj_t *args,
    mp_map_t *kw_args)
{
    __log_info("init()");
    machine_virtual_timer_obj_t *self = args[0];

    const char* name = get_timer_name( self );

    if(self->handle)
    {
        if( xTimerStop(self->handle, portMAX_DELAY) != pdPASS )
        {
            __log_error( "failed to stop timer %s", name );
        }
        else
        {
            self->enabled = false;
        }
    }

    virtual_timer_init_helper(self, n_args-1, args+1, kw_args);

    if(self->handle)
    {
        if( xTimerStart(self->handle, 0) != pdPASS )
        {
            __log_error( "failed to start timer %s", name );
        }
        else
        {
            self->enabled = true;
        }

    }

    return mp_const_none;
}

void hook_mpy_machine_timer_virtual_print(
    const void *print,
    void* obj)
{
    __log_info("print()");
    machine_virtual_timer_obj_t *self = obj;

    bool init = virtual_timer_validate(obj);

    const char* name = get_timer_name( self );
    const mp_print_t *p = print;

    mp_printf(p, "Virtual-Timer( name='%s', ", name ? name : "null");
    mp_printf(p, "init=%d, ", init);
    mp_printf(p, "period=%d ms, ", self->period);
    mp_printf(p, "repeat=%d, ", self->repeat);
    mp_printf(p, "enabled=%d )", self->enabled);
}

/** -------------------------------------------------------------------------- *
 * Virtual timers stats interface
 * --------------------------------------------------------------------------- *
 */
__mp_mod_ifdef(vtimers, CONFIG_SDK_MPY_HOOK_MACHINE_VIRTUAL_TIMERS_ENABLE)
__mp_mod_fun_0(vtimers, stats)(void)
{
    adt_list_t* it;
    bool empty = true;

    __access_lock();
    __adt_list_foreach(s_initialised_timers_head, it)
    {
        if(empty)
        {
            __log_output("== current initialized virtual timers\n");
        }
        machine_virtual_timer_obj_t* self = __list_item_to_obj(it);
        empty = false;
        const char* name = get_timer_name(self);

        __log_output("[%08x] repeat:%d enabled:%d period:%d name: %s\n",
            self,
            self->repeat,
            self->enabled,
            self->period,
            name ? name : "null"
            );
    }
    __access_unlock();
    if(empty)
    {
        __log_output("== no initialized virtual timers\n");
    }
    return mp_const_none;
}

#endif /* CONFIG_SDK_MPY_HOOK_MACHINE_VIRTUAL_TIMERS_ENABLE */

/* --- end of file ---------------------------------------------------------- */
