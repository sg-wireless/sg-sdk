# ---------------------------------------------------------------------------- #
# Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
# copies  of  the  Software,  and  to  permit  persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Author    Ahmed Sabry (SG Wireless)
#
# Desc      Implements simple lora OTAA activation and sending to Pybytes.
# ---------------------------------------------------------------------------- #

# --- test options ----------------------------------------------------------- #

try:
    enable_commissioning
except:
    enable_commissioning = True

try:
    test_duty_cycle
except:
    test_duty_cycle = 7000

try:
    open_ports
except:
    open_ports = [1, 2, 5, 7]           # ports to be opened

try:
    special_ports_callbacks
except:
    special_ports_callbacks = [2, 5, 3] # ports that have a spetial callbacks

try:
    full_report
except:
    full_report = False

# --- test vector ------------------------------------------------------------ #
# message sending options are:
#   port=<int>          default is 1
#   id=<int>            default is 0
#   confirm=<Boolean>   default is False
#   timeout=<int>       default is 0 --> no timeout
#   retries=<int>       default is 0 --> single retry
#   sync=<Boolean>      default is False
try:
    test_msgs
except:
    test_msgs = [
    {'port':1,'id':1001,'timeout': 3000,'confirm':True ,'sync':False},
    {'port':1,'id':1002,'timeout': 1000,'confirm':True ,'sync':False},
    {'port':2,'id':2001,'timeout':    0,'confirm':True ,'sync':False},
    {'port':1,'id':1003,'timeout': 7000,'confirm':True ,'sync':False},
    {'port':5,'id':5001,'timeout':    0,'confirm':False,'sync':True },
    {'port':1,'id':1004,'timeout':20000,'confirm':True ,'sync':False},
    {'port':1,'id':1005,'timeout':    0,'confirm':True ,'sync':True },
    {'port':3,'id':3001,'timeout':30000,'confirm':False,'sync':False},
    {'port':5,'id':5002,'timeout':30000,'confirm':False,'sync':False},
    {'port':1,'id':1006,'timeout':60000,'confirm':False,'sync':True },
    {'port':5,'id':5003,'timeout':60000,'confirm':False,'sync':False},
    {'port':1,'id':1007,'timeout':90000,'confirm':True ,'sync':True },
    {'port':5,'id':5004,'timeout':90000,'confirm':True ,'sync':True },
]

# --- test options ----------------------------------------------------------- #
# disable fw logs
# import logs
# logs.filter_subsystem('lora', False)
import lora
import time
import _thread

# switch to lora raw if not there

def do_commission():
    import ubinascii
    # start commisioning
    # the following credits used while testing over TTN
    dev_eui  = ubinascii.unhexlify('70B3D57ED005BEB3')
    join_eui = ubinascii.unhexlify('0000000000000000')
    app_key  = ubinascii.unhexlify('F2F8EAAEAE2C0D9366D202AC043C870B')
    nwk_key  = ubinascii.unhexlify('5CF7E351911784C74C761926108041C9')
    lora.commission(type=lora._commission.OTAA, DevEUI=dev_eui, \
        JoinEUI=join_eui, AppKey=app_key, NwkKey=nwk_key)
    lora.join()
    while lora.is_joined() == False:
        pass

# --- ports handling --------------------------------------------------------- #

def init_test_ports():
    for p in open_ports:
        lora.port_open(p)
def port_is_opened(port):
    for p in open_ports:
        if p == port:
            return True
    return False
def port_has_special_callback(port):
    for p in special_ports_callbacks:
        if p == port:
            return True
    return False

# attach required callback
def get_event_str(event):
    if event == lora._event.EVENT_TX_CONFIRM:
        return 'EVENT_TX_CONFIRM'
    elif event == lora._event.EVENT_TX_DONE:
        return 'EVENT_TX_DONE'
    elif event == lora._event.EVENT_TX_TIMEOUT:
        return 'EVENT_TX_TIMEOUT'
    elif event == lora._event.EVENT_TX_FAILED:
        return 'EVENT_TX_FAILED'
    elif event == lora._event.EVENT_RX_DONE:
        return 'EVENT_RX_DONE'
    elif event == lora._event.EVENT_RX_TIMEOUT:
        return 'EVENT_RX_TIMEOUT'
    elif event == lora._event.EVENT_RX_FAIL:
        return 'EVENT_RX_FAIL'
    else:
        return 'UNKNOWN'

