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
 * @brief   synchoronization objects resources management APIs.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdbool.h>

#define __log_subsystem     lora
#define __log_component     util_sync_obj
#include "log_lib.h"

#include "stub_system.h"
#include "lora_sync_obj.h"

/** -------------------------------------------------------------------------- *
 * internal data structure
 * --------------------------------------------------------------------------- *
 */
#define __max_num_of_objects        (10)

static struct {
    const char * name;
    void*       obj;
    bool        in_use;
    bool        has_waiting;
    bool        initialized;
} s_resources_wheel[ __max_num_of_objects ];

static void* s_access_mutex = NULL;

/** -------------------------------------------------------------------------- *
 * helper macros
 * --------------------------------------------------------------------------- *
 */
#define __sync_obj_wheel_access_lock() lora_stub_mutex_lock( s_access_mutex )
#define __sync_obj_wheel_access_unlock() lora_stub_mutex_unlock( s_access_mutex)
#define __pre_check()   __log_assert(s_access_mutex != NULL, \
                            "'lora_sync_obj_wheel' not initialized")

#define __log_sync_obj(_opr, _id, _name) __log_info(" opr: "            \
    __green__"%-+10s"__default__"  id: "__green__"%2d"__default__       \
    "  name: "__blue__"%s", #_opr, _id, _name ? _name : "-- na --")

/** -------------------------------------------------------------------------- *
 * APIs implementation
 * --------------------------------------------------------------------------- *
 */

void  sync_obj_init(void)
{
    static bool initialized = false;

    if( ! initialized )
    {
        __log_info("init sync objects wheel");
        s_access_mutex = lora_stub_mutex_new();
        memset(s_resources_wheel, 0, sizeof(s_resources_wheel));
    }
}

sync_obj_t sync_obj_acquire(const char* name)
{
    sync_obj_init();

    __pre_check();

    int i;

    __sync_obj_wheel_access_lock();

    for( i = 0; i < __max_num_of_objects; ++i )
    {
        if(s_resources_wheel[i].in_use == false)
        {
            if(s_resources_wheel[i].initialized == false)
            {
                __log_info("-- init new wheel resource object");
                s_resources_wheel[i].obj = lora_stub_sem_new();
                s_resources_wheel[i].initialized = true;
                __log_assert(s_resources_wheel[i].obj,
                    "wheel resource alloc failed");
            }
            s_resources_wheel[i].in_use = true;
            s_resources_wheel[i].has_waiting = false;
            s_resources_wheel[i].name = name;
            __log_sync_obj(acquire, i, name);
            __sync_obj_wheel_access_unlock();
            return i;
        }
    }

    __sync_obj_wheel_access_unlock();

    __log_assert(0, "no available resources in the objects wheel");

    return 0;
}

void  sync_obj_release(sync_obj_t obj)
{
    __pre_check();

    int i = obj;

    __sync_obj_wheel_access_lock();

    __log_assert( i < __max_num_of_objects && s_resources_wheel[i].in_use &&
        s_resources_wheel[i].initialized, "non valid sync wheel object" );

    s_resources_wheel[i].in_use = false;

    if(s_resources_wheel[i].has_waiting)
    {
        __log_sync_obj(signal, i, s_resources_wheel[i].name);
        lora_stub_sem_signal(s_resources_wheel[i].obj);
        s_resources_wheel[i].has_waiting = false;
    }

    __log_sync_obj(release, i, s_resources_wheel[i].name);

    __sync_obj_wheel_access_unlock();
}

void  sync_obj_wait(sync_obj_t obj)
{
    __pre_check();

    int i = obj;

    __sync_obj_wheel_access_lock();

    __log_assert( i < __max_num_of_objects && s_resources_wheel[i].in_use &&
        s_resources_wheel[i].initialized, "non valid sync wheel object" );

    __log_sync_obj(wait, i, s_resources_wheel[i].name);

    s_resources_wheel[i].has_waiting = true;

    __sync_obj_wheel_access_unlock();

    lora_stub_sem_wait(s_resources_wheel[i].obj);
}

void  sync_obj_signal(sync_obj_t obj)
{
    __pre_check();

    int i = obj;

    __sync_obj_wheel_access_lock();

    __log_assert( i < __max_num_of_objects && s_resources_wheel[i].in_use &&
        s_resources_wheel[i].initialized, "non valid sync wheel object" );

    lora_stub_sem_signal(s_resources_wheel[i].obj);

    __log_sync_obj(signal, i, s_resources_wheel[i].name);

    s_resources_wheel[i].has_waiting = false;
    
    __sync_obj_wheel_access_unlock();

}

/* --- end of file ---------------------------------------------------------- */
