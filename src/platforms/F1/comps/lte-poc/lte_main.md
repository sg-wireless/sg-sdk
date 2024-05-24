
## Constructors

### class LTE.LTE(…)

Create and configure a LTE object. See \_\_init\_\_ for params of configuration.

```python

from LTE import LTE
lte = LTE()

```

## Methods

### lte.\_\_init\_\_([carrier='standard’, cid=1, mode=None, baudrate=115200, debug=None])

This method is used to set up the LTE subsystem. Optionally specify

-   `carrier name`. The currently available options are:
    -   `'att'`
    -   `'verizon'`
    -   `'standard'`
    -  `'docomo'`
    -  `'kddi'`
    -  `'telstra'`
    -  `'tmo'`
    -  `'verizon-no-roaming'`
    -  `'3gpp-conformance'`

- `cid` is the connection id. Most operators use cid=1 except Verizon which uses cid=3 when using a Verizon issued SIM card
- `mode` is LTE.CATM1 or LTE.NBIOT. If not specified, modem will use current setting
- `baudrate` is the speed with which the modem is operating. Default is 115200bps
- `debug`. True or False, display additional debugging output


### lte.deinit([reset=False])

Disables LTE modem completely. This reduces the power consumption to the minimum. Call this before entering deepsleep.
Optional parameter reset was added for compatibility with legacy code and is not used


### lte.attach([apn=None, type='IP', cid=None, band=None, bands=None])

Enable radio functionality and attach to the LTE network authorised by the inserted SIM card. Optionally specify:

-   `band`  : to scan for networks. If no band (or `None`) is specified, the currently configured bands will be scanned (this is persistent through resetting the modem). The possible values for the band are:  `1,2,3,4,5,8,12,13,17,18,19,20,25,26,28,66 or 71`.
-  `bands` : a tuple of mutiple band entries (see band above)
-   `apn`  : Specify the APN (Access point Name).
-   `cid`  : connection ID, see  `LTE.\_\_init()\_\_` and `LTE.connect()`. when the ID is set here it will be remembered when doing connect so no need to specify again
-   `type`  : PDP context type either  `IP`  or  `IPV4V6`. These are options to specify PDP type ‘Packet Data protocol’ either IP [Internet Protocol] or IPV4V6  [Internet Protocol version 4 and version 6] , that depend on what the Network supports.


### lte.isattached()

Returns  `True`  if the cellular mode is attached to the network.  `False`  otherwise.


### lte.is_attached()

Returns  `True`  if the cellular mode is attached to the network.  `False`  otherwise.
Same as `lte.isattached()` and provided for compatibility with legacy scripts


### lte.detach()

Gracefully detach the modem from the LTE-M network and disable the radio functionality.


### lte.connect([cid=None])

Start a data session and obtain and IP address. Optionally specify a CID (Connection ID) for the data session. The arguments are:

-   `cid`:  connection ID, see  `LTE.\_\_init()\_\_` and `LTE.attach()`.


### lte.isconnected()

Returns  `True`  if there is an active LTE data session and IP address has been obtained.  `False`  otherwise.


### lte.is_connected()

Returns  `True`  if there is an active LTE data session and IP address has been obtained.  `False`  otherwise.


### lte.disconnect()

End the data session with the network.


### lte.send_at_cmd(cmd, [timeout=-1, wait_ok_error=False, check_error=False])

Send an AT command directly to the modem. Returns the raw response from the modem as a string object. You can find the possible AT commands  [here](https://docs.pycom.io/gitbook/assets/Monarch-LR5110-ATCmdRefMan-rev6_noConfidential.pdf).

> If a data session is active (i.e. the modem is  _connected_), you will need to  `lte.pause_ppp()`  and  `lte.resume_ppp`  around the AT command.

Example:

```python
lte.send_at_cmd('AT+CEREG?')    # check for network registration manually (sames as lte.isattached())

```

Optionally the response can be parsed for pretty printing:

-   `timeout`  : specify the timeout milliseconds the esp32 chip will wait after the AT command to receive the response. -1 means wait forever
-  `wait_ok_error` : wait for the modem to respond with OK or ERROR after sending the command. Not all commands return OK or ERROR which means the command might be waiting forever. Some commands such as `AT+SQNINS` run longer than the maximum timeout, and setting `wait_ok_error=True` is required to get the results
- `check_error` : Will check if an error occured and raise an exception. Helpful in scripts that should abort if an error occurs.


### lte.reset()

Perform a hardware reset on the cellular modem. This function can take up to 5 seconds to return as it waits for the modem to shutdown and reboot.

### lte.pause_ppp()

Suspend PPP session with LTE modem. this function can be used when needing to send AT commands which is not supported in PPP mode.


### lte.resume_ppp()

Resumes PPP session with LTE modem.


### lte.mode([new_mode=None])

If no parameter is specified, return the current operating mode (0 for LTE.CATM1 and 1 for LTE.NBIOT)
If `new_mode` is specified, switches the modem into the specified operating mode. Use LTE.CATM1 or LTE.NBIOT
Example: `lte.mode(new_mode=LTE.CATM1)`
> The modem will reset and switch to the new operating mode

### lte.power_on([wait_ok=True])

Turn the LTE modem power on. Will optionally wait until the modem answers with OK.


### lte.power_off([force=False])

Turn off power to the LTE modem. Will gracefully shut down the LTE connection unless force=True is used.


### lte.check_sim_present()

Check if a SIM card is present and readable


### lte.check_power()

Returns True if the lte module is powered on, otherwise false


### lte.print_pretty_response(rsp, [flush=False, prefix=None])

Removes unnecessary line feed and OK/ERROR output from a modem response before printing the response on the REPL


### lte.return_pretty_response(resp)

Removes unnecessary line feed and OK/ERROR output from a modem response before returning the resp


### lte.read_rsp([size=None, timeout=-1, wait_ok_error=False, check_error=False])

This function allows reading unsolicited responses from the modem. These are responses sent by the modem without being requested using `lte.send_at_cmd()`. The response can be formatted with `lte.return_pretty_response` and `lte.print_pretty_response` if desired.


### lte.check_ppp()

Function will raise an exception if the modem is in active ppp mode.


### lte.ifconfig()

Function will return a tuple of IP address information from the PPP stack

### lte.imei()

Function will return the IMEI  (International Mobile Equipment Identity) number from the LTE module

### lte.iccid()

Function will return the SIM card ICCID (Integrated Circuit Card Identification number). This function will check if a SIM card is present.

## Constants

-   `LTE.CATM1`  : For use in CATM1 mode
    
-   `LTE.NBIOT`  : For use in NBIOT mode
    


> Written with [StackEdit](https://stackedit.io/).
