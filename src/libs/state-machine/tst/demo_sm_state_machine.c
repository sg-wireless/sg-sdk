/* -------------------------------------------------------------------------- */
/*     auto-generated state machine file                                      */
/* -------------------------------------------------------------------------- */
#ifdef CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE

#include "state_machine.h"
#include "demo_sm_state_machine.h"

/* --- METHODS -------------------------------------------------------------- */
void __sm_action_fun(demo_sm, A_enter)(void* data);
void __sm_action_fun(demo_sm, A_leave)(void* data);
void __sm_action_fun(demo_sm, B_enter)(void* data);
void __sm_action_fun(demo_sm, B_leave)(void* data);
void __sm_action_fun(demo_sm, C_enter)(void* data);
void __sm_action_fun(demo_sm, C_leave)(void* data);
void __sm_action_fun(demo_sm, D_enter)(void* data);
void __sm_action_fun(demo_sm, D_leave)(void* data);


/* --- INPUTS --------------------------------------------------------------- */
static input_table_t sm_demo_sm_inputs_table[] = {
    [__sm_input_id(demo_sm, x)] = { "x" },
    [__sm_input_id(demo_sm, y)] = { "y" },
    [__sm_input_id(demo_sm, z)] = { "z" },
    [__sm_input_id(demo_sm, w)] = { "w" },
};
#define sm_demo_sm_inputs_table_size \
    (sizeof(sm_demo_sm_inputs_table)/sizeof(input_table_t))

/* --- ACTIONS -------------------------------------------------------------- */
static action_table_t sm_demo_sm_actions_table[] = {
    [__sm_action_id(demo_sm, action_a_b)] = {
        "action_a_b",
        __sm_action_fun(demo_sm, action_a_b)},
    [__sm_action_id(demo_sm, action_a_c)] = {
        "action_a_c",
        __sm_action_fun(demo_sm, action_a_c)},
    [__sm_action_id(demo_sm, action_a_d)] = {
        "action_a_d",
        __sm_action_fun(demo_sm, action_a_d)},
    [__sm_action_id(demo_sm, do_nothing)] = {
        "do_nothing",
        0},
    [__sm_action_id(demo_sm, action_b_a)] = {
        "action_b_a",
        __sm_action_fun(demo_sm, action_b_a)},
    [__sm_action_id(demo_sm, action_b_c)] = {
        "action_b_c",
        __sm_action_fun(demo_sm, action_b_c)},
    [__sm_action_id(demo_sm, action_b_d)] = {
        "action_b_d",
        __sm_action_fun(demo_sm, action_b_d)},
    [__sm_action_id(demo_sm, action_c_a)] = {
        "action_c_a",
        __sm_action_fun(demo_sm, action_c_a)},
    [__sm_action_id(demo_sm, action_c_d)] = {
        "action_c_d",
        __sm_action_fun(demo_sm, action_c_d)},
    [__sm_action_id(demo_sm, action_d_a)] = {
        "action_d_a",
        __sm_action_fun(demo_sm, action_d_a)},
};
#define sm_demo_sm_actions_table_size \
    (sizeof(sm_demo_sm_actions_table)/sizeof(action_table_t))

/* --- STATES --------------------------------------------------------------- */
/* --- state -> A ----------------------------------------------------------- */
static state_trans_table_t sm_demo_sm_A_trans_table [] = {
    {
        .input_id = __sm_input_id(demo_sm, x),
        .action_id = __sm_action_id(demo_sm, action_a_b),
        .next_state_id = __sm_state_id(demo_sm, B),
    },
    {
        .input_id = __sm_input_id(demo_sm, y),
        .action_id = __sm_action_id(demo_sm, action_a_c),
        .next_state_id = __sm_state_id(demo_sm, C),
    },
    {
        .input_id = __sm_input_id(demo_sm, z),
        .action_id = __sm_action_id(demo_sm, action_a_d),
        .next_state_id = __sm_state_id(demo_sm, D),
    },
    {
        .input_id = __sm_input_id(demo_sm, w),
        .action_id = __sm_action_id(demo_sm, do_nothing),
        .next_state_id = __sm_state_id(demo_sm, A),
    },
};
#define sm_demo_sm_A_trans_table_size \
    (sizeof(sm_demo_sm_A_trans_table)/sizeof(state_trans_table_t))

/* --- state -> B ----------------------------------------------------------- */
static state_trans_table_t sm_demo_sm_B_trans_table [] = {
    {
        .input_id = __sm_input_id(demo_sm, x),
        .action_id = __sm_action_id(demo_sm, action_b_a),
        .next_state_id = __sm_state_id(demo_sm, A),
    },
    {
        .input_id = __sm_input_id(demo_sm, y),
        .action_id = __sm_action_id(demo_sm, action_b_c),
        .next_state_id = __sm_state_id(demo_sm, C),
    },
    {
        .input_id = __sm_input_id(demo_sm, z),
        .action_id = __sm_action_id(demo_sm, action_b_d),
        .next_state_id = __sm_state_id(demo_sm, D),
    },
};
#define sm_demo_sm_B_trans_table_size \
    (sizeof(sm_demo_sm_B_trans_table)/sizeof(state_trans_table_t))

