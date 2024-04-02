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
 * @brief   This file introduces common C abstract data type called indexed
 *          double linked list (IDLL).
 * --------------------------------------------------------------------------- *
 */

/* -- includes -------------------------------------------------------------- */
#include <stdint.h>
#include "utils_misc.h"

#include "log_lib.h"

/* -- design & description -------------------------------------------------- */

/* Generic Indexed Double Linked List:(IDLL)
 * -----------------------------------------
 * 
 *                      +----------------+   -+-               -+-
 *               base[0]|       .        |    | links           |
 *                      |       .        |    | displacement    |
 *                      |  +----------+  |   -+-                |
 *                      |  |   next   |  |    | u8/16/32        | node
 *                      |  +----------+  |   -+-                | size
 *                      |  |   prev   |  |    | u8/16/32        |
 *                      |  +----------+  |   -+-                |
 *                      |       .        |                      |
 *                      +----------------+                     -+-
 *       base[node_size]|                |
 *                      .                .
 *                      .                .
 *                      +----------------+
 * base[(N-1)*node_size]|       .        |
 *                      |       .        |      Where: N = Max Array elements
 *                      |  +----------+  |
 *                      |  |   next   |  |
 *                      |  +----------+  |
 *                      |  |   prev   |  |
 *                      |  +----------+  |
 *                      |       .        |
 *                      +----------------+
 * Operations:
 * -----------
 *  >> init, shift, unshift, push, pop, insert, del
 * 
 * Demo Example:
 * -------------
 *             0,  1,  2,  3,  4,  5,  6,  7,  8,  9
 *  arr[10] = [23, 4,  55, 90, 54, 77, 0,  44, 64, 88]
 *  head_idx
 *  
 *  operation       head_idx    dll
 *  init            <invalid>   []
 *  shift(0)        0           [23]
 *  shift(3)        3           [90, 23]
 *  shift(9)        9           [88, 90, 23]
 *  push(2)         9           [88, 90, 23, 55]
 *  del(0)          9           [88, 90, 55]
 *  insert(5,9)     5           [77, 88, 90, 55]
 *  insert(1,2)     5           [77, 88, 90, 4, 55]
 *  del(5)          9           [88, 90, 4, 55]
 *  unshift()       3           [90, 4, 55]
 *  unshift()       1           [4, 55]
 *  pop()           1           [4]
 *  pop()           <invalid>   []
 */

/* -- idll object definition ------------------------------------------------ */
typedef struct {
    const char* name;
    uint8_t     node_size;      /* size of the node */
    uint8_t     link_offset;    /* links displacement */
    enum {
        __idll_link_type_uint_8__,
        __idll_link_type_uint_16__,
        __idll_link_type_uint_32__
    }           link_type;      /* size of the individual link */
    void*       base;           /* pointer to the start of the array of nodes */
    void*       p_head;         /* if set, it points to an external header of
                                   one of the following types
                                    - idll_uint8_head_t
                                    - idll_uint16_head_t
                                    - idll_uint32_head_t
                                   if set to NULL, the 'head' member will be
                                   used as a head keeper instead */
    uint32_t    head;           /* if 'p_head == NULL', this member variable
                                   will carry the list head info */
    void*       methods;        /* for internal use only by idll module */
} idll_t;
#define __typeof__idll_link_type_uint_8__    uint8_t
#define __typeof__idll_link_type_uint_16__   uint16_t
#define __typeof__idll_link_type_uint_32__   uint32_t
#define __idll_node_links_members(link_type)    \
    __concat(__typeof, link_type) next, prev

/**
 * @brief   calculate the offset of the links member variables (next & prev)
 *          within the IDLL node structure
 * @param datatype  the node structure data type.
 * @param link_member_name the member name of the idll links in the node struct
 * @return the links offset displacement
 */
#define __idll_links_offset(datatype, link_member_name) \
    ((uint32_t)&(((datatype*)0)->link_member_name))

/**
 * @brief   define the IDLL object and fill mandatory members.
 * 
 * @param   obj         the required IDLL object name
 * @param   base_ptr    pointer to the first node of the nodes array
 * @param   node_type   the type name of the node struct
 * @param   links_type  the type of the IDLL links. it must be one of:
 *                      __idll_link_type_uint_(8|16|32)__
 * @param   link_member_name    the variable name of the links members
 */
#define __idll_def_obj(                                 \
            obj,                                        \
            base_ptr,                                   \
            node_type,                                  \
            links_type,                                 \
            link_member_name)                           \
    idll_t obj = {                                      \
        .base = base_ptr,                               \
        .link_offset = __idll_links_offset(node_type,   \
                            link_member_name),          \
        .link_type = links_type,                        \
        .node_size = sizeof(node_type)                  \
    }

/**
 * @brief   changes the list header referred by the idll_obj to an external
 *          header 'new_list_header'
 * 
 * @param   idll_obj    the object name of the IDLL
 * @param   new_list_header the external
 */
#define __idll_obj_switch_header_ref(idll_obj, new_list_header)             \
    do {                                                                    \
        __log_assert(sizeof(new_list_header) == (1 << idll_obj.link_type),  \
            "fatal: switch idll obj header to non-compliant header size");  \
        idll_obj.p_head = &(new_list_header);                               \
    } while (0)

