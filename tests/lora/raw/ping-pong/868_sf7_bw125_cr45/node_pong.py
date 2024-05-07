

import lora
import time

lora.initialize()

# switch to lora raw if not there
lora.mode(lora._mode.RAW)

lora.radio_params(reset_all=True)
lora.radio_params(
    region=lora._region.REGION_EU868,
    tx_power = 10,
    sf = 7,
    bandwidth = lora._bw.BW_125KHZ,
    coding_rate = lora._cr.CODING_4_5,
    preamble = 8,
    tx_timeout = 5000,
    tx_iq = False,
    rx_timeout = 5000,
    rx_iq = False
)
lora.stats()

# define the callback
ping_received = False
def lora_callback(event, bytes):
    global ping_received
    if event == lora._event.EVENT_RX_DONE:
        print(bytes['data'])
        msg = bytes['data'].decode("utf-8")
        if msg == 'PING':
            print(msg)
            lora.send('PONG')
            ping_received = True
    pass

lora.callback(handler=lora_callback)

# start continuous reception
lora.recv_cont_start()

while ping_received == False:
    time.sleep(1)

lora.recv_cont_stop()
lora.deinit()
