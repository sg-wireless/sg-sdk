/* -------------------------------------------------------------------------- */
/*     auto-generated state machine file                                      */
/* -------------------------------------------------------------------------- */
#include "state_machine.h"
#include "lora_raw_state_machine.h"

/* --- METHODS -------------------------------------------------------------- */
void __sm_action_fun(lora_raw, idle_enter)(void* data);
void __sm_action_fun(lora_raw, tx_enter)(void* data);
void __sm_action_fun(lora_raw, rx_enter)(void* data);
void __sm_action_fun(lora_raw, rx_cont_enter)(void* data);
void __sm_action_fun(lora_raw, toa_leave)(void* data);
void __sm_action_fun(lora_raw, tx_temp_enter)(void* data);
void __sm_action_fun(lora_raw, toa_temp_leave)(void* data);


/* --- INPUTS --------------------------------------------------------------- */
static input_table_t sm_lora_raw_inputs_table[] = {
    [__sm_input_id(lora_raw, req_tx)] = { "req_tx" },
    [__sm_input_id(lora_raw, req_rx)] = { "req_rx" },
    [__sm_input_id(lora_raw, radio_irq)] = { "radio_irq" },
    [__sm_input_id(lora_raw, req_rx_cont)] = { "req_rx_cont" },
    [__sm_input_id(lora_raw, tx_done)] = { "tx_done" },
    [__sm_input_id(lora_raw, tx_timeout)] = { "tx_timeout" },
    [__sm_input_id(lora_raw, opr_timeout)] = { "opr_timeout" },
    [__sm_input_id(lora_raw, rx_done)] = { "rx_done" },
    [__sm_input_id(lora_raw, rx_timeout)] = { "rx_timeout" },
    [__sm_input_id(lora_raw, rx_fail)] = { "rx_fail" },
    [__sm_input_id(lora_raw, toa_expire)] = { "toa_expire" },
    [__sm_input_id(lora_raw, end_rx_cont)] = { "end_rx_cont" },
    [__sm_input_id(lora_raw, end_tx_cont)] = { "end_tx_cont" },
};
#define sm_lora_raw_inputs_table_size \
    (sizeof(sm_lora_raw_inputs_table)/sizeof(input_table_t))

/* --- ACTIONS -------------------------------------------------------------- */
static action_table_t sm_lora_raw_actions_table[] = {
    [__sm_action_id(lora_raw, start_tx)] = {
        "start_tx",
        __sm_action_fun(lora_raw, start_tx)},
    [__sm_action_id(lora_raw, start_rx)] = {
        "start_rx",
        __sm_action_fun(lora_raw, start_rx)},
    [__sm_action_id(lora_raw, process_irq)] = {
        "process_irq",
        __sm_action_fun(lora_raw, process_irq)},
    [__sm_action_id(lora_raw, handle_tx_done)] = {
        "handle_tx_done",
        __sm_action_fun(lora_raw, handle_tx_done)},
    [__sm_action_id(lora_raw, handle_tx_timeout)] = {
        "handle_tx_timeout",
        __sm_action_fun(lora_raw, handle_tx_timeout)},
    [__sm_action_id(lora_raw, handle_rx_done)] = {
        "handle_rx_done",
        __sm_action_fun(lora_raw, handle_rx_done)},
    [__sm_action_id(lora_raw, handle_rx_timeout)] = {
        "handle_rx_timeout",
        __sm_action_fun(lora_raw, handle_rx_timeout)},
    [__sm_action_id(lora_raw, handle_rx_fail)] = {
        "handle_rx_fail",
        __sm_action_fun(lora_raw, handle_rx_fail)},
    [__sm_action_id(lora_raw, back_to_rx)] = {
        "back_to_rx",
        __sm_action_fun(lora_raw, back_to_rx)},
    [__sm_action_id(lora_raw, postpone)] = {
        "postpone",
        0},
    [__sm_action_id(lora_raw, stop_rx_cont)] = {
        "stop_rx_cont",
        __sm_action_fun(lora_raw, stop_rx_cont)},
    [__sm_action_id(lora_raw, do_nothing)] = {
        "do_nothing",
        0},
    [__sm_action_id(lora_raw, radio_sleep)] = {
        "radio_sleep",
        __sm_action_fun(lora_raw, radio_sleep)},
};
#define sm_lora_raw_actions_table_size \
    (sizeof(sm_lora_raw_actions_table)/sizeof(action_table_t))

/* --- STATES --------------------------------------------------------------- */
/* --- state -> idle -------------------------------------------------------- */
static state_trans_table_t sm_lora_raw_idle_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, req_tx),
        .action_id = __sm_action_id(lora_raw, start_tx),
        .next_state_id = __sm_state_id(lora_raw, tx),
    },
    {
        .input_id = __sm_input_id(lora_raw, req_rx),
        .action_id = __sm_action_id(lora_raw, start_rx),
        .next_state_id = __sm_state_id(lora_raw, rx),
    },
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, process_irq),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, req_rx_cont),
        .action_id = __sm_action_id(lora_raw, start_rx),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
};
#define sm_lora_raw_idle_trans_table_size \
    (sizeof(sm_lora_raw_idle_trans_table)/sizeof(state_trans_table_t))

