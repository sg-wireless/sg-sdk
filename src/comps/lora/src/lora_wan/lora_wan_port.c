/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @author  Ahmed Sabry (SG Wireless)
 * 
 * @brief   lora-wan transactions Ports.
 *          Transactions are specified by the Tx or Rx on specified port number.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdbool.h>

#define __log_subsystem     lora
#define __log_component     wan_port
#include "log_lib.h"
#include "utils_bitarray.h"
#include "buffers_chain.h"

#include "lora_sync_obj.h"
#include "lora_wan_port.h"
#include "stub_system.h"
#include "stub_timers.h"

/** -------------------------------------------------------------------------- *
 * chained buffers memory space definition
 * --------------------------------------------------------------------------- *
 */
static void* mem_mgr_mutex_handle = NULL;
static void lora_buf_mem_mgr_lock(void)
{
    if(mem_mgr_mutex_handle == NULL)
        mem_mgr_mutex_handle = lora_stub_mutex_new();
    lora_stub_mutex_lock(mem_mgr_mutex_handle);
}
static void lora_buf_mem_mgr_unlock(void)
{
    lora_stub_mutex_unlock(mem_mgr_mutex_handle);
}

#ifdef CONFIG_LORA_WAN_TRX_BUFFERS_MEM_SPACE_SIZE
#define __buffers_mem_space_tx \
    __msize_kb(CONFIG_LORA_WAN_TX_BUFFERS_MEM_SPACE_SIZE)
#define __buffers_mem_space_rx \
    __msize_kb(CONFIG_LORA_WAN_RX_BUFFERS_MEM_SPACE_SIZE)
#else
#define __buffers_mem_space_tx     __msize_kb(6)
#define __buffers_mem_space_rx     __msize_kb(2)
#endif

__buf_chain_mem_def(_lora_wan_buf_mem_tx, __buffers_mem_space_tx, 32,
    lora_buf_mem_mgr_lock, lora_buf_mem_mgr_unlock);

__buf_chain_mem_def(_lora_wan_buf_mem_rx, __buffers_mem_space_rx, 32,
    lora_buf_mem_mgr_lock, lora_buf_mem_mgr_unlock);

/** -------------------------------------------------------------------------- *
 * mutual access management
 * --------------------------------------------------------------------------- *
 */
static void* mutex_handle = NULL;

#define __access_init()                             \
    do {                                            \
        if(mutex_handle == NULL)                    \
            mutex_handle = lora_stub_mutex_new();   \
    } while(0)
static void access_lock(void) {
    __access_init();
    lora_stub_mutex_lock(mutex_handle);
}
#define __access_lock()     access_lock()
#define __access_unlock()   lora_stub_mutex_unlock(mutex_handle)

/** -------------------------------------------------------------------------- *
 * macros and control data struct
 * --------------------------------------------------------------------------- *
 */

#define __min_port      1
#define __max_port      250
#define __ports_count   (__max_port - __min_port + 1)

static __bitarray_def(open_ports, __ports_count);

static lora_wan_port_t* ports_list;

/** -------------------------------------------------------------------------- *
 * APIs Implementation
 * --------------------------------------------------------------------------- *
 */
static void buf_chain_sync_wait(void* obj)
{
    lora_stub_sem_wait(obj);
}

static void buf_chain_sync_signal(void* obj)
{
    lora_stub_sem_signal(obj);
}

static void port_restart_tx_timeout_timer(lora_wan_port_t* p_port);

