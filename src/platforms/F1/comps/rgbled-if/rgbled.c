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
 * @brief   RGB-LED interfacing component
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes and externs
 * --------------------------------------------------------------------------- *
 */
#include "driver/periph_ctrl.h"
#include "driver/rmt.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "ws2812_control.h"

#include "state_machine.h"
#include "rgbled.h"

#define __log_subsystem F1
#define __log_component rgbled
#include "log_lib.h"
__log_component_def(F1, rgbled, blue, 1, 0);

extern void periph_module_clk_disable(periph_module_t periph);
extern void periph_module_clk_enable(periph_module_t periph);

/** -------------------------------------------------------------------------- *
 * macros
 * --------------------------------------------------------------------------- *
 */
#define __esp_call(__api_call, __err_msg, ret_val)      \
    do {                                                \
        esp_err_t err = __api_call;                     \
        if(err != ESP_OK) {                             \
            __log_error("(err_code:%d)" __err_msg, err);\
        }                                               \
    } while (0)

/** -------------------------------------------------------------------------- *
 * datasheet info
 * ==============
 * 
 * - data transfer time
 *      T0H = 220ns~380ns   high-voltage    0.38 * 80 / 2 = 15.2 (~15)
 *      T1H = 580ns~1us     high-voltage    0.58 * 80 / 2 = 23.2 (~23)
 *      T0L = 580ns~1us     low-voltage     0.58 * 80 / 2 = 23.2 (~23)
 *      T1L = 580ns~1us     low-voltage     0.58 * 80 / 2 = 23.2 (~23)
 *      TRSTL = >280us      low-voltage
 * 
 * - sequence chart
 * 
 *      signal         timing
 *      -----------------------------
 *      logic-0 |-- T0H --|-- T0L --|
 *      logic-1 |-- T1H --|-- T1L --|
 *      reset   |-- TRSTL --|
 * 
 * - data transmission method (for 1 RGB LEG)
 *   * Data transmit in order of GRB, high bit data at first
 *        |-------------------- refresh-cycle --------------------|- RESET -|
 *        |----- D1 ------|----- D2 ------| ..... |----- Dn ------|- RESET -|
 *        |- 1st 24-bits -|- 2nd 24-bits -| ..... |- 2nd 24-bits -|- reset -|
 *        |                \
 *        /                 \
 *       /    composed of    \________________________________________________
 *      /                                                                     \
 *     |-------- Green --------|--------- Red ---------|-------- Blue ---------|
 *     |G7|G6|G5|G4|G3|G2|G1|G0|R7|R6|R5|R4|R3|R2|R1|R0|B7|B6|B5|B4|B3|B2|B1|B0|
 * 
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * power management
 * --------------------------------------------------------------------------- *
 */
/**
 * TODO: to check gpio power consumption later on
 */
static void rmt_power_down(void)
{
    periph_module_clk_disable(PERIPH_RMT_MODULE);
    // __esp_call(gpio_reset_pin(CONFIG_WS2812_LED_RMT_TX_GPIO),
    //     "failed to reset gpio pin",);
}

static void rmt_power_up(void)
{
    // __esp_call(rmt_set_gpio(CONFIG_WS2812_LED_RMT_TX_CHANNEL,
    //     RMT_MODE_TX, CONFIG_WS2812_LED_RMT_TX_GPIO, false),
    //     "failed to set the gpio again",);
    periph_module_clk_enable(PERIPH_RMT_MODULE);
}

/** -------------------------------------------------------------------------- *
 * led control
 * --------------------------------------------------------------------------- *
 */
static void led_ctrl_set_color(uint32_t color)
{
    // the color hex content is XX_RR_GG_BB, but the actual rgb led
    // considers this sequence GG_RR_BB
    uint8_t *p = (uint8_t*)&color;
    struct led_state new_state;
    new_state.leds[0] = (uint32_t)p[0] | ((uint32_t)p[2] << 8)
        | ((uint32_t)p[1] << 16);

    __log_info("set color"
        " R:"__red__"%02x"__default__
        ", G: "__green__"%02x"__default__
        ", B: "__blue__"%02x"__default__,
        p[2], p[1], p[0]);

    rmt_power_up();
    ws2812_write_leds(new_state);
    rmt_power_down();
}

/** -------------------------------------------------------------------------- *
 * timer management
 * --------------------------------------------------------------------------- *
 */
