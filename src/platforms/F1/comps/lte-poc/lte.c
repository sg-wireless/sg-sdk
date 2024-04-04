/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 * copies  of  the  Software,  and  to  permit  persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 * IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @author  Ahmed Sabry (SG Wireless) -- Translated LTE.py into C
 * 
 * @note    The purpose of this file it to identically translate the LTE.py
 *          script written by "Christian Ehlers" to C implementation.
 *
 * @brief   This file is a pure translation to "LTE.py" into C language
 * --------------------------------------------------------------------------- *
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "string.h"
#include "driver/gpio.h"

#define __log_subsystem     lte
#include "log_lib.h"
__log_subsystem_def(lte, default, 1, 0)

#include "lte.h"
#include "ioexp.h"
#include "lte_uart.h"
#include "lte_ppp.h"
#include "lte_helpers.h"

#define __hal_timestamp_ms()    (esp_timer_get_time() / 1000)
#define __hal_delay_ms(ms)  do{ vTaskDelay(ms/portTICK_PERIOD_MS); } while(0)

/** -------------------------------------------------------------------------- *
 * APIs translation as programmed in "LTE.py"
 * --------------------------------------------------------------------------- *
 */
// -- private variables and default values
static char*    s_carrier   = NULL;
static int      s_cid       = 1;
static bool     s_debug     = true;
static bool     s_power     = false;
static bool     s_ppp_suspended = false;
static bool     s_in_ppp    = false;

static bool     s_initialized = false;

#define __helper_str_buf_size       (256)
static char     s_helper_str_buf[__helper_str_buf_size];
static char     s_helper_str_buf_2[__helper_str_buf_size];

static bool __ppp_suspend(void);

lte_error_t lte_init(const char* carrier, uint32_t cid, lte_mode_t mode,
    uint32_t baudrate, bool debug)
{
    /*  LTE.py -> LTE.__init__()
     *  ------------------------
     *  def __init__(self, carrier='standard', cid=1, mode=None,
     *          baudrate=115200, debug=None):
     *      if debug is None:
     *          import sgw
     *          debug = sgw.nvs_get('lte_debug', 0) == 1
     *      self.__ppp_suspended = False
     *      self.__carrier = carrier
     *      self.__cid = cid
     *      self.__debug = debug
     *      self.power_on(wait_ok=False)
     *      if LTE.lte_uart is None:
     *          LTE.lte_uart = UART(1, baudrate=baudrate, rx=48, tx=47, rts=33,
     *              cts=6, flow=(UART.CTS | UART.RTS), timeout=10)
     *      self.__power = True
     *      if LTE.lte_ppp is None:
     *          LTE.lte_ppp = PPP(LTE.lte_uart)
     *      if LTE.in_ppp is None:
     *          if self.__ppp_suspend():
     *              LTE.in_ppp = False
     *          else:
     *              raise OSError('Modem initialization failed!')
     */

    if(s_initialized) {
        __log_info("lte already initialized, try deinit() first");
        return __LTE_ERROR;
    }

    // -- local parameters init
    s_ppp_suspended = false;
    if(carrier == NULL)
        carrier = "standard";
    s_carrier = malloc(strlen(carrier) + 1); // TODO: free s_carrier in deinit
    strcpy(s_carrier, carrier);
    s_cid = cid;
    s_debug = debug;
    if(debug == true) {
        log_filter_subsystem(__stringify(__log_subsystem), true);
    }

    // -- power on the device
    lte_power_on(false);
    s_power = true;

    // -- init serial communication with the device
    lte_uart_init(baudrate);

    // -- init ppp client
    lte_ppp_init();

    if( __ppp_suspend() ) {
        s_in_ppp = false;
    } else {
        __log_error("Modem initialization failed!");
        return __LTE_ERROR;
    }

    s_initialized = true;
    return __LTE_OK;
}

bool lte_check_power(void)
{
    if( !s_power )
    {
        __log_error("Modem is not powered on!");
        __log_output("Modem is not powered on!\n");
    }
    return s_power;
}

void lte_print_pretty_response(const char* rsp, bool flush, const char* prefix)
{
    /*  LTE.py -> LTE.print_pretty_response()
     * --------------------------------------
     *  def print_pretty_response(self, rsp, flush=False, prefix=None):
     *      if prefix is not None:
     *          print(prefix, end=' ')
     *      lines = rsp.decode('ascii').split('\r\n')
     *      for line in lines:
     *          if 'OK' not in line and line != '':
     *              print(line)
     */

    if(prefix)
        __log_output("%s\n", prefix);

    int idx = 0;
    int tot_len = strlen(rsp);
    char* line_str = s_helper_str_buf;

    while(idx < tot_len) {
        int line_len = helper_str_copy(line_str, &rsp[idx],
            __helper_str_buf_size, "\r\n");
        bool has_ok = helper_str_search(line_str, "OK", NULL, 0, NULL);
        if(! has_ok && line_len )  {
            __log_output("%s\n", line_str);
        }
        idx += line_len;
        if(rsp[idx] == '\r' && rsp[idx + 1] == '\n')
            idx += 2;
    }
}