static void port_tx_timeout_timer_callback(void* data)
{
    lora_wan_port_t * p_port = data;
    __log_timer_expire("tx-timeout-timer");
    __log_debug("port:%p , num:%d", p_port, p_port->port_num);

    uint32_t ts = lora_stub_get_timestamp_ms();

    buf_header_t * p_buf_header;
    lora_wan_port_tx_msg_t * p_tx_msg;

    bool is_broken;
    do {
        lora_buf_mem_mgr_lock();
        is_broken = false;
        __adt_list_foreach(p_port->tx_buf_chain.list, p_buf_header)
        {
            p_tx_msg = (lora_wan_port_tx_msg_t*) p_buf_header->buf;

            __log_debug("-- compare curr_ts:%d , msg-ts:%d", ts,
                p_tx_msg->expire_timestamp);
            if(p_tx_msg->has_timeout && p_tx_msg->expire_timestamp <= ts) {
                // -- clear this message from the chained buffer
                bool is_sync = p_tx_msg->sync;
                sync_obj_t sync_obj = p_tx_msg->sync_obj;

                __log_info("found expired message -- clear it");
                lora_wan_port_ind_msg_t ind_msg = {
                    .type = __IND_TX_TIMEOUT,
                    .ind_params.tx = {
                        .msg_app_id = p_tx_msg->msg_app_id,
                        .msg_seq_num = p_tx_msg->msg_seq_num,
                    }
                };

                __log_info("PORT-QUEUE::expire() msg_id: %d, expire_at: %d",
                    p_tx_msg->msg_app_id,
                    p_tx_msg->expire_timestamp);

                buf_mem_chain_clear_buf(&p_port->tx_buf_chain, p_buf_header);

                lora_buf_mem_mgr_unlock();

                lora_wan_port_indication(p_port->port_num, &ind_msg);
                if(is_sync) {
                    sync_obj_release(sync_obj);
                }

                is_broken = true;
                break;
            }
        }
    } while ( is_broken );

    if( ! is_broken ) {
        lora_buf_mem_mgr_unlock();
    }

    port_restart_tx_timeout_timer(p_port);
}
static void port_restart_tx_timeout_timer(lora_wan_port_t* p_port)
{
    uint32_t ts = lora_stub_get_timestamp_ms();
    uint32_t min_period = (uint32_t)-1;
    uint32_t period;

    __log_info("stop tx timeout timer");
    lora_stub_timer_stop(p_port->tx_timeout_timer);

    buf_header_t * p_buf_header;
    lora_wan_port_tx_msg_t * p_tx_msg;

    __log_info("search for earliest deadline msg");
    bool is_broken;
    do {
        lora_buf_mem_mgr_lock();
        is_broken = false;
        __adt_list_foreach(p_port->tx_buf_chain.list, p_buf_header)
        {
            p_tx_msg = (lora_wan_port_tx_msg_t*) p_buf_header->buf;

            if(p_tx_msg->has_timeout) {
                if(p_tx_msg->expire_timestamp > ts)
                {
                    period = p_tx_msg->expire_timestamp - ts;
                    if( period < min_period ) {
                        min_period = period;
                    }
                }
                else
                {
                    bool is_sync = p_tx_msg->sync;
                    sync_obj_t sync_obj = p_tx_msg->sync_obj;

                    // -- clear this message from the chained buffer
                    __log_info("found expired message -- clear it");
                    lora_wan_port_ind_msg_t ind_msg = {
                        .type = __IND_TX_TIMEOUT,
                        .ind_params.tx = {
                            .msg_app_id = p_tx_msg->msg_app_id,
                            .msg_seq_num = p_tx_msg->msg_seq_num
                        }
                    };
                    buf_mem_chain_clear_buf(&p_port->tx_buf_chain,
                        p_buf_header);
                    lora_buf_mem_mgr_unlock();

                    lora_wan_port_indication(p_port->port_num, &ind_msg);
                    if(is_sync) {
                        sync_obj_release(sync_obj);
                    }
                    is_broken = true;
                    break;
                }
            }
        }
    } while (is_broken);

    if( ! is_broken)  {
        lora_buf_mem_mgr_unlock();
    }

    if( min_period < (uint32_t)-1 ) {
        __log_info("restarting the tx timeout timer period: %d", min_period);
        lora_stub_timer_start(p_port->tx_timeout_timer, min_period);
    } else {
        __log_info("no message with timeout");
    }
}


static void port_rx_timeout_timer_callback(void* data)
{

}

