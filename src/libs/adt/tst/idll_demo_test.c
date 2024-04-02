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
 * --------------------------------------------------------------------------- *
 * Copyright (c) 2022, Pycom Limited.
 *
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * @author  Ahmed Sabry (Pycom, SG Wireless)
 * 
 * @brief   This file implements the indexed double linked list demo example.
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_SDK_ADT_IDLL_DEMO_TEST_ENABLE

#include "mp_lite_if.h"

#define __log_subsystem     libs
#define __log_component     idll
#include "log_lib.h"

#include "idll.h"
/** -------------------------------------------------------------------------- *
 * demo test implementation
 * --------------------------------------------------------------------------- *
 */

void idll_new_unittest(void);

__mp_mod_ifdef(idll, CONFIG_SDK_ADT_IDLL_DEMO_TEST_ENABLE);
__mp_mod_fun_0(idll, unittest)(void) {
    idll_new_unittest();
    return mp_const_none;
}

static void run_test_1(void);
static void run_test_2(void);
static void run_test_3(void);

void idll_new_unittest(void)
{
    __log_output_header("[ Indexed Double Linked List Unittest ]", 100, '=');
    __log_output("\n");
    __log_output_header("[ test 1 ]", 100, '=');
    run_test_1();
    __log_output_header("[ test 2 ]", 100, '=');
    run_test_2();
    __log_output_header("[ test 3 ]", 100, '=');
    run_test_3();
    __log_output_fill(100, '=', true);
}

static void run_test_1(void)
{
    int i;

    /* define a testing array */
    #define __test_array_max  4
    struct _test_array_t{
        int     d1;
        uint8_t d8;
        __idll_node_links_members(__idll_link_type_uint_8__);
        int d2;
    } test_array[__test_array_max], * p_node, * p_anchor;

    /* fill the test array */
    for(i=0; i<__test_array_max; ++i) {
        test_array[i].d1 = i;
        test_array[i].d2 = i * i;
        test_array[i].d8 = 0xA0 + i;
    }

    /* define an idll object and init it */
    __idll_def_obj(idll_obj, test_array, struct _test_array_t,
        __idll_link_type_uint_8__, next);
    idll_init(&idll_obj);
    #define __test_array_display()                                  \
        idll_foreach(&idll_obj, p_node) {                               \
            __log_output("- idll item: d1:%2d , d2:%2d , d8:%02x\n",  \
                p_node->d1, p_node->d2, p_node->d8);                \
        }

    __log_output("===== [ test 1 ] =====\n");
    __log_output("-- we have indexed list from node:0 to node:%d\n",
        __test_array_max-1);
    __log_output("-- shifting: 0 , 1\n");
    for(i = 0; i < __test_array_max - 2; ++i) idll_shift(&idll_obj, &test_array[i]);
    __test_array_display();

    __log_output("-- inserting: (node:3 @ node:1)\n");
    p_node = test_array + __test_array_max - 1;
    p_anchor = test_array + 1;
    idll_insert(&idll_obj, p_node, p_anchor);
    __test_array_display();

    __log_output("-- inserting: (node:2 @ node:0)\n");
    p_node = test_array + __test_array_max - 2;
    p_anchor = test_array;
    idll_insert(&idll_obj, p_node, p_anchor);
    __test_array_display();

    __log_output("-- deleting: node:3\n");
    p_node = test_array + 3;
    idll_del(&idll_obj, p_node);
    __test_array_display();

    __log_output("-- poping every thing\n");
    while((p_node = idll_pop(&idll_obj)) != NULL) {
        __log_output("- idll item: d1:%2d , d2:%2d , d8:%02x\n",
            p_node->d1, p_node->d2, p_node->d8);
    }

    __log_output("-- shifting all nodes\n");
    for(i=0; i < __test_array_max; ++i) idll_shift(&idll_obj, test_array + i);
    __test_array_display();

    __log_output("-- unshifting everything\n");
    while((p_node = idll_unshift(&idll_obj)) != NULL) {
        __log_output("- idll item: d1:%2d , d2:%2d , d8:%02x\n",
            p_node->d1, p_node->d2, p_node->d8);
    }

    __log_output("-- pushing all nodes\n");
    for(i=0; i < __test_array_max; ++i) idll_push(&idll_obj, test_array + i);
    __test_array_display();

    __log_output("-- poping everything\n");
    while((p_node = idll_pop(&idll_obj)) != NULL) {
        __log_output("- idll item: d1:%2d , d2:%2d , d8:%02x\n",
            p_node->d1, p_node->d2, p_node->d8);
    }
}