def is_tx_event(evt):
    return evt == lora._event.EVENT_TX_CONFIRM or   \
            evt == lora._event.EVENT_TX_DONE or     \
            evt == lora._event.EVENT_TX_TIMEOUT or  \
            evt == lora._event.EVENT_TX_FAILED

def is_rx_event(evt):
    return evt == lora._event.EVENT_RX_DONE or      \
            evt == lora._event.EVENT_RX_TIMEOUT or  \
            evt == lora._event.EVENT_RX_FAIL

def main_callback(port, event, data):
    # print('PORT-{}:: lora event [ {} ] --> data: {}'
    #     .format( 'X' if port == 0xff else port, get_event_str(event), data))
    if is_tx_event(event):
        info = get_msg_info(port, data)
        if not port_is_opened(port):
            info['verdict'] = False
            info['cause'] = 'port is closed'
        if port_has_special_callback(port) and port != info['port']:
            info['verdict'] = False
            info['cause'] = 'callback in wrong function'
        if info['timeout'] != 0:
            ts = time.time()
            to = info['ts'] + info['timeout']//1000
            if event == lora._event.EVENT_TX_TIMEOUT and ts != to:
                info['verdict'] = False
                info['cause'] = 'incorrect time of timeout event ts({})~to({})'\
                    .format(ts, to)
            if event != lora._event.EVENT_TX_TIMEOUT and ts > to:
                info['verdict'] = False
                info['cause'] = 'event occurred out of allowed timeout ' + \
                    'ts({})~to({})'.format(ts, to)
        info['verdict'] = True
        info['event'] = event

def port_any_cb(event, data):
    main_callback(0xff, event, data)

def special_port_callback(port):
    def a(event, data):
        main_callback(port, event, data)
    return a

def init_ports_callbacks():
    lora.callback(handler=port_any_cb)
    for p in special_ports_callbacks:
        lora.callback(handler=special_port_callback(p) , port=p)

def get_msg_info(port, id) -> dict:
    for info in test_msgs:
        if (port == 0xff or port == info['port']) and id == info['id']:
            return info
    return None

# --- main test proc --------------------------------------------------------- #

def thread_exec_sync_msg(port, id, timeout, confirm, retries):
    ts1 = time.time()
    str = 'msg on port {}, id({}), timeout({}), confirm({}), sync({})'. \
        format(port, id, timeout, confirm, True)
    lora.send(str, port=port, id=id, timeout=timeout, confirm=confirm,
        retries=retries, sync=True)
    info = get_msg_info(port, id)
    if timeout:
        ts2 = time.time()
        to  = timeout//1000 + ts1
        if ts2 > to:
            info['verdict'] = False
            info['cause'] = 'message sync after allowed timeout'
    info['synced'] = True

def print_table_header(table_fmt):
    fmt_str = ''
    tot_w = 1
    for col in table_fmt:
        desc = col[1]
        tot_w += 3
        if 'w' in desc:
            tot_w += desc['w']
        else:
            tot_w += len(desc['cap'])
    print('-' * tot_w)
    for col in table_fmt:
        desc = col[1]
        fmt_str = '| {'
        if 'w' in desc:
            fmt_str += ':^' + str(desc['w']) + 's'
        fmt_str += '} '
        print(fmt_str.format(desc['cap']), end='')
    print('|')
    print('-' * tot_w)

def print_table_raw(table_fmt, raw_dict):
    for col in table_fmt:
        col_name = col[0]
        desc = col[1]
        fmt_str = '| {'
        if desc['type'] == 's':     # string
            if 'w' in desc:
                fmt_str += ':^' + str(desc['w']) + 's'
            else:
                fmt_str += ':^' + str(len(desc['cap'])) + 's'
            fmt_str += '} '
            if col_name in raw_dict:
                val = raw_dict[col_name]
            else:
                val = ''
            print(fmt_str.format(val), end='')
        elif desc['type'] == 'd':    # integer
            if 'w' in desc:
                w = desc['w']
            else:
                w = len(desc['cap'])
            if col_name in raw_dict:
                fmt_str += ':' + str(w) + 'd} '
                print(fmt_str.format(raw_dict[col_name]), end='')
            else:
                fmt_str += ':' + str(w) + 's} '
                print(fmt_str.format(''), end='')
        elif desc['type'] == 'b':    # Boolean
            if 'w' in desc:
                w = desc['w']
            else:
                w = len(desc['cap'])
            if col_name in raw_dict:
                fmt_str += ':' + str(w) + 's} '
                val = 'True' if raw_dict[col_name] else 'False'
                print(fmt_str.format(val), end='')
            else:
                fmt_str += ':' + str(w) + 's} '
                print(fmt_str.format(''), end='')
    print('|')

