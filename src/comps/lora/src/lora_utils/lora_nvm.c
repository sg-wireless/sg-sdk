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
 * @brief   lora nvm handling mechanism
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * include
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

#define __log_subsystem lora
#define __log_component util_nvm
#include "log_lib.h"

#include "stub_nvm.h"
#include "stub_system.h"
#include "lora_nvm.h"

/** -------------------------------------------------------------------------- *
 * configs and helpful macros
 * --------------------------------------------------------------------------- *
 */
#define __lora_nvm_load_magic   ( 0xA5A5B5B5u )

#define __log_nvm_debug(_key, _act, _buf, _len, _cmt)   \
    __log_info(                                         \
        " key: "__yellow__"%-12s"__default__            \
        " act: "__green__"%-15s"__default__             \
        " buf: "__blue__"%p"__default__"  "             \
        " len: "__blue__"%-4d"__default__               \
        " cmt: "__cyan__"%s"__default__,                \
        _key, _act, _buf, _len, _cmt                    \
    )

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */
void lora_nvm_handle_change(
    const char* key_name,
    void*       p_mem,
    uint32_t    mem_size,
    lora_nvm_load_defaults_t* load_default_method
    )
{
    /**
     * [ procedure ]
     * -------------
     *      Do these checks:
     *          CH-A these is no previous clone for this record in the NvM
     *          CH-B if it has a wrong loading magic number
     *          CH-C if it has an incorrect crc32
     *      IF CH-A failed:
     *          [ LABEL-1 ]
     *          - WRITE the correct magic number
     *          - CALL the load_default_method() if specified
     *          - CALC the new CRC32 value and write it
     *          - STORE the record
     *      ELSE IF CH-B failed:
     *          - LOAD the clone from the NvM into the record memory
     *          - Do CH-B and CH-C again
     *          IF CH-B failed || CH-C failed:
     *              GOTO [ LABEL-1 ]
     *      ELSE IF CH-C failed:
     *          - CALC the CRC32 value and write it in the record memory
     *          - STORE a record clone into the NvM
     *      ELSE
     *          - Do nothing as no change
     *      END
     */

    __log_assert( mem_size >= sizeof(lora_nvm_record_tail_t),
        "size of the nvm record (%d) < the minimum size (%d)", mem_size,
        sizeof(lora_nvm_record_tail_t));
    lora_nvm_record_tail_t * p_tail = (void*)
        ((uint8_t*)p_mem + mem_size - sizeof(lora_nvm_record_tail_t));

    bool valid_nvm_key = false;

    uint32_t crc;
 
    valid_nvm_key = lora_stub_nvm_check(key_name);
    if( ! valid_nvm_key )
    {
        init_record_and_store:

        __log_nvm_debug(key_name, "init-record", p_mem, mem_size, "--");

        p_tail->magic = __lora_nvm_load_magic;
        if( load_default_method )
        {
            load_default_method( p_mem, mem_size );
        }
        crc = lora_stub_crc32(0, p_mem, mem_size - 4);
        p_tail->crc32 = crc;

        __log_nvm_debug(key_name, "store-clone", p_mem, mem_size, "--");

        lora_stub_nvm_store(key_name, p_mem, mem_size);
        lora_stub_nvm_sync();
        return;
    }

    bool valid_magic = (p_tail->magic == __lora_nvm_load_magic);

    if( ! valid_magic )
    {
        __log_nvm_debug(key_name, "load-clone", p_mem, mem_size, "--");

        lora_stub_nvm_load(key_name, p_mem, mem_size);
        valid_magic = (p_tail->magic == __lora_nvm_load_magic);
        if( ! valid_magic )
        {
            __log_nvm_debug(key_name, "verify-record", p_mem,
                mem_size, "loading magic number invalid "__red__"corrupted");
            goto init_record_and_store;
        }
        crc = lora_stub_crc32(0, p_mem, mem_size - 4);
        bool valid_crc32 = (crc == p_tail->crc32);
        if( ! valid_crc32 )
        {
            __log_nvm_debug(key_name, "verify-record", p_mem,
                mem_size, "loaded clone crc failed "__red__"corrupted");
            goto init_record_and_store;
        }

        __log_nvm_debug(key_name, "load-success", p_mem, mem_size, "--");
        return;
    }

    crc = lora_stub_crc32(0, p_mem, mem_size - 4);
    if( crc != p_tail->crc32 )
    {
        __log_nvm_debug(key_name, "change-detected", p_mem, mem_size, "--");

        p_tail->crc32 = crc;

        __log_nvm_debug(key_name, "store-clone", p_mem, mem_size, "--");

        lora_stub_nvm_store(key_name, p_mem, mem_size);
        lora_stub_nvm_sync();
        return;
    }

    __log_nvm_debug(key_name, "no-change", p_mem, mem_size, "--");
}

/* --- end of file ---------------------------------------------------------- */
