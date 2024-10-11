/* -------------------------------------------------------------------------- */
/*     auto-generated state machine file                                      */
/* -------------------------------------------------------------------------- */
#include "state_machine.h"
#include "lora_wan_state_machine.h"

/* --- METHODS -------------------------------------------------------------- */
void __sm_action_fun(lora_wan, joined_enter)(void* data);


/* --- INPUTS --------------------------------------------------------------- */
static input_table_t sm_lora_wan_inputs_table[] = {
    [__sm_input_id(lora_wan, join_req)] = { "join_req" },
    [__sm_input_id(lora_wan, mac_req)] = { "mac_req" },
    [__sm_input_id(lora_wan, join_done)] = { "join_done" },
    [__sm_input_id(lora_wan, join_fail)] = { "join_fail" },
    [__sm_input_id(lora_wan, commission)] = { "commission" },
    [__sm_input_id(lora_wan, lct_on)] = { "lct_on" },
    [__sm_input_id(lora_wan, duty_cycle)] = { "duty_cycle" },
    [__sm_input_id(lora_wan, req_class)] = { "req_class" },
    [__sm_input_id(lora_wan, timeout)] = { "timeout" },
    [__sm_input_id(lora_wan, class_chg)] = { "class_chg" },
    [__sm_input_id(lora_wan, lct_off)] = { "lct_off" },
    [__sm_input_id(lora_wan, rejoin_req)] = { "rejoin_req" },
};
#define sm_lora_wan_inputs_table_size \
    (sizeof(sm_lora_wan_inputs_table)/sizeof(input_table_t))

/* --- ACTIONS -------------------------------------------------------------- */
static action_table_t sm_lora_wan_actions_table[] = {
    [__sm_action_id(lora_wan, start_join)] = {
        "start_join",
        __sm_action_fun(lora_wan, start_join)},
    [__sm_action_id(lora_wan, process_mac)] = {
        "process_mac",
        __sm_action_fun(lora_wan, process_mac)},
    [__sm_action_id(lora_wan, switch_slass)] = {
        "switch_slass",
        __sm_action_fun(lora_wan, switch_slass)},
    [__sm_action_id(lora_wan, restart_join)] = {
        "restart_join",
        __sm_action_fun(lora_wan, restart_join)},
    [__sm_action_id(lora_wan, commission)] = {
        "commission",
        __sm_action_fun(lora_wan, commission)},
    [__sm_action_id(lora_wan, lct_enter)] = {
        "lct_enter",
        __sm_action_fun(lora_wan, lct_enter)},
    [__sm_action_id(lora_wan, start_trx)] = {
        "start_trx",
        __sm_action_fun(lora_wan, start_trx)},
    [__sm_action_id(lora_wan, trx_timeout)] = {
        "trx_timeout",
        __sm_action_fun(lora_wan, trx_timeout)},
    [__sm_action_id(lora_wan, ind_class)] = {
        "ind_class",
        __sm_action_fun(lora_wan, ind_class)},
    [__sm_action_id(lora_wan, do_nothing)] = {
        "do_nothing",
        0},
    [__sm_action_id(lora_wan, lct_commission)] = {
        "lct_commission",
        __sm_action_fun(lora_wan, lct_commission)},
    [__sm_action_id(lora_wan, lct_join)] = {
        "lct_join",
        __sm_action_fun(lora_wan, lct_join)},
    [__sm_action_id(lora_wan, lct_exit)] = {
        "lct_exit",
        __sm_action_fun(lora_wan, lct_exit)},
    [__sm_action_id(lora_wan, lct_handle)] = {
        "lct_handle",
        __sm_action_fun(lora_wan, lct_handle)},
    [__sm_action_id(lora_wan, lct_joined)] = {
        "lct_joined",
        __sm_action_fun(lora_wan, lct_joined)},
};
#define sm_lora_wan_actions_table_size \
    (sizeof(sm_lora_wan_actions_table)/sizeof(action_table_t))

/* --- STATES --------------------------------------------------------------- */
/* --- state -> not_joined -------------------------------------------------- */
static state_trans_table_t sm_lora_wan_not_joined_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, join_req),
        .action_id = __sm_action_id(lora_wan, start_join),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, mac_req),
        .action_id = __sm_action_id(lora_wan, process_mac),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_done),
        .action_id = __sm_action_id(lora_wan, switch_slass),
        .next_state_id = __sm_state_id(lora_wan, chg_class),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_fail),
        .action_id = __sm_action_id(lora_wan, restart_join),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, commission),
        .action_id = __sm_action_id(lora_wan, commission),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_on),
        .action_id = __sm_action_id(lora_wan, lct_enter),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
};
#define sm_lora_wan_not_joined_trans_table_size \
    (sizeof(sm_lora_wan_not_joined_trans_table)/sizeof(state_trans_table_t))