static void* timer_init(const char* name, void* arg, void(*p_callback)(void*))
{
    TimerHandle_t handle = xTimerCreate("lora-port", 0,
        pdFALSE, arg, (void*)p_callback);
    __log_assert(handle != NULL, "timer create failed");

    __log_debug("ctor() -> new timer '%s' , handle:%p", name, handle);
    return (void*)handle;
}
static void timer_del(void* handle)
{
    __log_debug("~dtor() -> delete timer of handle:%p", handle);
    xTimerDelete(handle, 0);
}
static void timer_start(void* handle)
{
    __log_debug("start timer of handle:%p", handle);
    BaseType_t ret;
    ret = xTimerStart(handle, 0);
    __log_assert(ret == pdPASS, "-- failed --");
}
static void timer_stop(void* handle)
{
    __log_debug("stop timer of handle:%p", handle);
    BaseType_t ret;
    ret = xTimerStop(handle, portMAX_DELAY);
    __log_assert(ret == pdPASS, "-- failed --");
}
static void timer_set_period(void* handle, uint32_t msec)
{
    __log_debug("set period (%d msec) for timer of handle:%p", msec, handle);
    BaseType_t ret;
    ret = xTimerChangePeriod(handle, msec / portTICK_PERIOD_MS, 0);
    __log_assert(ret == pdPASS, "-- failed --");
}

/* --------------------------------------------------------------------------- *
 * service access protection
 * --------------------------------------------------------------------------- *
 */
static SemaphoreHandle_t s_access_mutex = NULL;
#define __service_access_ctor()                                 \
    do {                                                        \
        if(!s_access_mutex) {                                   \
            s_access_mutex = xSemaphoreCreateMutex();           \
            __log_assert(s_access_mutex != NULL,                \
                "failed to create ioexp access guard mutex");   \
        }                                                       \
    } while(0)
#define __service_access_dtor()                                 \
    do {                                                        \
        if(s_access_mutex)                                      \
        {                                                       \
            vSemaphoreDelete(s_access_mutex);                   \
            s_access_mutex = NULL;                              \
        }                                                       \
    } while(0)

#define __service_access_lock()                                 \
    if(s_access_mutex)                                          \
        xSemaphoreTake(s_access_mutex, portMAX_DELAY)
#define __service_access_unlock()                               \
    if(s_access_mutex)                                          \
        xSemaphoreGive(s_access_mutex)

/** -------------------------------------------------------------------------- *
 * service process management
 * --------------------------------------------------------------------------- *
 */
static esp_event_base_t s_event_base = "rgbled";
static esp_event_handler_instance_t s_event_handler_instance = NULL;

static void(*s_service_callback)(void*) = NULL;
static void(*s_service_cleaner_callback)(void*) = NULL;
static void* s_service_callback_context = NULL;

static bool s_service_is_running = false;

static void service_main_handler(
    void* event_handler_arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    __log_info("start event handling");
    __service_access_lock();
    if(!s_service_is_running)
    {
        __log_error("no service is running");
        __service_access_unlock();
        return;
    }
    if(s_service_callback)
    {
        s_service_callback(s_service_callback_context);
    }
    __service_access_unlock();
}

static void service_ctor(void)
{
    if(s_event_handler_instance) {
        __log_error("ctor() -> already initialized");
        return;
    }
    __log_info("ctor() -> service");
    __esp_call(esp_event_handler_instance_register(s_event_base, 0,
        service_main_handler, NULL, &s_event_handler_instance),
        "failed to register event handler instance",);
    __service_access_ctor();
}
static void service_dtor(void)
{
    if(s_event_handler_instance == NULL) {
        __log_error("dtor() -> already de-initialized");
        return;
    }
    __log_info("dtor() -> service");

    __esp_call(esp_event_handler_instance_unregister(s_event_base, 0,
        s_event_handler_instance),
        "failed to unregister event handler instance",);
    s_event_handler_instance = NULL;
    __service_access_dtor();
}

static void service_trigger(void)
{
    __log_info("post event");
    __esp_call(esp_event_post(s_event_base, 0, NULL, 0, 0),
        "failed to post an event",);
}

static void service_start(
    void (* service_handler_callback)(void*),
    void (* service_clean_callback)(void*),
    void* service_context)
{
    __log_info("service start");
    __service_access_lock();
    if(s_service_is_running && s_service_cleaner_callback)
    {
        s_service_cleaner_callback(s_service_callback_context);
    }
    s_service_callback = service_handler_callback;
    s_service_callback_context = service_context;
    s_service_cleaner_callback = service_clean_callback;
    s_service_is_running = true;
    __service_access_unlock();

    service_trigger();
}

static void service_stop(void)
{
    __log_info("service stop");
    __service_access_lock();
    s_service_callback = NULL;
    s_service_is_running = false;
    if(s_service_cleaner_callback)
    {
        s_service_cleaner_callback(s_service_callback_context);
        s_service_cleaner_callback = NULL;
    }
    s_service_callback_context = NULL;
    __service_access_unlock();
}

