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
 * @brief   This file populate a uPython module to demonstrate and test the
 *          state-machine lib component
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * include
 * --------------------------------------------------------------------------- *
 */
#include "mp_lite_if.h"

#define __log_subsystem libs
#include "log_lib.h"
#include "state_machine.h"

#ifdef CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE
#include "demo_sm_state_machine.h"

/** -------------------------------------------------------------------------- *
 * state machine test
 * --------------------------------------------------------------------------- *
 */
// set the generated state-machine code preprocessor flag
__sm_ifdef(demo_sm, CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE)

// set the transition tables
// ---- state A transition table
__sm_trans(demo_sm, A, x, action_a_b, B)
__sm_trans(demo_sm, A, y, action_a_c, C)
__sm_trans(demo_sm, A, z, action_a_d, D)
__sm_trans(demo_sm, A, w, do_nothing, A)

// ---- state B transition table
__sm_trans(demo_sm, B, x, action_b_a, A)
__sm_trans(demo_sm, B, y, action_b_c, C)
__sm_trans(demo_sm, B, z, action_b_d, D)

// ---- state C transition table
__sm_trans(demo_sm, C, x, do_nothing, C)
__sm_trans(demo_sm, C, y, action_c_a, A)
__sm_trans(demo_sm, C, z, action_c_d, D)

// ---- state D transition table
__sm_trans(demo_sm, D, x, do_nothing, D)
__sm_trans(demo_sm, D, y, do_nothing, D)
__sm_trans(demo_sm, D, z, action_d_a, A)

// -- define actions
// -- undefined action, will be just a state transition withou action execution
#define __log_movement(__from, __to)                    \
    __log_info("action moving from "                    \
    __yellow__ __stringify(__from) __default__" -> "    \
    __green__ __stringify(__to) __default__)
#define __log_entering(__state)                         \
    __log_info(__green__"entering state "               \
    __yellow__ __stringify(__state) __default__)
#define __log_leaving(__state)                          \
    __log_info(__red__  "leaving  state "               \
    __yellow__ __stringify(__state) __default__)
#define __log_default(__state)                          \
    __log_info(__cyan__  "default  state "              \
    __yellow__ __stringify(__state) __cyan__            \
    " action" __default__)

__sm_action(demo_sm, action_a_b)(void* data){__log_movement(A, B);}
__sm_action(demo_sm, action_a_c)(void* data){__log_movement(A, C);}
__sm_action(demo_sm, action_a_d)(void* data){__log_movement(A, D);}
__sm_action(demo_sm, action_b_a)(void* data){__log_movement(B, A);}
__sm_action(demo_sm, action_b_c)(void* data){__log_movement(B, C);}
__sm_action(demo_sm, action_b_d)(void* data){__log_movement(B, D);}
__sm_action(demo_sm, action_c_a)(void* data){
    __log_movement(C, A);
    __sm_ch_state(demo_sm, B);
}
__sm_action(demo_sm, action_c_d)(void* data){__log_movement(C, D);}
__sm_action(demo_sm, action_d_a)(void* data){__log_movement(D, A);}

// -- defining optional state enter/leave actions
__sm_state_enter(demo_sm, A)(void* data){__log_entering(A);}
__sm_state_leave(demo_sm, A)(void* data){__log_leaving(A);}

__sm_state_enter(demo_sm, B)(void* data){__log_entering(B);}
__sm_state_leave(demo_sm, B)(void* data){__log_leaving(B);}

__sm_state_enter(demo_sm, C)(void* data){__log_entering(C);}
__sm_state_leave(demo_sm, C)(void* data){__log_leaving(C);}

__sm_state_enter(demo_sm, D)(void* data){__log_entering(D);}
__sm_state_leave(demo_sm, D)(void* data){__log_leaving(D);}

//-- defining optional default state actions
__sm_state_default_action(demo_sm, A)(void* data){__log_default(A);}
__sm_state_default_action(demo_sm, B)(void* data){__log_default(B);}
__sm_state_default_action(demo_sm, C)(void* data){__log_default(C);}
__sm_state_default_action(demo_sm, D)(void* data){__log_default(D);}

/** -------------------------------------------------------------------------- *
 * module functions
 * --------------------------------------------------------------------------- *
 */
static log_filter_save_state_t libs_sm = {
    .subsystem_name = "libs",
    .component_name = "state_machine"
};
static log_filter_save_state_t default_default = {
    .subsystem_name = "default",
    .component_name = "default"
};
static log_filter_save_state_t log_filter = {
    .subsystem_name = "log",
    .component_name = "filter"
};
void open_logs(void)
{
    log_filter_save_state(&log_filter, false);
    log_filter_save_state(&default_default, false);
    log_filter_save_state(&libs_sm, true);
}
void close_logs(void)
{
    log_filter_restore_state(&libs_sm);
    log_filter_restore_state(&default_default);
    log_filter_restore_state(&log_filter);
}
__mp_mod_ifdef(state_machine, CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE)
__mp_mod_fun_1(state_machine, run)(mp_obj_t obj)
{
    open_logs();
    input_id_t input = MP_OBJ_SMALL_INT_VALUE(obj);
    __sm_run(demo_sm, input, NULL);
    close_logs();
    return mp_const_none;
}

__mp_mod_fun_0(state_machine, disp)(void)
{
    open_logs();
    __sm_disp(demo_sm);
    close_logs();
    return mp_const_none;
}

#endif /* CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE */
/* --- end of file ---------------------------------------------------------- */
