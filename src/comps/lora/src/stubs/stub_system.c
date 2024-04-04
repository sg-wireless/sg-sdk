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
 * @brief   This file defines the stub for Semtech system functions.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <string.h>
#include <stdint.h>
#include <sys/time.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "esp_random.h"
#include "esp_crc.h"

#include "utilities.h"
#include "lora_port.h"
#include "stub_system.h"
#include "system/systime.h"

#define __log_subsystem  lora
#define __log_component  stub_system
#include "log_lib.h"

static lora_port_get_timestamp_ms_t * p_get_timestamp_msec;
static lora_port_delay_ms_t         * p_delay_msec;
static lora_port_mutex_new_t        * p_mutex_new;
static lora_port_mutex_lock_t       * p_mutex_lock;
static lora_port_mutex_unlock_t     * p_mutex_unlock;
static lora_port_sem_new_t          * p_sem_new;
static lora_port_sem_wait_t         * p_sem_wait;
static lora_port_sem_signal_t       * p_sem_signal;
static lora_port_crc32_calc_t       * p_crc32_calc;

void lora_stub_system_init(void* p_init_params)
{
    __log_info("ctor() -> intialize lora system stub");
    lora_port_params_t * ptr = p_init_params;

    p_get_timestamp_msec    = ptr->get_timestamp_msec;
    p_delay_msec            = ptr->delay_msec;
    p_mutex_new             = ptr->mutex_new;
    p_mutex_lock            = ptr->mutex_lock;
    p_mutex_unlock          = ptr->mutex_unlock;
    p_sem_new               = ptr->sem_new;
    p_sem_wait              = ptr->sem_wait;
    p_sem_signal            = ptr->sem_signal;
    p_crc32_calc            = ptr->crc32_calc;
}

TimerTime_t TimerGetCurrentTime( void )
{
    if(p_get_timestamp_msec)
        return p_get_timestamp_msec();
    else
        return 0;
}

TimerTime_t TimerGetElapsedTime( TimerTime_t past )
{
    return TimerGetCurrentTime() - past;
}

SysTime_t SysTimeAdd( SysTime_t a, SysTime_t b )
{
    SysTime_t c =  { .Seconds = 0, .SubSeconds = 0 };

    c.Seconds = a.Seconds + b.Seconds;
    c.SubSeconds = a.SubSeconds + b.SubSeconds;
    if( c.SubSeconds >= 1000 )
    {
        c.Seconds++;
        c.SubSeconds -= 1000;
    }
    return c;
}

SysTime_t SysTimeSub( SysTime_t a, SysTime_t b )
{
    SysTime_t c = { .Seconds = 0, .SubSeconds = 0 };

    c.Seconds = a.Seconds - b.Seconds;
    c.SubSeconds = a.SubSeconds - b.SubSeconds;
    if( c.SubSeconds < 0 )
    {
        c.Seconds--;
        c.SubSeconds += 1000;
    }
    return c;
}

void SysTimeSet( SysTime_t sysTime )
{
    /* Lora stack corrects the time in seconds only
     * so, we will correct the time in seconds here as well
     */
    /** TODO: to be moved to the lora-port */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec = sysTime.Seconds;
    __log_debug("-- setting new time: %ld", tv.tv_sec);

    struct tm *lt;
    time_t raw_time = tv.tv_sec;
    lt = localtime(&raw_time);
    __log_debug("SysTimeSet() --> %s", asctime(lt));

    settimeofday(&tv, NULL);
}

SysTime_t SysTimeGet( void )
{
    // uint32_t timestamp = p_get_timestamp_msec();
    // SysTime_t time;
    // time.Seconds = timestamp / 1000;
    // time.SubSeconds = timestamp % 1000;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    SysTime_t time = {.Seconds = tv.tv_sec, .SubSeconds = tv.tv_usec / 1000U};

    struct tm *lt;
    time_t raw_time = time.Seconds;
    lt = localtime(&raw_time);
    __log_debug("SysTimeGet() --> %s", asctime(lt));

    return time;
}

SysTime_t SysTimeGetMcuTime( void )
{
    return SysTimeGet();
}

void srand1( uint32_t seed )
{
    //__log_debug("random seed: %d", seed);
}

int32_t randr( int32_t min, int32_t max )
{
    uint32_t rn = esp_random();
    uint32_t period = max - min + 1;
    rn %= period;

    int32_t num = min + rn;

    //__log_debug("random number: %d", num);

    return num;
}

void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    memcpy(dst, src, size);
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
    memset(dst, value, size);
}

int8_t Nibble2HexChar( uint8_t a );

uint32_t lora_stub_crc32(uint32_t initial, void* buf, uint32_t len)
{
    if(p_crc32_calc)
        return p_crc32_calc(initial, buf, len);
    else
        return 0;
}

uint32_t Crc32( uint8_t *buffer, uint16_t length )
{
    return lora_stub_crc32(0, buffer, length);
}

static void* s_board_cs_mutex = NULL;
void BoardCriticalSectionInit(void)
{
    static bool initialized = false;
    if( ! initialized && p_mutex_new )
    {
        __log_info("init Board CS mutex");
        s_board_cs_mutex = p_mutex_new();
        initialized = true;
    }
}

void BoardCriticalSectionBegin( uint32_t *mask )
{
    __log_assert(s_board_cs_mutex != NULL, "board CS mutex not initialized");
    if(p_mutex_lock)
        p_mutex_lock(s_board_cs_mutex);
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
    __log_assert(s_board_cs_mutex != NULL, "board CS mutex not initialized");
    if(p_mutex_unlock)
        p_mutex_unlock(s_board_cs_mutex);
}

void DelayMs( uint32_t ms )
{
    if(p_delay_msec)
        p_delay_msec(ms);
}

void lora_stub_delay_msec( uint32_t ms )
{
    DelayMs( ms );
}

void* lora_stub_mutex_new(void)
{
    if(p_mutex_new)
        return p_mutex_new();

    return NULL;
}
void lora_stub_mutex_lock(void* handle)
{
    if(p_mutex_lock)
        p_mutex_lock(handle);
}
void lora_stub_mutex_unlock(void* handle)
{
    if(p_mutex_unlock)
        p_mutex_unlock(handle);
}

void* lora_stub_sem_new(void)
{
    if(p_sem_new)
        return p_sem_new();
    return NULL;
}

void lora_stub_sem_wait(void* handle)
{
    if(p_sem_wait)
        p_sem_wait(handle);
}

void lora_stub_sem_signal(void* handle)
{
    if(p_sem_signal)
        p_sem_signal(handle);
}

uint32_t lora_stub_get_timestamp_ms(void)
{
    return TimerGetCurrentTime();
}

/* --- end of file ---------------------------------------------------------- */
