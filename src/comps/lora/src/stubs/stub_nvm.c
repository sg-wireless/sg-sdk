/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   This file defines the stub for Semtech nvm functions.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdlib.h>
#include "nvmm.h"
#include "utilities.h"

#define __log_subsystem     lora
#define __log_component     stub_nvm
#include "log_lib.h"

#include "lora_port.h"
#include "stub_nvm.h"
#include "LoRaMac.h"

/** -------------------------------------------------------------------------- *
 * nvm stub definitions
 * --------------------------------------------------------------------------- *
 */

static lora_port_nvm_check_t * p_nvm_check;
static lora_port_nvm_load_t  * p_nvm_load;
static lora_port_nvm_store_t * p_nvm_store;
static lora_port_nvm_clear_t * p_nvm_clear;
static lora_port_nvm_sync_t  * p_nvm_sync;

void lora_stub_nvm_init(void * p_init_params)
{
    __log_info("ctor() -> intialize lora nvm stub");
    lora_port_params_t * ptr = p_init_params;
    p_nvm_check = ptr->nvm_check;
    p_nvm_load  = ptr->nvm_load;
    p_nvm_store = ptr->nvm_store;
    p_nvm_clear = ptr->nvm_clear;
    p_nvm_sync  = ptr->nvm_sync;
}

bool lora_stub_nvm_check(const char* key)
{
    if(p_nvm_check)
        return p_nvm_check(key);
    else
        __log_warn("port "__red__"nvm_check"__default__" disconnected");
    return false;
}

void lora_stub_nvm_load(const char* key, void* buf, uint32_t length)
{
    if(p_nvm_load)
        p_nvm_load(key, buf, length);
    else
        __log_warn("port "__red__"nvm_load"__default__" disconnected");
}

void lora_stub_nvm_store(const char* key, void* buf, uint32_t length)
{
    if(p_nvm_store)
        p_nvm_store(key, buf, length);
    else
        __log_warn("port "__red__"nvm_store"__default__" disconnected");
}

void lora_stub_nvm_clear(const char* key)
{
    if(p_nvm_clear)
        p_nvm_clear(key);
    else
        __log_warn("port "__red__"nvm_clear"__default__" disconnected");
}

void lora_stub_nvm_sync(void)
{
    if(p_nvm_sync)
        p_nvm_sync();
    else
        __log_warn("port "__red__"nvm_sync"__default__" disconnected");
}

/* --- end of file ---------------------------------------------------------- */
