# ---------------------------------------------------------------------------- #
# global imports
# ---------------------------------------------------------------------------- #
import lora
import time

# ---------------------------------------------------------------------------- #
# rx part
# ---------------------------------------------------------------------------- #
total = 0
counter = 0
h_file = None
def lora_rx_callback(event, evt_data):
    global total
    global h_file
    global counter
    if event == lora._event.EVENT_RX_DONE:
        if len(evt_data) == 3 and evt_data == b'EOF':
            print('---- file received ----')
            h_file.close()
            lora.recv_cont_stop()
        else:
            total += len(evt_data)
            print(f'rx chunk # {counter:3d} received: {total}')
            counter += 1
            h_file.write(evt_data)
            time.sleep(0.1)
            lora.send('ACK', sync=True)
        pass
    elif event == lora._event.EVENT_RX_FAIL:
        print('-- rx failed')
    elif event == lora._event.EVENT_TX_FAILED:
        print('-- tx failed')
    elif event == lora._event.EVENT_TX_DONE:
        pass
    else:
        print('error: unknown error')

def demo_rx_file(filename, sf=7, bw=lora._bw.BW_250KHZ, payload=200):
    global h_file
    global counter
    global total
    total = 0
    counter = 0
    lora.mode(lora._mode.RAW)
    lora.radio_params(
        region=lora._region.REGION_EU868,
        sf=sf,
        bandwidth=bw,
        payload=payload
    )
    lora.stats()
    print(f'--> open file for writing {filename}')
    h_file = open(filename, 'wb')
    print('--> device is in lora tx/rx mode now')
    lora.callback(handler = lora_rx_callback)
    lora.recv_cont_start()

# ---------------------------------------------------------------------------- #
# tx part
# ---------------------------------------------------------------------------- #
def send_new_chunk():
    global counter
    global h_file
    global total
    buf = h_file.read(200)
    if buf:
        total += len(buf)
        print(f'tx chunk # {counter:3d} sent: {total}')
        counter += 1
        lora.send(buf, sync=True)
    else:
        lora.send('EOF', sync=True)
        print('--- file ended ---')
        h_file.close()
        lora.recv_cont_stop()

def lora_tx_callback(event, evt_data):
    if event == lora._event.EVENT_RX_DONE:
        if evt_data == b'ACK':
            time.sleep(0.1)
            send_new_chunk()
        pass
    elif event == lora._event.EVENT_TX_DONE:
        pass
    elif event == lora._event.EVENT_RX_FAIL:
        print('-- rx failed')
    elif event == lora._event.EVENT_TX_FAILED:
        print('-- tx failed')
    else:
        print('error: unknown error')


def demo_tx_file(filename, sf=7, bw=lora._bw.BW_250KHZ, payload=200):
    global h_file
    global counter
    global total
    total = 0
    counter = 0
    lora.mode(lora._mode.RAW)
    lora.radio_params(
        region=lora._region.REGION_EU868,
        sf=sf,
        bandwidth=bw,
        payload=payload,
        crc_on=True
    )
    lora.stats()
    print(f'--> open file for reading {filename}')
    h_file = open(filename, 'rb')
    print('--> device is in lora tx/rx mode now')
    lora.callback(handler = lora_tx_callback)
    lora.recv_cont_start()
    send_new_chunk()

def demo_cancel():
    lora.recv_cont_stop()
    h_file.close()
