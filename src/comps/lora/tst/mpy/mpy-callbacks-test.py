import lora
import lora_unittest

def get_evt_str(evt):
    if evt != None:
        if evt == lora._event.EVENT_TX_DONE:
            return 'EVENT_TX_DONE'
        elif evt == lora._event.EVENT_TX_TIMEOUT:
            return 'EVENT_TX_TIMEOUT'
        elif evt == lora._event.EVENT_TX_FAILED:
            return 'EVENT_TX_FAILED'
        elif evt == lora._event.EVENT_TX_CONFIRM:
            return 'EVENT_TX_CONFIRM'
        elif evt == lora._event.EVENT_RX_DONE:
            return 'EVENT_RX_DONE'
        elif evt == lora._event.EVENT_RX_TIMEOUT:
            return 'EVENT_RX_TIMEOUT'
        elif evt == lora._event.EVENT_RX_FAIL:
            return 'EVENT_RX_FAIL'
    return '--UNKNOWN-EVENT--'

def cb_on_any(event, evt_data):
    print('(  cb_on_any   )--> event: ' + get_evt_str(event))
    if evt_data != None:
        print(evt_data)
    pass

def cb_on_port_1(event, evt_data):
    print('( cb_on_port_1 )--> event: ' + get_evt_str(event))
    if evt_data != None:
        print(evt_data)
    pass

port_1_triggers = lora._event.EVENT_RX_DONE | lora._event.EVENT_TX_DONE
lora_unittest.test_callback_set(handler=cb_on_port_1, trigger=port_1_triggers, port=1)
lora_unittest.test_callback_set(handler=cb_on_any)
lora_unittest.test_gen_all_callbacks()
