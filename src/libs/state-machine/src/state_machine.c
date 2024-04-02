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
 * @brief   This file contain the implementation of the state-machine APIs
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define __log_subsystem     libs
#define __log_component     state_machine
#include "log_lib.h"
__log_component_def(libs, state_machine, default, 1, 0)

#include "state_machine.h"

/** -------------------------------------------------------------------------- *
 * printing functions
 * --------------------------------------------------------------------------- *
 */
#define __width   85

static const char* null_str = "-- null --";

static void state_machine_log_state_trans(state_machine_t* p_sm,
    state_id_t ps_id, state_table_t * p_ps,
    input_id_t in_id, input_table_t * p_in,
    action_id_t act_id, action_table_t * p_act,
    state_id_t ns_id, state_table_t * p_ns,
    bool err, bool warn, bool exec, const char * msg
    )
{
    // -- log upper cap
    __log_printf("+%"__stringify(__width)"c+\n", '-');

    // -- log state machine name
    __log_printf("|%+-"__stringify(__width)"s|\n", p_sm->name);

    // -- draw a middle cap
    __log_printf("+");
    __log_printf_fill(__col_w(1, 4, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(2, 4, __width, 1) * 2 + 1, '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(4, 4,__width, 1), '-', false);
    __log_printf("+\n");

    #if 0
    // -- draw the header
    __log_printf("|");
    __log_printf_field("present-state", __col_w(1, 4,__width, 1), ' ',
        __center__, false);
    __log_printf("|");
    __log_printf_field("input", __col_w(2, 4, __width, 1), ' ',
        __center__, false);
    __log_printf("/");
    __log_printf_field("action", __col_w(3, 4, __width, 1), ' ',
        __center__, false);
    __log_printf("|");
    __log_printf_field("next-state", __col_w(4, 4, __width, 1), ' ',
        __center__, false);
    __log_printf("|\n");

    // -- draw a middle cap
    __log_printf("+");
    __log_printf_fill(__col_w(1, 4, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(2, 4, __width, 1) * 2 + 1, '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(4, 4,__width, 1), '-', false);
    __log_printf("+\n");
    #endif

    // -- draw the actual state transition
    __log_printf("|");
    if(! p_ps ){
        __log_printf(__red__"--: ");
    } else {
        __log_printf(__yellow__"%2d: ", ps_id);
    }
    __log_printf_field(p_ps ? p_ps->name : null_str,
        __col_w(1, 4, __width, 1) - 4, ' ', __left__, false);
    __log_printf(__default__"|");
    if(! p_in ){
        __log_printf(__red__"--: ");
    } else {
        __log_printf(__blue__"%2d: ", in_id);
    }
    __log_printf_field(p_in ? p_in->name : null_str,
        __col_w(2, 4, __width, 1) - 4, ' ', __left__, false);
    __log_printf(__default__"/");
    if(! p_act ){
        __log_printf(__red__"--: ");
    } else {
        __log_printf(__cyan__"%2d: ", act_id);
    }
    __log_printf_field(p_act ? p_act->name : null_str,
        __col_w(3, 4, __width, 1) - 4, ' ', __left__, false);
    __log_printf(__default__"|");
    if(! p_ns ){
        __log_printf(__red__"--: ");
    } else {
        __log_printf(__yellow__"%2d: ", ns_id);
    }
    __log_printf_field(p_ns ? p_ns->name : null_str,
        __col_w(4, 4, __width, 1) - 4, ' ', __left__, false);
    __log_printf(__default__"|\n");

    // -- log lower cap
    __log_printf("+");
    __log_printf_fill(__col_w(1, 4, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(2, 4, __width, 1) * 2 + 1, '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(4, 4,__width, 1), '-', false);
    __log_printf("+");

    if( err || warn || exec ) {
        __log_printf("\n|");
        __log_printf(" %s msg => %s", err ? " err" : warn ? "warn" : "exec",
            err ? __red__ : __yellow__);
        __log_printf_field(msg, __width - 13, ' ', __left__, false);
        __log_printf(__default__"|\n");
        __log_printf("+%"__stringify(__width)"c+", '-');
    }

    __log_endl();
}

void state_machine_print(state_machine_t* p_sm)
{
    // -- state machine name
    __log_printf("+%"__stringify(__width)"c+\n", '-');

    __log_printf("|%+-"__stringify(__width)"s|\n", p_sm->name);

    // -- inputs, actions, states
    __log_printf("+");
    __log_printf_fill(__col_w(1, 3, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(2, 3, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(3, 3, __width, 1), '-', false);
    __log_printf("+\n");

    __log_printf("|");
    __log_printf_field("states", __col_w(1, 3, __width, 1), ' ',__left__,false);
    __log_printf("|");
    __log_printf_field("inputs", __col_w(2, 3,__width, 1), ' ',__left__,false);
    __log_printf("|");
    __log_printf_field("actions", __col_w(3, 3,__width, 1), ' ',__left__,false);
    __log_printf("|\n");

    int count_inputs  = p_sm->inputs_table_size;
    int count_actions = p_sm->actions_table_size;
    int count_states  = p_sm->state_table_size;
    int i = 0, j = 0, k = 0;

    input_table_t*  p_inputs  = p_sm->inputs_table;
    action_table_t* p_actions = p_sm->actions_table;
    state_table_t*  p_states  = p_sm->state_table;

    while( count_actions || count_inputs || count_states )
    {
        __log_printf("|");
        if(count_states)
        {
            __log_printf("%2d: "__yellow__, i);
            __log_printf_field(p_states[i].name ? p_states[i].name : null_str,
                __col_w(1, 3, __width, 1) - 4, ' ', __left__, false);
            ++ i;
            -- count_states;
        }
        else
        {
            __log_printf_fill(__col_w(1, 3, __width, 1), ' ', false);
        }
        __log_printf(__default__"|");

        if(count_inputs)
        {
            __log_printf("%2d: "__blue__, j);
            __log_printf_field(p_inputs[j].name ? p_inputs[j].name : null_str,
                __col_w(2, 3, __width, 1) - 4, ' ', __left__, false);
            ++ j;
            -- count_inputs;
        }
        else
        {
            __log_printf_fill(__col_w(2, 3, __width, 1), ' ', false);
        }
        __log_printf(__default__"|");

        if(count_actions)
        {
            __log_printf("%2d: "__cyan__, k);
            __log_printf_field(p_actions[k].name ? p_actions[k].name : null_str,
                __col_w(3, 3, __width, 1) - 4, ' ', __left__, false);
            ++ k;
            -- count_actions;
        }
        else
        {
            __log_printf_fill(__col_w(3, 3, __width, 1), ' ', false);
        }
        __log_printf(__default__"|\n");
    }

    __log_printf("+");
    __log_printf_fill(__col_w(1, 3, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(2, 3, __width, 1), '-', false);
    __log_printf("+");
    __log_printf_fill(__col_w(3, 3, __width, 1), '-', false);
    __log_printf("+\n");

    __log_printf("|%+-"__stringify(__width)"s|\n", "state-transition-tables");
    __log_printf("+%"__stringify(__width)"c+\n", '-');

    __log_printf("|");
    __log_printf_field("id: input", __col_w(1, 4, __width, 1), ' ',
        __left__, false);
    __log_printf(" ");
    __log_printf_field("id:0x address: action-handler",
        __col_w(2, 4, __width, 1) * 2 + 1, ' ', __left__, false);
    __log_printf(" ");
    __log_printf_field("id: next-state", __col_w(4, 4, __width, 1), ' ',
        __left__, false);
    __log_printf("|\n");

    __log_printf("+%"__stringify(__width)"c+", '-');

    const char* tr_table_str = " transition table of ";
    count_states  = p_sm->state_table_size;
    for(i = 0; i < count_states; ++i)
    {
        const char* state_name = p_states[i].name ? p_states[i].name : null_str;
        __log_printf("\n|%s '"__yellow__"%2d: %s"__default__"'",
            tr_table_str, i, state_name);
        int fill_w = __width - strlen(tr_table_str) - strlen(state_name) - 7;
        if(i == p_sm->present_state) {
            const char * ps_str = " *<-- PRESENT-STATE";
            __log_printf(__green__"%s"__default__, ps_str);
            fill_w -= strlen(ps_str);
        }
        __log_printf_fill(fill_w, ' ', false);
        __log_printf("|\n");

        int tr_count = p_states[i].trans_table_size;
        state_trans_table_t* p_tr = p_states[i].trans_table;

        for(j = 0; j < tr_count; ++j)
        {
            input_id_t in_id = p_tr[j].input_id;
            const char* in_str = p_inputs[in_id].name;
            if(! in_str ) in_str = null_str;

            action_id_t act_id = p_tr[j].action_id;
            const char* act_str = p_actions[act_id].name;
            if(! act_str ) act_str = null_str;

            state_id_t ns_id = p_tr[j].next_state_id;
            const char* ns_str = p_states[ns_id].name;
            if(! ns_str ) ns_str = null_str;

            __log_printf("|");
            __log_printf(__blue__"%2d: %s", in_id, in_str);
            __log_printf_fill(__col_w(1,4,__width,1) - strlen(in_str) - 4 + 1,
                ' ', false);
            __log_printf(__cyan__"%2d:0x%s%08x"__cyan__": %s",
                act_id, p_actions[act_id].fun ? __green__"" : __red__"",
                p_actions[act_id].fun, act_str);
            __log_printf_fill(__col_w(2,4,__width,1)*2-strlen(act_str)-15+2,
                ' ', false);
            __log_printf(__yellow__"%2d: %s", ns_id, ns_str);
            __log_printf_fill(__col_w(4,4,__width,1) - strlen(ns_str) - 4,
                ' ', false);
            __log_printf(__default__"|\n");
        }
        __log_printf("+%"__stringify(__width)"c+", '-');
    }

    __log_endl();
}

/** -------------------------------------------------------------------------- *
 * State machine driver
 * --------------------------------------------------------------------------- *
 */
void state_machine_run(state_machine_t* p_sm, input_id_t input_id, void* data)
{
    __log_assert(p_sm, "state machine reference null pointer");

    if( !p_sm )
        return;

    state_id_t      present_state_id = p_sm->present_state;
    state_table_t*  p_state = present_state_id < p_sm->state_table_size ?
        & p_sm->state_table[present_state_id] : NULL;

    input_table_t* p_input = input_id < p_sm->inputs_table_size ?
        & p_sm->inputs_table[input_id] : NULL;

    state_trans_table_t* p_trans = p_state ? p_state->trans_table : NULL;
    uint32_t entries = p_state ? p_state->trans_table_size : 0;

    action_table_t* p_action = NULL;
    action_id_t     action_id = 0;

    state_table_t*  p_next_state = NULL;
    state_id_t      next_state_id = 0;

    const char * err_msg = NULL;
    if( ! p_input && ! p_state )
        err_msg = "invalid input and present-state ids";
    else if( ! p_input  )
        err_msg = "invalid input id";
    else if( ! p_state  )
        err_msg = "invalid present-state id";
    
    if( err_msg )
        goto report_error_and_exit;

    p_sm->state_changed_manually = false;
    while( entries -- )
    {
        if( input_id == p_trans->input_id )
        {
            action_id = p_trans->action_id;
            p_action = action_id < p_sm->actions_table_size ?
                & p_sm->actions_table[action_id] : NULL;

            next_state_id = p_trans->next_state_id;
            p_next_state = next_state_id < p_sm->state_table_size ?
                & p_sm->state_table[next_state_id] : NULL;

            err_msg = NULL;
            if( ! p_action && ! p_next_state )
                err_msg = "invalid action and next-state ids";
            else if( ! p_action )
                err_msg = "invalid action id";
            else if( ! p_next_state )
                err_msg = "invalid next-state id";

            if( err_msg )
                goto report_error_and_exit;

            const char* msg = NULL;
            if( present_state_id != next_state_id )
            {
                if(p_state->leave && p_next_state->enter)
                    msg = "ps->leave(), ns->enter(), act()";
                else if(p_state->leave)
                    msg = "ps->leave(), act()";
                else if(p_next_state->enter)
                    msg = "ns->enter(), act()";
            }
            state_machine_log_state_trans(p_sm, present_state_id, p_state,
                input_id, p_input, action_id, p_action, next_state_id,
                p_next_state, false, false, msg != NULL, msg);

            if( present_state_id != next_state_id )
            {
                if(p_state->leave)
                    p_state->leave(data);
                if(p_next_state->enter)
                    p_next_state->enter(data);
            }

            if(p_action->fun)
            {
                p_action->fun(data);
            }

            if(p_sm->state_changed_manually)
            {
                next_state_id = p_sm->present_state;
                p_next_state = & p_sm->state_table[next_state_id];

                state_machine_log_state_trans(p_sm, present_state_id, p_state,
                    input_id, p_input, action_id, p_action, next_state_id,
                    p_next_state, false, false, true,"state changed manually");
            }
            else
            {
                p_sm->present_state = next_state_id;
            }

            return;
        }
        ++ p_trans;
    }

    if(p_state->default_action)
    {
        state_machine_log_state_trans(p_sm, present_state_id, p_state,
            input_id, p_input, action_id, p_action, next_state_id,
            p_next_state, false, true, false, "running default state action");

        p_state->default_action(data);
    }
    else
    {
        state_machine_log_state_trans(p_sm, present_state_id, p_state,
            input_id, p_input, action_id, p_action, next_state_id,
            p_next_state, false, true, false, "un-handled input");
    }

    return;

    report_error_and_exit:

    state_machine_log_state_trans(p_sm, present_state_id, p_state,
        input_id, p_input, action_id, p_action, next_state_id,
        p_next_state, true, false, false, err_msg);
}

void state_machine_change_state_manually(state_machine_t* p_sm,
    state_id_t ns_id)
{
    __log_assert(p_sm, "invalid state machine pointer");

    __log_assert(ns_id < p_sm->inputs_table_size,
        "invalid state machine input id");

    p_sm->present_state = ns_id;
    p_sm->state_changed_manually = true;
}

/* --- end of file ---------------------------------------------------------- */
