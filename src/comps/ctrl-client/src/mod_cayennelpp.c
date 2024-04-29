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
 *
 */
/**
 * @file mod_cayennelpp.c
 * @brief
 */
/***********************************************Include************************/

#include "CayenneLpp.h"
#include "log_lib.h"
#include "mp_lite_if.h"
/****************** Global Definitions and Declarations ***********************/
/****************** Constant / Macro Definitions ******************************/
/****************** Type Declarations *****************************************/
typedef struct {
    mp_obj_base_t base;
    int cursor;
    uint8_t buff[CAYENNE_LPP_MAXBUFFER_SIZE];
} cayennelpp_obj_t;
/****************** Function Declarations *************************************/
extern const mp_obj_module_t __mp_mod_obj(cayennelpp);
extern const mp_obj_type_t __mp_class_obj(cayennelpp, cayennelpp);
/****************** Variable Declarations *************************************/
/****************** Function Prototype ****************************************/
__mp_mod_class_method_0(cayennelpp, cayennelpp, get_size)(mp_obj_t self_in) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(self->cursor);
}

__mp_mod_class_method_0(cayennelpp, cayennelpp, reset)(mp_obj_t self_in) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    self->cursor = 0;
    return mp_const_none;
}

__mp_mod_class_method_0(cayennelpp, cayennelpp, get_buffer)(mp_obj_t self_in) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_obj_list_t *list = mp_obj_new_list(0, NULL);
    for (size_t i = 0; i < self->cursor; i++) {
        //printf("buf[%d]=%d\n", i, self->buff[i]);
        mp_obj_list_append(list, MP_OBJ_NEW_SMALL_INT(self->buff[i]));
    }

    return MP_OBJ_FROM_PTR(list);
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_DigInput)(mp_obj_t self_in, mp_obj_t __arg_1,
                                      mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_int)) {
        int channel = mp_obj_get_int(__arg_1);
        int value = mp_obj_get_int(__arg_2);
        self->cursor = CayenneLpp_Add_DigitalInput(self->buff, self->cursor,
                                                   channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,int) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_DigOutput)(mp_obj_t self_in, mp_obj_t __arg_1,
                                       mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_int)) {
        int channel = mp_obj_get_int(__arg_1);
        int value = mp_obj_get_int(__arg_2);
        self->cursor = CayenneLpp_Add_DigitalOutput(self->buff, self->cursor,
                                                    channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,int) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_AnalogInput)(mp_obj_t self_in, mp_obj_t __arg_1,
                                         mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_int)) {
        int channel = mp_obj_get_int(__arg_1);
        int value = mp_obj_get_int(__arg_2);
        self->cursor = CayenneLpp_Add_AnalogInput(self->buff, self->cursor,
                                                  channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,int) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_AnalogOutput)(mp_obj_t self_in, mp_obj_t __arg_1,
                                          mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_int)) {
        int channel = mp_obj_get_int(__arg_1);
        int value = mp_obj_get_int(__arg_2);
        self->cursor = CayenneLpp_Add_AnalogOutput(self->buff, self->cursor,
                                                   channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,int) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_Luminosity)(mp_obj_t self_in, mp_obj_t __arg_1,
                                        mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_int)) {
        int channel = mp_obj_get_int(__arg_1);
        int value = mp_obj_get_int(__arg_2);
        self->cursor =
            CayenneLpp_Add_Luminosity(self->buff, self->cursor, channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,int) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_Presence)(mp_obj_t self_in, mp_obj_t __arg_1,
                                      mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_int)) {
        int channel = mp_obj_get_int(__arg_1);
        int value = mp_obj_get_int(__arg_2);
        self->cursor =
            CayenneLpp_Add_Presence(self->buff, self->cursor, channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,int) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_Temperature)(mp_obj_t self_in, mp_obj_t __arg_1,
                                         mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_float)) {
        int channel = mp_obj_get_int(__arg_1);
        float value = mp_obj_get_float(__arg_2);
        //printf("channel:%d,value:%f,cursor:%d\n", channel, value, self->cursor);
        self->cursor = CayenneLpp_Add_Temperature(self->buff, self->cursor,
                                                  channel, value);
        // for (size_t i = 0; i < self->cursor; i++) {
        //     printf("buf[%d]=%d\n", i, self->buff[i]);
        // }
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,float) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_Humidity)(mp_obj_t self_in, mp_obj_t __arg_1,
                                      mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_float)) {
        int channel = mp_obj_get_int(__arg_1);
        float value = mp_obj_get_float(__arg_2);
        self->cursor = CayenneLpp_Add_RelativeHumidity(self->buff, self->cursor,
                                                       channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,float) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_2(cayennelpp, cayennelpp,
                        add_BarometricPressure)(mp_obj_t self_in,
                                                mp_obj_t __arg_1,
                                                mp_obj_t __arg_2) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if ((mp_obj_get_type(__arg_1) == &mp_type_int) &&
        (mp_obj_get_type(__arg_2) == &mp_type_float)) {
        int channel = mp_obj_get_int(__arg_1);
        float value = mp_obj_get_float(__arg_2);
        self->cursor = CayenneLpp_Add_BarometricPressure(
            self->buff, self->cursor, channel, value);
    } else {
        mp_raise_TypeError(MP_ERROR_TEXT("expected (int,float) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_var(cayennelpp, cayennelpp, add_Accelerometer,
                          4)(size_t n_args, const mp_obj_t *args) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if ((mp_obj_get_type(args[1]) == &mp_type_int) &&
        (mp_obj_get_type(args[2]) == &mp_type_float) &&
        (mp_obj_get_type(args[3]) == &mp_type_float) &&
        (mp_obj_get_type(args[4]) == &mp_type_float)) {
        int channel = mp_obj_get_int(args[1]);
        float x = mp_obj_get_float(args[2]);
        float y = mp_obj_get_float(args[3]);
        float z = mp_obj_get_float(args[4]);
        self->cursor = CayenneLpp_Add_Accelerometer(self->buff, self->cursor,
                                                    channel, x, y, z);
    } else {
        mp_raise_TypeError(
            MP_ERROR_TEXT("expected (int,float,float,float) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_var(cayennelpp, cayennelpp, add_Gyrometer,
                          4)(size_t n_args, const mp_obj_t *args) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if ((mp_obj_get_type(args[1]) == &mp_type_int) &&
        (mp_obj_get_type(args[2]) == &mp_type_float) &&
        (mp_obj_get_type(args[3]) == &mp_type_float) &&
        (mp_obj_get_type(args[4]) == &mp_type_float)) {
        int channel = mp_obj_get_int(args[1]);
        float x = mp_obj_get_float(args[2]);
        float y = mp_obj_get_float(args[3]);
        float z = mp_obj_get_float(args[4]);
        self->cursor = CayenneLpp_Add_Gyrometer(self->buff, self->cursor,
                                                channel, x, y, z);
    } else {
        mp_raise_TypeError(
            MP_ERROR_TEXT("expected (int,float,float,float) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_method_var(cayennelpp, cayennelpp, add_GPS,
                          4)(size_t n_args, const mp_obj_t *args) {
    cayennelpp_obj_t *self = MP_OBJ_TO_PTR(args[0]);
    if ((mp_obj_get_type(args[1]) == &mp_type_int) &&
        (mp_obj_get_type(args[2]) == &mp_type_float) &&
        (mp_obj_get_type(args[3]) == &mp_type_float) &&
        (mp_obj_get_type(args[4]) == &mp_type_float)) {
        int channel = mp_obj_get_int(args[1]);
        float x = mp_obj_get_float(args[2]);
        float y = mp_obj_get_float(args[3]);
        float z = mp_obj_get_float(args[4]);
        self->cursor =
            CayenneLpp_Add_Gps(self->buff, self->cursor, channel, x, y, z);
    } else {
        mp_raise_TypeError(
            MP_ERROR_TEXT("expected (int,float,float,float) argument"));
    }
    return mp_const_none;
}

__mp_mod_class_new(cayennelpp, cayennelpp)(const mp_obj_type_t *type,
                                           size_t n_args, size_t n_kw,
                                           const mp_obj_t *args) {
    (void)n_kw;

    cayennelpp_obj_t *self = m_new_obj(cayennelpp_obj_t);
    if (self == NULL) {
        __log_debug("not enough memory");
        return mp_const_none;
    }
    self->base.type = type;
    self->cursor = 0;

    return MP_OBJ_FROM_PTR(self);
}