lora_port_error_t lora_wan_port_open(lora_wan_port_t* p_port, int num)
{
    __access_init();

    if(num < __min_port || num > __max_port)
    {
        __log_error("invalid lora port number: %d -- allowed range(%d ~ %d)",
            num, __min_port, __max_port);
        return __PORT_BAD_NUM;
    }

    int idx = num - __min_port;

    __access_lock();
    if( __bitarray_get(open_ports, idx) )
    {
        __access_unlock();
        return __PORT_IN_USE;
    }
    __log_info("ctor() --> port %d", num);

    __bitarray_set(open_ports, idx);
    memset(p_port, 0, sizeof(lora_wan_port_t));

    __adt_list_push(ports_list, p_port);
    p_port->port_num = num;
    p_port->msg_seq_counter = 0;

    // -- channels initialization
    __buf_chain_init(&p_port->rx_buf_chain, "rx-channel",
        lora_stub_sem_new(), buf_chain_sync_wait, buf_chain_sync_signal);
    __buf_chain_init(&p_port->tx_buf_chain, "tx-channel",
        lora_stub_sem_new(), buf_chain_sync_wait, buf_chain_sync_signal);
    __buf_chain_init(&p_port->ind_buf_chain, "ind-channel",
        lora_stub_sem_new(), buf_chain_sync_wait, buf_chain_sync_signal);
    
    // -- channels connections to the mem space manager
    buf_mem_chain_connect(&__buf_chain_mgr_id(_lora_wan_buf_mem_rx),
        &p_port->rx_buf_chain);
    buf_mem_chain_connect(&__buf_chain_mgr_id(_lora_wan_buf_mem_tx),
        &p_port->tx_buf_chain);
    buf_mem_chain_connect(&__buf_chain_mgr_id(_lora_wan_buf_mem_rx),
        &p_port->ind_buf_chain);

    p_port->tx_timeout_timer = lora_stub_timer_init("port-tx-timeout",
        port_tx_timeout_timer_callback, p_port);
    p_port->rx_timeout_timer = lora_stub_timer_init("port-rx-timeout",
        port_rx_timeout_timer_callback, p_port);

    __access_unlock();
    return __PORT_OK;
}

lora_port_error_t lora_wan_port_close(lora_wan_port_t* p_port)
{
    __log_assert(p_port, "invalid port pointer");
    int idx = p_port->port_num - __min_port;
    __access_lock();
    if( __bitarray_get(open_ports, idx) )
    {
        __bitarray_clr(open_ports, idx);
    }
    __log_info("~dtor() --> port %d", p_port->port_num);
    lora_stub_timer_delete(p_port->tx_timeout_timer);
    lora_stub_timer_delete(p_port->rx_timeout_timer);
    buf_mem_chain_disconnect(&__buf_chain_mgr_id(_lora_wan_buf_mem_rx),
        &p_port->rx_buf_chain);
    buf_mem_chain_disconnect(&__buf_chain_mgr_id(_lora_wan_buf_mem_tx),
        &p_port->tx_buf_chain);
    buf_mem_chain_disconnect(&__buf_chain_mgr_id(_lora_wan_buf_mem_rx),
        &p_port->ind_buf_chain);
    __adt_list_del(ports_list, p_port);
    __access_unlock();

    return __PORT_OK;
}

lora_port_error_t lora_port_close_all(void)
{
    lora_wan_port_t* p_port;
    __access_lock();

    do{
        p_port = __adt_list_unshift(ports_list);

        if(p_port)
        {
            __access_unlock();
            lora_wan_port_close(p_port);
            __access_lock();
        }

    } while(p_port != NULL);

    __access_unlock();

    return __PORT_OK;
}

lora_port_error_t lora_wan_port_tx(
    lora_wan_port_t*    p_port,
    lora_tx_params_t*   p_tx_params
    )
{
    lora_port_error_t ret = __PORT_OK;
    int idx = p_port->port_num - __min_port;
    if( idx >= __ports_count )
    {
        __log_error("try sending on invalid port number: %d",
            p_port->port_num);
        return __PORT_BAD_NUM;
    }
    __access_lock();
    if( __bitarray_get(open_ports, idx) == 0 )
    {
        __log_error("try sending on closed port : %d", p_port->port_num);
        __access_unlock();
        return __PORT_NOT_OPENED;
    }

    buf_chain_error_t err;

    lora_wan_port_tx_msg_t msg_header = {0};

    msg_header.msg_seq_num = (p_port->msg_seq_counter)++;
    msg_header.msg_app_id = p_tx_params->msg_app_id;
    msg_header.len = p_tx_params->len;
    msg_header.port_num = p_port->port_num;
    msg_header.retries = p_tx_params->retries;

    if(p_tx_params->sync) {
        msg_header.sync = 1;
        msg_header.sync_obj = sync_obj_acquire("lora-wan-tx-msg");
    }

    if(p_tx_params->timeout) {
        msg_header.has_timeout = 1;
        uint32_t curr_time = lora_stub_get_timestamp_ms();
        msg_header.expire_timestamp = curr_time + p_tx_params->timeout;
        __log_info("PORT-TX::queue() msg_id: %d, tout: %d, expire_at: %d, "
            "curr_time: %d",
            p_tx_params->msg_app_id,
            p_tx_params->timeout,
            msg_header.expire_timestamp, curr_time);
    }

    msg_header.confirm = p_tx_params->confirm;

    __log_info("tx msg[seq:%d , api_id:%d, len:%d] "
        "sync: %s, confirm:%s, timeout:%s ~ %d msec",
        msg_header.msg_seq_num, p_tx_params->msg_app_id, p_tx_params->len,
        g_yes_no[p_tx_params->sync], g_yes_no[p_tx_params->confirm],
        g_yes_no[p_tx_params->timeout ? 1 : 0], p_tx_params->timeout);

    err = buf_mem_chain_write_2(&p_port->tx_buf_chain, (void*)&msg_header,
        sizeof(msg_header), p_tx_params->buf, p_tx_params->len, false);

    __access_unlock();

    if(err == __BUF_CHAIN_OK) {
        if(p_tx_params->timeout) {
            port_restart_tx_timeout_timer(p_port);
        }
        if(p_tx_params->sync) {
            sync_obj_wait(msg_header.sync_obj);
        }
    }
    else if(err == __BUF_CHAIN_NO_SPACE)
        ret = __PORT_NO_MEMORY;
    else
        ret = __PORT_UNNOWN_ERROR;

    return ret;
}

