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
 * @brief   RGB-LED interfacing component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __RGBLED_H__
#define __RGBLED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @note    any requested RGB-LED service cancels the previous running service
 *          i.e., consider the following requests sequence:
 *              rgbled_color(0xff, 0, 0);
 *                  // -->  light the led with red color continuously
 * 
 *              rgbled_heartbeat_start(0, 0, 0xff);
 *                  // -->  start heartbeat service by coloring the led in flash
 *                  //      mode with a certain duty cycle.
 * 
 *              rgbled_color(0, 0xff, 0);
 *                  // -->  turn off the heartbeat service and continuously
 *                  //      light the led with the green color
 */

/**
 * @brief   initialize the RGB-LED driver and interface components
 */
void rgbled_init(void);

/**
 * @brief   de-initialize the RGB-LED driver and interface components
 */
void rgbled_deinit(void);

/**
 * @brief   sets the RGB-LED color value. the color value is a combination of
 *          three basic colors (red, green and blue)
 * 
 * @param   red     color red component value
 * @param   green   color green component value
 * @param   blue    color blue component value
 */
void rgbled_color(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief   sets the RGB-LED color value. the color value is a combination of
 *          three basic colors (red, green and blue)
 * 
 * @param   u32_color_value a u32 number with this hex format '0xXXRRGGBB'
 *              the least significat byte is byte[0], 
 *              the most significat byte is byte[3],
 *              byte[0] corresponds to the blue component of the color
 *              byte[1] corresponds to the green component of the color
 *              byte[2] corresponds to the blue component of the color
 *              byte[3] don't care value or negligible
 */
void rgbled_color_u32(uint32_t u32_color_value);

/**
 * @brief   turn off the led light ( no color )
 */
void rgbled_clear(void);

/**
 * @brief   set the heartbeat service configs
 * 
 * @param   color_val RGB color value
 * @param   duty_cycle_time_ms the heartbeat duty-cycle time
 * @param   light_on_percentage the percentage of the duty-cycle time in which
 *              to light the led with the requested color
 */
void rgbled_heartbeat_service_config(
    uint32_t color_val,
    uint32_t duty_cycle_time_ms,
    uint8_t  light_on_percentage);

/**
 * @brief   start the heartbeat service
 */
void rgbled_heartbeat_service_start(void);

/**
 * @brief   returns the status of the heartbeat service
 */
bool rgbled_heartbeat_service_status(void);

/**
 * @brief   stops any running rgbled service
 */
void rgbled_service_stop(void);

/**
 * @brief   a specification of a decorated light time period
 */
typedef struct {
    uint32_t    u32_color_value;    /**< color value */
    uint32_t    period_ms;          /**< applicaple time for this segment */
    uint8_t     light_on_percentage;/**< light on percentage of the period */
    uint8_t     loop_count;         /**< repeat count of this time segment */
} rgbled_light_cycle_desc_t;

/**
 * @brief   it starts a decorated light timeline service
 * 
 * @param   p_time_segments a pointer to a time decoration segments
 * @param   segments_count number of segments
 * @param   repeat to continuously repeat the whole sequence
 */
void rgbled_light_decoration_service_start(
    const rgbled_light_cycle_desc_t* p_time_segments,
    uint32_t segments_count,
    bool repeat);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __RGBLED_H__ */