int lte_return_pretty_response(const char* resp, char* out_buf, uint32_t len)
{
    /** LTE.py -> LTE.print_pretty_response()
     *  -------------------------------------
     *  def return_pretty_response(self, rsp):
     *      ret_str = ''
     *      lines = rsp.decode('UTF-8').split('\r\n')
     *      for line in lines:
     *          if 'OK' not in line and len(line) > 0:
     *              ret_str += (line + '\n')
     *      return ret_str
     */

    int idx = 0;
    int tot_len = strlen(resp);
    int w_idx = 0;
    int rem_len = len - 1;
    out_buf[idx] = 0;
    bool first_line = true;

    while(idx < tot_len && rem_len) {

        int line_len = tot_len - idx;
        const char* p_line_str = &resp[idx];

        bool found = helper_str_search(p_line_str, "\r\n", NULL, 0, &line_len);

        bool has_ok = false;
        if(line_len > 0) {
            has_ok = helper_str_search(p_line_str, "OK", NULL, line_len, NULL);
        }

        #if 0
        if( !has_ok && line_len ) {
            int min_len = line_len < rem_len ? line_len+1 : rem_len;
            helper_str_copy(&out_buf[w_idx], p_line_str, min_len, NULL);
            w_idx += min_len - 1;
            if(min_len < rem_len) {
                out_buf[w_idx++] = '\n';
                out_buf[w_idx] = '\0';
            }
            rem_len -= min_len;
        }
        #else
        if( !has_ok && line_len ) {
            int copy_len;
            if(line_len + !first_line/*\n*/< rem_len) {
                copy_len = line_len;
            } else {
                copy_len = rem_len - !first_line;
            }
            if(!first_line)
                out_buf[w_idx++] = '\n';
            helper_str_copy(&out_buf[w_idx], p_line_str, copy_len+1, NULL);
            w_idx += copy_len;
            rem_len -= copy_len + !first_line;
            first_line = false;
        }
        #endif

        idx += line_len + (found ? 2 : 0);
    }
    return w_idx;
}

static lte_error_t lte_read_rsp_step(int timeout, bool wait_ok_error, bool check_error,
    char* rsp, int rsp_buf_size, bool disable_first_delay);
lte_error_t lte_read_rsp(int timeout, bool wait_ok_error, bool check_error,
    char* rsp, int rsp_buf_size)
{
    return lte_read_rsp_step(timeout, wait_ok_error, check_error,
                rsp, rsp_buf_size, false);
}
static lte_error_t lte_read_rsp_step(int timeout, bool wait_ok_error, bool check_error,
    char* rsp, int rsp_buf_size, bool disable_first_delay)
{
    /*  LTE.py -> LTE.read_rsp()
     *  ------------------------
     *  def read_rsp(self, size=None, timeout=-1, wait_ok_error=False,
     *              check_error=False):
     *      self.check_power()
     *      utime.sleep(.25)
     *      if timeout < 0:
     *          timeout = sys.maxsize
     *      elif timeout is None:
     *          timeout = 0
     *      rsp = b''
     *      if wait_ok_error:
     *          while ('OK' not in rsp and 'ERROR' not in rsp):
     *              while not LTE.lte_uart.any() and timeout > 0:
     *                  utime.sleep_ms(1)
     *                  timeout -= 1
     *              if self.__debug:
     *                  print('rsp before: {}'.format(rsp))
     *              if size is not None:
     *                  new = LTE.lte_uart.read(size)
     *                  if rsp is not None and new is not None:
     *                      rsp += new
     *                  elif new is not None:
     *                      rsp = new
     *              else:
     *                  new = LTE.lte_uart.read()
     *                  if rsp is not None and new is not None:
     *                      rsp += new
     *                  elif new is not None:
     *                      rsp = new
     *              if self.__debug:
     *                  print('rsp after: {}'.format(rsp))
     *      else:
     *          while LTE.lte_uart.any():
     *              while not LTE.lte_uart.any() and timeout > 0:
     *                  utime.sleep_ms(1)
     *                  timeout -= 1
     *              if self.__debug:
     *                  print('rsp before: {}'.format(rsp))
     *              if size is not None:
     *                  new = LTE.lte_uart.read(size)
     *                  if rsp is not None and new is not None:
     *                      rsp += new
     *                  elif new is not None:
     *                      rsp = new
     *              else:
     *                  new = LTE.lte_uart.read()
     *                  if rsp is not None and new is not None:
     *                      rsp += new
     *                  elif new is not None:
     *                      rsp = new
     *              if self.__debug:
     *                  print('rsp after: {}'.format(rsp))
     *              utime.sleep_ms(5)
     *      if rsp is not None:
     *          if self.__debug:
     *              print('rsp return: {}'.format(rsp))
     *          if check_error and 'ERROR' in rsp:
     *              raise OSError('AT command returned ERROR!')
     *          return rsp
     *      else:
     *          return b''
     */
    if( !lte_check_power() ) {
        return __LTE_ERROR;
    }

    if(!disable_first_delay)
        __hal_delay_ms(250);
    else
        __hal_delay_ms(40);

    if(timeout < 0)
    {
        timeout = (int)((unsigned int)(-1) >> 1);
    }
    rsp[0] = 0;

    int rx_idx = 0;
    if(wait_ok_error)
    {
        while( ! helper_str_search(rsp, "OK", NULL, 0, NULL) &&
                ! helper_str_search(rsp, "ERROR", NULL, 0, NULL))
        {
            while( lte_uart_any() == 0 && timeout > 0 
                && rx_idx < rsp_buf_size - 1 )
            {
                __hal_delay_ms(1);
                timeout -= 1;
            }
            if(s_debug) {
                __log_output("rsp before: %s\n", rsp);
            }
            if(rsp_buf_size - rx_idx < 0)
            {
                __log_error("fatal insufficient buffer");
                return __LTE_ERROR;
            }
            int bytes = lte_uart_read((uint8_t*)&rsp[rx_idx],
                rsp_buf_size - rx_idx, 0);
            rx_idx += bytes;
            // rsp[rx_idx] = 0;
            if(s_debug) {
                __log_output("rsp after: %s\n", rsp);
            }
        }
    }
    else
    {
        while( lte_uart_any() && rx_idx < rsp_buf_size - 1 )
        {
            while( lte_uart_any() == 0 && timeout > 0 )
            {
                __hal_delay_ms(1);
                timeout -= 1;
            }
            if(s_debug) {
                __log_output("rsp before: %s\n", rsp);
            }
            int bytes = lte_uart_read((uint8_t*)&rsp[rx_idx],
                rsp_buf_size - rx_idx, 0);
            rx_idx += bytes;
            rsp[rx_idx] = 0;
            if(s_debug) {
                __log_output("rsp after: %s\n", rsp);
            }
        }
        if(rx_idx >= rsp_buf_size - 1) {
            // __log_warn("buffer is not enough");
            return __LTE_SMALL_BUF;
        }
    }
    if(rx_idx > 0)
    {
        if(s_debug) {
            __log_output("rsp return: %s\n", rsp);
        }
        if(check_error && helper_str_search(rsp, "ERROR", NULL, 0, NULL))
        {
            __log_error("AT command returned ERROR!");
            return __LTE_ERROR;
        }
    }
    return __LTE_OK;
}