lora_port_error_t lora_wan_port_rx(
    lora_wan_port_t*    p_port,
    lora_rx_params_t*   p_rx_params
    )
{
    lora_port_error_t ret = __PORT_OK;
    int idx = p_port->port_num - __min_port;
    if( idx >= __ports_count )
    {
        __log_error("try receiving on invalid port number: %d",
            p_port->port_num);
        return __PORT_BAD_NUM;
    }
    __access_lock();
    if( __bitarray_get(open_ports, idx) == 0 )
    {
        __log_error("try receiving on closed port : %d", p_port->port_num);
        __access_unlock();
        return __PORT_NOT_OPENED;
    }
    p_port->is_rx_pending = true;
    __access_unlock();

    uint32_t len;
    buf_chain_error_t err;
    err = buf_mem_chain_read(
        &p_port->rx_buf_chain,
        p_rx_params->buf,
        &len,
        p_rx_params->sync);

    if(err == __BUF_CHAIN_OK )
        *p_rx_params->p_len = len;
    else if(err == __BUF_CHAIN_NO_DATA)
        ret = __PORT_NO_RX_DATA;
    else
        ret = __PORT_UNNOWN_ERROR;

    return ret;
}

bool lora_wan_is_pending_tx(void)
{
    lora_wan_port_t* p_port;

    __adt_list_foreach(ports_list, p_port) {
        if( p_port->tx_buf_chain.list )
        {
            return true;
        }
    }
    return false;
}

lora_port_error_t lora_wan_get_tx_data(
    uint8_t*    buf,
    uint8_t*    p_len
    )
{
    lora_wan_port_t* p_port;
    buf_chain_error_t err;
    uint32_t len;
    __adt_list_foreach(ports_list, p_port) {
        err = buf_mem_chain_read(&p_port->tx_buf_chain, buf, &len, false);
        if( err == __BUF_CHAIN_OK ) {
            *p_len = len;
            return __PORT_OK;
        }
    }
    return __PORT_NO_TX_DATA;
}

lora_port_error_t lora_wan_port_rx_indication(
    int         port_num,
    lora_wan_port_ind_msg_t * p_ind_msg,
    uint8_t*    buf,
    uint8_t     len
    )
{
    int idx = port_num - __min_port;
 
    __access_lock();

    if( __bitarray_get(open_ports, idx) == 0 )
    {
        __log_error("incoming rx data on closed port : %d", port_num);
        __access_unlock();
        return __PORT_NOT_OPENED;
    }

    lora_wan_port_t* p_port;
    buf_chain_error_t err;
    __adt_list_foreach(ports_list, p_port) {
        if( p_port->port_num == port_num ) {
            __access_unlock();
            err = buf_mem_chain_write_2(&p_port->rx_buf_chain,
                (void*)p_ind_msg, sizeof(*p_ind_msg), buf, len, false);
            if(err == __BUF_CHAIN_OK)
            {
                if(p_port->callback)
                    p_port->callback(__LORA_EVENT_INDICATION, NULL);
                return __PORT_OK;
            }
            else
            {
                return __PORT_NO_MEMORY;
            }
        }
    }
    __access_unlock();

    return __PORT_NOT_OPENED;
}

