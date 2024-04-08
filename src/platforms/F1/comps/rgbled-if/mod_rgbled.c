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
 * @brief   micropython module for RGB-LED functionality
 * --------------------------------------------------------------------------- *
 */

#include "mp_lite_if.h"

#define __log_subsystem F1
#define __log_component rgbled
#include "log_lib.h"

#include "rgbled.h"

/** -------------------------------------------------------------------------- *
 * module functions implementations
 * --------------------------------------------------------------------------- *
 */

__mp_mod_class_const(rgbled, _color, RED,       0x00FF0000);
__mp_mod_class_const(rgbled, _color, GREEN,     0x0000FF00);
__mp_mod_class_const(rgbled, _color, BLUE,      0x000000FF);
__mp_mod_class_const(rgbled, _color, YELLOW,    0x00FFFF00);
__mp_mod_class_const(rgbled, _color, MAGENTA,   0x00FF00FF);
__mp_mod_class_const(rgbled, _color, CYAN,      0x0000FFFF);
__mp_mod_class_const(rgbled, _color, WHITE,     0x00FFFFFF);

__mp_mod_init(rgbled)(void)
{
    rgbled_init();
    return mp_const_none;
}

__mp_mod_fun_0(rgbled, initialize)(void)
{
    rgbled_init();
    return mp_const_none;
}

__mp_mod_fun_0(rgbled, deinit)(void)
{
    rgbled_deinit();
    return mp_const_none;
}

__mp_mod_fun_1(rgbled, color)(mp_obj_t color_obj)
{
    if( ! MP_OBJ_IS_INT(color_obj) )
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("passing non integer argument"));
        return mp_const_none;
    }

    rgbled_color_u32(mp_obj_get_int(color_obj));

    return mp_const_none;
}

__mp_mod_fun_var_between(rgbled, heartbeat, 0, 3)(
    size_t __arg_n, const mp_obj_t * __arg_v)
{
    if(__arg_n == 0) {
        return rgbled_heartbeat_service_status() ?
            mp_const_true : mp_const_false;
    }

    if(__arg_n == 1) {
        if( !mp_obj_is_bool(__arg_v[0]) ) {
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("passing one argument should be bool"
                "to enable/disable the heartbeat service"));
        }
        bool is_running = rgbled_heartbeat_service_status();
        if(__arg_v[0] == mp_const_true) {
            if(! is_running) {
                rgbled_heartbeat_service_start();
            }
        } else {
            if( is_running ) {
                rgbled_service_stop();
            }
        }
        return mp_const_none;
    }

    if( __arg_n != 3 || (
        !( MP_OBJ_IS_INT(__arg_v[0])
        && MP_OBJ_IS_INT(__arg_v[1])
        && MP_OBJ_IS_INT(__arg_v[2]))
        ))
    {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("passing more than one argument must be three"
            "integer arguments to config the service and start it"
            "<color> <cycle-time> <blink-percentage>"));
        return mp_const_none;
    }

    rgbled_heartbeat_service_config(
        mp_obj_get_int(__arg_v[0]),
        mp_obj_get_int(__arg_v[1]),
        mp_obj_get_int(__arg_v[2])
        );

    if( ! rgbled_heartbeat_service_status() ) {
        rgbled_heartbeat_service_start();
    }

    return mp_const_none;
}

__mp_mod_fun_0(rgbled, service_stop)(void)
{
    rgbled_service_stop();

    return mp_const_none;
}

__mp_mod_fun_2(rgbled, decoration)(
    mp_obj_t decoration_sequence, mp_obj_t loop)
{
    if(!mp_obj_is_bool(loop)) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("second argument must be bool"));
    }
    mp_obj_t *items;
    size_t len;
    mp_obj_get_array(decoration_sequence, & len, &items);

    rgbled_light_cycle_desc_t* p_decoration =
        malloc(sizeof(rgbled_light_cycle_desc_t) * len);
    if(!p_decoration)
    {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("not available memory"));
    }

    for(int i = 0; i < len; i++)
    {
        mp_obj_t *seg_items;
        size_t seg_items_len;

        if(mp_obj_get_type(items[i]) != &mp_type_tuple) {
            free(p_decoration);
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("decoration array must of tuples of four"));
        }

        mp_obj_tuple_get(items[i], &seg_items_len, &seg_items);
        if(seg_items_len != 4) {
            free(p_decoration);
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("decoration array must of tuples of four"));
        }

        if(! (
            MP_OBJ_IS_INT(seg_items[0]) &&
            MP_OBJ_IS_INT(seg_items[1]) &&
            MP_OBJ_IS_INT(seg_items[2]) &&
            MP_OBJ_IS_INT(seg_items[3])
        )) {
            free(p_decoration);
            mp_raise_msg(&mp_type_OSError,
                MP_ERROR_TEXT("decoration array tuples must be of 4 integers"));
        }

        p_decoration[i].u32_color_value = mp_obj_get_int(seg_items[0]);
        p_decoration[i].period_ms = mp_obj_get_int(seg_items[1]);
        p_decoration[i].light_on_percentage = mp_obj_get_int(seg_items[2]);
        p_decoration[i].loop_count = mp_obj_get_int(seg_items[3]);
    }

    rgbled_light_decoration_service_start(p_decoration,
        len, loop == mp_const_true);

    free(p_decoration);

    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
