/* -------------------------------------------------------------------------- */
/*     auto-generated state machine file                                      */
/* -------------------------------------------------------------------------- */
#ifndef __LORA_RAW_STATE_MACHINE_H__
#define __LORA_RAW_STATE_MACHINE_H__

/* --- includes ------------------------------------------------------------- */
#include "state_machine.h"

/* --- METHODS -------------------------------------------------------------- */

void __sm_action_fun(lora_raw, start_tx)(void* data);
void __sm_action_fun(lora_raw, start_rx)(void* data);
void __sm_action_fun(lora_raw, process_irq)(void* data);
void __sm_action_fun(lora_raw, handle_tx_done)(void* data);
void __sm_action_fun(lora_raw, handle_tx_timeout)(void* data);
void __sm_action_fun(lora_raw, handle_rx_done)(void* data);
void __sm_action_fun(lora_raw, handle_rx_timeout)(void* data);
void __sm_action_fun(lora_raw, handle_rx_fail)(void* data);
void __sm_action_fun(lora_raw, back_to_rx)(void* data);
void __sm_action_fun(lora_raw, stop_rx_cont)(void* data);
void __sm_action_fun(lora_raw, radio_sleep)(void* data);

void __sm_action_fun(lora_raw, idle_default)(void* data);
void __sm_action_fun(lora_raw, tx_default)(void* data);
void __sm_action_fun(lora_raw, rx_default)(void* data);
void __sm_action_fun(lora_raw, rx_cont_default)(void* data);
void __sm_action_fun(lora_raw, toa_default)(void* data);
void __sm_action_fun(lora_raw, tx_temp_default)(void* data);
void __sm_action_fun(lora_raw, toa_temp_default)(void* data);
void __sm_action_fun(lora_raw, tx_cont_default)(void* data);

/* --- INPUTS --------------------------------------------------------------- */
enum {
    __sm_input_id(lora_raw, req_tx),
    __sm_input_id(lora_raw, req_rx),
    __sm_input_id(lora_raw, radio_irq),
    __sm_input_id(lora_raw, req_rx_cont),
    __sm_input_id(lora_raw, tx_done),
    __sm_input_id(lora_raw, tx_timeout),
    __sm_input_id(lora_raw, opr_timeout),
    __sm_input_id(lora_raw, rx_done),
    __sm_input_id(lora_raw, rx_timeout),
    __sm_input_id(lora_raw, rx_fail),
    __sm_input_id(lora_raw, toa_expire),
    __sm_input_id(lora_raw, end_rx_cont),
    __sm_input_id(lora_raw, end_tx_cont),
};

/* --- ACTIONS -------------------------------------------------------------- */
enum {
    __sm_action_id(lora_raw, start_tx),
    __sm_action_id(lora_raw, start_rx),
    __sm_action_id(lora_raw, process_irq),
    __sm_action_id(lora_raw, handle_tx_done),
    __sm_action_id(lora_raw, handle_tx_timeout),
    __sm_action_id(lora_raw, handle_rx_done),
    __sm_action_id(lora_raw, handle_rx_timeout),
    __sm_action_id(lora_raw, handle_rx_fail),
    __sm_action_id(lora_raw, back_to_rx),
    __sm_action_id(lora_raw, postpone),
    __sm_action_id(lora_raw, stop_rx_cont),
    __sm_action_id(lora_raw, do_nothing),
    __sm_action_id(lora_raw, radio_sleep),
};

/* --- STATES --------------------------------------------------------------- */
enum {
    __sm_state_id(lora_raw, idle),
    __sm_state_id(lora_raw, tx),
    __sm_state_id(lora_raw, rx),
    __sm_state_id(lora_raw, rx_cont),
    __sm_state_id(lora_raw, toa),
    __sm_state_id(lora_raw, tx_temp),
    __sm_state_id(lora_raw, toa_temp),
    __sm_state_id(lora_raw, tx_cont),
};

/* --- MACHINE -------------------------------------------------------------- */
extern state_machine_t __sm_machine_id(lora_raw);

/* --- end of file ---------------------------------------------------------- */
#endif /* __LORA_RAW_STATE_MACHINE_H__ */