#define __at_buf_size   (256)
static char s_at_buffer[__at_buf_size];
lte_error_t lte_send_at_cmd(const char* cmd, int timeout,
    bool wait_ok_error, bool check_error, char* rsp, int resp_size)
{
    /*  LTE.py -> LTE.send_at_cmd()
     *  ---------------------------
     *  def send_at_cmd(self, cmd='AT', timeout=-1, wait_ok_error=False,
     *                  check_error=False):
     *      self.check_power()
     *      self.check_ppp()
     *      LTE.lte_uart.flush()
     *      if self.__debug:
     *          print('AT: {}'.format(cmd))
     *      LTE.lte_uart.write(cmd + '\r\n')
     *      return self.return_pretty_response(
     *          self.read_rsp(timeout=timeout, wait_ok_error=wait_ok_error,
     *                        check_error=check_error))
     */
    if( !lte_check_power() || !lte_check_ppp() ) {
        return __LTE_ERROR;
    }

    lte_uart_flush();

    if(s_debug) {
        __log_output("AT: %s\n", cmd);
    }

    if(strlen(cmd) + 2/* \r\n */ + 1/* \0 */ > __at_buf_size) {
        __log_error("AT command length > buffer length");
        return __LTE_ERROR;
    }

    int at_cmd_len = helper_str_copy(s_at_buffer, cmd, __at_buf_size, NULL);
    s_at_buffer[at_cmd_len++] = '\r';
    s_at_buffer[at_cmd_len++] = '\n';
    s_at_buffer[at_cmd_len] = '\0';

    __log_debug("AT-CMD >> "__yellow__"%s"__default__, s_at_buffer);
    lte_uart_write((const uint8_t*)s_at_buffer, at_cmd_len);

    lte_error_t err;
    int idx = 0;
    bool disable_first_delay = false;
    while(idx < resp_size - 1)
    {
        err = lte_read_rsp_step(timeout, wait_ok_error, check_error,
                s_at_buffer, __at_buf_size, disable_first_delay);
        disable_first_delay = true;
        if(err != __LTE_ERROR) {
            idx += lte_return_pretty_response(s_at_buffer, &rsp[idx],
                resp_size - idx);
        }
        if(err == __LTE_OK)
            break;
        if(err == __LTE_ERROR)
            return __LTE_ERROR;
    }
    // if(lte_read_rsp(timeout, wait_ok_error, check_error,
    //             s_at_buffer, __at_buf_size) == __LTE_ERROR) {
    //     return __LTE_ERROR;
    // }
    // __log_debug("AT-RSP >> "__green__"%s"__default__, s_at_buffer);

    // lte_return_pretty_response(s_at_buffer, rsp, resp_size);

    __log_debug("AT-RSP >> "__green__"%s"__default__, rsp);

    return __LTE_OK;
}

lte_error_t lte_get_mode(int * p_mode)
{
    /*  LTE.py -> __get_mode()
     *  ----------------------
     *  def __get_mode(self):
     *      try:
     *          result = self.send_at_cmd('AT+SQNMODEACTIVE?', check_error=True)
     *          if '+SQNMODEACTIVE:' in result:
     *              mode = int(result.split(':')[1].strip())
     *              return (mode - 1)
     *      except:
     *          return None
     */
    if( !lte_check_power() ) {
        return __LTE_ERROR;
    }
    lte_error_t err = lte_send_at_cmd("AT+SQNMODEACTIVE?",
        __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
        true, s_helper_str_buf, __helper_str_buf_size);
    if( err == __LTE_OK ) {
        int mode;
        int idx;
        const char* search_str = "+SQNMODEACTIVE:";
        if(helper_str_search(s_helper_str_buf, search_str, NULL, 0, &idx)) {
            if(helper_str_read_int(&s_helper_str_buf[idx + strlen(search_str)],
                                &mode)) {
                *p_mode = mode - 1;
            } else {
                err = __LTE_ERROR;
            }
        }
    }
    return err;
}