/* --- state -> tx ---------------------------------------------------------- */
static state_trans_table_t sm_lora_raw_tx_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, tx_done),
        .action_id = __sm_action_id(lora_raw, handle_tx_done),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, tx_timeout),
        .action_id = __sm_action_id(lora_raw, handle_tx_timeout),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, opr_timeout),
        .action_id = __sm_action_id(lora_raw, handle_tx_timeout),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, process_irq),
        .next_state_id = __sm_state_id(lora_raw, tx),
    },
};
#define sm_lora_raw_tx_trans_table_size \
    (sizeof(sm_lora_raw_tx_trans_table)/sizeof(state_trans_table_t))

/* --- state -> rx ---------------------------------------------------------- */
static state_trans_table_t sm_lora_raw_rx_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, process_irq),
        .next_state_id = __sm_state_id(lora_raw, rx),
    },
    {
        .input_id = __sm_input_id(lora_raw, rx_done),
        .action_id = __sm_action_id(lora_raw, handle_rx_done),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, rx_timeout),
        .action_id = __sm_action_id(lora_raw, handle_rx_timeout),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, opr_timeout),
        .action_id = __sm_action_id(lora_raw, handle_rx_timeout),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, rx_fail),
        .action_id = __sm_action_id(lora_raw, handle_rx_fail),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, req_tx),
        .action_id = __sm_action_id(lora_raw, start_tx),
        .next_state_id = __sm_state_id(lora_raw, tx),
    },
};
#define sm_lora_raw_rx_trans_table_size \
    (sizeof(sm_lora_raw_rx_trans_table)/sizeof(state_trans_table_t))

/* --- state -> rx_cont ----------------------------------------------------- */
static state_trans_table_t sm_lora_raw_rx_cont_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, end_rx_cont),
        .action_id = __sm_action_id(lora_raw, stop_rx_cont),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, process_irq),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
    {
        .input_id = __sm_input_id(lora_raw, rx_done),
        .action_id = __sm_action_id(lora_raw, handle_rx_done),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
    {
        .input_id = __sm_input_id(lora_raw, req_tx),
        .action_id = __sm_action_id(lora_raw, start_tx),
        .next_state_id = __sm_state_id(lora_raw, tx_temp),
    },
};
#define sm_lora_raw_rx_cont_trans_table_size \
    (sizeof(sm_lora_raw_rx_cont_trans_table)/sizeof(state_trans_table_t))

/* --- state -> toa --------------------------------------------------------- */
static state_trans_table_t sm_lora_raw_toa_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, toa_expire),
        .action_id = __sm_action_id(lora_raw, back_to_rx),
        .next_state_id = __sm_state_id(lora_raw, rx),
    },
    {
        .input_id = __sm_input_id(lora_raw, opr_timeout),
        .action_id = __sm_action_id(lora_raw, handle_rx_timeout),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, postpone),
        .next_state_id = __sm_state_id(lora_raw, toa),
    },
    {
        .input_id = __sm_input_id(lora_raw, req_tx),
        .action_id = __sm_action_id(lora_raw, start_tx),
        .next_state_id = __sm_state_id(lora_raw, tx),
    },
};
#define sm_lora_raw_toa_trans_table_size \
    (sizeof(sm_lora_raw_toa_trans_table)/sizeof(state_trans_table_t))

/* --- state -> tx_temp ----------------------------------------------------- */
static state_trans_table_t sm_lora_raw_tx_temp_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, end_rx_cont),
        .action_id = __sm_action_id(lora_raw, do_nothing),
        .next_state_id = __sm_state_id(lora_raw, tx),
    },
    {
        .input_id = __sm_input_id(lora_raw, tx_done),
        .action_id = __sm_action_id(lora_raw, handle_tx_done),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
    {
        .input_id = __sm_input_id(lora_raw, tx_timeout),
        .action_id = __sm_action_id(lora_raw, handle_tx_timeout),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
    {
        .input_id = __sm_input_id(lora_raw, opr_timeout),
        .action_id = __sm_action_id(lora_raw, handle_tx_timeout),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, process_irq),
        .next_state_id = __sm_state_id(lora_raw, tx_temp),
    },
};
#define sm_lora_raw_tx_temp_trans_table_size \
    (sizeof(sm_lora_raw_tx_temp_trans_table)/sizeof(state_trans_table_t))

