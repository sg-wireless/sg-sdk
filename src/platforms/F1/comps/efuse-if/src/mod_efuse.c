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
 * @brief   micropython module for eFuses interface component.
 * --------------------------------------------------------------------------- *
 */

#include "mp_lite_if.h"
#define __log_subsystem     F1
#define __log_component     efuse
#include "log_lib.h"
#include "efuse_if.h"

/** -------------------------------------------------------------------------- *
 * module function
 * --------------------------------------------------------------------------- *
 */
__mp_mod_init(efuse_if)(void)
{
    efuse_if_init();
    return mp_const_none;
}

__mp_mod_fun_0(efuse_if, layout_version)(void)
{
    efuse_layout_version_t ver;
    efuse_if_read_layout_version(&ver);
    return mp_obj_new_bytearray(sizeof(ver), &ver);
}

__mp_mod_fun_0(efuse_if, lora_mac)(void)
{
    efuse_lora_mac_t lora_mac;
    efuse_if_read_lora_mac(&lora_mac);
    return mp_obj_new_bytearray(sizeof(lora_mac), &lora_mac);
}

__mp_mod_fun_0(efuse_if, serial_number)(void)
{
    efuse_serial_number_t serial_number;
    efuse_if_read_serial_number(&serial_number);
    return mp_obj_new_bytearray(sizeof(serial_number), &serial_number);
}

__mp_mod_fun_0(efuse_if, hw_id)(void)
{
    efuse_hw_id_t hw_id;
    efuse_if_read_hw_id(&hw_id);
    return mp_obj_new_bytearray(sizeof(hw_id), &hw_id);
}

__mp_mod_fun_0(efuse_if, project_id)(void)
{
    efuse_project_id_t project_id;
    efuse_if_read_project_id(&project_id);
    return mp_obj_new_bytearray(sizeof(project_id), &project_id);
}

__mp_mod_fun_0(efuse_if, wifi_mac)(void)
{
    efuse_wifi_mac_t wifi_mac;
    efuse_if_read_wifi_mac(&wifi_mac);
    return mp_obj_new_bytearray(sizeof(wifi_mac), &wifi_mac);
}

/* --- end of file ---------------------------------------------------------- */
