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
 * @brief   Defines an extended Radio functionality required specifically
 *          by the lora-stack.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

#define __log_subsystem     lora
#define __log_component     radio_ext
#include "log_lib.h"
#include "radio_ext.h"
#include "sx126x_defs.h"

#include "sx126x-board.h"


/** -------------------------------------------------------------------------- *
 * API implementation
 * --------------------------------------------------------------------------- *
 */
uint32_t lora_radio_ext_get_irqs( void )
{
    __log_debug("read irq status");
    uint16_t irqs = SX126xGetIrqStatus( );

    return ( irqs & __sx126x_irq_all_lora );
}

int8_t lora_radio_ext_get_max_tx_power( void )
{
    return 22;
}

int8_t lora_radio_ext_get_min_tx_power( void )
{
    return -9;
}

const char* lora_radio_ext_get_chip_name( void )
{
    return "SX1262";
}

/* --- end of file ---------------------------------------------------------- */
