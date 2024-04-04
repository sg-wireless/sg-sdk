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
 * @brief   This file implements the unittest for the lora-stack callbacks.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#define __log_subsystem  lora
#define __log_component  mpy_lora
#include "log_lib.h"
#include "mp_lite_if.h"
#include "mpirq.h"
#include "lora.h"
#include "lora_wan_duty.h"
#include "mphalport.h"

#include "mpy_lora_cb_if.h"

void lora_unittest_lora_stack_callback(lora_event_t event, void* event_data);

/** -------------------------------------------------------------------------- *
 * unittest implementation
 * --------------------------------------------------------------------------- *
 */
static lora_wan_ind_params_t* p_test_ind_params_vector;
static int test_ind_params_vector_len = 0;
static int test_ind_params_vector_idx = 0;
void lora_unittest_stub_get_indication(lora_wan_ind_params_t* ind_params)
{
    __log_debug("stb indication");
    if(test_ind_params_vector_idx >= test_ind_params_vector_len)
    {
        lora_wan_ind_params_t ind = { .event = __LORA_EVENT_NONE };
        __log_error("exceeding max ind test vector");
        *ind_params = ind;
        return;
    }
    *ind_params = p_test_ind_params_vector[test_ind_params_vector_idx++];
}

__mp_mod_fun_kw(lora_unittest, test_callback_set, 0)
    (size_t n_args, const mp_obj_t *pos_args, mp_map_t* kw_args)
{
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_trigger, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __MPY_LORA_CB_ON_ANY}},
        { MP_QSTR_port, MP_ARG_KW_ONLY | MP_ARG_INT,
            {.u_int = __port_any}},
        { MP_QSTR_handler, MP_ARG_KW_ONLY | MP_ARG_OBJ | MP_ARG_REQUIRED,
            {.u_obj  = mp_const_none}},
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args),
        allowed_args, args);
    #define __arg_trigger_int   args[0].u_int
    #define __arg_port_int      args[1].u_int
    #define __arg_handler_obj   args[2].u_obj

    mpy_lora_callback_set(__arg_port_int, __arg_trigger_int, __arg_handler_obj);

    return mp_const_none;
    #undef __arg_trigger_int
    #undef __arg_port_int
    #undef __arg_handler_obj
}

static uint8_t test_buf[100];
static lora_wan_ind_params_t ind_vec_1 [] = {
    {
        .event = __LORA_EVENT_TX_DONE,
        .tx.msg_app_id = 1000,
        .port_num = 1
    },
    {
        .event = __LORA_EVENT_TX_DONE,
        .tx.msg_app_id = 1002,
        .port_num = 2
    },
    {
        .event = __LORA_EVENT_TX_FAIL,
        .tx.msg_app_id = 1006,
        .port_num = 1
    },
    {
        .event = __LORA_EVENT_TX_FAIL,
        .tx.msg_app_id = 1007,
        .port_num = 2
    },
    {
        .event = __LORA_EVENT_RX_DONE,
        .tx.msg_app_id = 1008,
        .port_num = 1,
        .buf = test_buf,
        .len = 11
    },
    {
        .event = __LORA_EVENT_NONE
    }
};
__mp_mod_fun_0(lora_unittest, test_gen_all_callbacks)(void)
{
    uint8_t test_data[] = "test data";
    lora_raw_rx_event_data_t rx_event_data = {
        .buf = test_data,
        .len = sizeof(test_data),
    };

    lora_unittest_lora_stack_callback(__LORA_EVENT_TX_DONE, NULL);
    lora_unittest_lora_stack_callback(__LORA_EVENT_TX_TIMEOUT, NULL);
    lora_unittest_lora_stack_callback(__LORA_EVENT_TX_FAIL, NULL);
    lora_unittest_lora_stack_callback(__LORA_EVENT_TX_CONFIRM, NULL);
    lora_unittest_lora_stack_callback(__LORA_EVENT_RX_DONE, &rx_event_data);
    lora_unittest_lora_stack_callback(__LORA_EVENT_RX_TIMEOUT, NULL);
    lora_unittest_lora_stack_callback(__LORA_EVENT_RX_FAIL, NULL);

    const char* test_str = "port 1 test buf";
    memcpy(ind_vec_1[4].buf, test_str, strlen(test_str));
    ind_vec_1[4].len = strlen(test_str);
    p_test_ind_params_vector = ind_vec_1;
    test_ind_params_vector_len = sizeof(ind_vec_1) / sizeof(ind_vec_1[0]);
    test_ind_params_vector_idx = 0;

    lora_unittest_lora_stack_callback(__LORA_EVENT_INDICATION, NULL);

    return mp_const_none;
}
