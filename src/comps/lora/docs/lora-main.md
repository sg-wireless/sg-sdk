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
 ! @brief   LoRa documentation main file.
 !----------------------------------------------------------------------------->

<!------------------------------------------------------------------------------
 ! Header and TOC
 !----------------------------------------------------------------------------->
# LoRa API Documentation

## Contents

* [Initialization](#init)
* [LoRa Modes](#modes)
* [LoRa Test Stub](#test-stub)
* [LoRa Events and Callback](#lora-callback)
* [LoRa RAW APIs](#lora-raw)
* [LoRa WAN APIs](#lora-wan)
* [LoRa Certification Mode](#cert-mode)

<!------------------------------------------------------------------------------
 ! Initialization
 !----------------------------------------------------------------------------->
<div id="init"></div>

## Initialization

To initialize the LoRa stack, you need to do `import lora` to be able to call
any lora utility and to automatically initialize the saved operating lora mode:

```python
import lora     # mandatory before any lora function call
                # automatically initialize lora for current operating mode

lora.deinit()   # deinit the stack
                # all lora calls will be ignored after it

lora.lora.initialize()
                # initialize the stack again and back to normal operation
```

<!------------------------------------------------------------------------------
 ! LoRa Modes
 !----------------------------------------------------------------------------->
<div id="modes"></div>

## LoRa Modes
There are two available modes for lora; *LoRa-RAW* and *LoRa-WAN*.
```python
lora.mode()                 # displays the current operating LoRa mode
lora.mode(lora._mode.RAW)   # switch mode to LoRa-RAW
lora.mode(lora._mode.WAN)   # switch mode to LoRa-WAN
```

<!------------------------------------------------------------------------------
 ! LoRa Test stub
 !----------------------------------------------------------------------------->
<div id="test-stub"></div>

## LoRa Test stub
In case of testing and no need to connect a user level callback, lora-stack
provides an internal stub for callbacks to be used while testing.

```python
lora.callback_stub_connect()    # connect the internal lora-stack callback stub
lora.callback_stub_disconnect() # to disconnect it and connect user provided one
```

<!------------------------------------------------------------------------------
 ! LoRa Events and Callbacks
 !----------------------------------------------------------------------------->
<div id="lora-callback"></div>

## LoRa Events and Callbacks

LoRa events and Callback system are explained [here](lora-callback.md)

<!------------------------------------------------------------------------------
 ! LoRa RAW APIs
 !----------------------------------------------------------------------------->
<div id="lora-raw"></div>

### LoRa RAW APIs

Lora RAW mode API documentation can be found [here](lora-raw.md)

<!------------------------------------------------------------------------------
 ! LoRa WAN APIs
 !----------------------------------------------------------------------------->
<div id="lora-wan"></div>

### LoRa WAN APIs

Lora RAW mode API documentation can be found [here](lora-wan.md)

<!------------------------------------------------------------------------------
 ! LoRa Certification Mode
 !----------------------------------------------------------------------------->
<div id="cert-mode"></div>

### LoRa Certification Mode

LoRa Certification Mode documentation can be found [here](lora-lctt.md)

<!--- end of file ------------------------------------------------------------->