/** -------------------------------------------------------------------------- *
 * basic decoration service implementation
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    uint32_t color;
    uint32_t time_on;
    uint32_t time_off;
    uint32_t repeat_count;
} decoration_time_sequence_t;

static struct {
    decoration_time_sequence_t* p_seq;
    uint32_t sequences_count;
    uint32_t current_seq_idx;
    uint32_t sub_idx;
    bool     repeat;
    enum {__decoration_idle, __decoration_light_on, __decoration_light_off}
        state;
} s_decoration_service_ctx;

static void* s_decoration_timer = NULL;

static void decoration_service_cleaner(void* ctx);

static void decoration_service_handler(void* ctx)
{
    decoration_time_sequence_t * p_seq;
    p_seq = &s_decoration_service_ctx.p_seq[
        s_decoration_service_ctx.current_seq_idx];
    if(s_decoration_service_ctx.state == __decoration_idle ||
        s_decoration_service_ctx.state == __decoration_light_off)
    {
        start_off_state:
        __log_info("decoration state %s",
            s_decoration_service_ctx.state == __decoration_idle ?
            "idle" : "off");
        if(p_seq->time_on == 0)
        {
            s_decoration_service_ctx.state = __decoration_light_on;
            goto start_on_state;
        }

        timer_set_period(s_decoration_timer, p_seq->time_on);
        timer_start(s_decoration_timer);
        led_ctrl_set_color(p_seq->color);
        s_decoration_service_ctx.state = __decoration_light_on;
    }
    else if(s_decoration_service_ctx.state == __decoration_light_on)
    {
        start_on_state:
        __log_info("decoration state on");

        if(++ s_decoration_service_ctx.sub_idx >= p_seq->repeat_count)
        {
            s_decoration_service_ctx.sub_idx = 0;
            ++ s_decoration_service_ctx.current_seq_idx;
            if(s_decoration_service_ctx.current_seq_idx >= 
                s_decoration_service_ctx.sequences_count) {

                if(s_decoration_service_ctx.repeat) {
                    s_decoration_service_ctx.current_seq_idx = 0;
                } else {
                    s_decoration_service_ctx.state = __decoration_idle;
                    led_ctrl_set_color(0);
                    return;
                }
            }
        }

        s_decoration_service_ctx.state = __decoration_light_off;
        if(p_seq->time_off == 0)
        {
            goto start_off_state;
        }
        timer_set_period(s_decoration_timer, p_seq->time_off);
        timer_start(s_decoration_timer);
        led_ctrl_set_color(0);
    }
    else
    {
        __log_error("decoration state unknown");
    }
}

static void decoration_service_cleaner(void* ctx)
{
    __log_info("clean decoration resources");
    timer_stop(s_decoration_timer);
    timer_del(s_decoration_timer);
    s_decoration_timer = NULL;
    s_decoration_service_ctx.state = __decoration_idle;
    if(s_decoration_service_ctx.p_seq)
    {
        free(s_decoration_service_ctx.p_seq);
    }
    s_decoration_service_ctx.p_seq = NULL;
    led_ctrl_set_color(0);
    /**
     * note this dlay is important to let the RGB-LED sense its reset time
     * before taking new color config, otherwise, if the color is set
     * immediately without this delay, the LED will will not sense the reset
     * time and the new color will not be set properly
     */
    vTaskDelay(10 / portTICK_PERIOD_MS);
}

static void decoration_service_timer_callback(void* args)
{
    __log_info("timer callback");
    service_trigger();
}

static void decoration_service_start(
    const rgbled_light_cycle_desc_t* p_time_segments,
    uint32_t segments_count,
    bool repeat)
{
    __log_info("decoration service start");
    service_stop();

    /**
     * note:
     * ----
     * the allocated memory here will be kept as long as the service is a live
     * and it will be freed in the cleaner function of the service.
     */
    decoration_time_sequence_t* p_seq = 
        malloc(sizeof(decoration_time_sequence_t) * segments_count);
    if(!p_seq) {
        __log_output(__red__"service can not be started,"
            " malloc issue\n"__default__);
        return;
    }

    for(int i = 0; i < segments_count; i++)
    {
        if(p_time_segments[i].light_on_percentage > 100) {
            free(p_seq);
            __log_output(__red__"service can not be started,"
                " percentage [%d] > 100"__default__,
                p_time_segments[i].light_on_percentage);
            return;
        }
        p_seq[i].color = p_time_segments[i].u32_color_value;
        uint32_t on_time = p_time_segments[i].light_on_percentage *
            p_time_segments[i].period_ms / 100;
        p_seq[i].time_on = on_time;
        p_seq[i].time_off = p_time_segments[i].period_ms - on_time;
        p_seq[i].repeat_count = p_time_segments[i].loop_count;
        __log_info("added color: %08x, period(ON:%4d OFF:%4d), count:%d\n",
            p_seq[i].color, on_time, p_seq[i].time_off, p_seq[i].repeat_count);
    }

    s_decoration_service_ctx.p_seq = p_seq;
    s_decoration_service_ctx.sequences_count = segments_count;
    s_decoration_service_ctx.repeat = repeat;
    s_decoration_service_ctx.state = __decoration_idle;
    s_decoration_service_ctx.current_seq_idx = 0;
    s_decoration_service_ctx.sub_idx = 0;

    if(s_decoration_timer == NULL)
    {
        s_decoration_timer = timer_init(
            "decoration", NULL, decoration_service_timer_callback);
    }

    service_start(decoration_service_handler, decoration_service_cleaner,
        &s_decoration_service_ctx);
}