/* -- idll external header definitions -------------------------------------- */
/**
 * The following header definitions are used when it is needed to position the
 * list header outside the idll_t object
 * example usage:
 * --------------
 *      // defining the node structure
 *      struct _node_t {
 *          uint8_t     data_1;
 *          uint16_t    next, prev;
 *          uint32_t    data_2;
 *      } array[ 10 ];
 * 
 *      // defining the IDLL object
 *      __idll_def_obj(idll_obj, array, struct _node_t, next, 16);
 * 
 *      // initializing the IDLL object
 *      idll_init( & idll_obj );
 * 
 *      // defining the external headers
 *      idll_uint16_head_t external_header_aa;
 *      idll_uint16_head_t external_header_bb;
 * 
 *      // initializing the external headers
 *      idll_init_head( & external_header_aa, __idll_link_type_uint_16__ );
 *      idll_init_head( & external_header_bb, __idll_link_type_uint_16__ );
 *      
 *      // switching the list to the external header
 *      __idll_obj_switch_header_ref( idll_obj, external_header_aa);
 *          // ... do some work on list refered by 'external_header_aa'
 *      __idll_obj_switch_header_ref( idll_obj, external_header_bb);
 *          // ... do some work on list refered by 'external_header_bb'
 */
typedef uint8_t  idll_uint8_head_t;
typedef uint16_t idll_uint16_head_t;
typedef uint32_t idll_uint32_head_t;

/**
 * @brief   Initialises the IDLL object. It encompasses the initialisation of
 *          list header defined in the same IDLL object.
 *          The external IDLL headers shall be initialized individually by the
 *          help of 'idll_init_head()' API.
 * @param p_idll    the address of the IDLL object
 * @return  none.
 */
void  idll_init     (idll_t* p_idll);

/**
 * @brief   Initialises the IDLL external header.
 * @details The external header type must be compatible of the list link type
 *          as in the following corresponding table:
 *              list-link-type              external-header-type
 *              --------------              --------------------
 *              __idll_link_type_uint_8__   idll_uint8_head_t
 *              __idll_link_type_uint_16__  idll_uint16_head_t
 *              __idll_link_type_uint_32__  idll_uint32_head_t
 * 
 * @param p_header  the address of the external IDLL header.
 * @param list_link_type a link type. it must be one of the following values:
 *                          __idll_link_type_uint_8__
 *                          __idll_link_type_uint_16__
 *                          __idll_link_type_uint_32__
 *                       o.w., assert occurs
 * @return none
 */
void  idll_init_head(void* p_header, uint32_t list_link_type);

/**
 * @brief   shifts a node into the list. shift( 1 in [0,2] ) results -> [1,0,2]
 * @param p_idll    the address of the IDLL object
 * @param p_node    pointer to the node to be shifted into the list
 * @return pointer to the shifted node
 */
void* idll_shift    (idll_t* p_idll, void* p_node);

/**
 * @brief   unshifts a node from the list. unshift( [1,0,2] ) results -> [0,2]
 * @param p_idll    the address of the IDLL object
 * @return pointer to the unshifted node, returns NULL if list is empty
 */
void* idll_unshift  (idll_t* p_idll);

/**
 * @brief   pushes a node into the list. push( 1 in [0,2] ) results -> [0,2,1]
 * @param p_idll    the address of the IDLL object
 * @param p_node    pointer to the node to be pushed into the list
 * @return pointer to the pushed node
 */
void* idll_push     (idll_t* p_idll, void* p_node);

/**
 * @brief   pushes a node into the list. pop( [0,2,1] ) results -> [0,2]
 * @param p_idll    the address of the IDLL object
 * @return pointer to the popped node, returns NULL if list is empty
 */
void* idll_pop      (idll_t* p_idll);

/**
 * @brief   inserts a node into the list. insert( 1@2 in [0,2] ) results [0,1,2]
 * @param p_idll    the address of the IDLL object
 * @param p_node    pointer to the node to be inserted into the list
 * @param p_anchor  pointer to the anchor node at which the node will be
 *                  inserted before it.
 * @return pointer to the inserted node
 */
void* idll_insert   (idll_t* p_idll, void* p_node, void* p_anchor);

/**
 * @brief   inserts a node into the list. del( 1 in [0,1,2] ) results [0,2]
 * @param p_idll    the address of the IDLL object
 * @param p_node    pointer to the node to be deleted from the list
 * @return pointer to the deleted node
 */
void* idll_del      (idll_t* p_idll, void* p_node);

/**
 * @brief   pick up an iterator for list traversinng.
 *          practically, it returns the header of the list
 * @param p_idll    the address of the IDLL object
 * @return pointer to an iterator node(the header), NULL if the list is empty
 */
void* idll_iter     (idll_t* p_idll);

/**
 * @brief   get the next node to a specific node in the list
 * @param p_idll    the address of the IDLL object
 * @return pointer to next node(the header), NULL if it is the last node
 */
void* idll_next     (idll_t* p_idll, void* iter);

/**
 * @brief   loops over all list nodes.
 * @param   p_idll pointer to the IDLL object
 * @param   iter   a pointer variable to the node type to be used as an iterator
 *                 this macro will automatically initialize and check it
 */
#define idll_foreach(p_idll, iter)          \
    for(    iter = idll_iter(p_idll);       \
            iter != NULL;                   \
            iter = idll_next(p_idll, iter) )

/**
 * @brief   searches the existence of a specific node in the IDLL.
 * @param   p_idll pointer to the IDLL object
 * @param   iter   a pointer to the node to look for.
 */
void* idll_find     (idll_t* p_idll, void* p_node);

/* -- EOF ------------------------------------------------------------------- */
