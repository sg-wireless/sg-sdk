

import can


can.init(RxPin,TxPin,Baud,Mode)

CanFilter(0,0xffffffff,True)
if can.any():
	CanRecBytes = can.recv();

SetCanFlags(extd,rtr,ss,self_rev,dlc_non_comp)

can.send(CanSendFlags,CanSendId,CanSendBytes)


