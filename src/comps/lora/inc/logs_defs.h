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
 * @brief   It defines the lora-stack logging subsystem and its components.
 *          It is used only for the log_lib generator.
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include "log_lib.h"

/* --- subsystem/components ------------------------------------------------- */

__log_component_def(drivers,    sx126x,         blue,       1, 1)

__log_subsystem_def(lora,                       default,    1, 0)
__log_subsystem_def(lora_pkg,                   cyan,       1, 0)

// -- lora_mgr  group
__log_component_def(lora,       mgr_api,        default,    1, 0)

// -- lora_port group
__log_component_def(lora,       port_spi,       yellow,     1, 0)
__log_component_def(lora,       port_sx126x,    yellow,     1, 0)
__log_component_def(lora,       port_system,    yellow,     1, 0)
__log_component_def(lora,       port_analyser,  yellow,     1, 0)

// -- lora_stub group
__log_component_def(lora,       stub_sx126x,    purple,     1, 0)
__log_component_def(lora,       stub_board,     purple,     1, 0)
__log_component_def(lora,       stub_main,      purple,     1, 0)
__log_component_def(lora,       stub_nvm,       purple,     1, 0)
__log_component_def(lora,       stub_system,    purple,     1, 0)
__log_component_def(lora,       stub_timers,    purple,     1, 0)

// -- lora_raw  group
__log_component_def(lora,       raw_api,        default,    1, 1)
__log_component_def(lora,       raw_process,    default,    1, 1)
__log_component_def(lora,       raw_sm,         default,    1, 1)
__log_component_def(lora,       raw_radio_if,   default,    1, 1)

// -- lora_wan  group
__log_component_def(lora,       wan_comision,   default,    1, 1)
__log_component_def(lora,       wan_process,    default,    1, 1)
__log_component_def(lora,       wan_lmh,        default,    1, 1)
__log_component_def(lora,       wan_mac,        default,    1, 1)
__log_component_def(lora,       wan_utils,      default,    1, 1)
__log_component_def(lora_pkg,   wan_compli,     default,    1, 0)
__log_component_def(lora_pkg,   wan_clock,      green,      1, 1)
__log_component_def(lora_pkg,   wan_frag,       purple,     1, 1)
__log_component_def(lora_pkg,   wan_mcast,      blue,       1, 1)
__log_component_def(lora_pkg,   wan_multipkg,   cyan,       1, 1)
__log_component_def(lora,       wan_nvm,        default,    1, 1)
__log_component_def(lora,       wan_api,        default,    1, 1)
__log_component_def(lora,       wan_sm,         default,    1, 1)
__log_component_def(lora,       wan_port,       default,    1, 1)

// -- lora_utils  group
__log_component_def(lora,       util_nvm,       blue,       1, 1)
__log_component_def(lora,       util_sync_obj,  blue,       1, 1)
__log_component_def(lora,       util_evt_hndle, blue,       1, 1)

// -- others
__log_component_def(lora,       radio_ext,      default,    1, 0)
__log_component_def(lora,       patched_file,   default,    1, 1)
__log_component_def(lora,       mpy_lora,       default,    1, 1)

/* --- end of file ---------------------------------------------------------- */
