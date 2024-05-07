
import lora
import time

lora.initialize()

# switch to lora raw if not there
lora.mode(lora._mode.RAW)

lora.radio_params(reset_all=True)
lora.radio_params(
    region=lora._region.REGION_EU868,
    tx_power = 10,
    sf = 12,
    bandwidth = lora._bw.BW_250KHZ,
    coding_rate = lora._cr.CODING_4_6,
    preamble = 8,
    tx_timeout = 5000,
    tx_iq = False,
    rx_timeout = 5000,
    rx_iq = False
)
lora.stats()

# define the callback
pong_received = False
def lora_callback(event, bytes):
    global pong_received
    if event == lora._event.EVENT_RX_DONE:
        print(bytes['data'])
        msg = bytes['data'].decode("utf-8")
        if msg == 'PONG':
            print(msg)
            pong_received = True
    pass
lora.callback(handler=lora_callback)

# start continuous reception
lora.recv_cont_start()

time.sleep(2)

# start chating by sending
lora.send('PING')

while pong_received == False:
    time.sleep(1)

lora.recv_cont_stop()

lora.deinit()