/* --- state -> chg_class --------------------------------------------------- */
static state_trans_table_t sm_lora_wan_chg_class_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, req_class),
        .action_id = __sm_action_id(lora_wan, switch_slass),
        .next_state_id = __sm_state_id(lora_wan, chg_class),
    },
    {
        .input_id = __sm_input_id(lora_wan, class_chg),
        .action_id = __sm_action_id(lora_wan, ind_class),
        .next_state_id = __sm_state_id(lora_wan, joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, mac_req),
        .action_id = __sm_action_id(lora_wan, process_mac),
        .next_state_id = __sm_state_id(lora_wan, chg_class),
    },
    {
        .input_id = __sm_input_id(lora_wan, duty_cycle),
        .action_id = __sm_action_id(lora_wan, do_nothing),
        .next_state_id = __sm_state_id(lora_wan, chg_class),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_req),
        .action_id = __sm_action_id(lora_wan, start_join),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, commission),
        .action_id = __sm_action_id(lora_wan, commission),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, timeout),
        .action_id = __sm_action_id(lora_wan, trx_timeout),
        .next_state_id = __sm_state_id(lora_wan, chg_class),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_on),
        .action_id = __sm_action_id(lora_wan, lct_enter),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
};
#define sm_lora_wan_chg_class_trans_table_size \
    (sizeof(sm_lora_wan_chg_class_trans_table)/sizeof(state_trans_table_t))

/* --- state -> lct --------------------------------------------------------- */
static state_trans_table_t sm_lora_wan_lct_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, duty_cycle),
        .action_id = __sm_action_id(lora_wan, lct_handle),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
    {
        .input_id = __sm_input_id(lora_wan, mac_req),
        .action_id = __sm_action_id(lora_wan, process_mac),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_req),
        .action_id = __sm_action_id(lora_wan, lct_join),
        .next_state_id = __sm_state_id(lora_wan, lct_join),
    },
    {
        .input_id = __sm_input_id(lora_wan, rejoin_req),
        .action_id = __sm_action_id(lora_wan, lct_join),
        .next_state_id = __sm_state_id(lora_wan, lct_join),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_off),
        .action_id = __sm_action_id(lora_wan, lct_exit),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
};
#define sm_lora_wan_lct_trans_table_size \
    (sizeof(sm_lora_wan_lct_trans_table)/sizeof(state_trans_table_t))

/* --- state -> joined ------------------------------------------------------ */
static state_trans_table_t sm_lora_wan_joined_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, duty_cycle),
        .action_id = __sm_action_id(lora_wan, start_trx),
        .next_state_id = __sm_state_id(lora_wan, trx),
    },
    {
        .input_id = __sm_input_id(lora_wan, mac_req),
        .action_id = __sm_action_id(lora_wan, process_mac),
        .next_state_id = __sm_state_id(lora_wan, joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_req),
        .action_id = __sm_action_id(lora_wan, start_join),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, commission),
        .action_id = __sm_action_id(lora_wan, commission),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, req_class),
        .action_id = __sm_action_id(lora_wan, switch_slass),
        .next_state_id = __sm_state_id(lora_wan, chg_class),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_on),
        .action_id = __sm_action_id(lora_wan, lct_enter),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
};
#define sm_lora_wan_joined_trans_table_size \
    (sizeof(sm_lora_wan_joined_trans_table)/sizeof(state_trans_table_t))

/* --- state -> trx --------------------------------------------------------- */
static state_trans_table_t sm_lora_wan_trx_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, duty_cycle),
        .action_id = __sm_action_id(lora_wan, start_trx),
        .next_state_id = __sm_state_id(lora_wan, trx),
    },
    {
        .input_id = __sm_input_id(lora_wan, mac_req),
        .action_id = __sm_action_id(lora_wan, process_mac),
        .next_state_id = __sm_state_id(lora_wan, trx),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_req),
        .action_id = __sm_action_id(lora_wan, start_join),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, commission),
        .action_id = __sm_action_id(lora_wan, commission),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, timeout),
        .action_id = __sm_action_id(lora_wan, trx_timeout),
        .next_state_id = __sm_state_id(lora_wan, joined),
    },
    {
        .input_id = __sm_input_id(lora_wan, req_class),
        .action_id = __sm_action_id(lora_wan, switch_slass),
        .next_state_id = __sm_state_id(lora_wan, trx),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_on),
        .action_id = __sm_action_id(lora_wan, lct_enter),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
};
#define sm_lora_wan_trx_trans_table_size \
    (sizeof(sm_lora_wan_trx_trans_table)/sizeof(state_trans_table_t))

