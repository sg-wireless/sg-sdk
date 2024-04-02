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

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include "utils_misc.h"

#define __log_subsystem     libs
#define __log_component     idll
#include "log_lib.h"
__log_component_def(libs, idll, cyan, 1, 0)

#include "idll.h"

/** -------------------------------------------------------------------------- *
 * macros
 * --------------------------------------------------------------------------- *
 */
#define __idll8_next(B, N, S, D) ((uint8_t*)((uint32_t)(B) + ((N)*(S)) + (D)))
#define __idll8_prev(B, N, S, D) (__idll8_next(B, N, S, D) + 1)
#define __idll8_invalid_link  ((uint8_t)(-1))

#define __idll16_next(B, N, S, D) ((uint16_t*)((uint32_t)(B) + ((N)*(S)) + (D)))
#define __idll16_prev(B, N, S, D) (__idll16_next(B, N, S, D) + 1)
#define __idll16_invalid_link  ((uint16_t)(-1))

#define __idll32_next(B, N, S, D) ((uint32_t*)((uint32_t)(B) + ((N)*(S)) + (D)))
#define __idll32_prev(B, N, S, D) (__idll32_next(B, N, S, D) + 1)
#define __idll32_invalid_link  ((uint32_t)(-1))

/** -------------------------------------------------------------------------- *
 * IDLL methods definition
 * --------------------------------------------------------------------------- *
 */
#if 0
static void* idll8_shift(idll_t* p_idll, void* p_node)
{
    uint8_t*p_head_idx = (uint8_t*) p_idll->p_head;
    uint8_t node_size  = p_idll->node_size;
    void*   base       = p_idll->base;
    uint8_t node_idx   = ((uint32_t)p_node - (uint32_t)base) / node_size;
    uint8_t links_offset = p_idll->link_offset;

    uint8_t* node_n;
    uint8_t* node_p;
    node_n = __idll8_next(base, node_idx, node_size, links_offset);
    node_p = node_n + 1;
    uint8_t head_idx = * p_head_idx;

    if(head_idx == __idll8_invalid_link)
    {
        * node_n = node_idx;
        * node_p = node_idx;
    }
    else
    {
        uint8_t* head_p;
        uint8_t  h_p_node_idx;
        uint8_t* prev_n;

        head_p = __idll8_prev(base, head_idx, node_size, links_offset);

        h_p_node_idx = * head_p;

        prev_n = __idll8_next(base, h_p_node_idx, node_size, links_offset);

        * node_n = head_idx;
        * node_p = h_p_node_idx;

        * head_p = node_idx;
        * prev_n = node_idx;
    }

    * p_head_idx = node_idx;

    return (uint8_t*)base + (node_idx * node_size);
}
#endif
#define __idll_shift_template(__s)                                          \
    static void* idll##__s##_shift( idll_t* p_idll, void* p_node )          \
    {                                                                       \
        uint##__s##_t*p_head_idx = (uint##__s##_t*) p_idll->p_head;         \
        uint8_t node_size = p_idll->node_size;                              \
        void*   base = p_idll->base;                                        \
        uint##__s##_t node_idx =                                            \
                        ((uint32_t)p_node - (uint32_t)base) / node_size;    \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        uint##__s##_t *node_n, *node_p;                                     \
        node_n = __idll##__s##_next(base, node_idx, node_size,links_offset);\
        node_p = node_n + 1;                                                \
        uint##__s##_t head_idx = * p_head_idx;                              \
        if(head_idx == __idll##__s##_invalid_link)                          \
        {                                                                   \
            * node_n = node_idx;                                            \
            * node_p = node_idx;                                            \
        }                                                                   \
        else                                                                \
        {                                                                   \
            uint##__s##_t* head_p;                                          \
            uint##__s##_t  h_p_node_idx;                                    \
            uint##__s##_t* prev_n;                                          \
            head_p = __idll##__s##_prev(base, head_idx, node_size,          \
                links_offset);                                              \
            h_p_node_idx = * head_p;                                        \
            prev_n = __idll##__s##_next(base, h_p_node_idx, node_size,      \
                links_offset);                                              \
            * node_n = head_idx;                                            \
            * node_p = h_p_node_idx;                                        \
            * head_p = node_idx;                                            \
            * prev_n = node_idx;                                            \
        }                                                                   \
        * p_head_idx = node_idx;                                            \
        return (uint8_t*)base + (node_idx * node_size);                     \
    }

