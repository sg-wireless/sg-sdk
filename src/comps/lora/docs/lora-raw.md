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
 ! @brief   LoRa RAW mode documentation.
 !----------------------------------------------------------------------------->

<!------------------------------------------------------------------------------
 ! Header and TOC
 !----------------------------------------------------------------------------->
# LoRa RAW API Documentation

### Available LoRa RAW APIs Summary

| API Call | Brief description |
|:---|:---|
|[`lora.stats()`](#stats)|displays the current stats of lora RAW|
|[`lora.radio_params()`](#radio_params)|set one or more radio parameter
|[`lora.callback()`](#callback)|set a user level callback|
|[`lora.send()`](#send)|transmit a given data over LoRa|
|[`lora.recv()`](#recv)|open a timed-out rx window to listen to any incoming data|
|[`lora.recv_cont_start()`](#recv_cont)|switch to continuous rx mode|
|[`lora.recv_cont_stop()`](#recv_cont)|close the continuous rx mode|
|[`lora.tx_continuous_wave_start()`](#tx_continuous_wave)|start tx continuous wave operation|
|[`lora.tx_continuous_wave_stop()`](#tx_continuous_wave)|stops tx continuous wave operation|

<!------------------------------------------------------------------------------
 ! LoRa Raw Settings
 !----------------------------------------------------------------------------->
<div id="stats"></div>

### LoRa Raw Settings

To display the current settings of the lora RAW, use `lora.stats()`, 
  Then you will experience something like this:

```
>>> lora.stats()
    regional params
        region         : EU-868
        frequency      : 868000000 Hz
        freq_khz       : 868000.000 KHz
        freq_mhz       : 868.000 MHz
    modulation params
        sf             : 12
        bandwidth      : 125 KHz
        coding_rate    : 4_7
    packet params
        preamble       : 8
        payload        : 51
        crc_on         : False
    lora tranceiver
        chip           : SX1262
        max tx_power   : +22 dBm
    tx params
        tx_power       : +10 dBm
        antenna_gain   : +1.00 dBi
        tx_power_eff   : +9 dBm
        tx_timeout     : 6000 msec
        tx_iq          : False
    rx params
        rx_timeout     : 6000 msec
        rx_iq          : False
```

Here is the meaning of each displayed parameter:

- **`regional params`**: The parameters corresponding to the current region
    - `region`: The region in which the device will operate.
    - `frequency`, `freq_khz` or `freq_mhz`: The required frequency in **Hz**,
        **KHz** or **MHz** respectively.

- **`modulation params`**: The current modulation parameters which are;
    spreading-factor `sf`, `bandwidth` and `coding_rate`

- **`packet params`**: parameters related to the packet data constraints such as
    `preamble` length, current maximum `payload` size, and if the `crc_on` is
    applied to the payload or not

- **`lora tranceiver`**: shows the current info about the current used tranceiver
    such as the `chip` used and the maximum `tx_power` it can produce.

- **`tx params`**: the current tx settings;
    - the current desired `tx_power` including the antenna gain
    - the `antenna_gain`; should be set according to the current HW prescribed
        antenna gain to be taken into consideration while determining the chip
        output tx power
    - the `tx_power_eff` which is the actual effective chip output power after
        subtracting the antenna gain from the desired `tx_power`
    - `tx_timeout` the time-out of sending a message; it should be sufficient
        enough according to the time on air required for the current modulation
        parameters.
    - `tx_iq` indicates whether inverted IQ polarity feature is enabled or not

- **`rx params`**: the current rx settings;
    - `rx_timeout` the rx window time in non continuous reception
    - `rx_iq` indicates whether inverted IQ polarity feature is enabled or not

To reset all parameters to the region defaults, provide `reset_all` flag like:
```
>>> lora.radio_params(reset_all=True)
```

<!------------------------------------------------------------------------------
 ! Modifying Radio Parameters
 !----------------------------------------------------------------------------->
<div id="radio_params"></div>

### Modifying Radio Parameters

To change any radio parameter, use `lora.radio_params()` which takes its
parameters as in the following BNF formatted description:

```BNF
<radio-param-change-call> ::=
        "lora.radio_params(" <param-value-pair>  <params-list> ")"

<params-list> ::=
        "," <param-value-pair>
        | <params-list>
        | ""

<param-value-pair> ::=
    reset_all   "=" <bool-value>            ; reset to factory settings
    | region      "=" <region-value>          ; change the region
    | frequency   "=" <freq-value-in-hz>      ; desired freq in Hz
    | freq_khz    "=" <freq-value-in-khz>     ; desired freq in KHz
    | freq_mhz    "=" <freq-value-in-mhz>     ; desired freq in MHz
    | tx_power    "=" <signed-int-value>      ; desired tx power
    | sf          "=" <sf-value>              ; spreading factor
    | coding_rate "=" <cr-value>              ; coding rate
    | preamble    "=" <integer-value>         ; preamble length
    | bandwidth   "=" <bw-value>              ; band-width
    | tx_iq       "=" <bool-value>            ; inverted IQ feature

<bool-value> ::= "True" | "False"

<region-value> ::= "lora._region." <region>
<region> ::= 
    "REGION_AS923" | "REGION_AU915" | "REGION_CN470" | "REGION_CN779"
    | "REGION_EU433" | "REGION_EU868" | "REGION_IN865" | "REGION_KR920"
    | "REGION_RU864" | "REGION_US915"

<freq-value-in-hz> ::= <positive-integer-value>
<freq-value-in-khz> ::= <floating-point-value>
<freq-value-in-mhz> ::= <floating-point-value>

<sf-value> ::= "7" | "8" | "9" | "10" | "11" | "12"

<bw-value> ::= "lora._bw." <bw>
<bw> ::= "BW_125KHZ" | "BW_250KHZ" | "BW_500KHZ"

<cr-value> ::= "lora._cr." <cr>
<cr> ::= "CODING_4_5" | "CODING_4_6" | "CODING_4_7" | "CODING_4_8"
```


Examples:

```python
# setting a new lora region to set the lora constraints to this region
lora.radio_params(region=lora._region.REGION_EU433)

# set frequency to 433.3 MHz
lora.radio_params(freq_mhz=433.3)   # accepted because it is valid region freq

# set frequency to 435 MHz
lora.radio_params(freq_mhz=433.3)   # accepted because it is valid region freq

lora.radio_params(freq_mhz=435)     # regected because it is non-valid region freq
    # error: incompatible frequency 435000000 with the region EU-433

# set the tx power to 5 dBm
lora.radio_params(tx_power=5)
lora.stats()
    #    tx_power       : +5 dBm        --> desired tx-power
    #    antenna_gain   : +2.15 dBi     --> current set antenna-gain
    #    tx_power_eff   : +2 dBm        --> actual effective lora chip output power
lora.radio_params(antenna_gain=1)       # the effective power will change accordingly
lora.stats()
    #    tx_power       : +5 dBm        --> desired tx-power
    #    antenna_gain   : +1.00 dBi     --> current set antenna-gain
    #    tx_power_eff   : +4 dBm        --> actual effective lora chip output power

# setting out of region valid tx power will be regicted for saftey
lora.radio_params(tx_power=45)
    # error: invalid chip power +45 dBm -- chip SX1262 tx power range ( -8 ~ +23 ) dBm considering antenna gain 1.00 dBi
    # error: invalid tx-power 45 

# setting spreading factor to 8 and BW to 250 and coding rate to 4/6
lora.radio_params(sf = 8, bandwidth = lora._bw.BW_250KHZ, coding_rate = lora._cr.CODING_4_6)
    # modulation params
    #     sf             : 8
    #     bandwidth      : 250
    #     coding_rate    : 4_6

# setting wrong values will be regected and the whole parameters will be ignored
lora.radio_params(bandwidth=9, tx_power=44, sf=90) # gived the following reported errors
error: invalid chip power +44 dBm -- chip SX1262 tx power range ( -7 ~ +24 ) dBm considering antenna gain 2.15 dBi
error: invalid argument value 'tx_power'
error: invalid argument value 'sf'
error: invalid argument value 'bandwidth'

```

> **Note**:
> Changing the region, will reset the entire radio parameters to the defaults
> of this new region

> Note:
> The lora interface provides some class constants for some radio parameters:
>   - `lora._bw`: contains all supported band width values
>   - `lora._cr`: contains all supported coding rate values
>   - `lora._region`: contains all supported regions values
>   ```python
>   # Example
>   # you can see the allowed values constans, by pressing the class names
>   # followed by double <tab>
>
>   >>> lora._bw.       # press <tab> <tab> to see the following list
>   BW_125KHZ       BW_250KHZ       BW_500KHZ
>
>   >>> lora._cr.       # press <tab> <tab> to see the following list
>   CODING_4_5      CODING_4_6      CODING_4_7      CODING_4_8
>
>   >>> lora._region.   # press <tab> <tab> to see the following list
>   REGION_AS923    REGION_AU915    REGION_CN470    REGION_CN779
>   REGION_EU433    REGION_EU868    REGION_IN865    REGION_KR920
>   REGION_RU864    REGION_US915
>   ```

> Note:
> To change the frequency value, it can be done through one of these parameters
> (`frequency`, `freq_khz` or `freq_mhz`), however it is possible to specify
> one or more of those parameters. Hence in that case, the specified parameters
> will be considered in a priority fashion. `frequency` parameter has highest
> consideration priority and `freq_mhz` has lowest consideration priority.
>   ```python
>   # consider the current radio frequency parameter is 868.000 MHz
>
>   >>> lora.radio_params(frequency=868000000, freq_mhz=868.3)
>   # the specified `frequency` parameter will be considered first, but because
>   # it has the same value of the current frequency, it will be bypassed,
>   # then the next specified `freq_mhz` parameter will be considered, and
>   # the radio frequency will be changed accordingly.
>   # --> hence the current radio frequency parameter becomes 868.300 MHz
>
>   >>> lora.radio_params(freq_mhz=868.3, frequency=868000000)
>   # the highest priority parameter `frequency` will be considered first.
>   # and because it holds newer value than the current radio frequency, the 
>   # radio frequency will be modified accordingly.
>   # --> hence the current radio frequency parameter becomes 868.000 MHz
>   # --> and the next specified `freq_mhz` parameter is neglicted
>   ```

<!------------------------------------------------------------------------------
 ! Setting LoRa RAW user Callback
 !----------------------------------------------------------------------------->
<div id="callback"></div>

### Setting LoRa RAW user Callback

To set a user level callback to listen the RX events, see the following example
to know the available events that can come in the callback

Example

```python
def get_event_str(event, bytes):
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
    pass

def lora_callback(event, evt_data):
    print('lora event [ {} ] --> data: {}'
        .format(get_event_str(event), evt_data))

lora.callback( handler = lora_callback )

```

<!------------------------------------------------------------------------------
 ! Send (TX) Data
 !----------------------------------------------------------------------------->
<div id="send"></div>

### Send (TX) Data

To send a specific data message, it takes the following parameters:
- `message`: it is the normal data buffer, it could be a normal string or byte array
- `timeout`: it is an optional argument to specify the tx operation deadline
the default timeout will be the radio `tx_timeout` parameter
- `sync`: it is an optional argument to perform this operation synchronously
or asynchronously (default: `sync=False`)

Examples

```python

lora.send("test message")   # sends a message asynchronously and with tx_timeout
                            # as a full tx operation timeout

# send a message asynchronously and the full tx operation shall canceled after 1 second
lora.send("test message", timeout=1000)

# send a message synchronously and the full tx operation shall canceled after 1 second
# in this case, the caller will be blocked until tx succeeded or timeout is over
lora.send("test message", timeout=1000, sync=True)
```

<!------------------------------------------------------------------------------
 ! Receive (RX) Data
 !----------------------------------------------------------------------------->
<div id="recv"></div>

### Receive (RX) Data

To receive a data and it takes the following parameters:

- `timeout`: it is an optional argument to specify the tx operation deadline
the default timeout will be the radio `rx_timeout` parameter
- `sync`: it is an optional argument to perform this operation synchronously
or asynchronously (default: `sync=True`)
    - `sync` the function will return the received message
    - `async` the received message will be returned in the RX event in the callback

Example:

```python
lora.recv()   # waits for `rx_timeout` radio parameter or until a data is received

# wait for maximum 2 second or until a data is received
lora.recv(timeout=2000)

# place a receive request and return immediately
#   - if a data is received before 3 second timeout, it will be returned in the callback
#   - if timeout happened, the rx operation will be canceled and a timeout event will be
#     fed back in the callback
lora.recv(timeout=3000, sync=True)
```

<!------------------------------------------------------------------------------
 ! RX Continuous Mode
 !----------------------------------------------------------------------------->
<div id="recv_cont"></div>

### RX Continuous Mode

LoRa RAW can operate in continuous reception mode and any received data will be
thrown in the registered user callback

Example:

```python
lora.recv_cont_start()    # starting the RX continuous mode

lora.recv_cont_stop()     # exiting the RX continuous mode
```

<!------------------------------------------------------------------------------
 ! Continuous TX Wave mode
 !----------------------------------------------------------------------------->
<div id="tx_continuous_wave"></div>

### Continuous TX Wave mode

Sets the radio tranciver to continuous transmission mode for testing.

The tx continuous wave mode does not use the normal parameters set by the
`lora.radio_params()` method, but instead it uses the following parameters
- `tx_power` the required tx power during the test
- `frequency` the required test frequency in Hz (default: 868 MHz)
- `timeout` an optional timeout in milliseconds (default: 10 seconds)

> Remark: if the system is in sending or receiving operation, the operation
> will be cancelled and the system will start serving the tx continuous wave
> test command. After timeout is over, the system will go to its IDLE state.

Example:

```python
lora.tx_continuous_wave_start(      # starting the TX continuous wave mode
    tx_power = 20,                  # use tx_power = 20dBm
    frequency = 433000000,          # test frequency 433 MHz
    timeout = 20000)                # timeout for the tx continuous wave 20 sec

lora.tx_continuous_wave_stop()    # exiting the TX continuous wave mode
```
<!--- end of file ------------------------------------------------------------->
