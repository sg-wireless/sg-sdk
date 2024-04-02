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
 * @brief   This is an interface to the generic linked list abstract data type.
 * --------------------------------------------------------------------------- *
 */
#ifndef __ADT_LIST_H__
#define __ADT_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */
/**
 * a linked list header structure that should be placed at the top of any node
 * structure.
 */
typedef struct _adt_list_s {
    struct _adt_list_s * next;
    struct _adt_list_s * prev;
} adt_list_t;

/** -------------------------------------------------------------------------- *
 * Macros APIs
 * --------------------------------------------------------------------------- *
 */
/**
 * A macro to perform a linked list push action. The push action puts the new
 * node at the end of the list. It do the required typecasting before calling
 * the underlying function.
 * Example to show how push action works:
 *      L = [3, 4, 5]
 *      push(L, 7) => L = [3, 4, 5, 7]
 */
#define __adt_list_push(_list, _node)   \
    adt_list_push( (adt_list_t**)&(_list), (void*)(_node))

/**
 * A macro to perform a linked pop push action. The pop action remove the last
 * element of the list and returns it. It do the required typecasting before
 * calling the underlying function.
 * Example to show how pop action works:
 *      L = [3, 4, 5]
 *      x = pop(L) => L = [3, 4], x = 5
 */
#define __adt_list_pop(_list)           \
    adt_list_pop( (adt_list_t**)&(_list))

/**
 * A macro to perform a linked shift action. The shift action inserts the new
 * element at the beginning of the list. It do the required typecasting before
 * calling the underlying function.
 * Example to show how shift action works:
 *      L = [3, 4, 5]
 *      shift(L, 9) => L = [9, 3, 4, 5]
 */
#define __adt_list_shift(_list, _node)  \
    adt_list_shift( (adt_list_t**)&(_list), (void*)(_node))

/**
 * A macro to perform a linked unshift action. The unshift action removes the
 * first element of the list and returns it. It do the required typecasting
 * before calling the underlying function.
 * Example to show how shift action works:
 *      L = [3, 4, 5]
 *      x = unshift(L) => L = [4, 5] , x = 3
 */
#define __adt_list_unshift(_list)       \
    adt_list_unshift( (adt_list_t**)&(_list))

/**
 * A macro to perform a linked delete action. It deletes any given node
 * regardless of its place in the linked list. It do the required typecasting
 * before calling the underlying function.
 * Example to show how delete action works:
 *      L = [3, 4, 5]
 *      del(L, 4) => L = [3, 5]
 *      del(L, 3) => L = [5]
 *      del(L, 5) => L = []
 */
#define __adt_list_del(_list, _node)     \
    adt_list_del( (adt_list_t**)&(_list), (void*)(_node))

/**
 * A macro to perform a iterate for each element in the linked list.
 * A Demo Example:
 *      // define the node struct
 *      typedef {
 *          adt_list_t  list;   // the linked list must be the first member
 *          int         data;
 *      } node_t;
 * 
 *      // nodes objects definitions, by static or dynamic allocation
 *      // here in this example, we are using static allocated nodes
 *      node_t  nodes[] = {
 *          {.data = 1}, {.data = 2}, {.data = 3}, {.data = 4}, {.data = 5}
 *      };
 * 
 *      // define the linked list header, it is simply a pointer to the
 *      // node struct
 *      node_t * list;
 * 
 *      // insert all elements to the list
 *      for(int i = 0; i < sizeof(nodes)/sizeof(nodes[0]); ++i)
 *      {
 *          __adt_list_push(list, &nodes[i]);
 *      }
 * 
 *      // to loop for each element, define an iterator pointer
 *      node_t * it;
 *      __adt_list_foreach(list, it)
 *      {
 *          __log_output("node data: %d\n", it->data);
 *      }
 */
#define __adt_list_foreach(_list, _it)                          \
    bool __first = true;                                        \
    for(_it = (void*)(_list);                                   \
        _it && (__first || (uint32_t)_it != (uint32_t)(_list)); \
        _it = (void*)((adt_list_t*)_it)->next, __first = false )

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
/**
 * @brief   inserts a new node to the end of the linked list.
 * 
 * @param   list a linked list header address
 * @param   node a linked list new node pointer
 */
void adt_list_push(adt_list_t ** list, adt_list_t* node);

/**
 * @brief   removes a node from the end of the linked list and returns it.
 *          it is the reverse action of the push action
 * 
 * @param   list a linked list header address
 * 
 * @returns a pointer to the removed last node of the linked list
 *          NULL if the list is empty
 */
void* adt_list_pop(adt_list_t ** list);

/**
 * @brief   inserts a new node to the beginning of the linked list.
 * 
 * @param   list a linked list header address
 * @param   node a linked list new node pointer
 */
void adt_list_shift(adt_list_t ** list, adt_list_t* node);

/**
 * @brief   removes a node from the beginning of the linked list and returns it.
 *          it is the reverse action of the shift action
 * 
 * @param   list a linked list header address
 * 
 * @returns a pointer to the removed first node of the linked list
 *          NULL if the list is empty
 */
void* adt_list_unshift(adt_list_t ** list);

/**
 * @brief   removes a node from anywhere in the linked list
 * 
 * @param   list a linked list header address
 * @param   node a linked list node to be deleted
 */
void adt_list_del(adt_list_t ** list, adt_list_t* node);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __ADT_LIST_H__ */