#if 0
static void* idll8_unshift( idll_t* p_idll )
{
    uint8_t*p_head_idx = (uint8_t*) p_idll->p_head;
    uint8_t node_size  = p_idll->node_size;
    void*   base       = p_idll->base;
    uint8_t links_offset = p_idll->link_offset;

    uint8_t head_idx = * p_head_idx;

    if(head_idx == __idll8_invalid_link)
    {
        /* list is empty */
        return NULL;
    }
    uint8_t* head_n = __idll8_next(base, head_idx, node_size, links_offset);
    if( * head_n == head_idx )
    {
        /* list has only one node */
        * p_head_idx = __idll8_invalid_link;
    }
    else
    {
        /* list has more than one node */
        * p_head_idx = * head_n;
        uint8_t* head_p = head_n + 1;
        uint8_t  h_n_node_idx = * head_n;
        uint8_t  h_p_node_idx = * head_p;
        uint8_t* next_p = __idll8_prev(base, h_n_node_idx, node_size,
            links_offset);
        uint8_t* prev_n = __idll8_next(base, h_p_node_idx, node_size,
            links_offset);
        * prev_n = h_n_node_idx;
        * next_p = h_p_node_idx;
    }
    return (uint8_t*)base + (head_idx * node_size);
}
#endif
#define __idll_unshift_template(__s)                                        \
    static void* idll##__s##_unshift( idll_t* p_idll )                      \
    {                                                                       \
        uint##__s##_t*p_head_idx = (uint##__s##_t*) p_idll->p_head;         \
        uint8_t node_size = p_idll->node_size;                              \
        void*   base = p_idll->base;                                        \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        uint##__s##_t head_idx = * p_head_idx;                              \
        if(head_idx == __idll##__s##_invalid_link)                          \
        {                                                                   \
            /* list is empty */                                             \
            return NULL;                                                    \
        }                                                                   \
        uint##__s##_t* head_n = __idll##__s##_next(                         \
            base, head_idx, node_size, links_offset);                       \
        if( * head_n == head_idx )                                          \
        {                                                                   \
            /* list has only one node */                                    \
            * p_head_idx = __idll##__s##_invalid_link;                      \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* list has more than one node */                               \
            * p_head_idx = * head_n;                                        \
            uint##__s##_t* head_p = head_n + 1;                             \
            uint##__s##_t  h_n_node_idx = * head_n;                         \
            uint##__s##_t  h_p_node_idx = * head_p;                         \
            uint##__s##_t* next_p = __idll##__s##_prev(                     \
                base, h_n_node_idx, node_size, links_offset);               \
            uint##__s##_t* prev_n = __idll##__s##_next(                     \
                base, h_p_node_idx, node_size, links_offset);               \
            * prev_n = h_n_node_idx;                                        \
            * next_p = h_p_node_idx;                                        \
        }                                                                   \
        return (uint8_t*)base + (head_idx * node_size);                     \
    }
