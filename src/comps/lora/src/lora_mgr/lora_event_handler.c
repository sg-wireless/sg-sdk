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
 * @brief   This file implements the interface between the lora-stack and the
 *          ESP32 Event handler component.
 * 
 * TODO: to be abstracted to be platform independent. 
 * --------------------------------------------------------------------------- *
 */
#include "esp_event.h"

#define __log_subsystem  lora
#define __log_component  util_evt_hndle
#include "log_lib.h"

#include "lora_event_handler.h"

/** -------------------------------------------------------------------------- *
 * interface functions implementation
 * --------------------------------------------------------------------------- *
 */
#define __esp_call_assert(__api_call, __err_msg)    \
    do {                                            \
        esp_err_t err = __api_call;                 \
        __log_assert(err == ESP_OK,                 \
            "(err_code:%d)" __err_msg, err);        \
    } while (0)

static esp_event_base_t s_lora_event_base = "lora_evt";
static esp_event_loop_handle_t s_lora_event_loop_handler;
static bool s_is_initialized = false;
static void lora_event_handler_exec(
    void* event_handler_arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data)
{
    __log_info(" { start lora event");
    ((lora_evt_handler_t*)event_handler_arg)(event_data);
    __log_info(" } end lora event");
}

void lora_event_handler_init(void)
{
    if(s_is_initialized)
        return;
    s_is_initialized = true;
    esp_event_loop_args_t event_task_args = {
        .queue_size = 5,
        .task_name = "lora_evt",
        .task_priority = uxTaskPriorityGet(NULL),
        .task_stack_size = 1024 * 4,
        .task_core_id = tskNO_AFFINITY
    };
    __esp_call_assert( esp_event_loop_create(&event_task_args,
            &s_lora_event_loop_handler),
        "lora event loop handler init fail");
}

void lora_event_handler_issue(uint32_t evt_id, void* evt_data, uint32_t size)
{
    __esp_call_assert( esp_event_post_to(s_lora_event_loop_handler,
        s_lora_event_base, evt_id, evt_data, size, 0),
        "esp_event_post_to");
}

#define __max_events    5
struct {
    uint32_t evt_id;
    esp_event_handler_instance_t instance;
} registered_events[__max_events];

static void keep_event_instance(uint32_t evt_id,
    esp_event_handler_instance_t instance)
{
    int i;
    int first = 0;
    bool save_first = false;
    for(i = 0; i < __max_events; ++i)
    {
        if(registered_events[i].evt_id == 0)
        {
            if(save_first == false) {
                save_first = true;
                first  = i;
            }
        }
        else if(registered_events[i].evt_id == evt_id)
        {
            registered_events[i].instance = instance;
            return;
        }
    }
    registered_events[first].evt_id = evt_id;
    registered_events[first].instance = instance;
}
static esp_event_handler_instance_t get_event_instance(uint32_t evt_id)
{
    int i;
    for(i = 0; i < __max_events; ++i)
    {
        if(registered_events[i].evt_id == evt_id)
        {
            return registered_events[i].instance;
        }
    }
    return NULL;
}

void lora_event_handler_register(uint32_t evt_id,
    lora_evt_handler_t* p_evt_handler)
{
    esp_event_handler_instance_t instance;
    __esp_call_assert( esp_event_handler_instance_register_with(
            s_lora_event_loop_handler,
            s_lora_event_base, evt_id,
            lora_event_handler_exec, p_evt_handler, &instance),
        "lora event handler register");
    keep_event_instance(evt_id, instance);
}

void lora_event_handler_deregister(uint32_t evt_id)
{
    esp_event_handler_instance_unregister_with(
        s_lora_event_loop_handler,
        s_lora_event_base, evt_id, get_event_instance(evt_id));
}

/* --- end of file ---------------------------------------------------------- */
