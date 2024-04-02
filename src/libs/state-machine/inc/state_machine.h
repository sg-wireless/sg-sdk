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
 * @brief   This file represents the interface to state-machine library.
 * --------------------------------------------------------------------------- *
 */
#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * state-machine macro interface
 * the generator will use these macros to generate the appropriate state-machine
 * definition code
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * Description
 * ===========
 * § This module offers an abstract way to define a finite state-machine
 * 
 * § Defining a new state machine:
 *      * In any source file you can define one or more state machine
 * 
 *      * the state machine is defined by its state transition table. and it is
 *        the minimal requirement to define a state machine.
 * 
 *      * in the state machine definition macros, we use only a human readable
 *        names.
 * 
 *      * The following set of macros are used to define the state machine and
 *        they are used by the state-machine generation script to collect the
 *        state machine information and generate the proper meta data for it.
 *          >> state-transition macro:
 *          __sm_trans(<state-machine>, <present-state>, <action>, <next-state>)
 *              where
 *                  <state-machine>: the state machine unique name
 *                  <present-state>: the current or present state
 *                  <action>:    the action to be triggerred during transition
 *                  <next-state>: the next state of the machine
 * 
 *          >> action definition macro
 *          It is an optional, it an action is not set, no action will execute
 *          but state transition will still occur
 *              __sm_action(<state-machine>, <action>)(void*)
 *              { <action-body> }
 * 
 *          >> there are an optional state enter and leave optional actions.
 *          the user can define optionally an action to be executed usually
 *          when leaving a state and an action to be executed when entering
 *          a state.
 *              __sm_state_enter(<state-machine>, <state>)(void*)
 *              { <action-body> }
 *              __sm_state_leave(<state-machine>, <state>)(void*)
 *              { <action-body> }
 *          
 *          Note: now we have three possible action will be executed upon
 *          state is going to be transitted from state X to state Y as follows
 *              state-X-leave-action if exist
 *              state-Y-enter-action if exist
 *              state-transition-action if exist
 * 
 *      * changing state manually (conditionally): It is possible to change
 *        the next state manually by calling this macro __sm_ch_state() in
 *        any of the state machine actions.
 * 
 *      * an optional preprocessor ifdef macro can be defined, so that the
 *        generated code can be enabled/disabled upon this macro definition
 *        to do that, use this macro:
 *              __sm_ifdef(<state-machine-name>, <PREPROCESSOR_MACRO>)
 * 
 *      * an optional state default action can be specified to be executed
 *        upon receiving an inputs which don't have state transition entry
 *        for the current state. It could be defined by the following macro:
 *              __sm_state_default_action(<state-machine>, <state>)(void*)
 *              { <default-action-body> }
 * 
 * § State-machine generation:
 *   after defining the state-machine, the state_machine_gen.py script can run
 *   as:    state_machine_gen.py  <gen-dir> <input-files>
 *   it will by its turn do the required parsing and generation of the state
 *   machine meta data in two files:
 *      <state-machine-name>_state_machine.c
 *      <state-machine-name>_state_machine.h
 *   then, it is mandatory to include the header file in the state-machine
 *   definition source file
 *      #include "<state-machine-name>_state_machine.h"
 * 
 * --------------------------------------------------------------------------- *
 */
/**
 * the following parameter names meaning are applicaple to all macros:
 * 
 *      _sm     the state-machine name
 *      _st     the state name
 *      _in     the input name
 *      _ps     the present-state name
 *      _ns     the next-state name
 *      _ac     the action name
 */

/**
 * it is used to define a state-machine preprocessor macro, to be used by the
 * the state machine generator to enclose the whole generated code by this
 * macro using #ifdef preprocessor.
 * 
 * @param   _def    the preprocessor definition macro
 */
#define __sm_ifdef(_sm, _def)