static void run_test_2(void)
{
    #ifdef CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE
    typedef struct _test_arr_2_s {
        int data;
        __idll_node_links_members(__idll_link_type_uint_16__);
    } test_arr_2_t;

    test_arr_2_t test_arr_2 [10];

    /* define the idll object */
    __idll_def_obj(idll_obj, test_arr_2, test_arr_2_t,
        __idll_link_type_uint_16__, next);

    /* define and init an external list header */
    idll_uint16_head_t head16_idx;

    /* switch the object to use the external header */
    __idll_obj_switch_header_ref(idll_obj, head16_idx);

    /* test data as the example in the API  header file */
    int test_values [] = {23, 4,  55, 90, 54, 77, 0,  44, 64, 88};
    #define __test_values_len    sizeof(test_values)/sizeof(test_values[0])

    /* defining the test case data structure */
    struct {
        enum { 
            op_init, op_shift, op_unshift, op_push, op_pop, op_del, op_insert
        } opr;
        uint16_t arg_0, arg_1;
        uint16_t expected_head_idx;
        uint16_t expected_len;
        uint8_t  expected[__test_values_len];
        uint8_t  filler[__test_values_len];
    } test_vector[] = {
        {op_init,   0, 0,0xFFFF,  0, {0}, {0}},
        {op_shift,  0, 0, 0,    1, {23}, {0}},
        {op_shift,  3, 0, 3,    2, {90, 23}, {0}},
        {op_shift,  9, 0, 9,    3, {88, 90, 23}, {0}},
        {op_push,   2, 0, 9,    4, {88, 90, 23, 55}, {0}},
        {op_del,    0, 0, 9,    3, {88, 90, 55}, {0}},
        {op_insert, 5, 9, 5,    4, {77, 88, 90, 55}, {0}},
        {op_insert, 1, 2, 5,    5, {77, 88, 90,  4, 55}, {0}},
        {op_del,    5, 0, 9,    4, {88, 90,  4, 55}, {0}},
        {op_unshift,0, 0, 3,    3, {90,  4, 55}, {0}},
        {op_unshift,0, 0, 1,    2, { 4, 55}, {0}},
        {op_pop,    0, 0, 1,    1, { 4}, {0}},
        {op_pop,    0, 0,0xFFFF,  0, {0}, {0}}
    };

    /* fill in the test values */
    int i;
    for(i=0; i<__test_values_len; ++i)
    {
        test_arr_2[i].data = test_values[i];
    }

    /* run all test vector */
    test_arr_2_t * p_node;
    for(i = 0; i< sizeof(test_vector)/sizeof(test_vector[0]); ++i)
    {
        p_node = & test_arr_2[test_vector[i].arg_0];
        switch (test_vector[i].opr)
        {
        case op_init:
            __log_output("%-8s", "init");
            idll_init( &idll_obj );
            break;
        case op_shift:
            __log_output("%-8s", "shift");
            idll_shift( &idll_obj,  p_node );
            break;
        case op_unshift:
            __log_output("%-8s", "unshift");
            idll_unshift( &idll_obj );
            break;
        case op_push:
            __log_output("%-8s", "push");
            idll_push( &idll_obj,  p_node );
            break;
        case op_pop:
            __log_output("%-8s", "pop");
            idll_pop( &idll_obj );
            break;
        case op_del:
            __log_output("%-8s", "del");
            idll_del( &idll_obj,  p_node );
            break;
        case op_insert:
            __log_output("%-8s", "insert");
            idll_insert( &idll_obj,  p_node, &test_arr_2[test_vector[i].arg_1]);
            break;
        }

        __log_output("[ ");
        int counter = 0;
        idll_foreach(&idll_obj, p_node) {
            if(counter)
                __log_output(", ");
            __log_output("%2d", p_node->data);
            test_vector[i].filler[counter++] = p_node->data;
        }
        __log_output(" ]");

        if(test_vector[i].expected_head_idx == head16_idx
            && test_vector[i].expected_len == counter
            && memcmp(test_vector[i].expected,
                      test_vector[i].filler, counter) == 0)
        {
            __log_output(" == PASS ==\n");
        }
        else
        {
            __log_output(" == FAIL ==\n");
        }
    }
    #endif /* CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE */
}

static void run_test_3(void)
{
    int i;

    // -- define idll node structure
    typedef struct {
        int data;
        __idll_node_links_members(__idll_link_type_uint_8__);
    } node_t;

    // -- define array of nodes
    #define __data_array_size  20
    node_t data_array[__data_array_size];

    // -- init the array data
    for(i = 0; i < __data_array_size; ++i) {
        data_array[i].data = i;
    }

    // -- define idll object and init it
    __idll_def_obj(idll_obj, data_array, node_t,
        __idll_link_type_uint_8__, next);
    idll_init( & idll_obj );

    // -- define external list headers
    idll_uint8_head_t list_even;    // will collect nodes of even data
    idll_uint8_head_t list_odd;     // will collect nodes of odd data
    idll_init_head( &list_even, __idll_link_type_uint_8__ );
    idll_init_head( &list_odd,  __idll_link_type_uint_8__ );

    // -- loop and fill nodes in the corresponding list
    for(i = 0; i < __data_array_size; ++i) {
        if(i % 2 == 0) {
            __idll_obj_switch_header_ref(idll_obj, list_even);
        } else {
            __idll_obj_switch_header_ref(idll_obj, list_odd);
        }

        idll_push( &idll_obj, & data_array[i] );
    }

    // -- display the contents of the two lists
    node_t * p_node;
    int counter;

    __log_output(" even data list -> [ ");
    counter = 0;
    __idll_obj_switch_header_ref(idll_obj, list_even);
    idll_foreach( &idll_obj, p_node ) {
        if(counter++)
            __log_output(", ");
        __log_output("%2d", p_node->data);
    }
    __log_output(" ]\n");

    __log_output(" odd  data list -> [ ");
    counter = 0;
    __idll_obj_switch_header_ref(idll_obj, list_odd);
    idll_foreach( &idll_obj, p_node ) {
        if(counter++)
            __log_output(", ");
        __log_output("%2d", p_node->data);
    }
    __log_output(" ]\n");
}

/* --- end of file ---------------------------------------------------------- */
#endif /* CONFIG_SDK_ADT_IDLL_DEMO_TEST_ENABLE */