#if 0
static void* idll8_push(idll_t* p_idll, void* p_node)
{
    uint8_t*p_head_idx = (uint8_t*) p_idll->p_head;
    uint8_t node_size  = p_idll->node_size;
    void*   base       = p_idll->base;
    uint8_t node_idx   = ((uint32_t)p_node - (uint32_t)base) / node_size;
    uint8_t links_offset = p_idll->link_offset;

    uint8_t* node_n = __idll8_next(base, node_idx, node_size, links_offset);
    uint8_t* node_p = node_n + 1;
    uint8_t head_idx = * p_head_idx;
    if(head_idx == __idll8_invalid_link)
    {
        * node_n = node_idx;
        * node_p = node_idx;
        * p_head_idx = node_idx;
    }
    else
    {
        uint8_t* head_p = __idll8_prev(
            base, head_idx, node_size, links_offset);
        uint8_t  h_p_node_idx = * head_p;
        uint8_t* prev_n = __idll8_next(
            base, h_p_node_idx, node_size, links_offset);
        * node_n = head_idx;
        * node_p = h_p_node_idx;
        * head_p = node_idx;
        * prev_n = node_idx;
    }
    return (uint8_t*)base + (node_idx * node_size);
}
#endif
#define __idll_push_template(__s)                                           \
    static void* idll##__s##_push( idll_t* p_idll, void* p_node )           \
    {                                                                       \
        uint##__s##_t*p_head_idx = (uint##__s##_t*) p_idll->p_head;         \
        uint8_t node_size = p_idll->node_size;                              \
        void*   base = p_idll->base;                                        \
        uint##__s##_t node_idx =                                            \
                        ((uint32_t)p_node - (uint32_t)base) / node_size;    \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        uint##__s##_t* node_n = __idll##__s##_next(                         \
                base, node_idx, node_size, links_offset);                   \
        uint##__s##_t* node_p = node_n + 1;                                 \
        uint##__s##_t head_idx = * p_head_idx;                              \
        if(head_idx == __idll##__s##_invalid_link)                          \
        {                                                                   \
            * node_n = node_idx;                                            \
            * node_p = node_idx;                                            \
            * p_head_idx = node_idx;                                        \
        }                                                                   \
        else                                                                \
        {                                                                   \
            uint##__s##_t* head_p = __idll##__s##_prev(                     \
                base, head_idx, node_size, links_offset);                   \
            uint##__s##_t  h_p_node_idx = * head_p;                         \
            uint##__s##_t* prev_n = __idll##__s##_next(                     \
                base, h_p_node_idx, node_size, links_offset);               \
            * node_n = head_idx;                                            \
            * node_p = h_p_node_idx;                                        \
            * head_p = node_idx;                                            \
            * prev_n = node_idx;                                            \
        }                                                                   \
        return (uint8_t*)base + (node_idx * node_size);                     \
    }
#if 0
static void* idll8_pop( idll_t* p_idll )
{
    uint8_t*p_head_idx = (uint8_t*) p_idll->p_head;
    uint8_t node_size  = p_idll->node_size;
    void*   base       = p_idll->base;
    uint8_t links_offset = p_idll->link_offset;

    uint8_t head_idx = * p_head_idx;
    uint8_t ret_node_idx;

    if(head_idx == __idll8_invalid_link)
    {
        /* list is empty */
        return NULL;
    }
    uint8_t* head_p = __idll8_prev(base, head_idx, node_size, links_offset);
    if( * head_p == head_idx )
    {
        /* list has only one node */
        * p_head_idx = __idll8_invalid_link;
        ret_node_idx = head_idx;
    }
    else
    {
        /* list has more than one node */
        /* p_p_node -> p_node -> head -> n_node */
        uint8_t  h_p_node_idx = * head_p;
        uint8_t* prev_p = __idll8_prev(
            base, h_p_node_idx, node_size, links_offset);
        uint8_t  h_p_p_node_idx = * prev_p;
        uint8_t* prev_prev_n = __idll8_next(
            base, h_p_p_node_idx, node_size, links_offset);
        * prev_prev_n = head_idx;
        * head_p = h_p_p_node_idx;
        ret_node_idx = h_p_node_idx;
    }
    return (uint8_t*)base + (ret_node_idx * node_size);
}
#endif
#define __idll_pop_template(__s)                                            \
    static void* idll##__s##_pop( idll_t* p_idll )                          \
    {                                                                       \
        uint##__s##_t*p_head_idx = (uint##__s##_t*) p_idll->p_head;         \
        uint8_t node_size = p_idll->node_size;                              \
        void*   base = p_idll->base;                                        \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        uint##__s##_t head_idx = * p_head_idx;                              \
        uint##__s##_t ret_node_idx;                                         \
        if(head_idx == __idll##__s##_invalid_link)                          \
        {                                                                   \
            /* list is empty */                                             \
            return NULL;                                                    \
        }                                                                   \
        uint##__s##_t* head_p = __idll##__s##_prev(                         \
            base, head_idx, node_size, links_offset);                       \
        if( * head_p == head_idx )                                          \
        {                                                                   \
            /* list has only one node */                                    \
            * p_head_idx = __idll##__s##_invalid_link;                      \
            ret_node_idx = head_idx;                                        \
        }                                                                   \
        else                                                                \
        {                                                                   \
            /* list has more than one node */                               \
            /* p_p_node -> p_node -> head -> n_node */                      \
            uint##__s##_t  h_p_node_idx = * head_p;                         \
            uint##__s##_t* prev_p = __idll##__s##_prev(                     \
                base, h_p_node_idx, node_size, links_offset);               \
            uint##__s##_t  h_p_p_node_idx = * prev_p;                       \
            uint##__s##_t* prev_prev_n = __idll##__s##_next(                \
                base, h_p_p_node_idx, node_size, links_offset);             \
            * prev_prev_n = head_idx;                                       \
            * head_p = h_p_p_node_idx;                                      \
            ret_node_idx = h_p_node_idx;                                    \
        }                                                                   \
        return (uint8_t*)base + (ret_node_idx * node_size);                 \
    }

