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
 * @brief   Fuel Gauge micropython module
 * --------------------------------------------------------------------------- *
 */

/* --------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdio.h>
#include <stdbool.h>

#include "mp_lite_if.h"
#define __log_subsystem     F1
#define __log_component     fuelgauge
#include "log_lib.h"

#include "fuel_gauge.h"

/* --------------------------------------------------------------------------- *
 * defines
 * --------------------------------------------------------------------------- *
 */
#define __default_design_capacity_mAh   (1200)

/* --------------------------------------------------------------------------- *
 * constants
 * --------------------------------------------------------------------------- *
 */
static const char* s_error_not_init =
    "Fuel Gauge is not initialized or battary is not present";

/* --------------------------------------------------------------------------- *
 * mod functions
 * --------------------------------------------------------------------------- *
 */

__mp_mod_fun_kw(fuel_gauge, init, 0) (
    size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_designCapacity_mAh, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __default_design_capacity_mAh}},
        { MP_QSTR_minSysVoltage_mV, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}},
        { MP_QSTR_taperCurrent_mA, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0}}
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_design_capacity_mAh_int   args[0].u_int
    #define __arg_min_sys_voltage_mV_int    args[1].u_int
    #define __arg_taper_current_int         args[2].u_int

    if(!fuel_gauge_init(__arg_design_capacity_mAh_int,
                        __arg_min_sys_voltage_mV_int,
                        __arg_taper_current_int))
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("init error"));

    return mp_const_none;
    #undef __arg_design_capacity_mAh_int
    #undef __arg_min_sys_voltage_mV_int
    #undef __arg_taper_current_int
}

__mp_mod_fun_0(fuel_gauge, deinit)(void)
{
    fuel_gauge_deinit();
    return mp_const_none;
}

__mp_mod_fun_0(fuel_gauge, info)(void)
{
    fuel_gauge_info_t info;
    if( !fuel_gauge_read_info(&info) )
    {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(s_error_not_init));
    }

    static const qstr info_dict_keys[] = {
        MP_QSTR_voltage_mV,
        MP_QSTR_current_mA,
        MP_QSTR_temp_degC,
        MP_QSTR_charge_percent,
        MP_QSTR_health_percent,
        MP_QSTR_designCapacity_mAh,
        MP_QSTR_remainingCapacity_mAh,
        MP_QSTR_fullChargeCapacity_mAh,
        MP_QSTR_isCritical,
        MP_QSTR_isLow,
        MP_QSTR_isFull,
        MP_QSTR_isCharging,
        MP_QSTR_isDischarging
    };

    static mp_obj_tuple_t info_dict_obj = {
        .base = {&mp_type_attrtuple},
        .len = sizeof(info_dict_keys)/sizeof(info_dict_keys[0]),
        .items = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            MP_ROM_PTR((void *)info_dict_keys) }
    };

    info_dict_obj.items[0] = MP_ROM_INT(info.voltage_mV);
    info_dict_obj.items[1] = MP_ROM_INT(info.current_mA);
    info_dict_obj.items[2] = mp_obj_new_float(info.temp_degC);
    info_dict_obj.items[3] = MP_ROM_INT(info.soc_percent);
    info_dict_obj.items[4] = MP_ROM_INT(info.soh_percent);
    info_dict_obj.items[5] = MP_ROM_INT(info.designCapacity_mAh);
    info_dict_obj.items[6] = MP_ROM_INT(info.remainingCapacity_mAh);
    info_dict_obj.items[7] = MP_ROM_INT(info.fullChargeCapacity_mAh);
    info_dict_obj.items[8] = mp_obj_new_bool(info.isCritical);
    info_dict_obj.items[9] = mp_obj_new_bool(info.isLow);
    info_dict_obj.items[10] = mp_obj_new_bool(info.isFull);
    info_dict_obj.items[11] = mp_obj_new_bool(info.isCharging);
    info_dict_obj.items[12] = mp_obj_new_bool(info.isDischarging);

    return MP_OBJ_FROM_PTR(&info_dict_obj);
}

__mp_mod_fun_0(fuel_gauge, print)(void)
{
    fuel_gauge_info_t info;
    if( !fuel_gauge_read_info(&info) )
    {
        __log_output(__red__"%s\n"__default__, s_error_not_init);
        return mp_const_none;
    }

    #define __text_w    22
    #define __log_info_item(__prompt, __info, __unit)   \
        __log_output_field(__prompt, __text_w, ' ', __left__, false); \
        __log_output(__yellow__"%d "__default__ __unit"\n", __info)

    #define __log_info_item_bool(__prompt, __info)   \
        __log_output_field(__prompt, __text_w, ' ', __left__, false); \
        __log_output("%s\n", g_yes_no[__info == true])
    
    __log_info_item("Voltage", info.voltage_mV, "mV");
    __log_info_item("Current", info.current_mA, "mA");
    __log_output_field("Temprature", __text_w, ' ', __left__, false);
    __log_output(__yellow__"%.1f "__default__ "degC\n", info.temp_degC);
    __log_info_item("Charge State", info.soc_percent, "%%");
    __log_info_item("Health State", info.soh_percent, "%%");
    __log_info_item("Design Capacity", info.designCapacity_mAh, "mAh");
    __log_info_item("Remaining Capacity", info.remainingCapacity_mAh, "mAh");
    __log_info_item("Full Charge Capacity", info.fullChargeCapacity_mAh, "mAh");

    __log_info_item_bool("is critical", info.isCritical);
    __log_info_item_bool("is low", info.isLow);
    __log_info_item_bool("is full", info.isFull);
    __log_info_item_bool("is charging", info.isCharging);
    __log_info_item_bool("is discharging", info.isDischarging);

    return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
