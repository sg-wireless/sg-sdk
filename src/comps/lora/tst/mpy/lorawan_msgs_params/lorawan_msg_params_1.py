# --- test options ----------------------------------------------------------- #
enable_commissioning = False
test_duty_cycle = 7000
open_ports = [1, 2, 7]           # ports to be opened
special_ports_callbacks = [2, 7, 5, 3] # ports that have a spetial callbacks
full_report = False

# --- test vector ------------------------------------------------------------ #
# message sending options are:
#   port=<int>          default is 1
#   id=<int>            default is 0
#   confirm=<Boolean>   default is False
#   timeout=<int>       default is 0 --> no timeout
#   retries=<int>       default is 0 --> single retry
#   sync=<Boolean>      default is False
test_msgs = [
    {'port':1,'id':1001,'timeout': 3000,'confirm':True ,'sync':False},
    {'port':1,'id':1002,'timeout': 1000,'confirm':True ,'sync':False},
    {'port':2,'id':2001,'timeout':    0,'confirm':True ,'sync':False},
    {'port':1,'id':1003,'timeout': 7000,'confirm':True ,'sync':False},
    {'port':5,'id':5001,'timeout':    0,'confirm':False,'sync':True },
    {'port':7,'id':7001,'timeout':10000,'confirm':False,'sync':True },
]