#if 0
static void* idll8_insert(idll_t* p_idll, void* p_node, void* p_anchor)
{
    uint8_t*p_head_idx = (uint8_t*) p_idll->p_head;
    uint8_t node_size  = p_idll->node_size;
    void*   base       = p_idll->base;
    uint8_t new_node_idx = ((uint32_t)p_node - (uint32_t)base) / node_size;
    uint8_t anchor_idx = ((uint32_t)p_anchor - (uint32_t)base) / node_size;
    uint8_t links_offset = p_idll->link_offset;

    uint8_t head_idx = * p_head_idx;
    uint8_t* new_node_n = __idll8_next(
        base, new_node_idx, node_size, links_offset);
    uint8_t* new_node_p = new_node_n + 1;
    if( head_idx == anchor_idx )
    {
        * p_head_idx = new_node_idx;
    }
    /* prev_anchor -> new_node -> anchor */
    uint8_t* anchor_p = __idll8_prev(
        base, anchor_idx, node_size, links_offset);
    uint8_t  anchor_prev_idx = * anchor_p;
    uint8_t* anchor_prev_n = __idll8_next(
        base, anchor_prev_idx, node_size, links_offset);
    * anchor_prev_n = new_node_idx;
    * anchor_p = new_node_idx;
    * new_node_n = anchor_idx;
    * new_node_p = anchor_prev_idx;
    return (uint8_t*)base + (new_node_idx * node_size);
}
#endif
#define __idll_insert_template(__s)                                         \
    static void* idll##__s##_insert(                                        \
                idll_t* p_idll, void* p_node, void* p_anchor)               \
    {                                                                       \
        uint##__s##_t*p_head_idx = (uint##__s##_t*) p_idll->p_head;         \
        uint8_t node_size  = p_idll->node_size;                             \
        void*   base       = p_idll->base;                                  \
        uint##__s##_t new_node_idx =                                        \
                    ((uint32_t)p_node - (uint32_t)base) / node_size;        \
        uint##__s##_t anchor_idx =                                          \
                    ((uint32_t)p_anchor - (uint32_t)base) / node_size;      \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        uint##__s##_t head_idx = * p_head_idx;                              \
        uint##__s##_t* new_node_n = __idll##__s##_next(                     \
            base, new_node_idx, node_size, links_offset);                   \
        uint##__s##_t* new_node_p = new_node_n + 1;                         \
        if( head_idx == anchor_idx )                                        \
        {                                                                   \
            * p_head_idx = new_node_idx;                                    \
        }                                                                   \
        /* prev_anchor -> new_node -> anchor */                             \
        uint##__s##_t* anchor_p = __idll##__s##_prev(                       \
            base, anchor_idx, node_size, links_offset);                     \
        uint##__s##_t  anchor_prev_idx = * anchor_p;                        \
        uint##__s##_t* anchor_prev_n = __idll##__s##_next(                  \
            base, anchor_prev_idx, node_size, links_offset);                \
        * anchor_prev_n = new_node_idx;                                     \
        * anchor_p = new_node_idx;                                          \
        * new_node_n = anchor_idx;                                          \
        * new_node_p = anchor_prev_idx;                                     \
        return (uint8_t*)base + (new_node_idx * node_size);                 \
    }
