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
 * @brief   interfacing component to the semtech fragmentation protocol
 *          component.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

#define __log_subsystem     lora_pkg
#define __log_component     wan_frag
#include "log_lib.h"

#include "LoRaMac.h"
#include "LmHandler.h"
#include "LmHandlerTypes.h"
#include "LmhPackage.h"
#include "LmhpFragmentation.h"


/** -------------------------------------------------------------------------- *
 * Implementation
 * --------------------------------------------------------------------------- *
 */

static int8_t cb_FragDecoderWrite(uint32_t addr, uint8_t *data, uint32_t size);
static int8_t cb_FragDecoderRead(uint32_t addr, uint8_t *data, uint32_t size);
static void cb_OnProgress(
    uint16_t fragCounter, uint16_t fragNb,
    uint8_t fragSize, uint16_t fragNbLost );
static void cb_OnDone( int32_t status, uint32_t size );

static LmhpFragmentationParams_t s_lmh_fragmentation_params =
{
    .DecoderCallbacks = {
        .FragDecoderWrite = cb_FragDecoderWrite,
        .FragDecoderRead  = cb_FragDecoderRead
    },
    .OnProgress = cb_OnProgress,
    .OnDone = cb_OnDone
};

#define __log_callback(__cb_str) \
    __log_info(__yellow__"fragment callback"__default__" --> " \
                __cyan__ __cb_str);

static int8_t cb_FragDecoderWrite(uint32_t addr, uint8_t *data, uint32_t size)
{
    // __log_callback("frag decoder write");

    __log_dump(data, size, 16, __log_dump_flag_hide_address, __word_len_8);

    return 0;
}
static int8_t cb_FragDecoderRead(uint32_t addr, uint8_t *data, uint32_t size)
{
    __log_callback("frag decoder read");

    return 0;
}
static void cb_OnProgress(
    uint16_t fragCounter, uint16_t fragNb,
    uint8_t fragSize, uint16_t fragNbLost )
{
    __log_callback("frag on progress");
}
static void cb_OnDone( int32_t status, uint32_t size )
{
    __log_callback("frag on done");
}

void lora_proto_fragmentation_init(void)
{
    LmHandlerPackageRegister( PACKAGE_ID_FRAGMENTATION,
        &s_lmh_fragmentation_params );
}

/* --- end of file ---------------------------------------------------------- */
