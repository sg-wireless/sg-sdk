/* -------------------------------------------------------------------------- */
/*     auto-generated state machine file                                      */
/* -------------------------------------------------------------------------- */
#ifndef __DEMO_SM_STATE_MACHINE_H__
#define __DEMO_SM_STATE_MACHINE_H__

#ifdef CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE

/* --- includes ------------------------------------------------------------- */
#include "state_machine.h"

/* --- METHODS -------------------------------------------------------------- */

void __sm_action_fun(demo_sm, action_a_b)(void* data);
void __sm_action_fun(demo_sm, action_a_c)(void* data);
void __sm_action_fun(demo_sm, action_a_d)(void* data);
void __sm_action_fun(demo_sm, action_b_a)(void* data);
void __sm_action_fun(demo_sm, action_b_c)(void* data);
void __sm_action_fun(demo_sm, action_b_d)(void* data);
void __sm_action_fun(demo_sm, action_c_a)(void* data);
void __sm_action_fun(demo_sm, action_c_d)(void* data);
void __sm_action_fun(demo_sm, action_d_a)(void* data);

void __sm_action_fun(demo_sm, A_default)(void* data);
void __sm_action_fun(demo_sm, B_default)(void* data);
void __sm_action_fun(demo_sm, C_default)(void* data);
void __sm_action_fun(demo_sm, D_default)(void* data);

/* --- INPUTS --------------------------------------------------------------- */
enum {
    __sm_input_id(demo_sm, x),
    __sm_input_id(demo_sm, y),
    __sm_input_id(demo_sm, z),
    __sm_input_id(demo_sm, w),
};

/* --- ACTIONS -------------------------------------------------------------- */
enum {
    __sm_action_id(demo_sm, action_a_b),
    __sm_action_id(demo_sm, action_a_c),
    __sm_action_id(demo_sm, action_a_d),
    __sm_action_id(demo_sm, do_nothing),
    __sm_action_id(demo_sm, action_b_a),
    __sm_action_id(demo_sm, action_b_c),
    __sm_action_id(demo_sm, action_b_d),
    __sm_action_id(demo_sm, action_c_a),
    __sm_action_id(demo_sm, action_c_d),
    __sm_action_id(demo_sm, action_d_a),
};

/* --- STATES --------------------------------------------------------------- */
enum {
    __sm_state_id(demo_sm, A),
    __sm_state_id(demo_sm, B),
    __sm_state_id(demo_sm, C),
    __sm_state_id(demo_sm, D),
};

/* --- MACHINE -------------------------------------------------------------- */
extern state_machine_t __sm_machine_id(demo_sm);

/* --- end of file ---------------------------------------------------------- */

#endif /* CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE */
#endif /* __DEMO_SM_STATE_MACHINE_H__ */