def print_verdict():
    table_fmt = [
        ('verdict'  , { 'w' :  7,   'type': 's',    'cap' : 'verdict'   }),
        ('event'    , { 'w' : 18,   'type': 's',    'cap' : 'event'     }),
        ('port'     , { 'w' :  4,   'type': 'd',    'cap' : 'port'      }),
        ('id'       , { 'w' :  6,   'type': 'd',    'cap' : 'msg-id'    }),
        ('timeout'  , { 'w' :  7,   'type': 'd',    'cap' : 'timeout'   }),
        ('confirm'  , { 'w' :  7,   'type': 'b',    'cap' : 'confirm'   }),
        ('sync'     , { 'w' :  6,   'type': 'b',    'cap' : 'sync'      }),
        ('cause'    , {             'type': 's',    'cap' : 'fail-cause'}),
    ]
    if not full_report:
        table_fmt.pop(1)

    print_table_header(table_fmt)
    for msg in test_msgs:
        port = msg['port']
        id   = msg['id']
        timeout = msg['timeout']
        confirm = msg['confirm']
        sync = msg['sync']
        try:
            verdict = msg['verdict']
        except:
            verdict = False if port_is_opened(port) else True
        try:
            event = msg['event']
        except:
            event = '--'
        cause = msg['cause'] if verdict == False else '--'
        try:
            synced = msg['synced']
        except:
            synced = False
        if sync and not synced:
            verdict = False
            cause = 'not synced'
        raw_info = {
            'verdict'   : 'PASS' if verdict else 'FAIL',
            'event'     : get_event_str(event),
            'port'      : port,
            'id'        : id,
            'confirm'   : confirm,
            'sync'      : sync,
            'cause'     : cause if verdict else '--'
        }
        if full_report:
            raw_info['event'] = get_event_str(event)
        if timeout:
            raw_info['timeout'] = timeout
        print_table_raw(table_fmt, raw_info)
    pass

def run(test_suite, ports, ports_with_special_cb, \
        duty=10000, rejoin=False, complete_report=False):
    global enable_commissioning
    global test_duty_cycle
    global open_ports
    global special_ports_callbacks
    global full_report
    global test_msgs

    test_msgs = test_suite
    full_report = complete_report
    enable_commissioning = rejoin
    test_duty_cycle = duty
    open_ports = ports
    special_ports_callbacks = ports_with_special_cb

    lora.initialize()
    lora.mode(lora._mode.WAN)
    if enable_commissioning:
        do_commission()
    lora.duty_set(test_duty_cycle)
    lora.duty_start()
    init_test_ports()
    init_ports_callbacks()

    for msg in test_msgs:
        msg['ts'] = time.time()
        str = 'msg on port {}'.format(msg['port'])
        str += ', id({})'.format(msg['id'])
        str += ', timeout({}'.format(msg['timeout'])
        str += ', confirm({}'.format(msg['confirm'])
        str += ', sync({}'.format(msg['sync'])
        str += ', timestamp({}'.format(msg['ts'])
        if not msg['sync']:
            lora.send(str, port = msg['port'], id = msg['id'],
                timeout = msg['timeout'], confirm = msg['confirm'],
                retries = 2, sync = msg['sync'])
        else:
            _thread.start_new_thread(thread_exec_sync_msg,
                (msg['port'], msg['id'], msg['timeout'],msg['confirm'], 2))

    while lora.is_tx_pending():
        time.sleep(5)

    print_verdict()

    lora.duty_stop()
    lora.deinit()

if __name__ == "__main__":
    run(test_msgs, open_ports, special_ports_callbacks,
        rejoin = enable_commissioning, duty = test_duty_cycle,
        complete_report = full_report)

# --- end of file ------------------------------------------------------------ #
