# CAN API Documentation

This document describes the CAN API functions

## Functions

### `can.init`

Initializes the CAN interface with the specified parameters.
- **Parameters**:
  - `RxPin` (default: 1): Receive pin number.
  - `TxPin` (default: 2): Transmit pin number.
  - `Baud` (default: 250000): Baud rate.(2500,50000,1000000,125000,250000,500000,800000,1000000).
  - `Mode` (default: 0): CAN mode  (e.g., 0-NORMAL, 1-NO ACK，2-LISTEN_ONLY).

- **Usage**:
python can.init(RxPin, TxPin, Baud, Mode)
### `can.deinit`

Deinitializes the CAN interface.
- **Usage**:
python can.deinit()
### `can.send`

Sends a CAN message.
- **Parameters**:
  - `flags`: Message flags.
  - `id`: Message identifier.
  - `dat`: Data to send (string or bytes).

- **Usage**:
python can.send(flags, id, dat)
### `can.filter`

Sets a CAN filter.
- **Parameters**:
  - `dat`: Filter data (string or bytes).
  - `single_filter`: Boolean indicating if a single filter is used.

- **Usage**:
python can.filter(dat, single_filter)
### `can.any`

Checks if any CAN messages are available.
- **Returns**: Integer indicating the presence of messages.

- **Usage**:
python can.any()
### `can.recv`

Receives a CAN message.
- **Returns**: Bytes representing the received message.

- **Usage**:
python can.recv()
## Example
```python

from machine import Pin,UART,Timer,I2C,WDT
from micropython import const
import machine
import time
import can
import binascii
import struct

RxPin=39
TxPin=38
Baud=100000
Mode=0


SendDelay=0
CanSendFlags=0#Extended frame
CanSendId=0
CanSendBuf=''
CanRecBytes=bytes()
CanSendBytes=bytes()
T1S=0

def SetCanFlags(extd,rtr,ss,self_rev,dlc_non_comp):
    global CanSendFlags
    CanSendFlags=0
    if extd:#0-standard frame; 1- Extended frame
        CanSendFlags |= 0x01
    if rtr:#0- data frame,1-Remote Frame
        CanSendFlags |= 0x02
    if ss:#0-Error resend; 1-Single send
        CanSendFlags |= 0x04
    if self_rev:#0- Do not receive messages sent by oneself, 1- Receive messages sent by oneself
        CanSendFlags |= 0x08
    if dlc_non_comp:#0-Data length not exceeding 8 (ISO 11898-1); 1- Data length greater than 8 (non-standard)
        CanSendFlags |= 0x10
    
def time0_irq(time0):
    global CanSendFlags,CanSendId,SendDelay,T1S,CanRecBuf
    if T1S<1000:
      T1S+=1
    else:
      T1S=0
    if SendDelay:
        SendDelay-=1
        if SendDelay==0:
            CanSendFlags = int.from_bytes(CanRecBytes[0:4], 'little')
            CanSendId= int.from_bytes(CanRecBytes[4:8], 'little')
            len = CanRecBytes[8]
            #CanSendBuf = CanRecBytes[9:9+len].decode('utf-8')
            CanSendBytes = CanRecBytes[9:9+len]
            #print(CanSendBuf)
            #can.send(CanSendId,CanSendBuf)
            can.send(CanSendFlags,CanSendId,CanSendBytes)
            #can.send(0x123,'12345678')
def CanFilter(acceptance_code,acceptance_mask,single_filter):
    dat = acceptance_code.to_bytes(4,'little')
    dat += acceptance_mask.to_bytes(4,'little')
    can.filter(dat,single_filter)


def setup():
    CanFilter(0,0xffffffff,True)
    #CanFilter(0xe00001,0x1ffffe,True)
    #CanFilter(0xe00000,0x1fffff,False)
    #CanFilter(0xe0,0x1f,False)
    can.init(RxPin,TxPin,Baud,Mode)
    time0=Timer(0)
    time0.init(period=1,mode=Timer.PERIODIC,callback=time0_irq)

def loop():
    global SendDelay,CanRecBuf,CanRecBytes
    print('loop')
    while(1):
        if can.any():
            CanRecBytes = can.recv();
            print(CanRecBytes.hex(' '))
            SendDelay=200

if __name__=="__main__":
    setup()
    machine.lightsleep(10)
    loop()
```