lora_port_error_t lora_wan_port_indication(
    int         port_num,
    lora_wan_port_ind_msg_t * p_ind_msg)
{
    int idx = port_num - __min_port;
 
    __access_lock();

    if( __bitarray_get(open_ports, idx) == 0 )
    {
        __log_error("incoming ind msg on closed port : %d", port_num);
        __access_unlock();
        return __PORT_NOT_OPENED;
    }

    lora_wan_port_t* p_port;
    buf_chain_error_t err;
    __adt_list_foreach(ports_list, p_port) {
        if( p_port->port_num == port_num ) {
            __access_unlock();
            err = buf_mem_chain_write(&p_port->ind_buf_chain, 
                (void*)p_ind_msg, sizeof(lora_wan_port_ind_msg_t), true);
            if( err == __BUF_CHAIN_OK )
                p_port->callback(__LORA_EVENT_INDICATION, NULL);
            return __PORT_OK;
        }
    }
    __access_unlock();

    return __PORT_NOT_OPENED;
}

lora_port_error_t lora_wan_port_get_ind_params(
    lora_wan_ind_params_t * p_ind_param)
{
    lora_wan_port_ind_msg_t ind_msg;
    uint32_t len;
    lora_wan_port_t* p_port;
    buf_chain_error_t err;
    {
        __adt_list_foreach(ports_list, p_port) {
            err = buf_mem_chain_read(&p_port->ind_buf_chain, (void*)&ind_msg,
                &len, false);
            if(err == __BUF_CHAIN_OK) {
                switch( ind_msg.type ) {
                case __IND_TX_TIMEOUT:
                    p_ind_param->event = __LORA_EVENT_TX_TIMEOUT;
                    break;
                case __IND_TX_DONE:
                    p_ind_param->event = __LORA_EVENT_TX_DONE;
                    break;
                case __IND_TX_FAIL:
                    p_ind_param->event = __LORA_EVENT_TX_FAIL;
                    break;
                case __IND_TX_CONFIRM:
                    p_ind_param->event = __LORA_EVENT_TX_CONFIRM;
                    break;
                case __IND_RX_TIMEOUT:
                    p_ind_param->event = __LORA_EVENT_RX_TIMEOUT;
                    break;
                case __IND_RX_DONE:
                    /* rx done comes through the rx channels along with its
                       indication parameters */
                    return __PORT_UNNOWN_ERROR;
                }
                p_ind_param->tx.msg_app_id = ind_msg.ind_params.tx.msg_app_id;
                p_ind_param->tx.msg_seq_num = ind_msg.ind_params.tx.msg_seq_num;
                p_ind_param->tx.tx_power = ind_msg.ind_params.tx.tx_power;
                p_ind_param->tx.data_rate = ind_msg.ind_params.tx.data_rate;
                p_ind_param->tx.ul_frame_counter =
                    ind_msg.ind_params.tx.ul_frame_counter;
                p_ind_param->port_num = p_port->port_num;
                return __PORT_OK;
            }
        }
    }

    // -- check receive channel
    {
        __adt_list_foreach(ports_list, p_port) {
            uint32_t len;
            err = buf_mem_chain_read_2(&p_port->rx_buf_chain, (void*)&ind_msg,
                sizeof(ind_msg), p_ind_param->buf, &len, false);
            p_ind_param->len = len - sizeof(ind_msg);
            if(err == __BUF_CHAIN_OK)
            {
                p_ind_param->event = __LORA_EVENT_RX_DONE;
                p_ind_param->port_num = p_port->port_num;
                p_ind_param->rx.dl_frame_counter =
                    ind_msg.ind_params.rx.dl_frame_counter;
                p_ind_param->rx.rssi = ind_msg.ind_params.rx.rssi;
                p_ind_param->rx.snr = ind_msg.ind_params.rx.snr;
                p_ind_param->rx.data_rate = ind_msg.ind_params.rx.data_rate;
                return __PORT_OK;
            }
        }
    }

    p_ind_param->event = __LORA_EVENT_NONE;
    return __PORT_OK;
}

/* --- end of file ---------------------------------------------------------- */