#if 0
static void* idll8_del(idll_t* p_idll, void* p_del_node)
{
    uint8_t*p_head_idx = (uint8_t*) p_idll->p_head;
    uint8_t node_size  = p_idll->node_size;
    void*   base       = p_idll->base;
    uint8_t del_node_idx = ((uint32_t)p_del_node - (uint32_t)base) / node_size;
    uint8_t links_offset = p_idll->link_offset;

    uint8_t head_idx = * p_head_idx;
    uint8_t* del_node_n = __idll8_next(
        base, del_node_idx, node_size, links_offset);
    uint8_t* del_node_p = del_node_n + 1;
    uint8_t  del_prev_idx = * del_node_p;
    uint8_t  del_next_idx = * del_node_n;
    if( head_idx == del_node_idx )
    {
        /* del the head node */
        if(head_idx == del_next_idx)
        {
            /* only one node, hence reset the head */
            * p_head_idx = __idll8_invalid_link;
            goto exit;
        }
        else
        {
            /* transfer the head to the next of del_node */
            * p_head_idx = del_next_idx;
        }
    }

    /* del_prev -> del_node -> del_next */
    uint8_t* del_prev_n = __idll8_next(
        base, del_prev_idx, node_size, links_offset);
    uint8_t* del_next_p = __idll8_prev(
        base, del_next_idx, node_size, links_offset);

    * del_prev_n = del_next_idx;
    * del_next_p = del_prev_idx;

    exit:
    return (uint8_t*)base + (del_node_idx * node_size);
}
#endif
#define __idll_del_template(__s)                                            \
    static void* idll##__s##_del(idll_t* p_idll, void* p_del_node)          \
    {                                                                       \
        uint##__s##_t*p_head_idx = (uint##__s##_t*) p_idll->p_head;         \
        uint8_t node_size  = p_idll->node_size;                             \
        void*   base       = p_idll->base;                                  \
        uint##__s##_t del_node_idx =                                        \
                    ((uint32_t)p_del_node - (uint32_t)base) / node_size;    \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        uint##__s##_t head_idx = * p_head_idx;                              \
        uint##__s##_t* del_node_n = __idll##__s##_next(                     \
            base, del_node_idx, node_size, links_offset);                   \
        uint##__s##_t* del_node_p = del_node_n + 1;                         \
        uint##__s##_t  del_prev_idx = * del_node_p;                         \
        uint##__s##_t  del_next_idx = * del_node_n;                         \
        if( head_idx == del_node_idx )                                      \
        {                                                                   \
            /* del the head node */                                         \
            if(head_idx == del_next_idx)                                    \
            {                                                               \
                /* only one node, hence reset the head */                   \
                * p_head_idx = __idll##__s##_invalid_link;                  \
                goto exit;                                                  \
            }                                                               \
            else                                                            \
            {                                                               \
                /* transfer the head to the next of del_node */             \
                * p_head_idx = del_next_idx;                                \
            }                                                               \
        }                                                                   \
        /* del_prev -> del_node -> del_next */                              \
        uint##__s##_t* del_prev_n = __idll##__s##_next(                     \
            base, del_prev_idx, node_size, links_offset);                   \
        uint##__s##_t* del_next_p = __idll##__s##_prev(                     \
            base, del_next_idx, node_size, links_offset);                   \
        * del_prev_n = del_next_idx;                                        \
        * del_next_p = del_prev_idx;                                        \
        exit:                                                               \
        return (uint8_t*)base + (del_node_idx * node_size);                 \
    }

