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
 * @brief   lora-wan duty-cycle processing sub-component
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define __log_subsystem     lora
#include "log_lib.h"

#include "lora_wan_process.h"
#include "stub_timers.h"
#include "lora_proto_compliance.h"

/* --- macros --------------------------------------------------------------- */

#define __config_lora_wan_duty_cycle_default    \
    CONFIG_LORA_DUTY_CYCLE_APP_DEFAULT_DURATION_MS

#define __config_lora_wan_duty_cycle_test_default \
    CONFIG_LORA_DUTY_CYCLE_APP_DEFAULT_DURATION_MS

/** -------------------------------------------------------------------------- *
 * duty-cycle control
 * --------------------------------------------------------------------------- *
 */

static void* s_duty_cycle_timer;

static bool s_is_running = false;

static uint32_t  s_app_duty_cycle_period;
static uint32_t  s_tst_duty_cycle_period;
static uint32_t* s_p_duty_cycle_period;
static bool s_is_tst_enabled;

static const char* s_timer_name = "duty-cycle timer";

static void duty_cycle_timer_callback(void* arg);

static void duty_cycle_timer_ctor(void)
{
    __log_timer_ctor(s_timer_name);
    s_duty_cycle_timer = lora_stub_timer_init("duty-cycle-timer",
        duty_cycle_timer_callback, NULL);
}
static void duty_cycle_timer_dtor(void)
{
    __log_timer_dtor(s_timer_name);
    lora_stub_timer_delete(s_duty_cycle_timer);
}
static void duty_cycle_timer_start(void)
{
    __log_timer_start(s_timer_name, *s_p_duty_cycle_period);
    lora_stub_timer_start(s_duty_cycle_timer, *s_p_duty_cycle_period);
}
static void duty_cycle_timer_stop(void)
{
    __log_timer_stop(s_timer_name);
    lora_stub_timer_stop(s_duty_cycle_timer);
}

static void duty_cycle_timer_callback(void* arg)
{
    __log_timer_expire(s_timer_name);
    duty_cycle_timer_start();
    lora_wan_process_request(__LORA_WAN_PROCESS_TRX_DUTY_CYCLE, NULL);
}

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

void lora_wan_duty_compliance_switch(bool is_on)
{
    __log_info("duty-cycle compliance switch %s", g_on_off[is_on]);
    s_is_tst_enabled = is_on;

    s_p_duty_cycle_period = is_on 
        ? &s_tst_duty_cycle_period
        : &s_app_duty_cycle_period;
}

void lora_wan_duty_ctor(void)
{
    s_tst_duty_cycle_period = __config_lora_wan_duty_cycle_test_default;
    s_app_duty_cycle_period = __config_lora_wan_duty_cycle_default;

    lora_wan_duty_compliance_switch( lora_proto_compliance_get_state() );

    s_is_running = false;

    duty_cycle_timer_ctor();
}

void lora_wan_duty_dtor(void)
{
    s_p_duty_cycle_period = NULL;
    s_is_running = false;
    duty_cycle_timer_stop();
    duty_cycle_timer_dtor();
}

void lora_wan_duty_set(uint32_t period)
{
    s_app_duty_cycle_period = period;

    if( s_is_running && !s_is_tst_enabled )
    {
        duty_cycle_timer_stop();
        duty_cycle_timer_start();
    }
}

uint32_t lora_wan_duty_get(void)
{
    return s_app_duty_cycle_period;
}

void lora_wan_duty_start(void)
{
    s_is_running = true;

    if( !s_is_tst_enabled )
    {
        duty_cycle_timer_start();
    }
}

void lora_wan_duty_stop(void)
{
    s_is_running = false;

    if( !s_is_tst_enabled )
    {
        duty_cycle_timer_stop();
    }
}

void lora_wan_duty_suspend(void)
{
    if(s_is_running)
    {
        duty_cycle_timer_stop();
    }
}

void lora_wan_duty_resume(void)
{
    if( s_is_running && !s_is_tst_enabled )
    {
        duty_cycle_timer_start();
    }
}

void lora_wan_duty_compliance_tx_periodicity(uint32_t periodicity)
{
    if(periodicity)
    {
        s_tst_duty_cycle_period = periodicity;
        s_p_duty_cycle_period = &s_tst_duty_cycle_period;
    }
    else
    {
        s_p_duty_cycle_period = &s_app_duty_cycle_period;
    }

    duty_cycle_timer_stop();
    duty_cycle_timer_start();
}

/* --- end of file ---------------------------------------------------------- */