/**
 * a group of macros to identify the different state machine objects names from
 * the given human readable name.
 * 
 * @def __sm_machine_id     identify the state-machine main object name
 * @def __sm_state_id       identify the states enum constants
 * @def __sm_input_id       identify the inputs enum constants
 * @def __sm_action_id      identify the actions enum constants
 * @def __sm_action_fun     identify the action function name
 */
#define __sm_machine_id(_sm)        sm_##_machine_id_##_sm
#define __sm_state_id(_sm, _st)     sm_##_sm##_state_id_##_st
#define __sm_input_id(_sm, _in)     sm_##_sm##_input_id_##_in
#define __sm_action_id(_sm, _ac)    sm_##_sm##_action_id_##_ac
#define __sm_action_fun(_sm, _ac)   sm_##_sm##_action_fun_##_ac

/**
 * it is used to define a state transition. it is only used by the generator
 */
#define __sm_trans(_sm, _ps, _in, _ac, _ns)

/**
 * the following set of macros are used to define different types of state
 * machine actions
 * 
 * @def __sm_action         action to be executed when upon a state transition
 * @def __sm_state_enter    action to be executed every time the state \a _st
 *                          is entered.
 * @def __sm_state_leave    action to be executed every time the state \a _st
 *                          is left.
 * @def __sm_state_default_action action to be executed when no state transition
 *                          is found for upon a certain input.
 */
#define __sm_state_enter(_sm, _st)   void __sm_action_fun(_sm, _st##_enter)
#define __sm_state_leave(_sm, _st)   void __sm_action_fun(_sm, _st##_leave)
#define __sm_action(_sm, _ac)       void __sm_action_fun(_sm, _ac)
#define __sm_state_default_action(_sm, _st)  \
    void __sm_action_fun(_sm, _st ## _default)

/**
 * a macro to be called to start processing the state-machine input
 */
/**
 * a macro to be called to start processing the state-machine input
 */
#define __sm_run(_sm, _in, data)            \
    state_machine_run(&__sm_machine_id(_sm),\
        _in,                                \
        data)

/**
 * a macro to be called to change state manually upon a desired condition
 */
#define __sm_ch_state(_sm, _st)             \
    state_machine_change_state_manually(    \
        &__sm_machine_id(_sm),              \
        __sm_state_id(_sm, _st))

/**
 * a macro to retrieve the current state id for conditional checking purposes
 */
#define __sm_present_state_id(_sm) (__sm_machine_id(_sm).present_state)

/**
 * a demonstrative macro for displaying the whole state-machine visually
 */
#define __sm_disp(_sm)  state_machine_print(&__sm_machine_id(_sm))

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */
typedef uint32_t state_id_t;
typedef uint32_t input_id_t;
typedef uint32_t action_id_t;
typedef void state_action_t( void* data );

typedef struct {
    const char * name;
} input_table_t;

typedef struct {
    const char*     name;
    state_action_t* fun;
} action_table_t;

typedef struct {
    input_id_t      input_id;
    state_id_t      next_state_id;
    action_id_t     action_id;
} state_trans_table_t;

typedef struct {
    const char*             name;
    state_trans_table_t *   trans_table;
    uint8_t                 trans_table_size;
    state_action_t*         default_action;
    state_action_t*         enter;
    state_action_t*         leave;
} state_table_t;

typedef struct {
    const char*     name;
    input_table_t*  inputs_table;
    uint32_t        inputs_table_size;
    action_table_t* actions_table;
    uint32_t        actions_table_size;
    state_table_t*  state_table;
    uint32_t        state_table_size;
    state_id_t      present_state;
    bool            state_changed_manually;
} state_machine_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
void state_machine_run(
    state_machine_t*    state_machine,
    input_id_t          input_id,
    void*               data
    );

void state_machine_print(
    state_machine_t*    state_machine
    );

void state_machine_change_state_manually(
    state_machine_t* p_sm,
    state_id_t id
    );

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __STATE_MACHINE_H__ */