/* --- state -> lct_idle ---------------------------------------------------- */
static state_trans_table_t sm_lora_wan_lct_idle_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, commission),
        .action_id = __sm_action_id(lora_wan, lct_commission),
        .next_state_id = __sm_state_id(lora_wan, lct_idle),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_req),
        .action_id = __sm_action_id(lora_wan, lct_join),
        .next_state_id = __sm_state_id(lora_wan, lct_join),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_off),
        .action_id = __sm_action_id(lora_wan, lct_exit),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
};
#define sm_lora_wan_lct_idle_trans_table_size \
    (sizeof(sm_lora_wan_lct_idle_trans_table)/sizeof(state_trans_table_t))

/* --- state -> lct_join ---------------------------------------------------- */
static state_trans_table_t sm_lora_wan_lct_join_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_wan, mac_req),
        .action_id = __sm_action_id(lora_wan, process_mac),
        .next_state_id = __sm_state_id(lora_wan, lct_join),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_done),
        .action_id = __sm_action_id(lora_wan, lct_joined),
        .next_state_id = __sm_state_id(lora_wan, lct),
    },
    {
        .input_id = __sm_input_id(lora_wan, join_fail),
        .action_id = __sm_action_id(lora_wan, lct_join),
        .next_state_id = __sm_state_id(lora_wan, lct_join),
    },
    {
        .input_id = __sm_input_id(lora_wan, lct_off),
        .action_id = __sm_action_id(lora_wan, lct_exit),
        .next_state_id = __sm_state_id(lora_wan, not_joined),
    },
};
#define sm_lora_wan_lct_join_trans_table_size \
    (sizeof(sm_lora_wan_lct_join_trans_table)/sizeof(state_trans_table_t))

/* --- states-table --------------------------------------------------------- */
static state_table_t sm_lora_wan_states_table [] = {
    [__sm_state_id(lora_wan, not_joined)] = {
        .name = "not_joined",
        .trans_table = sm_lora_wan_not_joined_trans_table,
        .trans_table_size = sm_lora_wan_not_joined_trans_table_size,
    },
    [__sm_state_id(lora_wan, chg_class)] = {
        .name = "chg_class",
        .trans_table = sm_lora_wan_chg_class_trans_table,
        .trans_table_size = sm_lora_wan_chg_class_trans_table_size,
    },
    [__sm_state_id(lora_wan, lct)] = {
        .name = "lct",
        .trans_table = sm_lora_wan_lct_trans_table,
        .trans_table_size = sm_lora_wan_lct_trans_table_size,
    },
    [__sm_state_id(lora_wan, joined)] = {
        .name = "joined",
        .trans_table = sm_lora_wan_joined_trans_table,
        .trans_table_size = sm_lora_wan_joined_trans_table_size,
        .enter = __sm_action_fun(lora_wan, joined_enter),
    },
    [__sm_state_id(lora_wan, trx)] = {
        .name = "trx",
        .trans_table = sm_lora_wan_trx_trans_table,
        .trans_table_size = sm_lora_wan_trx_trans_table_size,
    },
    [__sm_state_id(lora_wan, lct_idle)] = {
        .name = "lct_idle",
        .trans_table = sm_lora_wan_lct_idle_trans_table,
        .trans_table_size = sm_lora_wan_lct_idle_trans_table_size,
    },
    [__sm_state_id(lora_wan, lct_join)] = {
        .name = "lct_join",
        .trans_table = sm_lora_wan_lct_join_trans_table,
        .trans_table_size = sm_lora_wan_lct_join_trans_table_size,
    },
};
#define sm_lora_wan_states_table_size \
    (sizeof(sm_lora_wan_states_table)/sizeof(state_table_t))


/* --- MACHINE -------------------------------------------------------------- */

state_machine_t __sm_machine_id(lora_wan) = {
    .name = "lora_wan",
    .inputs_table = sm_lora_wan_inputs_table,
    .inputs_table_size = sm_lora_wan_inputs_table_size,
    .actions_table = sm_lora_wan_actions_table,
    .actions_table_size = sm_lora_wan_actions_table_size,
    .state_table = sm_lora_wan_states_table,
    .state_table_size = sm_lora_wan_states_table_size,
};

/* --- end of file ---------------------------------------------------------- */
