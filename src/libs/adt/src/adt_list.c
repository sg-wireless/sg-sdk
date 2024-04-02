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
 * @brief   Implements the generic simple linked list abstract data type.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <string.h>
#include "adt_list.h"

/** -------------------------------------------------------------------------- *
 * APIs implementation
 * --------------------------------------------------------------------------- *
 */
void adt_list_push(adt_list_t ** list, adt_list_t* node)
{
    adt_list_t * p_head = *list;
    adt_list_t * p_node = node;

    if( p_head == NULL ) {
        p_head = p_node;
        p_node->next = p_node->prev = p_node;
        * list = p_head;
    } else {
        adt_list_t * p_last = p_head->prev;
        p_last->next = p_node;
        p_node->next = p_head;
        p_node->prev = p_last;
        p_head->prev = p_node;
    }
}

void* adt_list_pop(adt_list_t ** list)
{
    adt_list_t * p_head = *list;

    if( p_head == NULL )
        return NULL;

    if( p_head->next == p_head ) {
        *list = NULL;
        return p_head;
    } else {
        adt_list_t * p_node = p_head->prev;
        p_head->prev = p_node->prev;
        p_node->prev->next = p_head;
        return p_node;
    }
}

void adt_list_shift(adt_list_t ** list, adt_list_t* node)
{
    adt_list_t * p_head = *list;
    adt_list_t * p_node = node;

    if( p_head == NULL ) {
        p_head = p_node;
        p_node->next = p_node->prev = p_node;
        * list = p_head;
    } else {
        adt_list_t * p_last = p_head->prev;
        p_last->next = p_node;
        p_node->next = p_head;
        p_node->prev = p_last;
        p_head->prev = p_node;

        *list = p_node;
    }
}

void* adt_list_unshift(adt_list_t ** list)
{
    adt_list_t * p_head = *list;

    if( p_head == NULL )
        return NULL;

    if( p_head->next == p_head ) {
        *list = NULL;
        return p_head;
    } else {
        adt_list_t * p_node = p_head;
        p_head = p_head->next;
        p_head->prev = p_node->prev;
        p_node->prev->next = p_head;
        *list = p_head;
        return p_node;
    }
}

void adt_list_del(adt_list_t ** list, adt_list_t* node)
{
    adt_list_t * p_head = *list;

    if(p_head == NULL)
        return;

    if( node == p_head ) {
        if( p_head->next == p_head ) {
            * list = NULL;
            return;
        } else {
            * list = p_head->next;
        }
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/* --- end of file ---------------------------------------------------------- */