lte_error_t lte_attach(const char* apn, const char* type, int cid,
    int band, int* bands, int bands_count)
{
    /*  LTE.py -> LTE.attach()
     *  ----------------------
     *  def attach(self, apn=None, type='IP', cid=None, band=None, bands=None):
     *      self.check_power()
     *      self.check_ppp()
     *      if cid is not None:
     *          self.__cid = cid
     *      if band is not None and bands is not None:
     *          raise ValueError("Cannot specify both band and bands")
     *      if not self.check_sim_present():
     *          raise OSError('SIM card not present or PIN protected!')
     *      if band is not None:
     *          mode = self.__get_mode()
     *          resp = self.send_at_cmd(
     *              'AT+SQNBANDSEL={},"{}","{}"'.format(
     *                  mode, self.__carrier, band), check_error=True)
     *          if self.__debug:
     *              print(resp)
     *      if bands is not None:
     *          mode = self.__get_mode()
     *          band_str = ""
     *          for band in bands:
     *              band_str += "{},".format(band)
     *          resp = self.send_at_cmd(
     *              'AT+SQNBANDSEL={},"{}","{}"'.format(
     *                  mode, self.__carrier, band_str), check_error=True)
     *          if self.__debug:
     *              print(resp)
     *      if apn is not None:
     *          resp = self.send_at_cmd(
     *              'AT+CGDCONT={},"{}","{}"'.format(
     *                  self.__cid, type, apn), check_error=True)
     *          if self.__debug:
     *              print(resp)
     *      resp = self.send_at_cmd('AT+CFUN=1', check_error=True)
     *      if self.__debug:
     *          print(resp)
     */

    if( !lte_check_power() ) {
        return __LTE_ERROR;
    }

    if( ! lte_check_ppp() ) {
        return __LTE_ERROR;
    }
    if(cid != -1) {
        s_cid = cid;
    }
    if(band != -1 && bands_count > 0) {
        __log_output("Cannot specify both band and bands\n");
        return __LTE_ERROR;
    }
    if(!lte_check_sim_present()) {
        __log_output("SIM card not present or PIN protected!\n");
        return __LTE_ERROR;
    }

    lte_error_t err = __LTE_OK;
    if(band != -1) {
        int mode;
        if( lte_get_mode(&mode) != __LTE_OK ) {
            __log_error("failed to get mode");
            return __LTE_ERROR;
        }
        snprintf(s_helper_str_buf, __helper_str_buf_size - 1,
            "AT+SQNBANDSEL=%d,\"%s\",\"%d\"", mode, s_carrier, band);
        err = lte_send_at_cmd(s_helper_str_buf,
            __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
            true, s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK) {
            __log_error("failed to send the at command: %s", s_helper_str_buf);
            return err;
        }
        if(s_debug) {
            __log_output("%s\n", s_helper_str_buf_2);
        }
    }

    if(bands) {
        int mode;
        if( lte_get_mode(&mode) != __LTE_OK ) {
            __log_error("failed to get mode");
            return __LTE_ERROR;
        }
        int j = snprintf(s_helper_str_buf, __helper_str_buf_size - 1,
            "AT+SQNBANDSEL=%d,\"%s\",\"%d", mode, s_carrier, bands[0]);
        for(int i = 1; i < bands_count; ++i) {
            j += snprintf(s_helper_str_buf + j, __helper_str_buf_size - 1 - j,
            ",%d", bands[i]);
        }
        s_helper_str_buf[j] = '\"';
        s_helper_str_buf[j+1] = 0;

        err = lte_send_at_cmd(s_helper_str_buf,
            __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
            true, s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK) {
            __log_error("failed to send the at command: %s", s_helper_str_buf);
            return err;
        }
        if(s_debug) {
            __log_output("%s\n", s_helper_str_buf_2);
        }
    }
    if(apn) {
        snprintf(s_helper_str_buf, __helper_str_buf_size - 1,
            "AT+CGDCONT=%d,\"%s\",\"%s\"", s_cid, type, apn);
        err = lte_send_at_cmd(s_helper_str_buf,
            __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
            true, s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK) {
            __log_error("failed to send the at command: %s", s_helper_str_buf);
            return err;
        }
        if(s_debug) {
            __log_output("%s\n", s_helper_str_buf_2);
        }
    }
    err = lte_send_at_cmd("AT+CFUN=1",
        __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
        true, s_helper_str_buf_2, __helper_str_buf_size);
    if(err != __LTE_OK) {
        __log_error("failed to send the at command: %s", s_helper_str_buf);
        return err;
    }
    if(s_debug) {
        __log_output("%s\n", s_helper_str_buf_2);
    }
    return err;
}

bool lte_is_attached(void)
{
    /*  LTE.py -> is_attached()
     *  -----------------------
     *  def is_attached(self):
     *      self.check_power()
     *      if LTE.in_ppp:
     *          return True
     *      self.check_ppp()
     *      resp = self.send_at_cmd('AT+CEREG?', check_error=True)
     *      if self.__debug:
     *          print(resp)
     *      if '+CEREG: 1,1' in resp or '+CEREG: 1,5' in resp:
     *          return True
     *      if '+CEREG: 2,1' in resp or '+CEREG: 2,5' in resp:
     *          return True
     *      return False
     */
    if( !lte_check_power() ) {
        return false;
    }
    if(s_in_ppp)
        return true;
    if( !lte_check_ppp() ) {
        return false;
    }
    lte_error_t err = lte_send_at_cmd("AT+CEREG?",
        __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
        true, s_helper_str_buf_2, __helper_str_buf_size);
    if(err != __LTE_OK) {
        __log_error("failed to send the at command: %s", s_helper_str_buf);
        return false;
    }
    if(s_debug) {
        __log_output("%s\n", s_helper_str_buf_2);
    }

    if(helper_str_search(s_helper_str_buf_2, "+CEREG: 1,1", NULL, 0, NULL)
        || helper_str_search(s_helper_str_buf_2, "+CEREG: 1,5", NULL, 0, NULL)
        || helper_str_search(s_helper_str_buf_2, "+CEREG: 2,1", NULL, 0, NULL)
        || helper_str_search(s_helper_str_buf_2, "+CEREG: 2,5", NULL, 0, NULL)
        ) {
        return true;
    }

    return false;
}

