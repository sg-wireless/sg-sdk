<!--
Copyright (c) 2023-2024 SG Wireless - All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
copies  of  the  Software,  and  to  permit  persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Description: CTRL Client User API Documentation
-->

# CTRL API Documentation

## Contents

* [Sending Fields](#fields)
* [Configuration](#config)
* [Connection](#connection)
* [Miscellaneous](#miscellaneous)
* [Examples](#examples)


<div id="fields"></div>

## Fields

### ctrl.send_field(pin_number, value, [timestamp=0, device_token=None])

Send a field value to CTRL. Arguments are:
* `pin_number`: The pin/field number in CTRL, can be any integer value
* `value`: The value you want to send, this can be any type (int, float, string, etc.)
* `timestamp`: Optional. Unix timestamp in seconds. If set to 0 (default), the server will use the current time
* `device_token`: Optional. Device token for sending data to a different device

### ctrl.send_field_map(map, [timestamp=0, device_token=None])

Send multiple field values to CTRL in a single message using a dictionary/map. Arguments are:
* `map`: A dictionary where keys are pin/field numbers and values are the data to send. Example: `{1: 25.5, 2: 60.3, 3: "online"}`
* `timestamp`: Optional. Unix timestamp in seconds. If set to 0 (default), the server will use the current time
* `device_token`: Optional. Device token for sending data to a different device

### ctrl.send_ping_message()

Sends a ping (is-alive) message to CTRL. The platform will answer with a `pong` message if connected via WiFi or LTE-M

### ctrl.send_info_message()

Send an info message to CTRL containing the device type and firmware version.

### ctrl.send_battery_level(battery_level)

Sends the battery level to **[Ctrl](https://app.sgwireless.com/)**. The argument `battery_level` can be any integer.

You can define `battery_level` with a function depending on your hardware. 

```python
def battery_level():
    return 3.7
ctrl.send_battery_level(battery_level())
```

---

<div id="config"></div>

## Configuration

If CTRL support is active in the current firmware (check `import ctrl_cfg; ctrl_cfg.ctrl_on_boot()`)
it will load automatically. It will first look for a file `ctrl_config.json` in the file system.

If the file is found and the configuration looks valid, CTRL will try to connect to the cloud
platform based on the configured parameters. The user can upload a file `ctrl_project.json` onto
the local `/` to overwrite any of the parameters from `ctrl_config.json`. This allows for
project specific settings to be configured such as forcing SSL to be enabled or to disable
automatically starting the ctrl client on boot.

If no valid configuration is found, ctrl will load with an empty configuration to allow for
the `ctrl.activate()` command to be executed. This will allow for the device to be activated
via the python cli.

To manually load the CTRL client from your own scripts, the following code shows how it is
loaded from the build-in frozen code.

```python
import ctrl_cfg
if ctrl_cfg.ctrl_on_boot():
    import os
    import sys

    if 'ctrl_config' not in globals().keys():
        from ctrl_config import CtrlConfig
        from ctrl import Ctrl

        ctrl_config = CtrlConfig().read_config()

    if (not ctrl_config.get('ctrl_autostart', True)) and ctrl_config.get('cfg_msg') is not None:
        print(ctrl_config.get('cfg_msg'))
        print("Not starting CTRL as auto-start is disabled")

    else:
        # Load CTRL if it is not already loaded
        if 'ctrl' not in globals().keys():
            ctrl = Ctrl(ctrl_config, ctrl_config.get('cfg_msg') is None, True)

```



The CTRL API offers several helper functions to work with the configuration:

### ctrl.read_config([filename='/ctrl_config.json', reconnect=False])

Load the CTRL configuration file. By default, this is loaded from `/ctrl_config.json`
If reconnect=True, ctrl will disconnect and re-connect using the new configuration


### ctrl.get_config([key=None])

Returns the configuration. If key is specified, only configuration for the given key is returned.

### ctrl.update_config(key, [value=None, permanent=True, silent=False, reconnect=False])

Update a `key` and `value` of the default configuration file. This will **update** the existing
configuration setting to add the new values.

**additional options:**
* `permanent`: will call `ctrl.write_config()`. If set `False`, the new value will not be 
stored in the configuration file and only used this session.
* `silent`: set `silent` to `True` to not print a message to REPL.
* `reconnect`: calls `ctrl.reconnect()`

### ctrl.set_config(key, [value=None, permanent=True, silent=False, reconnect=False])

Set a `key` and `value` of the default configuration file. This will overwrite any existing
 settings for the specified key.

**additional options:**
* `permanent`: will call `ctrl.write_config()`. If set `False`, the new value will not be stored in the configuration file and only used this session.
* `silent`: set `silent` to `True` to not print to REPL.
* `reconnect`: calls `ctrl.reconnect()`

### ctrl.write_config([file='/ctrl_config.json', silent=False])

Writes the updated configuration to the default configuration file. The parameters:
* `file`: The file name and location
* `silent`: set `silent` to `True` to not print to REPL.

### ctrl.print_config()

Print the configuration settings to the REPL.
This is easier to read for a human


### ctrl.activate(activation_string)

Activate ctrl with the configuration pasted from the CTRL platform (under device/provisioning)
---

<div id="connection"></div>

## Connection

### ctrl.start([autoconnect=True])

This will manually start the ctrl client, with the option to set `autoconnect`. Setting `autoconnect` to `False` will not start the connection immediately.

### ctrl.connect()

Connect the device to CTRL following the loaded configuration file. You will need to load a configuration file before calling this. If you are using the WiFi or LTE-M connection, and it is already available, CTRL will use the existing connection.

### ctrl.enable_lte(carrie, apn, [type='IP', cid=1, band=None, bands=None, mode=0, fallback=False])

Enable connecting via LTE-M connection to CTRL. Enter the paramters you would normally enter for an LTE connection.
If fallback is True, will add LTE-M as the last option in the list of networks. Otherwise, it will
be added as the first option and the device will connect via LTE-M after reset.

### ctrl.enable_wifi(ssid, [password=None, fallback=False])

Enable connecting via WiFi to CTRL. Enter the paramters you would normally enter for a WiFi connection.
If fallback is True, will add WiFi as the last option in the list of networks. Otherwise, it will
be added as the first option and the device will connect via WiFi after reset.

### ctrl.connect_lte()

Manually connect to CTRL using LTE and the settings from the configuration file.

### ctrl.connect_wifi([timeout=120])

Manually connect to CTRL using WiFi and the settings from the configuration file. The `timeout` option is in seconds.

### ctrl.connect_lora_otaa([timeout=120])

Manually connect to CTRL using LoRa OTAA and the settings from the configuration file. The `timeout` option is in seconds.

### ctrl.disconnect()

Disconnect from CTRL gracefully. Closes the MQTT connection and socket.

### ctrl.reconnect()

Calls `ctrl.disconnect()` followed by `ctrl.connect()`

### ctrl.isconnected()

Returns the connection status to CTRL, can be `True` or `False`.

### ctrl.ifconfig()

Returns a tuple with IP information when connected over WiFi or LTE-M


### ctrl.enable_ssl()

Enable SSL on the CTRL connection

>Note that SSL might not be supported by your LTE connection

> **Note that SSL is not currently supported by the CTRL platform**

### ctrl.dump_ca([file='/cert/sgw-ca.pem'])

Write CTRL ROOT CA certificate to file.
In order for the firmware to load the certificate, it needs to be present
in the file system. While the firmware has this CA embedded, it needs
to be written to the file system in order to be used.

---

<div id="miscellaneous"></div>

## Miscellaneous

### ctrl.deepsleep(ms)

This will disconnect the current connection before going to deepsleep.
See [machine.deepsleep()](https://docs.micropython.org/en/v1.26.1/library/machine.html#machine.deepsleep) for more details.

### ctrl.print_cfg_msg()

This prints the configuration status message on the REPL.


### ctrl.message_queue_len()

Returns the length of the message queue

### ctrl.get_network_type()

Returns the network type currently in use

### ctrl.debug(new_level, [update_nvs=True])

Sets the debug level at new_level [0-65565]
update_nvs will preserve the setting after reset


### ctrl.ztp([new_status=None])

If no parameter is given, will return if ztp is currently enabled.
If new_status is True, will enable ztp during next boot
If new_status is False, will disable ztp during next boot and stop the current ztp activation process if running

---

<div id="examples"></div>

## Examples

### Example 1:

Assuming your device has been activated with one of the provisioning tools available,
the following code will send data regularly to the CTRL cloud:

```python
# Import what is necessary to create a thread
import time
import math

# Send data continuously to CTRL
while True:
    for i in range(0,20):
        ctrl.send_field(1, math.sin(i/10*math.pi))
        print('sent field {}'.format(i))
        time.sleep(10)

```

Optionally, you can send a timestamp:

```python
# Import what is necessary to create a thread
import time
import math

# Send data continuously to CTRL
while True:
    for i in range(0,20):
        ctrl.send_field(1, math.sin(i/10*math.pi), time.time())
        print('sent signal {}'.format(i))
        time.sleep(10)

```


---

## Deprecated API

### ctrl.send_signal(signal_number, value)

> **Deprecated: `send_signal` has been removed. Use `send_field` instead.**

<!--- end of file --->
