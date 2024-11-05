# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author    Ahmed Sabry (SG Wireless)
#
# Desc      Implements simple lora chat demo for the lora-raw mode
#           should be run on two different devices.
# ---------------------------------------------------------------------------- #

# disable logs
import logs
logs.filter_subsystem('lora', False)

# import the responsible module
import lora

# switch to lora raw if not there
if lora.mode() != lora._mode.RAW:
    lora.mode(lora._mode.RAW)

# define the callback
def lora_callback(context):
    if context['event'] == lora._event.RX_PACKET_EVENT:
        print(context['data'])
    pass
lora.callback(handler=lora_callback)

# start continuous reception
lora.recv_cont_start()

# start chating by sending
lora.send('Hello from LoRa chat')

# --- end of file ------------------------------------------------------------ #
