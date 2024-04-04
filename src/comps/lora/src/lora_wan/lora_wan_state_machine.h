/* -------------------------------------------------------------------------- */
/*     auto-generated state machine file                                      */
/* -------------------------------------------------------------------------- */
#ifndef __LORA_WAN_STATE_MACHINE_H__
#define __LORA_WAN_STATE_MACHINE_H__

/* --- includes ------------------------------------------------------------- */
#include "state_machine.h"

/* --- METHODS -------------------------------------------------------------- */

void __sm_action_fun(lora_wan, start_join)(void* data);
void __sm_action_fun(lora_wan, process_mac)(void* data);
void __sm_action_fun(lora_wan, process_radio)(void* data);
void __sm_action_fun(lora_wan, switch_slass)(void* data);
void __sm_action_fun(lora_wan, restart_join)(void* data);
void __sm_action_fun(lora_wan, commission)(void* data);
void __sm_action_fun(lora_wan, start_trx)(void* data);
void __sm_action_fun(lora_wan, trx_timeout)(void* data);
void __sm_action_fun(lora_wan, ind_class)(void* data);


/* --- INPUTS --------------------------------------------------------------- */
enum {
    __sm_input_id(lora_wan, join_req),
    __sm_input_id(lora_wan, mac_req),
    __sm_input_id(lora_wan, radio_evt),
    __sm_input_id(lora_wan, join_done),
    __sm_input_id(lora_wan, join_fail),
    __sm_input_id(lora_wan, commission),
    __sm_input_id(lora_wan, duty_cycle),
    __sm_input_id(lora_wan, req_class),
    __sm_input_id(lora_wan, timeout),
    __sm_input_id(lora_wan, class_chg),
};

/* --- ACTIONS -------------------------------------------------------------- */
enum {
    __sm_action_id(lora_wan, start_join),
    __sm_action_id(lora_wan, process_mac),
    __sm_action_id(lora_wan, process_radio),
    __sm_action_id(lora_wan, switch_slass),
    __sm_action_id(lora_wan, restart_join),
    __sm_action_id(lora_wan, commission),
    __sm_action_id(lora_wan, start_trx),
    __sm_action_id(lora_wan, trx_timeout),
    __sm_action_id(lora_wan, ind_class),
    __sm_action_id(lora_wan, do_nothing),
};

/* --- STATES --------------------------------------------------------------- */
enum {
    __sm_state_id(lora_wan, not_joined),
    __sm_state_id(lora_wan, chg_class),
    __sm_state_id(lora_wan, joined),
    __sm_state_id(lora_wan, trx),
};

/* --- MACHINE -------------------------------------------------------------- */
extern state_machine_t __sm_machine_id(lora_wan);

/* --- end of file ---------------------------------------------------------- */
#endif /* __LORA_WAN_STATE_MACHINE_H__ */
