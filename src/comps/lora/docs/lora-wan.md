<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 !
 ! Permission is hereby granted, free of charge, to any person obtaining a copy
 ! of this software and associated documentation files(the “Software”), to deal
 ! in the Software without restriction, including without limitation the rights
 ! to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 ! copies  of  the  Software,  and  to  permit  persons to whom the Software is
 ! furnished to do so, subject to the following conditions:
 !
 ! The above copyright notice and this permission notice shall be included in
 ! all copies or substantial portions of the Software.
 !
 ! THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 ! IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 ! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 ! AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 ! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ! OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 ! THE SOFTWARE.
 !
 ! @author  Ahmed Sabry (SG Wireless)
 !
 ! @brief   LoRa WAN mode documentation.
 !----------------------------------------------------------------------------->

<!------------------------------------------------------------------------------
 ! Header and TOC
 !----------------------------------------------------------------------------->
# LoRa WAN API Documentation

### Available LoRa WAN APIs Summary


| API Call | Brief description |
|:---|:---|
|[`lora.stats()`](#stats)|displays the current stats of lora WAN|
|[`lora.wan_params()`](#wan_params)|set the lora WAN regional parameters|
|[`lora.commission()`](#commission)|set the LoRa-WAN commissioning parameters|
|[`lora.join()`](#join)|start performing join procedure|
|[`lora.send()`](#send)|transmit a LoRa-WAN packet|
|[`lora.recv()`](#send)|receive a LoRa-WAN packet|
|[`lora.port_open()`](#port_open)|open a lora-wan port to be able to tx/rx over it|
|[`lora.port_close()`](#port_close)|close a lora-wan port, tx/rx on it will be discarded|
|[`lora.callback()`](#callback)|set a user level callback to listen to specifc events|
|[`lora.duty_get()`](#duty_cycle)|get the current duty-cycle in milliseconds|
|[`lora.duty_set()`](#duty_cycle)|set the the duty-cycle to a specific value|
|[`lora.duty_start()`](#duty_cycle)|start duty-cycle operation|
|[`lora.duty_stop()`](#duty_cycle)|stop duty-cycle operation|
|[`lora.enable_rx_listening()`](#rx_listening)|perform class-a cycle to fetch pending DL msg|
|[`lora.disable_rx_listening()`](#rx_listening)|if no pending UL msg, discard class-a cycle|
|[`lora.mode(adr=)`](#adr)|enable or disable Adaptive Data Rate (ADR)|
|[`lora.tx_airtime()`](#tx_airtime)|get last TX time-on-air in milliseconds|
|[`lora.last_rx_at()`](#last_rx_at)|get timestamp (ms since boot) of last network reception|
|[`lora.add_channel()`](#add_channel)|add or replace a LoRaWAN channel (dynamic channel plan regions)|
|[`lora.remove_channel()`](#remove_channel)|remove a LoRaWAN channel (dynamic channel plan regions)|
|[`lora.channel_mask()`](#channel_mask)|get or set the active channel mask (all regions, required for US915/AU915/CN470)|

<!------------------------------------------------------------------------------
 ! LoRa WAN Stats
 !----------------------------------------------------------------------------->
<div id="stats"></div>

### LoRa WAN Stats

Displays useful information about the current lora-WAN settings such as:
enabled `region`, current working `class`, Device EUI `DevEUI`, Join EUI,
current assigned `DevAddr` after the joined procedure, lora-wan operating
`version` and the type of `activation` whether `NONE`, `OTAA`, or `ABP`

Example:

```
lora.stats()
    # outputs
    # - region               : EU-868
    # - class                : class-A
    # - dev eui              : 39 2C 39 D1 5D 3E 12 10
    # - join eui             : EA 68 DE 1C 4B E0 20 F4
    # - dev addr             : 01 88 DB B8
    # - lorawan version      : val: 16778240 ( 1.0.4.0 )
    # - activation           : OTAA
```

<!------------------------------------------------------------------------------
 ! Setting LoraWAN parameters
 !----------------------------------------------------------------------------->
<div id="wan_params"></div>

### Setting LoraWAN parameters

To change the current operating `region` and forces the device to be in `class` `A`,
`B` or `C`

Example:

```python
lora.wan_params(region = lora._region.REGION_EU868, lwclass = lora._class.CLASS_A)
    # sets the lora region to EU868
    # sets the default working class to class A which is the default
```

<!------------------------------------------------------------------------------
 ! Device commissioning
 !----------------------------------------------------------------------------->
<div id="commission"></div>

### Device commissioning

Commissioning a device means preparing a new lora-wan end-device, hence if the
LoRa-Stack is prepared with previous end-device credentials, it will be cleared
and will be configured with the new credentials as if it is a completely new
end-device. In other words, the LoRa MAC layer state will start clean for a new
end-device session and previous session will be cleared.

If the provided credentials are same as previousely commissioned parameters, the
commissioning will be ignored.

the end-device commissioning credentials are as follows:

- `version=<version>` to specify the end-device LoRa standard. It takes one of
    the following:
    - `version=lora._version.VERSION_1_0_x` LoRa version 1.0.x
    - `version=lora._version.VERSION_1_1_x` LoRa version 1.1.x

- `type=lora._commission.OTAA`  Device will be commissioned using OTAA procedure
and the device shall perform the Join procedure before tx/rx with the network
in this activation method, the following keys shall be provided along with:
    - `DevEUI`  The device EUI
    - `JoinEUI` The Join EUI
    - `AppKey`  The AppKey
    - `NwkKey`  The NwkKey if version 1.1.x 

- `type=lora._commission.ABP` The device will not perform the join procedure and 
    it will send directly an UL message. The following parameters shall be
    provided:
    - `DevEUI`  The device EUI
    - `DevAddr` The device network address
    - `AppSKey` The application security key
    - `NwkSKey` The network security key

- `verify=True` To check the provided parameters are same as the current
  commissioned parameters or not without doing any commissioning processing.

*REMARK* If the device is already joined the network, after this commissioning
operation the device will not be considered joined and need to rejoin again
using the new provided commissioning parameters.

Example

```python
import lora
import ubinascii

# verify the existing commissioning
if lora.commission(
    verify  = True,
    type    = lora._commission.OTAA,
    version = lora._version.VERSION_1_0_X,
    DevEUI  = ubinascii.unhexlify('0000000000000000'),
    JoinEUI = ubinascii.unhexlify('0000000000000000'),
    AppKey  = ubinascii.unhexlify('00000000000000000000000000000000')
    ) == True:
    print('end-device is already commissioned')
else:
    print('end-device is not commissioned')

# OTAA Version 1.0.x Example
lora.commission(
    type    = lora._commission.OTAA,
    version = lora._version.VERSION_1_0_X,
    DevEUI  = ubinascii.unhexlify('0000000000000000'),
    JoinEUI = ubinascii.unhexlify('0000000000000000'),
    AppKey  = ubinascii.unhexlify('00000000000000000000000000000000')
    )

# OTAA Version 1.1.x Example
lora.commission(
    type    = lora._commission.OTAA,
    version = lora._version.VERSION_1_1_X,
    DevEUI  = ubinascii.unhexlify('0000000000000000'),
    JoinEUI = ubinascii.unhexlify('0000000000000000'),
    AppKey  = ubinascii.unhexlify('00000000000000000000000000000000'),
    NwkKey  = ubinascii.unhexlify('00000000000000000000000000000000')
    )

# ABP Version 1.0.x Example
lora.commission(
    type    = lora._commission.ABP,
    version = lora._version.VERSION_1_0_X,
    DevAddr = 0x00000000,
    DevEUI  = ubinascii.unhexlify('0000000000000000'),
    AppSKey = ubinascii.unhexlify('00000000000000000000000000000000'),
    NwkSKey = ubinascii.unhexlify('00000000000000000000000000000000')
    )

# ABP Version 1.1.x Example
lora.commission(
    type    = lora._commission.ABP,
    version = lora._version.VERSION_1_1_X,
    DevAddr = 0x00000000,
    DevEUI  = ubinascii.unhexlify('0000000000000000'),
    AppSKey = ubinascii.unhexlify('00000000000000000000000000000000'),
    NwkSKey = ubinascii.unhexlify('00000000000000000000000000000000')
    )

```

<!------------------------------------------------------------------------------
 ! Join
 !----------------------------------------------------------------------------->
<div id="join"></div>

### Join

Mandatory operation to let the device join the network and be able to TX/RX with
the lora-WAN server. In case of ABP activation, the end-device is considered
joined after commissioning and join here will not have any effect.

Example:

```python
import lora
import time

# start join procedure
lora.join()

# wait until join
while lora.is_joined() == False:
    time.sleep(2)
    pass
```

<!------------------------------------------------------------------------------
 ! Sending data
 !----------------------------------------------------------------------------->
<div id="send"></div>

### Sending data

The successfully joined device is capable to tx/rx with the LoRaWAN server. To
start tx/rx operation, the user shall open a port first using the
`lora.port_open()` first, otherwise, no tx/rx operation will be performed.

To plan an UL message. It takes the following parameters:
- `message` the message buffer to be sent, can be a normal string or byte array
- optional arguments:

| parameter-name | value-type | default-value | desc |
| :---: | :---: | :---: | :--- |
| `confirm` | bool | False | To receive an ack from network server upon its reception |
| `port` | int | 1 | on which lora-wan port to send this message |
| `retries` | int | 0 | number of retried until the UL tx succeeded |
| `timeout` | int | `no-timeout` | time-out in ms to perform the full UL operation |
| `sync` | int | False | block until timeout or operation success/failure |
| `id` | int | 0 | user defined message id to be returned in the callback |

Example:

```python
# send an asynchronous UL message, with id=0, and without confirmation, no
# retries upon tx failure, and no specified timeout which means the message will
# be scheduled for UL in its turn within the pending UL messages until the
# duty-cycle tx operation fetches it and send it.
lora.send('ul tx message')

# send a message like before message, but if timeout of 3 seconds passed,
# drop the message and don't send it
lora.send('ul tx message', timeout=3000)

# repeat the transmission for upto 2 times, the full operation timeout including
# the retries attempts is 20 seconds
lora.send('ul tx message', timeout=20000, retries=2)

# same as before message but wait for confirmation as well from the network
# server
lora.send('ul tx message', timeout=20000, retries=2, confirm=True)

# same as the previous message, but the caller will be blocked until timeout,
# or message is successfully sent and acked
lora.send('ul tx message', timeout=20000, retries=2, confirm=True, sync=True)
```

<!------------------------------------------------------------------------------
 ! Receiving Data
 !----------------------------------------------------------------------------->
<div id="recv"></div>

### Receiving Data

The received data will come in the callback only

<!------------------------------------------------------------------------------
 ! LoRa Ports
 !----------------------------------------------------------------------------->
<div id="port"></div>

### LoRa Ports

LoRa WAN sends/receives data over what is called `ports`, valid application
ports are from `1` to `223`.

> A port must be opened first before sending and receiving data.

Example:

```python
# opening port 1
lora.port_open(1)   # data can be tx/rx over port 1

lora.send('data', port=5)    # ignored because port 5 is not opened
lora.send('data', port=1)    # will be planned successfully for UL

# opening port 1
lora.port_open(5)   # data can be tx/rx over port 5
lora.send('data', port=5)    # now it will be planned successfully

lora.port_close(1)  # no tx/rx more over this port
lora.port_close(5)  # no tx/rx more over this port
```

<!------------------------------------------------------------------------------
 ! Callbacks `lora.callback()`
 !----------------------------------------------------------------------------->
<div id="callback"></div>

### Callbacks `lora.callback()`

It can set a user lever callback and it takes the following parameters:

- 'handler' a callbeack function to be called.
- 'trigger' an OR combination of the required events that can trigger to this
  callback.
- 'port' a special port of the incoming messages events (default `any`)

Example:

```python
def lora_callback(context):
    def get_class_const_name(__class, __const):
        for k,v in __class.__dict__.items():
            if v == __const:
                return k
        return 'unknown'
    print('lora event: {} with-context: {}'.format(
        get_class_const_name(lora._event, context['event']), context))
    pass

lora.callback( handler = lora_callback )
```

> **NOTE**: Refer to the comprehensive documentation on the LoRa-Callback system
> for more details [here](lora-callback.md).

<!------------------------------------------------------------------------------
 ! Duty cycle operations
 !----------------------------------------------------------------------------->
<div id="duty_cycle"></div>

### Duty cycle operations

The device is normally working in class-A and the device shall follow a
duty-cycle to perform an UL/DL operation. This duty cycle should be regulated to
respect the time-on-air for this device.

The available operation are `duty_set()`, `duty_get()`, `duty_start()`,
`duty_stop()`

Example

```python
lora.duty_set(15000)    # sets the duty cycle timer to 15 seconds
                        # which means that every 15 seconds the device will check
                        # if TX pending and send it, and listen in the RX window
                        # to any scheduled DL message for this device

lora.duty_get()         # retrieve the current duty cycle time

lora.duty_start()       # start duty cycle operation

lora.duty_stop()        # stop duty cycle operation
```

<!------------------------------------------------------------------------------
 ! RX listening
 !----------------------------------------------------------------------------->
<div id="rx_listening"></div>

### RX listening

RX listening means that the device will send a dummy UL message in case no
pending TX message is pending, so that the server will plan an RX window for
this device and hence the device can receive any pending DL message.

the default behaviour is that the RX listening is _**disabled**_

Example

```python
lora.enable_rx_listening()      # enable listening

lora.disable_rx_listening()     # disable listening
                                # the device will listen only when there is a
                                # real planned UL TX message.
```

<!------------------------------------------------------------------------------
 ! Adaptive Data Rate
 !----------------------------------------------------------------------------->
<div id="adr"></div>

### Adaptive Data Rate (ADR)

Adaptive Data Rate allows the LoRaWAN network server to optimise each device's
data rate and transmission power based on the observed link quality.  ADR is
**enabled by default** when switching to WAN mode.

Disable ADR when the device is mobile or the RF environment is expected to vary
frequently, so the network server does not lock the device to a data rate that
may become unsuitable.

The `adr` keyword argument is accepted by `lora.mode()` when switching to WAN
mode, and can also be changed at any time while the stack is running.

Example:

```python
import lora

# switch to WAN mode with ADR disabled from the start
lora.mode(lora._mode.WAN, adr=False)

# re-enable ADR later (e.g. once the device is stationary)
lora.mode(lora._mode.WAN, adr=True)
```

<!------------------------------------------------------------------------------
 ! TX airtime
 !----------------------------------------------------------------------------->
<div id="tx_airtime"></div>

### TX Airtime — `lora.tx_airtime()`

Returns the time-on-air (in milliseconds) of the most recently transmitted
LoRaWAN packet.  The value is updated immediately after each successful
transmission, before the RX windows open.

The returned value is `0` until the first packet has been sent in the current
session.

This is useful for duty-cycle management: by knowing the exact airtime of the
last frame you can compute the minimum off-time required by regional regulations
before the next transmission.

Example:

```python
import lora
import time

lora.send('hello')

# after the send callback fires:
airtime_ms = lora.tx_airtime()
print('last TX airtime: {} ms'.format(airtime_ms))

# simple 1%-duty-cycle guard (EU868 default sub-band)
min_off_time_ms = airtime_ms * 99
time.sleep_ms(min_off_time_ms)
```

<!------------------------------------------------------------------------------
 ! Last network RX timestamp
 !----------------------------------------------------------------------------->
<div id="last_rx_at"></div>

### Last Network RX Timestamp — `lora.last_rx_at()`

Returns the value of the monotonic millisecond timer (`utime.ticks_ms()`
compatible) at the moment the most recent downlink frame was received from the
network.

The timestamp is updated on:
- Any application-layer downlink (port 1–223)
- Any MAC-only downlink (port 0 / network commands)
- An uplink ACK (`AckReceived`) returned by the network server in response to
  a confirmed uplink

The returned value is `0` until the first downlink (or ACK) has been received
in the current session.

This is useful for implementing a network-connectivity watchdog: if
`utime.ticks_diff(utime.ticks_ms(), lora.last_rx_at())` exceeds a threshold,
the device can decide to re-join or perform a reset.

Example:

```python
import lora
import utime

WATCHDOG_TIMEOUT_MS = 10 * 60 * 1000   # 10 minutes without any network contact

def check_network_health():
    last_rx = lora.last_rx_at()
    if last_rx == 0:
        print('no downlink received yet')
        return False
    elapsed = utime.ticks_diff(utime.ticks_ms(), last_rx)
    if elapsed > WATCHDOG_TIMEOUT_MS:
        print('no network contact for {} ms — triggering rejoin'.format(elapsed))
        return False
    return True
```

<!------------------------------------------------------------------------------
 ! add_channel
 !----------------------------------------------------------------------------->
<div id="add_channel"></div>

### Adding a channel — `lora.add_channel()`

Adds or replaces a LoRaWAN channel in the MAC channel table.  This is only
available on **dynamic channel plan** regions (EU868, AS923, KR920, IN865,
RU864, EU433, CN779).  For US915/AU915/CN470 use
[`lora.channel_mask()`](#channel_mask) instead.

Keyword arguments (all required):

| Parameter | Type | Description |
| :---: | :---: | :--- |
| `index` | int | Channel index to add/replace (region-dependent range) |
| `frequency` | int | Centre frequency in Hz (e.g. `868100000`) |
| `dr_min` | int | Minimum data rate index (0–7, region-dependent) |
| `dr_max` | int | Maximum data rate index (0–7, region-dependent) |

Raises `OSError` if the MAC rejects the request (e.g. region does not support
dynamic channel plans).

Example:

```python
import lora

# Add an extra EU868 channel at 867.1 MHz, DR0–DR5
lora.add_channel(index=3, frequency=867100000, dr_min=0, dr_max=5)
```

<!------------------------------------------------------------------------------
 ! remove_channel
 !----------------------------------------------------------------------------->
<div id="remove_channel"></div>

### Removing a channel — `lora.remove_channel()`

Removes a previously added channel from the MAC channel table by its index.
Like `add_channel`, this only works on dynamic channel plan regions.

Raises `OSError` if the MAC rejects the request.

Example:

```python
import lora

# Remove the channel at index 3
lora.remove_channel(3)
```

<!------------------------------------------------------------------------------
 ! channel_mask
 !----------------------------------------------------------------------------->
<div id="channel_mask"></div>

### Channel mask — `lora.channel_mask()`

Gets or sets the active channel mask.  This works on **all regions** and is the
correct way to restrict channels on fixed channel plan regions such as **US915**,
AU915, and CN470.

**Get** (no arguments) — returns a list of 6 integers representing the current
channel mask words:

```python
mask = lora.channel_mask()
# e.g. [0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00FF, 0x0000]  ← US915 all channels
```

**Set** (one argument: a list/tuple of 1–6 integers) — programs the channel mask.
The mask is also written to the *default* mask so it survives a re-join.

```python
lora.channel_mask([w0, w1, w2, w3, w4, w5])
```

> **US915/AU915 word → channel mapping**
>
> | Word | 125 kHz channels | Mask bits | Sub-bands covered | 500 kHz ch |
> | :---: | :--- | :--- | :--- | :--- |
> | `mask[0]` | ch 0–7 | bits 0–7 (`0x00FF`) | sub-band 1 | — |
> | `mask[0]` | ch 8–15 | bits 8–15 (`0xFF00`) | sub-band 2 | — |
> | `mask[1]` | ch 16–23 | bits 0–7 (`0x00FF`) | sub-band 3 | — |
> | `mask[1]` | ch 24–31 | bits 8–15 (`0xFF00`) | sub-band 4 | — |
> | `mask[2]` | ch 32–39 | bits 0–7 (`0x00FF`) | sub-band 5 | — |
> | `mask[2]` | ch 40–47 | bits 8–15 (`0xFF00`) | sub-band 6 | — |
> | `mask[3]` | ch 48–55 | bits 0–7 (`0x00FF`) | sub-band 7 | — |
> | `mask[3]` | ch 56–63 | bits 8–15 (`0xFF00`) | sub-band 8 | — |
> | `mask[4]` | — | bits 0–7 | — | ch 64–71 (one per sub-band) |
> | `mask[5]` | — | — | — | (unused in US915) |
>
> **US915 channel → frequency reference**
>
> | Channel | Frequency | Sub-band | Mask word/bits |
> | :---: | :--- | :---: | :--- |
> | 0–7 | 902.3 + 0.2×n MHz | 1 | `mask[0]` bits 0–7 |
> | 8–15 | 903.9 + 0.2×n MHz | 2 | `mask[0]` bits 8–15 |
> | 16–23 | 905.5 + 0.2×n MHz | 3 | `mask[1]` bits 0–7 |
> | 24–31 | 907.1 + 0.2×n MHz | 4 | `mask[1]` bits 8–15 |
> | 32–39 | 908.7 + 0.2×n MHz | 5 | `mask[2]` bits 0–7 |
> | 40–47 | 910.3 + 0.2×n MHz | 6 | `mask[2]` bits 8–15 |
> | 48–55 | 911.9 + 0.2×n MHz | 7 | `mask[3]` bits 0–7 |
> | 56–63 | 913.5 + 0.2×n MHz | 8 | `mask[3]` bits 8–15 |
> | 64–71 | 903.0 + 1.6×n MHz | (500 kHz) | `mask[4]` bits 0–7 |

> **⚠️ US915 minimum-channels constraint**: the LoRaMac-node US915 region
> implementation rejects any mask where **exactly 1** of the 64 125 kHz channels
> (words 0–3) is enabled.  The only accepted counts are **0** (no 125 kHz
> channels) or **≥ 2**.  Setting exactly 1 channel raises `OSError`.
> This is a hard constraint in the LoRaWAN regional stack and cannot be worked
> around without modifying the region code.

Raises `ValueError` if the list length is outside 1–6.  Raises `OSError` if the
MAC rejects the mask (e.g. the US915 single-channel constraint above).

Examples:

```python
import lora

# US915: restrict to sub-band 1 only (ch 0–7 + 500 kHz ch 64)
lora.channel_mask([0x00FF, 0x0000, 0x0000, 0x0000, 0x0001, 0x0000])

# US915: restrict to sub-band 2 only (ch 8–15 + 500 kHz ch 65)
# (The Things Network US uses sub-band 2 by default)
lora.channel_mask([0xFF00, 0x0000, 0x0000, 0x0000, 0x0002, 0x0000])

# US915: restore all channels
lora.channel_mask([0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x00FF, 0x0000])

# EU868: restrict to channel 0 only (868.1 MHz) — single channel gateway
lora.channel_mask([0x0001])

# EU868: restrict to the three mandatory channels (868.1, 868.3, 868.5 MHz)
lora.channel_mask([0x0007])

# Read back the current mask
print(lora.channel_mask())
```

> **Single-channel gateway — EU868**: works perfectly.  Enable only the bit for
> the channel your gateway is listening on.  Example for 868.1 MHz (ch 0):
> ```python
> lora.channel_mask([0x0001])
> ```

> **Single-channel gateway — US915**: due to the minimum-channels constraint
> described above, you **cannot** restrict to exactly one 125 kHz channel.
> The closest approach is to enable the two channels closest to the one your
> gateway is listening on (from the same sub-band).  For a gateway on 902.3 MHz
> (ch 0), use:
> ```python
> lora.channel_mask([0x0003, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000])
> # ch 0 (902.3 MHz) + ch 1 (902.5 MHz) — the device will alternate between them
> ```
> If your gateway only understands a single frequency the device will still
> miss 50 % of its Join/uplink attempts.  For production use, restrict to a
> full sub-band (8 channels) that matches your gateway's configuration.

<!--- end of file ------------------------------------------------------------->