#define __idll_iter_template(__s)                                           \
    static void* idll##__s##_iter (idll_t* p_idll)                          \
    {                                                                       \
        uint8_t node_size  = p_idll->node_size;                             \
        void*   base       = p_idll->base;                                  \
        uint##__s##_t head_idx = *((uint##__s##_t*) p_idll->p_head);        \
                                                                            \
        if(head_idx != __idll##__s##_invalid_link)                          \
        {                                                                   \
            return (uint8_t*)base + (head_idx * node_size);                 \
        }                                                                   \
        return NULL;                                                        \
    }

#define __idll_next_template(__s)                                           \
    static void* idll##__s##_next(idll_t* p_idll, void* iter)               \
    {                                                                       \
        uint##__s##_t head_idx = *((uint##__s##_t*) p_idll->p_head);        \
        uint8_t node_size  = p_idll->node_size;                             \
        void*   base       = p_idll->base;                                  \
        uint8_t links_offset = p_idll->link_offset;                         \
                                                                            \
        void* ret = NULL;                                                   \
        if(iter != NULL)                                                    \
        {                                                                   \
            uint##__s##_t* iter_n = (uint##__s##_t*)                        \
                ((uint8_t*)iter + links_offset);                            \
            uint##__s##_t  iter_next_idx = *iter_n;                         \
            if(iter_next_idx != head_idx)                                   \
            {                                                               \
                ret = (uint8_t*)base + (iter_next_idx * node_size);         \
            }                                                               \
        }                                                                   \
        return ret;                                                         \
    }

/** -------------------------------------------------------------------------- *
 * IDLL main methods container structs
 * Each idll size has a container struct object for their methods
 * --------------------------------------------------------------------------- *
 */
typedef struct {
    void* (* shift   )(idll_t* p_idll, void* p_node);
    void* (* unshift )(idll_t* p_idll);

    void* (* push    )(idll_t* p_idll, void* p_node);
    void* (* pop     )(idll_t* p_idll);

    void* (* insert  )(idll_t* p_idll, void* p_node, void* p_anchor);
    void* (* del     )(idll_t* p_idll, void* p_node);

    void* (* iter    )(idll_t* p_idll);
    void* (* next    )(idll_t* p_idll, void* iter);
} idll_methods_t;

#define __idll_methods(__s__)                                   \
                                                                \
    __idll_shift_template(__s__)                                \
    __idll_unshift_template(__s__)                              \
    __idll_push_template(__s__)                                 \
    __idll_pop_template(__s__)                                  \
    __idll_insert_template(__s__)                               \
    __idll_del_template(__s__)                                  \
    __idll_iter_template(__s__)                                 \
    __idll_next_template(__s__)                                 \
                                                                \
    static idll_methods_t s_idll##__s__##_methods = {           \
        .shift      = idll##__s__##_shift,                      \
        .unshift    = idll##__s__##_unshift,                    \
        .push       = idll##__s__##_push,                       \
        .pop        = idll##__s__##_pop,                        \
        .insert     = idll##__s__##_insert,                     \
        .del        = idll##__s__##_del,                        \
        .iter       = idll##__s__##_iter,                       \
        .next       = idll##__s__##_next                        \
    };

#ifdef CONFIG_SDK_ADT_IDLL_8_BITS_IMPLEMENTATION_ENABLE
__idll_methods(8)
#endif

#ifdef CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE
__idll_methods(16)
#endif

#ifdef CONFIG_SDK_ADT_IDLL_32_BITS_IMPLEMENTATION_ENABLE
__idll_methods(32)
#endif

/** -------------------------------------------------------------------------- *
 * IDLL API implementation
 * --------------------------------------------------------------------------- *
 */
#if !defined(CONFIG_SDK_ADT_IDLL_8_BITS_IMPLEMENTATION_ENABLE)  || \
    !defined(CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE) || \
    !defined(CONFIG_SDK_ADT_IDLL_32_BITS_IMPLEMENTATION_ENABLE)
static void log_config_unsupported_error(const char* name, int links_type)
{
    int size_table [] = {
        [__idll_link_type_uint_8__] = 8,
        [__idll_link_type_uint_16__] = 16,
        [__idll_link_type_uint_32__] = 32
    };
    __log_error("idll("__cyan__"%s"__red__") "__yellow__"%d"__red__"-bit"
            " link size is disabled",
            name ? name : "unknown",
            size_table[links_type]);
}
#endif

void  idll_init     (idll_t* p_idll)
{
    if(p_idll->p_head == NULL)
    {
        p_idll->p_head = & p_idll->head;
    }
    __log_debug("init idll %d , links_offset: %d , node_size: %d",
        ( 1 << p_idll->link_type ) * 8,
        p_idll->link_offset, p_idll->node_size);

    if(p_idll->link_type == __idll_link_type_uint_8__) {
        #ifdef CONFIG_SDK_ADT_IDLL_8_BITS_IMPLEMENTATION_ENABLE
        p_idll->methods = & s_idll8_methods;
        *(uint8_t*)p_idll->p_head = __idll8_invalid_link;
        #else
        p_idll->methods = NULL;
        log_config_unsupported_error(p_idll->name, p_idll->link_type);
        #endif
        return;
    }

    if(p_idll->link_type == __idll_link_type_uint_16__) {
        #ifdef CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE
        p_idll->methods = & s_idll16_methods;
        *(uint16_t*)p_idll->p_head = __idll16_invalid_link;
        #else
        p_idll->methods = NULL;
        log_config_unsupported_error(p_idll->name, p_idll->link_type);
        #endif
        return;
    }

    if(p_idll->link_type == __idll_link_type_uint_32__) {
        #ifdef CONFIG_SDK_ADT_IDLL_32_BITS_IMPLEMENTATION_ENABLE
        p_idll->methods = & s_idll32_methods;
        *(uint32_t*)p_idll->p_head = __idll32_invalid_link;
        #else
        p_idll->methods = NULL;
        log_config_unsupported_error(p_idll->name, p_idll->link_type);
        #endif
    }
}