/* --- state -> toa_temp ---------------------------------------------------- */
static state_trans_table_t sm_lora_raw_toa_temp_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, end_rx_cont),
        .action_id = __sm_action_id(lora_raw, stop_rx_cont),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, toa_expire),
        .action_id = __sm_action_id(lora_raw, back_to_rx),
        .next_state_id = __sm_state_id(lora_raw, rx_cont),
    },
    {
        .input_id = __sm_input_id(lora_raw, radio_irq),
        .action_id = __sm_action_id(lora_raw, postpone),
        .next_state_id = __sm_state_id(lora_raw, toa_temp),
    },
    {
        .input_id = __sm_input_id(lora_raw, req_tx),
        .action_id = __sm_action_id(lora_raw, start_tx),
        .next_state_id = __sm_state_id(lora_raw, tx_temp),
    },
};
#define sm_lora_raw_toa_temp_trans_table_size \
    (sizeof(sm_lora_raw_toa_temp_trans_table)/sizeof(state_trans_table_t))

/* --- state -> tx_cont ----------------------------------------------------- */
static state_trans_table_t sm_lora_raw_tx_cont_trans_table [] = {
    {
        .input_id = __sm_input_id(lora_raw, tx_timeout),
        .action_id = __sm_action_id(lora_raw, radio_sleep),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
    {
        .input_id = __sm_input_id(lora_raw, end_tx_cont),
        .action_id = __sm_action_id(lora_raw, radio_sleep),
        .next_state_id = __sm_state_id(lora_raw, idle),
    },
};
#define sm_lora_raw_tx_cont_trans_table_size \
    (sizeof(sm_lora_raw_tx_cont_trans_table)/sizeof(state_trans_table_t))

/* --- states-table --------------------------------------------------------- */
static state_table_t sm_lora_raw_states_table [] = {
    [__sm_state_id(lora_raw, idle)] = {
        .name = "idle",
        .trans_table = sm_lora_raw_idle_trans_table,
        .trans_table_size = sm_lora_raw_idle_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, idle_default),
        .enter = __sm_action_fun(lora_raw, idle_enter),
    },
    [__sm_state_id(lora_raw, tx)] = {
        .name = "tx",
        .trans_table = sm_lora_raw_tx_trans_table,
        .trans_table_size = sm_lora_raw_tx_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, tx_default),
        .enter = __sm_action_fun(lora_raw, tx_enter),
    },
    [__sm_state_id(lora_raw, rx)] = {
        .name = "rx",
        .trans_table = sm_lora_raw_rx_trans_table,
        .trans_table_size = sm_lora_raw_rx_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, rx_default),
        .enter = __sm_action_fun(lora_raw, rx_enter),
    },
    [__sm_state_id(lora_raw, rx_cont)] = {
        .name = "rx_cont",
        .trans_table = sm_lora_raw_rx_cont_trans_table,
        .trans_table_size = sm_lora_raw_rx_cont_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, rx_cont_default),
        .enter = __sm_action_fun(lora_raw, rx_cont_enter),
    },
    [__sm_state_id(lora_raw, toa)] = {
        .name = "toa",
        .trans_table = sm_lora_raw_toa_trans_table,
        .trans_table_size = sm_lora_raw_toa_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, toa_default),
        .leave = __sm_action_fun(lora_raw, toa_leave),
    },
    [__sm_state_id(lora_raw, tx_temp)] = {
        .name = "tx_temp",
        .trans_table = sm_lora_raw_tx_temp_trans_table,
        .trans_table_size = sm_lora_raw_tx_temp_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, tx_temp_default),
        .enter = __sm_action_fun(lora_raw, tx_temp_enter),
    },
    [__sm_state_id(lora_raw, toa_temp)] = {
        .name = "toa_temp",
        .trans_table = sm_lora_raw_toa_temp_trans_table,
        .trans_table_size = sm_lora_raw_toa_temp_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, toa_temp_default),
        .leave = __sm_action_fun(lora_raw, toa_temp_leave),
    },
    [__sm_state_id(lora_raw, tx_cont)] = {
        .name = "tx_cont",
        .trans_table = sm_lora_raw_tx_cont_trans_table,
        .trans_table_size = sm_lora_raw_tx_cont_trans_table_size,
        .default_action = __sm_action_fun(lora_raw, tx_cont_default),
    },
};
#define sm_lora_raw_states_table_size \
    (sizeof(sm_lora_raw_states_table)/sizeof(state_table_t))


/* --- MACHINE -------------------------------------------------------------- */

state_machine_t __sm_machine_id(lora_raw) = {
    .name = "lora_raw",
    .inputs_table = sm_lora_raw_inputs_table,
    .inputs_table_size = sm_lora_raw_inputs_table_size,
    .actions_table = sm_lora_raw_actions_table,
    .actions_table_size = sm_lora_raw_actions_table_size,
    .state_table = sm_lora_raw_states_table,
    .state_table_size = sm_lora_raw_states_table_size,
};

/* --- end of file ---------------------------------------------------------- */
