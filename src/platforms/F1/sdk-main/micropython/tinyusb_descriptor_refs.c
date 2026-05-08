/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * --------------------------------------------------------------------------- *
 *
 * TinyUSB descriptor symbol references to ensure proper linking order
 * This forces the MicroPython descriptor symbols to be available during linking
 */

#include <stdint.h>

// External references to MicroPython TinyUSB descriptor callbacks
extern const uint8_t* tud_descriptor_device_cb(void);
extern const uint8_t* tud_descriptor_configuration_cb(uint8_t index);
extern const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid);

// Force references to these symbols to ensure they are linked
void __tinyusb_descriptor_refs_force_link(void)
{
    (void)tud_descriptor_device_cb;
    (void)tud_descriptor_configuration_cb; 
    (void)tud_descriptor_string_cb;
}