lte_error_t lte_reset(void)
{
    /*  LTE.py -> reset()
     *  -----------------
     *  def reset(self):
     *      self.check_power()
     *      self.check_ppp()
     *      LTE.lte_uart.flush()
     *      LTE.lte_uart.write('AT^RESET\r\n')
     *      for _ in range(10):
     *          resp = self.read_rsp(wait_ok_error=True)
     *          if self.__debug:
     *              print('reset_resp look_for +SHUTDOWN: {}'.format(resp))
     *          if b'+SHUTDOWN' in resp:
     *              break
     *          utime.sleep(.25)
     *      resp = b''
     *      for _ in range(20):
     *          resp = self.read_rsp()
     *          if self.__debug:
     *              print('reset_resp wait_for +SYSSTART: {}'.format(resp))
     *          if b'+SYSSTART' in resp:
     *              break
     *          utime.sleep(.25)
     *      for _ in range(25):
     *          if self.__debug:
     *              print('reset_resp wait_for AT/OK {}'.format(resp))
     *          LTE.lte_uart.write('AT\r\n')
     *          utime.sleep(.1)
     *          resp = self.read_rsp()
     *          if b'OK' in resp:
     *              break
     *          utime.sleep(.25)
     *      else:
     *          raise OSError('The modem did not respond with +SHUTDOWN')
     */
    if( !lte_check_power() ) {
        return __LTE_ERROR;
    }
    if( !lte_check_ppp() ) {
        return __LTE_ERROR;
    }

    lte_uart_flush();
    const char* rst_cmd = "AT^RESET\r\n";
    lte_uart_write((const uint8_t*)rst_cmd, strlen(rst_cmd));

    lte_error_t err = __LTE_OK;

    for(int i = 0; i < 10; ++i) {
        s_helper_str_buf_2[0] = 0;
        err = lte_read_rsp(__lte_read_rsp_default_timeout, true,
            __lte_read_rsp_default_check_error,
            s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK)
            return err;
        if( helper_str_search(s_helper_str_buf_2, "+SHUTDOWN", NULL, 0, NULL) )
        {
            break;
        }
        err = __LTE_ERROR;
        __hal_delay_ms(250);
    }

    for(int i = 0; i < 20; ++i) {
        s_helper_str_buf_2[0] = 0;
        err = lte_read_rsp(__lte_read_rsp_default_timeout,
            __lte_read_rsp_default_wait_ok_error,
            __lte_read_rsp_default_check_error,
            s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK)
            return err;
        if(s_debug) {
            __log_output("reset_resp wait_for +SYSSTART: %s\n",
                s_helper_str_buf_2);
        }
        if( helper_str_search(s_helper_str_buf_2, "+SYSSTART", NULL, 0, NULL) )
        {
            break;
        }
        err = __LTE_ERROR;
        __hal_delay_ms(250);
    }

    for(int i = 0; i < 25; ++i) {
        if(s_debug) {
            __log_output("reset_resp wait_for AT/OK %s\n", s_helper_str_buf_2);
        }
        lte_uart_write((const uint8_t*)"AT\r\n", 4);
        __hal_delay_ms(100);
        s_helper_str_buf_2[0] = 0;
        err = lte_read_rsp(__lte_read_rsp_default_timeout,
            __lte_read_rsp_default_wait_ok_error,
            __lte_read_rsp_default_check_error,
            s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK)
            return err;
        if( helper_str_search(s_helper_str_buf_2, "OK", NULL, 0, NULL) ) {
            break;
        }
        err = __LTE_ERROR;
        __hal_delay_ms(250);
    }

    if(err == __LTE_ERROR){
        __log_output("The modem did not respond with +SHUTDOWN\n");
    }
    return err;
}

lte_error_t lte_connect(int cid)
{
    /*  LTE.py -> connect()
     *  -------------------
     *  def connect(self, cid=None):
     *      self.check_power()
     *      self.check_ppp()
     *      if cid is not None:
     *          self.__cid = cid
     *      if not self.is_attached():
     *          raise OSError('Modem is not attached to a network!')
     *      resp = self.send_at_cmd('ATO'.format(self.__cid))
     *      if self.__debug:
     *          print(resp)
     *      if not 'CONNECT' in resp:
     *          utime.sleep(.1)
     *          resp = self.send_at_cmd('AT+CGDATA="PPP",{}'.format(self.__cid))
     *          for _ in range(25):
     *              if self.__debug:
     *                  print(resp)
     *              if 'CONNECT' in resp:
     *                  break
     *              elif 'ERROR' in resp:
     *                  utime.sleep(1)
     *                  resp = self.send_at_cmd(
     *                      'AT+CGDATA="PPP",{}'.format(self.__cid))
     *              else:
     *                  resp = self.return_pretty_response(self.read_rsp())
     *                  utime.sleep(.25)
     *          utime.sleep(.1)
     *      LTE.lte_ppp.active(True)
     *      LTE.lte_ppp.connect()
     *      LTE.in_ppp = True
     */
    lte_error_t err = __LTE_OK;

    if( !lte_check_power() ) {
        return __LTE_ERROR;
    }

    if( !lte_check_ppp() )
        return __LTE_ERROR;
    if(cid != -1)
        s_cid = cid;
    if( ! lte_is_attached() ) {
        __log_output("Modem is not attached to a network!\n");
        return __LTE_ERROR;
    }

    err = lte_send_at_cmd("ATO", __lte_send_at_cmd_def_timeout,
        __lte_send_at_cmd_def_wait_ok_error, __lte_send_at_cmd_def_check_error,
        s_helper_str_buf_2, __helper_str_buf_size);
    if(err != __LTE_OK) {
        return __LTE_ERROR;
    }

    if( ! helper_str_search(s_helper_str_buf_2, "CONNECT", NULL, 0, NULL) ) {
        __hal_delay_ms(100);
        snprintf(s_helper_str_buf, __helper_str_buf_size - 1,
            "AT+CGDATA=\"PPP\",%d", s_cid);
        err = lte_send_at_cmd(s_helper_str_buf,
            __lte_send_at_cmd_def_timeout, __lte_send_at_cmd_def_wait_ok_error,
            true, s_helper_str_buf_2, __helper_str_buf_size);
        if(err != __LTE_OK) {
            __log_error("failed to send the at command: %s", s_helper_str_buf);
            return err;
        }
        for(int i = 0; i < 25; ++i) {
            if(s_debug) {
                __log_output("%s\n", s_helper_str_buf_2);
            }
            if( helper_str_search(s_helper_str_buf_2, "CONNECT", NULL,
                                0, NULL) ) {
                break;
            } else if(helper_str_search(s_helper_str_buf_2, "CONNECT", NULL,
                                0, NULL)) {
                __hal_delay_ms(1000);
                snprintf(s_helper_str_buf, __helper_str_buf_size - 1,
                    "AT+CGDATA=\"PPP\",%d", s_cid);
                err = lte_send_at_cmd(s_helper_str_buf,
                    __lte_send_at_cmd_def_timeout,
                    __lte_send_at_cmd_def_wait_ok_error,
                    true, s_helper_str_buf_2, __helper_str_buf_size);
                if(err != __LTE_OK) {
                    __log_error("failed to send the at command: %s",
                            s_helper_str_buf);
                    return err;
                }
            } else {
                err = lte_read_rsp(__lte_read_rsp_default_timeout,
                    __lte_read_rsp_default_wait_ok_error,
                    __lte_read_rsp_default_check_error, s_helper_str_buf,
                    __helper_str_buf_size);
                if(err != __LTE_OK) {
                    __log_error("failed to read response");
                    return __LTE_ERROR;
                }
                lte_return_pretty_response(s_helper_str_buf, s_helper_str_buf_2,
                    __helper_str_buf_size);
            }
        }
        __hal_delay_ms(100);
    }
    lte_ppp_activate();
    lte_ppp_connect(__lte_pp_connect_authmode_default,
        __lte_pp_connect_username_default, __lte_pp_connect_password_default);
    s_in_ppp = true;
    return err;
}