/** -------------------------------------------------------------------------- *
 * APIs implementation
 * --------------------------------------------------------------------------- *
 */

static bool s_is_initialized = false;
static volatile bool s_heartbeat_is_running = false;
static uint32_t s_heartbeat_color =
    #ifdef CONFIG_RGBLED_HEARTBEAT_DEFAULT_COLOR
        CONFIG_RGBLED_HEARTBEAT_DEFAULT_COLOR;
    #else
        0x00000033;
    #endif

static uint32_t s_heartbeat_cycle =
    #ifdef CONFIG_RGBLED_HEARTBEAT_DEFAULT_CYCLE_TIME
    CONFIG_RGBLED_HEARTBEAT_DEFAULT_CYCLE_TIME;
    #else
    2000;
    #endif

static uint32_t s_heartbeat_percent =
    #ifdef CONFIG_RGBLED_HEARTBEAT_DEFAULT_BLINK_PERCENTAGE
    CONFIG_RGBLED_HEARTBEAT_DEFAULT_BLINK_PERCENTAGE;
    #else
    1;
    #endif

void rgbled_init(void)
{
    if(s_is_initialized)
    {
        return;
    }

    __log_info("ctor() -> rgbled");

    esp_event_loop_create_default();

    ws2812_control_init();
    service_ctor();
    s_is_initialized = true;
}

void rgbled_deinit(void)
{
    service_stop();
    service_dtor();
    rmt_driver_uninstall(CONFIG_WS2812_LED_RMT_TX_CHANNEL);
    s_is_initialized = false;
    s_heartbeat_is_running = false;
}

void rgbled_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if(!s_is_initialized)
    {
        return;
    }
    service_stop();
    s_heartbeat_is_running = false;
    led_ctrl_set_color(((uint32_t)red << 16) | ((uint32_t)green << 8)
        | ((uint32_t)blue));
}

void rgbled_color_u32(uint32_t u32_color_value)
{
    if(!s_is_initialized)
    {
        return;
    }
    service_stop();
    s_heartbeat_is_running = false;
    led_ctrl_set_color(u32_color_value);
}

void rgbled_clear(void)
{
    if(!s_is_initialized)
    {
        return;
    }
    service_stop();
    s_heartbeat_is_running = false;
    led_ctrl_set_color(0);
}

void rgbled_heartbeat_service_config(
    uint32_t color_val,
    uint32_t duty_cycle_time_ms,
    uint8_t  percentage)
{
    if(percentage >= 100 || percentage == 0) {
        __log_output("pad config percentage [%d]\n", percentage);
        return;
    }
    s_heartbeat_color = color_val;
    s_heartbeat_cycle = duty_cycle_time_ms;
    s_heartbeat_percent = percentage;

    if(s_heartbeat_is_running)
    {
        rgbled_heartbeat_service_start();
    }
}

void rgbled_heartbeat_service_start(void)
{
    if(!s_is_initialized)
    {
        return;
    }
    rgbled_light_cycle_desc_t seg = {
        .u32_color_value = s_heartbeat_color,
        .period_ms = s_heartbeat_cycle,
        .light_on_percentage = s_heartbeat_percent,
        .loop_count = 1
    };
    decoration_service_start(&seg, 1, true);
    s_heartbeat_is_running = true;
}

bool rgbled_heartbeat_service_status(void)
{
    return s_heartbeat_is_running;
}

void rgbled_service_stop(void)
{
    if(!s_is_initialized)
    {
        return;
    }
    service_stop();
    s_heartbeat_is_running = false;
}

void rgbled_light_decoration_service_start(
    const rgbled_light_cycle_desc_t* p_time_segments,
    uint32_t segments_count,
    bool repeat)
{
    if(!s_is_initialized)
    {
        return;
    }
    s_heartbeat_is_running = false;
    decoration_service_start(p_time_segments, segments_count, repeat);
}

/* --- end of file ---------------------------------------------------------- */
