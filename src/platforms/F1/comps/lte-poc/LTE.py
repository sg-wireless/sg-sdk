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
# Author    Christian Ehlers (SG Wireless)
#
# Desc      LTE helper library for ctrl client
# ---------------------------------------------------------------------------- #

import sys
import time
import ioexp
from machine import UART
from network import PPP

ctrl_enabled = globals().get('ctrl') is not None


class LTE():
    CATM1 = const(0)
    NBIOT = const(1)

    lte_ppp = None
    lte_uart = None
    in_ppp = None

    def __init__(
            self, carrier='standard', cid=1, mode=None, baudrate=115200,
            debug=None):
        self.__ppp_suspended = False
        self.__carrier = carrier
        self.__cid = cid
        self.__debug = debug
        self.__power = ioexp.lte_power()

        if LTE.lte_uart is None:
            LTE.lte_uart = UART(
                1, baudrate=baudrate, rx=48, tx=47, rts=33, cts=6,
                flow=(UART.CTS | UART.RTS),
                timeout=10)

        if not self.__power:
            if self.__debug: print('LTE power on')
            self.power_on(wait_ok=True)
            LTE.in_ppp = False

        if LTE.lte_ppp is None:
            LTE.lte_ppp = PPP(LTE.lte_uart)

        if LTE.in_ppp is None:
            if not self.__ppp_suspend():
                raise OSError('Modem initialization failed!')

    def init(self, carrier='standard', cid=1, mode=None, baudrate=115200,
                debug=None):
        self.__init__(carrier, cid, mode, baudrate, debug)

    def check_power(self):
        if not self.__power:
            raise OSError('Modem is not powered on!')

    def print_pretty_response(self, rsp, flush=False, prefix=None):
        if prefix is not None:
            print(prefix, end=' ')
        lines = rsp.decode('ascii').split('\r\n')
        for line in lines:
            if 'OK' not in line and line != '':
                print(line)

    def return_pretty_response(self, rsp):
        ret_str = ''
        lines = rsp.decode('UTF-8').split('\r\n')
        for line in lines:
            if 'OK' not in line and len(line) > 0:
                ret_str += (line + '\n')
        return ret_str

    def read_rsp(self, size=None, timeout=-1, wait_ok_error=False,
                 check_error=False):
        self.check_power()
        time.sleep(.25)
        if timeout < 0:
            timeout = sys.maxsize
        elif timeout is None:
            timeout = 0
        rsp = b''
        if wait_ok_error:
            while ('OK' not in rsp and 'ERROR' not in rsp):
                while not LTE.lte_uart.any() and timeout > 0:
                    time.sleep_ms(1)
                    timeout -= 1
                if self.__debug:
                    print('rsp before: {}'.format(rsp))
                if size is not None:
                    new = LTE.lte_uart.read(size)
                    if rsp is not None and new is not None:
                        rsp += new
                    elif new is not None:
                        rsp = new
                else:
                    new = LTE.lte_uart.read()
                    if rsp is not None and new is not None:
                        rsp += new
                    elif new is not None:
                        rsp = new
                if self.__debug:
                    print('rsp after: {}'.format(rsp))
        else:
            while LTE.lte_uart.any():
                while not LTE.lte_uart.any() and timeout > 0:
                    time.sleep_ms(1)
                    timeout -= 1
                if self.__debug:
                    print('rsp before: {}'.format(rsp))
                if size is not None:
                    new = LTE.lte_uart.read(size)
                    if rsp is not None and new is not None:
                        rsp += new
                    elif new is not None:
                        rsp = new
                else:
                    new = LTE.lte_uart.read()
                    if rsp is not None and new is not None:
                        rsp += new
                    elif new is not None:
                        rsp = new
                if self.__debug:
                    print('rsp after: {}'.format(rsp))
                time.sleep_ms(5)
        if rsp is not None:
            if self.__debug:
                print('rsp return: {}'.format(rsp))
            if check_error and 'ERROR' in rsp:
                raise OSError('AT command returned ERROR!')
            return rsp
        else:
            return b''

    def send_at_cmd(self, cmd='AT', timeout=-1, wait_ok_error=False,
                    check_error=False):
        self.check_power()
        self.check_ppp()
        LTE.lte_uart.flush()
        if self.__debug:
            print('AT: {}'.format(cmd))
        LTE.lte_uart.flush()
        LTE.lte_uart.write(cmd + '\r\n')
        return self.return_pretty_response(
            self.read_rsp(
                timeout=timeout, wait_ok_error=wait_ok_error,
                check_error=check_error))

    def attach(self, apn=None, type='IP', cid=None, band=None, bands=None):
        self.check_power()
        self.check_ppp()
        # Check if we are in CFUN=1 then return because we're already
        # attaching or attached.
        if cid is not None:
            self.__cid = cid
        if band is not None and bands is not None:
            raise ValueError("Cannot specify both band and bands")
        if not self.check_sim_present():
            raise OSError('SIM card not present or PIN protected!')
        if band is not None:
            mode = self.__get_mode()
            resp = self.send_at_cmd(
                'AT+SQNBANDSEL={},"{}","{}"'.format(mode,
                                                    self.__carrier, band),
                check_error=True)
            if self.__debug:
                print(resp)
        if bands is not None:
            mode = self.__get_mode()
            band_str = ""
            for band in bands:
                band_str += "{},".format(band)
            resp = self.send_at_cmd(
                'AT+SQNBANDSEL={},"{}","{}"'.format(mode, self.__carrier,
                                                    band_str), check_error=True)
            if self.__debug:
                print(resp)
        if apn is not None:
            resp = self.send_at_cmd(
                'AT+CGDCONT={},"{}","{}"'.format(self.__cid, type, apn),
                check_error=True)
            if self.__debug:
                print(resp)
        resp = self.send_at_cmd('AT+CFUN=1', check_error=True)
        if self.__debug:
            print(resp)

    def isattached(self):
        self.check_power()
        if LTE.in_ppp:
            return True
        self.check_ppp()
        resp = self.send_at_cmd('AT+CEREG?', check_error=True)
        if self.__debug:
            print(resp)
        if '+CEREG: 1,1' in resp or '+CEREG: 1,5' in resp:
            return True
        if '+CEREG: 2,1' in resp or '+CEREG: 2,5' in resp:
            return True
        resp = self.send_at_cmd('AT+CFUN?', check_error=True)
        if "+CFUN: 1" not in resp:
            self.send_at_cmd('AT+CFUN=1', check_error=True)
        return False

    def reset(self):
        self.check_power()
        self.check_ppp()
        LTE.lte_uart.flush()
        LTE.lte_uart.write('AT^RESET\r\n')
        for _ in range(10):
            resp = self.read_rsp(wait_ok_error=True)
            if self.__debug:
                print('reset_resp look_for +SHUTDOWN: {}'.format(resp))
            if b'+SHUTDOWN' in resp:
                break
            time.sleep(.25)
        resp = b''
        for _ in range(20):
            resp = self.read_rsp()
            if self.__debug:
                print('reset_resp wait_for +SYSSTART: {}'.format(resp))
            if b'+SYSSTART' in resp:
                break
            time.sleep(.25)
        for _ in range(25):
            if self.__debug:
                print('reset_resp wait_for AT/OK {}'.format(resp))
            LTE.lte_uart.write('AT\r\n')
            time.sleep(.1)
            resp = self.read_rsp()
            if b'OK' in resp:
                break
            time.sleep(.25)
        else:
            raise OSError('The modem did not respond with +SHUTDOWN')

    def connect(self, cid=None):
        self.check_power()
        self.check_ppp()
        if cid is not None:
            self.__cid = cid
        if not self.is_attached():
            raise OSError('Modem is not attached to a network!')

        # Due to possible issues with the ATO command in LR8.2.0.2-59200
        # this is currently disabled

        # resp = self.send_at_cmd('ATO'.format(self.__cid))
        # if self.__debug:
        #     print(resp)
        # if not 'CONNECT' in resp:
        if True:
            time.sleep(.1)
            resp = self.send_at_cmd('AT+CGDATA="PPP",{}'.format(self.__cid))
            for _ in range(25):
                if self.__debug:
                    print(resp)
                if 'CONNECT' in resp:
                    break
                elif 'ERROR' in resp:
                    time.sleep(1)
                    resp = self.send_at_cmd(
                        'AT+CGDATA="PPP",{}'.format(self.__cid))
                else:
                    resp = self.return_pretty_response(self.read_rsp())
                    time.sleep(.25)
            time.sleep(.1)
        # LTE.lte_ppp.active(True) hangs when switching from False to
        # True unless LTE.lte_ppp = PPP(LTE.lte_uart) is used first
        LTE.lte_ppp = PPP(LTE.lte_uart)
        LTE.lte_ppp.active(True)
        LTE.lte_ppp.connect()
        LTE.in_ppp = True

    def __get_mode(self):
        try:
            result = self.send_at_cmd('AT+SQNMODEACTIVE?', check_error=True)
            if '+SQNMODEACTIVE:' in result:
                mode = int(result.split(':')[1].strip())

                return (mode - 1)
        except:
            return None

    def mode(self, new_mode=None):
        self.check_power()
        self.check_ppp()
        if new_mode is None:
            return self.__get_mode()
        else:
            current_mode = self.__get_mode()
            if (int(new_mode) != int(current_mode)):
                self.send_at_cmd('AT+CFUN=0', check_error=True)
                try:
                    self.send_at_cmd(
                        'AT+SQNMODEACTIVE={}'.format(str(int(new_mode) + 1)),
                        wait_ok_error=True, check_error=True)
                    self.reset()
                except Exception as ex:
                    if self.__debug:
                        sys.print_exception(ex)
                    print('Error switching operating mode!')

    def ifconfig(self):
        if LTE.in_ppp:
            return LTE.lte_ppp.ifconfig()

    def disconnect(self):
        self.check_power()
        if ctrl_enabled:
            raise OSError('Please use ctrl.disconnect() instead!')
        if LTE.in_ppp:
            LTE.lte_ppp.active(False)
            self.__ppp_suspend()
            self.send_at_cmd('ATH', wait_ok_error=True, check_error=True)

    def isconnected(self):
        if LTE.in_ppp:
            return self.lte_ppp.isconnected()
        else:
            return False

    def is_connected(self):
        return self.isconnected()

    def is_attached(self):
        return self.isattached()

    def detach(self):
        self.disconnect()
        resp = self.send_at_cmd('AT+CFUN?', wait_ok_error=True,
                                check_error=True)

        if "+CFUN: 1" in resp:
            # According to Sequans, should go to CFUN=4 first before going
            # to CFUN=0 when disconnecting from the network. This is to ensure
            # SIM card remains writable during detach
            self.send_at_cmd('AT+CFUN=4', wait_ok_error=True, check_error=True)
            self.send_at_cmd('AT+CFUN=0', wait_ok_error=True, check_error=True)
        if "+CFUN: 4" in resp:
            self.send_at_cmd('AT+CFUN=0', wait_ok_error=True, check_error=True)


    def deinit(self, reset=False):
        if LTE.in_ppp:
            self.disconnect()
        self.detach()
        self.power_off()

    def check_sim_present(self):
        self.check_ppp()
        # Check first if we're in CFUN=1, then we don't need to switch
        # to CFUN=4 to read the SIM card
        resp = self.send_at_cmd('AT+CFUN?', check_error=True)
        if "+CFUN: 1" not in resp:
            resp = self.send_at_cmd(
                'AT+CFUN=4', wait_ok_error=True, check_error=True)
        for _ in range(5):
            time.sleep(.25)
            resp = self.send_at_cmd('AT+CPIN?', wait_ok_error=True)
            if ("+CPIN: READY" in resp):
                return True
        return False

    def power_on(self, wait_ok=True):
        ioexp.lte_power(True)
        self.__power = True
        if wait_ok:
            time.sleep(1)
            self.send_at_cmd(wait_ok_error=True)

    def power_off(self, force=False):
        self.check_power()

        if LTE.lte_ppp.active():
            LTE.lte_ppp.active(False)

        if not force:
            if LTE.in_ppp:
                if not self.__ppp_suspend():
                    raise OSError('communication error! Use force=True')
            self.detach()
        ioexp.lte_power(False)
        self.__power = False

    def pause_ppp(self):
        self.check_power()
        if self.__debug:
            print('This is pause_ppp...')
        self.check_power()
        if ctrl_enabled:
            raise OSError('Command not available when CTRL is enabled!')
        LTE.lte_ppp.active(False)
        if self.__ppp_suspend():
            self.__ppp_suspended = True
        else:
            raise OSError('LTE modem communication failed!')

    def __ppp_suspend(self, wait_at=True):
        resp = b''
        for _ in range(5):
            LTE.lte_uart.flush()
            if self.__debug:
                print('Writing +++')
            LTE.lte_uart.write('+++')
            time.sleep(1)
            if wait_at and self.__wait_at(25, 250):
                LTE.in_ppp = False
                return True
            else:
                return True
        return False

    def __wait_at(self, attempts, sleep_ms):
        for counter in range(attempts):
            # LTE.lte_uart.flush()
            LTE.lte_uart.write('AT\r\n')
            time.sleep_ms(sleep_ms)
            resp = self.read_rsp()
            if self.__debug:
                print("AT#{}: {}".format(counter, resp))
            if b'OK' in resp:
                return True
        return False

    def resume_ppp(self):
        self.check_power()
        if self.__ppp_suspended:
            resp = self.send_at_cmd(
                    'ATO0',
                    timeout=9500,
                    wait_ok_error=True,
                    check_error=False)
            if 'CONNECT' in resp:
                self.__ppp_suspended = False
                # LTE.lte_ppp.active(True) hangs when switching from False to
                # True unless LTE.lte_ppp = PPP(LTE.lte_uart) is used first
                LTE.lte_ppp = PPP(LTE.lte_uart)
                LTE.lte_ppp.active(True)
                LTE.lte_ppp.connect()
                return
        self.__ppp_suspended = False
        raise OSError("PPP session not in suspend state or resume failed!")

    def check_ppp(self):
        if LTE.in_ppp and not self.__ppp_suspended:
            raise OSError("Operation not possible while in PPP mode!")

    def imei(self):
        resp = self.send_at_cmd('AT+CGSN=1')
        if "CGSN:" in resp:
            try:
                return(resp.split('\"')[1])
            except:
                raise OSError('Unable to parse IMEI command response!')

    def iccid(self):
        if not self.check_sim_present():
            raise OSError('SIM card not present or PIN protected!')
        resp = self.send_at_cmd('AT+SQNCCID?')
        if "+SQNCCID:" in resp:
            try:
                return(resp.split('\"')[1])
            except:
                raise OSError('Unable to parse ICCID command response!')