void idll_init_head(void* p_header, uint32_t list_link_type)
{
    if(list_link_type == __idll_link_type_uint_8__) {
        #ifdef CONFIG_SDK_ADT_IDLL_8_BITS_IMPLEMENTATION_ENABLE
        *(uint8_t*)p_header = __idll8_invalid_link;
        #else
        log_config_unsupported_error(NULL, __idll_link_type_uint_8__);
        #endif
        return;
    }

    if(list_link_type == __idll_link_type_uint_16__) {
        #ifdef CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE
        *(uint16_t*)p_header = __idll16_invalid_link;
        #else
        log_config_unsupported_error(NULL, __idll_link_type_uint_16__);
        #endif
        return;
    }

    if(list_link_type == __idll_link_type_uint_32__) {
        #ifdef CONFIG_SDK_ADT_IDLL_32_BITS_IMPLEMENTATION_ENABLE
        *(uint32_t*)p_header = __idll32_invalid_link;
        #else
        log_config_unsupported_error(NULL, __idll_link_type_uint_32__);
        #endif
    }
}

static bool idll_checker(idll_t* p_idll)
{
    if(p_idll->link_type == __idll_link_type_uint_8__) {
        #ifdef CONFIG_SDK_ADT_IDLL_8_BITS_IMPLEMENTATION_ENABLE
        return true;
        #else
        log_config_unsupported_error(p_idll->name, p_idll->link_type);
        return false;
        #endif
    }

    if(p_idll->link_type == __idll_link_type_uint_16__) {
        #ifdef CONFIG_SDK_ADT_IDLL_16_BITS_IMPLEMENTATION_ENABLE
        return true;
        #else
        log_config_unsupported_error(p_idll->name, p_idll->link_type);
        return false;
        #endif
    }

    if(p_idll->link_type == __idll_link_type_uint_32__) {
        #ifdef CONFIG_SDK_ADT_IDLL_32_BITS_IMPLEMENTATION_ENABLE
        return true;
        #else
        log_config_unsupported_error(p_idll->name, p_idll->link_type);
        return false;
        #endif
    }
    return false;
}

void* idll_shift    (idll_t* p_idll, void* p_node)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->shift(p_idll, p_node)
        : NULL;
}
void* idll_unshift  (idll_t* p_idll)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->unshift(p_idll)
        : NULL;
}

void* idll_push     (idll_t* p_idll, void* p_node)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->push(p_idll, p_node)
        : NULL;
}
void* idll_pop      (idll_t* p_idll)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->pop(p_idll)
        : NULL;
}

void* idll_insert   (idll_t* p_idll, void* p_node, void* p_anchor)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->insert(p_idll, p_node, p_anchor)
        : NULL;
}
void* idll_del      (idll_t* p_idll, void* p_node)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->del(p_idll, p_node)
        : NULL;
}

void* idll_iter     (idll_t* p_idll)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->iter(p_idll)
        : NULL;
}

void* idll_next     (idll_t* p_idll, void* iter)
{
    return idll_checker(p_idll)
        ? ((idll_methods_t*)p_idll->methods)->next(p_idll, iter)
        : NULL;
}

void* idll_find     (idll_t* p_idll, void* p_node)
{
    if(idll_checker(p_idll))
    {
        void* iter;
        idll_foreach(p_idll, iter) {
            if(p_node == iter)
                return iter;
        }
    }
    return NULL;
}

/* --- end of file ---------------------------------------------------------- */