/* --- state -> C ----------------------------------------------------------- */
static state_trans_table_t sm_demo_sm_C_trans_table [] = {
    {
        .input_id = __sm_input_id(demo_sm, x),
        .action_id = __sm_action_id(demo_sm, do_nothing),
        .next_state_id = __sm_state_id(demo_sm, C),
    },
    {
        .input_id = __sm_input_id(demo_sm, y),
        .action_id = __sm_action_id(demo_sm, action_c_a),
        .next_state_id = __sm_state_id(demo_sm, A),
    },
    {
        .input_id = __sm_input_id(demo_sm, z),
        .action_id = __sm_action_id(demo_sm, action_c_d),
        .next_state_id = __sm_state_id(demo_sm, D),
    },
};
#define sm_demo_sm_C_trans_table_size \
    (sizeof(sm_demo_sm_C_trans_table)/sizeof(state_trans_table_t))

/* --- state -> D ----------------------------------------------------------- */
static state_trans_table_t sm_demo_sm_D_trans_table [] = {
    {
        .input_id = __sm_input_id(demo_sm, x),
        .action_id = __sm_action_id(demo_sm, do_nothing),
        .next_state_id = __sm_state_id(demo_sm, D),
    },
    {
        .input_id = __sm_input_id(demo_sm, y),
        .action_id = __sm_action_id(demo_sm, do_nothing),
        .next_state_id = __sm_state_id(demo_sm, D),
    },
    {
        .input_id = __sm_input_id(demo_sm, z),
        .action_id = __sm_action_id(demo_sm, action_d_a),
        .next_state_id = __sm_state_id(demo_sm, A),
    },
};
#define sm_demo_sm_D_trans_table_size \
    (sizeof(sm_demo_sm_D_trans_table)/sizeof(state_trans_table_t))

/* --- states-table --------------------------------------------------------- */
static state_table_t sm_demo_sm_states_table [] = {
    [__sm_state_id(demo_sm, A)] = {
        .name = "A",
        .trans_table = sm_demo_sm_A_trans_table,
        .trans_table_size = sm_demo_sm_A_trans_table_size,
        .default_action = __sm_action_fun(demo_sm, A_default),
        .enter = __sm_action_fun(demo_sm, A_enter),
        .leave = __sm_action_fun(demo_sm, A_leave),
    },
    [__sm_state_id(demo_sm, B)] = {
        .name = "B",
        .trans_table = sm_demo_sm_B_trans_table,
        .trans_table_size = sm_demo_sm_B_trans_table_size,
        .default_action = __sm_action_fun(demo_sm, B_default),
        .enter = __sm_action_fun(demo_sm, B_enter),
        .leave = __sm_action_fun(demo_sm, B_leave),
    },
    [__sm_state_id(demo_sm, C)] = {
        .name = "C",
        .trans_table = sm_demo_sm_C_trans_table,
        .trans_table_size = sm_demo_sm_C_trans_table_size,
        .default_action = __sm_action_fun(demo_sm, C_default),
        .enter = __sm_action_fun(demo_sm, C_enter),
        .leave = __sm_action_fun(demo_sm, C_leave),
    },
    [__sm_state_id(demo_sm, D)] = {
        .name = "D",
        .trans_table = sm_demo_sm_D_trans_table,
        .trans_table_size = sm_demo_sm_D_trans_table_size,
        .default_action = __sm_action_fun(demo_sm, D_default),
        .enter = __sm_action_fun(demo_sm, D_enter),
        .leave = __sm_action_fun(demo_sm, D_leave),
    },
};
#define sm_demo_sm_states_table_size \
    (sizeof(sm_demo_sm_states_table)/sizeof(state_table_t))


/* --- MACHINE -------------------------------------------------------------- */

state_machine_t __sm_machine_id(demo_sm) = {
    .name = "demo_sm",
    .inputs_table = sm_demo_sm_inputs_table,
    .inputs_table_size = sm_demo_sm_inputs_table_size,
    .actions_table = sm_demo_sm_actions_table,
    .actions_table_size = sm_demo_sm_actions_table_size,
    .state_table = sm_demo_sm_states_table,
    .state_table_size = sm_demo_sm_states_table_size,
};

/* --- end of file ---------------------------------------------------------- */

#endif /* CONFIG_SDK_LIBS_EXAMPLE_STATE_MACHINE */
