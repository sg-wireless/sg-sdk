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
 * @brief   This file implemented the hook over function 'machine_hw_i2c_init'
 *          in a way where the used i2c pins for io-exp should be utilized 
 *          carefully through the micropython interface to preserve the board
 *          ioexp functionalities.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#define __log_subsystem     F1
#define __log_component     ioexp
#include "log_lib.h"
#include "ioexp.h"

/* --- hook implementation -------------------------------------------------- */

void hook_mpy_machine_hw_i2c_init(int port, int scl, int sda,
    uint32_t freq, uint32_t timeout_ms,
    bool *p_i2c_initialized, bool *p_i2c_config_error)
{
    __log_info("calling i2c init hook");

    *p_i2c_initialized = false;
    *p_i2c_config_error = false;

    if(port == 0)
    {
        if(ioexp_micropython_req_i2c_init(port, scl, sda, freq, timeout_ms))
        {
            *p_i2c_initialized = true;
        }
        else
        {
            *p_i2c_config_error = true;
        }        
    }
}

/* --- end of file ---------------------------------------------------------- */