lte_error_t lte_mode(lte_mode_t new_mode)
{
    /*  LTE.py -> mode()
     *  ----------------
     *  def mode(self, new_mode=None):
     *      self.check_power()
     *      self.check_ppp()
     *      if new_mode is None:
     *          return self.__get_mode()
     *      else:
     *          current_mode = self.__get_mode()
     *          if (new_mode != current_mode):
     *              self.send_at_cmd('AT+CFUN=0', check_error=True)
     *              try:
     *                  self.send_at_cmd(
     *                      'AT+SQNMODEACTIVE={}'.format(new_mode+1),
     *                      wait_ok_error=True, check_error=True)
     *                  self.reset()
     *              except:
     *                  print('Error switching operating mode!')
     */
    if( !lte_check_power() ) {
        return __LTE_ERROR;
    }

    if( ! lte_check_ppp() ) {
        return __LTE_ERROR;
    }

    if(new_mode != __LTE_MODE__CATM1 && new_mode != __LTE_MODE__NBIOT) {
        __log_output("invalid LTE mode!\n");
        return __LTE_ERROR;
    }

    int mode;
    lte_error_t err = lte_get_mode(&mode);
    if(err != __LTE_OK) {
        __log_output("failed to retrieve current LTE mode\n");
        return err;
    }

    if(mode == new_mode) {
        return __LTE_OK;
    }

    err = lte_send_at_cmd("AT+CFUN=0", __lte_send_at_cmd_def_timeout,
            __lte_send_at_cmd_def_wait_ok_error, true, s_helper_str_buf_2,
            __helper_str_buf_size);
    if(err != __LTE_OK) {
        __log_error("failed to send the at command: AT+CFUN=0");
        return err;
    }
    snprintf(s_helper_str_buf, __helper_str_buf_size - 1,
        "AT+SQNMODEACTIVE=%d", new_mode+1);
    err = lte_send_at_cmd(s_helper_str_buf,
        __lte_send_at_cmd_def_timeout,
        true, true, s_helper_str_buf_2, __helper_str_buf_size);
    if(err != __LTE_OK) {
        __log_error("failed to send the at command: %s", s_helper_str_buf);
        return err;
    }
    err = lte_reset();
    if(err != __LTE_OK) {
        __log_error("failed to reset");
        return err;
    }

    return __LTE_OK;
}

lte_error_t lte_ifconfig(lte_ppp_ifconfig_t * p_if_config)
{
    /*  LTE.py -> ifconfig()
     *  --------------------
     *  def ifconfig(self):
     *      if LTE.in_ppp:
     *          return LTE.lte_ppp.ifconfig()
     */
    lte_ppp_get_ifconfig(p_if_config);
    return __LTE_OK;
}
static bool __wait_at(int attempts, int sleep_ms);
static bool __ppp_suspend(void)
{
    /*  LTE.py -> __ppp_suspend()
     *  -------------------------
     *  def __ppp_suspend(self):
     *      resp = b''
     *      for _ in range(5):
     *          LTE.lte_uart.flush()
     *          if self.__debug:
     *              print('Writing +++')
     *          LTE.lte_uart.write('+++')
     *          utime.sleep(1)
     *          if self.__wait_at(25, 250):
     *              return True
     *      return False
     */

    for(int i = 0; i < 5; ++i) {
        lte_uart_flush();
        if(s_debug) {
            __log_output("Writing +++\n");
        }
        lte_uart_write((const uint8_t*)"+++", 3);
        __hal_delay_ms(1000);
        if(__wait_at(25, 250)) {
            return true;
        }
    }
    return false;
}

