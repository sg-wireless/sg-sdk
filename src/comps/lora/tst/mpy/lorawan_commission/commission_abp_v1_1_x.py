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
# Desc      Implements simple lora ABP activation.
# ---------------------------------------------------------------------------- #

# required imports
import logs         # for system logging management
import lora         # for lora-stack
import ubinascii    # for hex/string conversions
import time         # for time manipulation

# disable fw logs
logs.filter_subsystem('lora', False)

# switch to lora-wan if not
lora.mode(lora._mode.WAN)

# ---------------------------------------------------------------------------- #
# Step (1) : Configuring the end-device -- This preparation should be done ONCE
#            for a new end-device and should not be repeated unless the device
#            credentials are to be changed.
# ---------------------------------------------------------------------------- #
# configure the stack on region EU-868
lora.wan_params(
    region=lora._region.REGION_EU868,
    lwclass=lora._class.CLASS_A)

# start end-device commisioning
# IMPORTANT:
#   commissioning must be done only ONCE in the end-device life cycle
#   after device is registered in the LoRa WAN server.
#   doing commissioning means you are resetting the LoRa MAC layer with a
#   newly registered device. If the commissioning is requested with the same
#   previous parameters, the call will be discarded.
lora.commission(
    type    = lora._commission.ABP,
    version = lora._version.VERSION_1_1_X,
    DevAddr = 0x00000000,
    DevEUI  = ubinascii.unhexlify('0000000000000000'),
    AppSKey = ubinascii.unhexlify('00000000000000000000000000000000'),
    NwkSKey = ubinascii.unhexlify('00000000000000000000000000000000')
    )

if not lora.is_joined():
    # start join procedure
    # Note: The joining procedure does not have a real effect in case of ABP
    #   activation, because after commissioning a device with ABP method,
    #   the device can start sending and receiving normally, because it
    #   considered itself as aready joined the network
    lora.join() # it will do nothing in ABP here

    while not lora.is_joined():
        print("wait joining ...")
        time.sleep(2)
        pass
    print("-- JOINED --")
    lora.stats()

# ---------------------------------------------------------------------------- #
# Step (2) : Normal end-device LoRa operation.
#            This should be happened every time the device is reset
#            operation sequence:
#               -- open the target lora application port
#               -- register the required application callback
#               -- start the duty-cycle operation
#               -- enable the end-device listening, so that the device can
#                  receive awaiting messages at the LoRa server
# ---------------------------------------------------------------------------- #
# open a working port
lora.port_open(1)

# attach required callback
def get_event_str(event):
    if event == lora._event.EVENT_TX_CONFIRM:
        return 'EVENT_TX_CONFIRM'
    elif event == lora._event.EVENT_TX_DONE:
        return 'EVENT_TX_DONE'
    elif event == lora._event.EVENT_TX_TIMEOUT:
        return 'EVENT_TX_TIMEOUT'
    elif event == lora._event.EVENT_TX_FAILED:
        return 'EVENT_TX_FAILED'
    elif event == lora._event.EVENT_TX_CONFIRM:
        return 'EVENT_TX_CONFIRM'
    elif event == lora._event.EVENT_RX_DONE:
        return 'EVENT_RX_DONE'
    elif event == lora._event.EVENT_RX_TIMEOUT:
        return 'EVENT_RX_TIMEOUT'
    elif event == lora._event.EVENT_RX_FAIL:
        return 'EVENT_RX_FAIL'
    else:
        return 'UNKNOWN'

def port_any_cb(event, evt_data):
    print('lora event [ {} ] --> data: {}'
        .format(get_event_str(event), evt_data))

lora.callback(handler=port_any_cb)

# start duty cycle
lora.duty_set(10000)
lora.enable_rx_listening()
lora.duty_start()

# schedule sending some test messages
i = 1000
while i < 1010:
    lora.send('tx-message-with-id-{}'.format(i), port=1, confirm=True, id = i)
    i = i + 1

# --- end of file ------------------------------------------------------------ #
