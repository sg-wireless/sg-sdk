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
 * @brief   This file is a stub for LmHandler Semteck application helper module.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <stdio.h>
#include "utilities.h"
#include "LoRaMac.h"
#include "NvmDataMgmt.h"

#define __log_subsystem     lora
#include "log_lib.h"
#include "lora_wan_nvm.h"

/** -------------------------------------------------------------------------- *
 * stub function definitions
 * --------------------------------------------------------------------------- *
 */
void NvmDataMgmtEvent( uint16_t notifyFlags )
{
    __log_info("lmh stub --> nvm event");

    lora_nvm_handle_data_change( notifyFlags );
}

uint16_t NvmDataMgmtStore( void )
{
    __log_info("lmh stub --> nvm store");
    return 0;
}

uint16_t NvmDataMgmtRestore( void )
{
    __log_info("lmh stub --> nvm restore");

    lora_nvm_restore();

    /**
     * always return true to not let the LmHandler component resets the Mac
     * layer parameters. And in the LmHandler OnNvmDataChange callback function
     * we will make the desired parameters provisioning to the Mac layer
     * according to the Pycom needs.
     */
    return true;
}

bool NvmDataMgmtFactoryReset( void )
{
    __log_info("lmh stub --> nvm factory reset");
    return true;
}

/* --- end of file ---------------------------------------------------------- */