static bool __wait_at(int attempts, int sleep_ms)
{
    /*  LTE.py -> __wait_at()
     *  ---------------------
     *  def __wait_at(self, attempts, sleep_ms):
     *      for counter in range(attempts):
     *          # LTE.lte_uart.flush()
     *          LTE.lte_uart.write('AT\r\n')
     *          utime.sleep_ms(sleep_ms)
     *          resp = self.read_rsp()
     *          if self.__debug:
     *              print("AT#{}: {}".format(counter, resp))
     *          if b'OK' in resp:
     *              return True
     *      return False
     */
    for(int counter = 0; counter < attempts; ++counter) {
        lte_uart_write((const uint8_t*)"AT\r\n", 4);
        __hal_delay_ms(sleep_ms);
        if(lte_read_rsp(__lte_read_rsp_default_timeout,
            __lte_read_rsp_default_wait_ok_error,
            __lte_read_rsp_default_check_error,
            s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK )
        {
            return false;
        }
        if(s_debug) {
            __log_output("AT#%d: %s\n", counter, s_helper_str_buf_2);
        }
        if( helper_str_search(s_helper_str_buf_2, "OK", NULL, 0, NULL) ) {
            return true;
        }
    }
    return false;
}

lte_error_t lte_disconnect(void)
{
    /*  LTE.py -> disconnect()
     *  ----------------------
     *  def disconnect(self):
     *      self.check_power()
     *      if ctrl_enabled:
     *          raise OSError('Please use ctrl.disconnect() instead!')
     *      self.lte_ppp.active(False)
     *      self.__ppp_suspend()
     *      LTE.in_ppp = False
     *      self.send_at_cmd('ATH', wait_ok_error=True, check_error=True)
     */

    if(! lte_check_power() ) {
        return __LTE_ERROR;
    }

    // -- the ctrl check here is not valid, handle such thing in its scope

    lte_ppp_deactivate();
    __ppp_suspend();
    s_in_ppp = false;
    return lte_send_at_cmd("ATH", __lte_send_at_cmd_def_timeout, true, true,
        s_helper_str_buf_2, __helper_str_buf_size);
}

bool lte_is_connected(void)
{
    /*  LTE.py -> is_connected()
     *  ------------------------
     *  def is_connected(self):
     *      self.check_power()
     *      return LTE.in_ppp
     */
    if(! lte_check_power() ) {
        return false;
    }
    return s_in_ppp;
}

bool lte_isconnected(void)
{
    /*  LTE.py -> isconnected()
     *  -----------------------
     *  def isconnected(self):
     *      return self.is_connected()
     */
    return lte_is_connected();
}

bool lte_isattached(void)
{
    /*  LTE.py -> isattached()
     *  ----------------------
     *  def isattached(self):
     *      return self.is_attached()
     */
    return lte_is_attached();
}

lte_error_t lte_detach(void)
{
    /*  LTE.py -> detach()
     *  ------------------
     *  def detach(self):
     *      self.disconnect()
     *      self.send_at_cmd('AT+CFUN=4', wait_ok_error=True, check_error=True)
     *      utime.sleep(.5)
     *      self.send_at_cmd('AT+CFUN=0', wait_ok_error=True, check_error=True)
     */

    if(lte_disconnect() != __LTE_OK)
        return __LTE_ERROR;
    if(lte_send_at_cmd("AT+CFUN=4", __lte_send_at_cmd_def_timeout, true, true,
        s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK){
        return __LTE_ERROR;
    }
    __hal_delay_ms(500);
    if(lte_send_at_cmd("AT+CFUN=0", __lte_send_at_cmd_def_timeout, true, true,
        s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK){
        return __LTE_ERROR;
    }

    return __LTE_OK;
}

lte_error_t lte_deinit(void)
{
    /*  LTE.py -> deinit()
     *  ------------------
     *  self.disconnect()
     *      self.detach()
     *      self.power_off()
     */
    if(lte_disconnect() != __LTE_OK)
        return __LTE_ERROR;
    if(lte_detach() != __LTE_OK)
        return __LTE_ERROR;
    if(lte_power_off(false) != __LTE_OK)
        return __LTE_ERROR;
    
    return __LTE_OK;
}

bool lte_check_sim_present(void)
{
    /*  LTE.py -> check_sim_present()
     *  -----------------------------
     *  def check_sim_present(self):
     *      self.check_ppp()
     *      self.check_ppp()
     *      resp = self.send_at_cmd(
     *          'AT+CFUN=4', wait_ok_error=True, check_error=True)
     *      for _ in range(5):
     *          utime.sleep(.25)
     *          resp = self.send_at_cmd('AT+CPIN?', wait_ok_error=True)
     *          if ("+CPIN: READY" in resp):
     *              return True
     *      return False
     */
    if(! lte_check_power() ) {
        return false;
    }
    if(!lte_check_ppp())
        return false;
    
    if(lte_send_at_cmd("AT+CFUN=4", __lte_send_at_cmd_def_timeout, true, true,
        s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK ) {
        return false;
    }
    for(int i=0; i < 5; ++i) {
        __hal_delay_ms(250);
        if(lte_send_at_cmd("AT+CPIN?", __lte_send_at_cmd_def_timeout,
            true, __lte_send_at_cmd_def_check_error,
            s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK) {
            return false;
        }
        if(helper_str_search(s_helper_str_buf_2, "+CPIN: READY", NULL, 0, NULL))
            return true;
    }
    return false;
}

bool lte_power_on(bool wait_ok)
{
    /*  LTE.py -> power_on()
     *  --------------------
     *  def power_on(self, wait_ok=True):
     *      ioexp.lte_power_on()
     *      self.__power = True
     *      if wait_ok:
     *          utime.sleep(1)
     *          self.send_at_cmd(wait_ok_error=True)
     */
    ioexp_lte_chip_power_on();
    s_power = true;
    if(wait_ok)
    {
        __hal_delay_ms(1000);
        if(lte_send_at_cmd(__lte_send_at_cmd_def_cmd,
            __lte_send_at_cmd_def_timeout,
            true,
            __lte_send_at_cmd_def_check_error,
            s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK)
            return false;
    }
    return true;
}

lte_error_t lte_power_off(bool force)
{
    /*  LTE.py -> power_off()
     *  ---------------------
     *  def power_off(self, force=False):
     *      self.check_power()
     *      if not force:
     *          if self.lte_ppp.active():
     *              self.lte_ppp.active(False)
     *          if LTE.in_ppp:
     *              if not self.__ppp_suspend():
     *                  raise OSError('communication error! Use force=True')
     *          resp = self.send_at_cmd(
     *              'AT+CFUN?', wait_ok_error=True, check_error=True)
     *          if "+CFUN: 1" in resp:
     *              self.send_at_cmd(
     *                  'AT+CFUN=4', wait_ok_error=True, check_error=True)
     *              utime.sleep(.25)
     *              self.send_at_cmd(
     *                  'AT+CFUN=0', wait_ok_error=True, check_error=True)
     *          if "+CFUN: 4" in resp:
     *              self.send_at_cmd(
     *                  'AT+CFUN=0', wait_ok_error=True, check_error=True)
     *          utime.sleep(.25)
     *      ioexp.lte_power_off()
     *      self.__power = False
     */
    if(! lte_check_power() ) {
        return __LTE_ERROR;
    }
    if(!force) {
        if(lte_ppp_is_active())
            lte_ppp_deactivate();
        if(s_in_ppp)
            if(! __ppp_suspend())
                return __LTE_ERROR;

        if(lte_send_at_cmd("AT+CFUN?", __lte_send_at_cmd_def_timeout,
            true, true,
            s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK) {
            return __LTE_ERROR;
        }

        if(helper_str_search(s_helper_str_buf_2, "+CFUN: 1", 0, 0, 0)) {
            if(lte_send_at_cmd("AT+CFUN=4", __lte_send_at_cmd_def_timeout,
                true, true,
                s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK) {
                return __LTE_ERROR;
            }
            __hal_delay_ms(250);
            if(lte_send_at_cmd("AT+CFUN=0", __lte_send_at_cmd_def_timeout,
                true, true,
                s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK) {
                return __LTE_ERROR;
            }
        }
        if(helper_str_search(s_helper_str_buf_2, "+CFUN: 4", 0, 0, 0)) {
            if(lte_send_at_cmd("AT+CFUN=0", __lte_send_at_cmd_def_timeout,
                true, true,
                s_helper_str_buf_2, __helper_str_buf_size) != __LTE_OK) {
                return __LTE_ERROR;
            }
        }
        __hal_delay_ms(250);
    }
    ioexp_lte_chip_power_off();
    s_power = false;
    return __LTE_OK;
}

lte_error_t lte_pause_ppp(void)
{
    /*  LTE.py -> pause_ppp()
     *  ---------------------
     *  def pause_ppp(self):
     *      if self.__debug:
     *          print('This is pause_ppp...')
     *      self.check_power()
     *      if ctrl_enabled:
     *          raise OSError('Command not available when CTRL is enabled!')
     *      self.lte_ppp.active(False)
     *      if self.__ppp_suspend():
     *          self.__ppp_suspended = True
     *      else:
     *          raise OSError('LTE modem communication failed!')
     */
    if(s_debug) {
        __log_output("This is pause_ppp...\n");
    }
    if(!lte_check_power()) {
        return __LTE_ERROR;
    }

    //-- checking for control stuff here is not valid, it should be done in the
    //      control scope

    lte_ppp_deactivate();

    if(__ppp_suspend())
        s_ppp_suspended = true;
    else {
        __log_output("LTE modem communication failed!\n");
        return __LTE_ERROR;
    }

    return __LTE_OK;
}

lte_error_t lte_resume_ppp(void)
{
    /*  LTE.py -> resume_ppp()
     *  ----------------------
     *  def resume_ppp(self):
     *      self.check_power()
     *      if self.__ppp_suspended:
     *          if self.send_at_cmd('ATO', timeout=9500, wait_ok_error=True,
     *                  check_error=True):
     *              self.__ppp.active(True)
     *              self.__ppp.connect()
     *      else:
     *          raise OSError("PPP session not in suspend state!")
     */
    if(!lte_check_power()) {
        return __LTE_ERROR;
    }
    if(s_ppp_suspended) {
        if(lte_send_at_cmd("ATO", 9500, true, true,
                s_helper_str_buf_2, __helper_str_buf_size) == __LTE_OK) {
            lte_ppp_activate();
            lte_ppp_connect(__lte_pp_connect_authmode_default,
                __lte_pp_connect_username_default,
                __lte_pp_connect_password_default);
        } else {
            return __LTE_ERROR;
        }
    } else {
        __log_output("PPP session not in suspend state!\n");
        return __LTE_ERROR;
    }
    return __LTE_OK;
}

bool lte_check_ppp(void)
{
    /** LTE.py -> check_ppp()
     *  ---------------------
     *  def check_ppp(self):
     *      if LTE.in_ppp and not self.__ppp_suspended:
     *          raise OSError("Operation not possible while in PPP mode!")
     */
    if(s_in_ppp && ! s_ppp_suspended) {
        __log_output("Operation not possible while in PPP mode!\n");
        return false;
    }
    return true;
}

/* --- end of file ---------------------------------------------------------- */
