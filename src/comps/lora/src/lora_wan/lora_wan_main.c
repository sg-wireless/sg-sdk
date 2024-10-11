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
 * @brief   lora-wan mode manager sub-component
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include <stdint.h>

#define __log_subsystem  lora
#define __log_component  wan_api
#include "log_lib.h"
#include "lora.h"
#include "lora_wan_nvm.h"
#include "lora_event_handler.h"

#include "LoRaMac.h"
#include "LmHandler.h"
#include "RegionCommon.h"
#include "secure-element.h"
#include "radio.h"
#include "system/systime.h"
#include "lora_mac_utils.h"
#include "lora_mac_handler.h"
#include "lora_commission.h"
#include "stub_timers.h"
#include "lora_wan_process.h"
#include "lora_wan_duty.h"
#include "lora_wan_port.h"
#include "lora_nvm.h"
#include "lora_mode.h"
#include "utils_bitarray.h"
#include "lora_proto_compliance.h"
#include "lora_wan_radio_process.h"

/** -------------------------------------------------------------------------- *
 * applications ports alloc/free
 * --------------------------------------------------------------------------- *
 */
#define __max_app_ports  10
static lora_wan_port_t s_ports[__max_app_ports];
static __bitarray_def(allocated_ports, __max_app_ports);
static lora_wan_port_t* port_alloc(void)
{
    int i;
    for(i = 0; i < __max_app_ports; ++i)
    {
        if( __bitarray_get(allocated_ports, i) == 0 )
        {
            __bitarray_set(allocated_ports, i);
            return &s_ports[i];
        }
    }
    return NULL;
}
static void port_free(lora_wan_port_t* p_port)
{
    int idx = ((uint8_t*)p_port - (uint8_t*)s_ports) / sizeof(lora_wan_port_t);

    if(idx < __max_app_ports)
        __bitarray_clr(allocated_ports, idx);
}
static void port_free_all(void)
{
    int i;
    for(i = 0; i < __max_app_ports; ++i)
    {
        if( __bitarray_get(allocated_ports, i) )
        {
            __bitarray_clr(allocated_ports, i);
        }
    }
}
static lora_wan_port_t* port_get(int port_num)
{
    int i;
    for(i = 0; i < __max_app_ports; ++i)
    {
        if( __bitarray_get(allocated_ports, i) &&
            s_ports[i].port_num == port_num)
        {
            return &s_ports[i];
        }
    }
    return NULL;
}
static void port_set_common_callback(void* arg)
{
    int i;
    for(i = 0; i < __max_app_ports; ++i)
    {
        if( __bitarray_get(allocated_ports, i) )
        {
            s_ports[i].callback = arg;
        }
    }
}

/** -------------------------------------------------------------------------- *
 * LCT ( LoRa Certification Test ) mode management for end-devices that are
 * targeted to be sent to the ATH (Authorization House).
 * Those devices shall be in the production mode with test port opened
 * by default and the system in the LCT mode of interaction.
 * Once the port is closed, the system will operate in the normal application
 * mode forever and the test port can not be opened again.
 * --------------------------------------------------------------------------- *
 */
#ifdef CONFIG_LORA_LCT_INIT_DEVICE_INTO_CERTIFICATION_MODE_ONCE
static struct {
    bool lct_state;
    lora_nvm_record_tail_t record_tail; // -- needed by the lora_nvm.h
} s_lct_ath_settings;

static const char* s_lct_ath_nvm_key = "lct-ath";

static void lct_ath_nvm_load_defaults_callback(void* ptr, uint32_t size)
{
    (void) ptr; (void) size;
    s_lct_ath_settings.lct_state = true;
}
static void lct_ath_nvm_handle_nvm(void)
{
    lora_nvm_handle_change(
        s_lct_ath_nvm_key,
        &s_lct_ath_settings,
        sizeof(s_lct_ath_settings),
        lct_ath_nvm_load_defaults_callback
        );
}
#endif /* CONFIG_LORA_LCT_INIT_DEVICE_INTO_CERTIFICATION_MODE_ONCE */

/** -------------------------------------------------------------------------- *
 * LoRa APIs Definition
 * --------------------------------------------------------------------------- *
 */
static lora_error_t lora_wan_ctor(void)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora wan ctor()");

    void lw_radio_process_ctor(void);
    lw_radio_process_ctor();

    lora_commission_ctor();

    // -- lora mac handler
    extern void lmh_init(void);
    lmh_init();

    #ifdef CONFIG_LORA_LCT_MODE

    #ifdef CONFIG_LORA_LCT_INIT_DEVICE_INTO_CERTIFICATION_MODE_ONCE

    /**
     * By calling the ath_nvm handler, the ath:lct-state will become true for
     * the first time only. and the LoRaMAC compliance state will be activated
     * and the system will become ready for LCT testing.
     * The ath:lct-state will be stored as false directly after setting the
     * LCT testing mode. hence after exiting from the LCT testing mode by the
     * TCL itself, the system will not come back to this mode again.
     * 
     * Note, if the NVM flash is erased, the system will enter it again.
     */
    lct_ath_nvm_handle_nvm();
    if( s_lct_ath_settings.lct_state )
    {
        /**
         * init the certification mode only once for the ATH.
         */
        __log_info("open the certification mode for the device");
        lora_proto_compliance_set_state(true);
        s_lct_ath_settings.lct_state = false;
        lct_ath_nvm_handle_nvm();
    } else {
        /**
         * nothing to do, if the certification mode state should remain the
         * same until the TCL (Test Control Layer) orders to close it, then
         * it will not be opened again for this device.
         */
    }

    #else /* CONFIG_LORA_LCT_INIT_DEVICE_INTO_CERTIFICATION_MODE_ONCE */

        /**
         * The certification mode enable/disable will be done using the control
         * APIs.
         * The state here after initialization will be kept as in the previous
         * working state before device reset or stack re-initialization.
         */

    #endif /* CONFIG_LORA_LCT_INIT_DEVICE_INTO_CERTIFICATION_MODE_ONCE */

    #else /* CONFIG_LORA_LCT_MODE */

    /*
     * default certification mode for mass production devices are disables
     */
    __log_info("close certification mode for ever");
    lora_proto_compliance_set_state(false);

    #endif /* CONFIG_LORA_LCT_MODE */

    lora_wan_duty_ctor();
    lora_wan_process_ctor();

    return ret;
}
static lora_error_t lora_wan_dtor(void)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora wan ~dtor()");

    lora_wan_duty_dtor();
    lora_stub_timers_stop_all();
    lora_wan_process_dtor();

    LoRaMacStop();

    if( LoRaMacDeInitialization() != LORAMAC_STATUS_OK ) {
        __log_error("lora mac deinit failed");
    }

    lora_port_close_all();
    port_free_all();

    /** TODO: reset radio and put it into sleep */
    Radio.Sleep();

    return ret;
}
static lora_error_t lora_wan_stats(void)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora wan stats()");

    lora_utils_stats();

    return ret;
}
static lora_error_t lora_wan_tx(lora_tx_params_t * p_tx_params)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora wan tx()");

    lora_wan_port_t * p_port = port_get(p_tx_params->port);

    if(p_port == NULL)
    {
        __log_error("msg tx failed: port %d not opened", p_tx_params->port);
        ret = __LORA_ERROR;
    }
    else
    {
        __log_debug("tx over port %d, p_port: %p", p_port->port_num, p_port);
        if(lora_wan_port_tx(p_port, p_tx_params) != __PORT_OK)
        {
            __log_error("msg tx failed");
        }
        else
        {
            __log_info("msg tx success");
        }
    }

    return ret;
}
static lora_error_t lora_wan_rx(lora_rx_params_t * p_rx_params)
{
    lora_error_t ret = __LORA_OK;

    __log_info("lora wan rx()");

    return ret;
}

static lora_event_callback_t* p_common_callback;

static lora_error_t lora_wan_ioctl(uint32_t ioctl, void* arg)
{
    lora_error_t ret = __LORA_OK;

    if( ioctl == __LORA_IOCTL_SET_CALLBACK )
    {
        __log_info("ioctl -> set callback");
        lora_callback_t * p_cb = arg;

        if( p_cb->port ==__port_any )
        {
            p_common_callback = p_cb->callback;
            port_set_common_callback(p_common_callback);
        }
        else
        {
            lora_wan_port_t * p_port = port_get(p_cb->port);

            if( p_port )
            {
                __log_error("set callback successful on port %d", p_cb->port);
                p_port->callback = p_cb->callback;
            }
            else
            {
                __log_error("set callback on closed port %d", p_cb->port);
            }
        }
    }
    else if( ioctl == __LORA_IOCTL_CHECK_COMMISSION )
    {
        __log_info("ioctl -> check commissoining parameters");
        return lora_commission_check( arg ) ? __LORA_OK : __LORA_ERROR;
    }
    else if( ioctl == __LORA_IOCTL_SET_COMMISSION )
    {
        if(! lora_commission_check( arg ))
        {
            __log_info("ioctl -> set commissoining parameters");
            lora_commission_set( arg );
            lora_wan_process_request(__LORA_WAN_PROCESS_COMMISSION, NULL);
        }
        else
        {
            __log_info("ioctl -> device is already commissioned with the"
                "given parameters");
        }
    }
    else if( ioctl == __LORA_IOCTL_JOIN )
    {
        __log_info("ioctl -> join request");
        lora_wan_process_request(__LORA_WAN_PROCESS_JOIN_REQUEST, NULL);
    }
    else if( ioctl == __LORA_IOCTL_JOIN_STATUS )
    {
        __log_info("ioctl -> get join status");
        lora_wan_join_status_req_t req = {
            .sync_obj = sync_obj_acquire("join-status-req")
        };
        lora_wan_process_request(__LORA_WAN_PROCESS_JOIN_STATUS_REQ, &req);
        sync_obj_wait(req.sync_obj);
        sync_obj_release(req.sync_obj);
        *((bool*)arg) = req.is_joined;
    }
    else if( ioctl == __LORA_IOCTL_DUTY_CYCLE_SET )
    {
        uint32_t period = *(uint32_t*)arg;
        __log_info("ioctl -> set duty-cycle period: %d msec", period);
        lora_wan_duty_set(period);
    }
    else if( ioctl == __LORA_IOCTL_DUTY_CYCLE_GET )
    {
        uint32_t period = lora_wan_duty_get();
        __log_info("ioctl -> set duty-cycle period: %d msec", period);
        *(uint32_t*)arg = period;
    }
    else if( ioctl == __LORA_IOCTL_DUTY_CYCLE_START )
    {
        __log_info("ioctl -> start duty-cycle");
        lora_wan_duty_start();
    }
    else if( ioctl == __LORA_IOCTL_DUTY_CYCLE_STOP )
    {
        __log_info("ioctl -> stop duty-cycle");
        lora_wan_duty_stop();
    }
    else if( ioctl == __LORA_IOCTL_IS_PENDING_TX )
    {
        __log_info("ioctl -> is there any pending TX messages");
        *((bool*)arg) = lora_wan_is_pending_tx() || lora_wan_process_busy();
    }
    else if( ioctl == __LORA_IOCTL_ENABLE_RX_LISTENING )
    {
        __log_info("ioctl -> enable rx litening");
        lora_wan_enable_rx_listening();
    }
    else if( ioctl == __LORA_IOCTL_DISABLE_RX_LISTENING )
    {
        __log_info("ioctl -> disable rx litening");
        lora_wan_disable_rx_listening();
    }
    else if( ioctl == __LORA_IOCTL_PORT_OPEN )
    {
        uint32_t port_num = *(uint32_t*)arg;
        __log_info("ioctl -> port open: %d", port_num);

        lora_wan_port_t* p_port = port_get(port_num);

        if(p_port)
        {
            __log_warn("port %d already opened", port_num);
        }
        else
        {
            p_port = port_alloc();
            
            if(p_port == NULL)
            {
                __log_warn("exceeds maximum allowed open ports");
            }
            else
            {
                if(lora_wan_port_open(p_port, port_num) == __PORT_OK)
                {
                    __log_info("port %d opened successfully: %p",
                        port_num, p_port);
                    if(p_common_callback)
                        p_port->callback = p_common_callback;
                }
                else
                {
                    __log_error("port %d open error", port_num);
                    port_free(p_port);
                }
            }
        }
    }
    else if( ioctl == __LORA_IOCTL_PORT_CLOSE )
    {
        uint32_t port_num = *(uint32_t*)arg;
        __log_info("ioctl -> port close: %d", port_num);

        lora_wan_port_t* p_port = port_get(port_num);

        if( p_port == NULL )
        {
            __log_warn("port %d is already closed", port_num);
        }
        else
        {
            if( lora_wan_port_close(p_port) == __PORT_OK )
            {
                __log_info("port %d closed successfully", port_num);
                memset(p_port, 0, sizeof(lora_wan_port_t));
            }
            else
            {
                __log_warn("port %d is already closed", port_num);
            }
            port_free(p_port);
        }
    }
    else if( ioctl == __LORA_IOCTL_PORT_GET_IND_PARAM )
    {
        __log_info("ioctl -> get available indication");
        lora_wan_port_get_ind_params(arg);
    }
    else if( ioctl == __LORA_IOCTL_GET_PARAM )
    {
        __log_info("ioctl -> get lorawan param");
        lora_wan_param_t * p_param = arg;
        if( p_param->type == __LORA_WAN_PARAM_REGION) {
            p_param->param.region = lmh_get_region();
        } else if (p_param->type == __LORA_WAN_PARAM_CLASS) {
            p_param->param.class = lmh_get_class();
        } else if (p_param->type == __LORA_WAN_PARAM_PAYLOAD) {
            p_param->param.payload = lmh_get_tx_payload_size();
        } else if (p_param->type == __LORA_WAN_PARAM_SYS_RX_ERR) {
            p_param->param.sys_rx_err = lmh_get_sys_rx_error();
        } else if (p_param->type == __LORA_WAN_PARAM_CAL_ENABLE) {
            p_param->param.cal_enable = lw_rxwin_calibration_get_enablement();
        } else if (p_param->type == __LORA_WAN_PARAM_CAL_RXWIN_TIME_SHIFT) {
            p_param->param.cal_time_shift =
                lw_rxwin_calibration_get_time_shift();
        } else if (p_param->type == __LORA_WAN_PARAM_CAL_RXWIN_EXTENSION) {
            p_param->param.cal_time_extension =
                lw_rxwin_calibration_get_time_extension();
        } else {
            __log_error("unknown lorawan parameter : %d", p_param->type);
        }
    }
    else if( ioctl == __LORA_IOCTL_SET_PARAM )
    {
        __log_info("ioctl -> set lorawan param");
        lora_wan_param_t * p_param = arg;
        if( p_param->type == __LORA_WAN_PARAM_REGION) {
            if( lmh_get_region() !=  p_param->param.region)
            {
                lmh_set_region(p_param->param.region);
                lora_wan_dtor();
                lora_nvm_clear_all();
                lora_wan_ctor();
            }
        } else if (p_param->type == __LORA_WAN_PARAM_CLASS) {
            lmh_set_class(p_param->param.class);
        } else if (p_param->type == __LORA_WAN_PARAM_SYS_RX_ERR) {
            lmh_set_sys_rx_error(p_param->param.sys_rx_err);
        } else if (p_param->type == __LORA_WAN_PARAM_CAL_ENABLE) {
            lw_rxwin_calibration_set_enablement(p_param->param.cal_enable);
        } else if (p_param->type == __LORA_WAN_PARAM_CAL_RXWIN_TIME_SHIFT) {
            lw_rxwin_calibration_set_time_shift(p_param->param.cal_time_shift);
        } else if (p_param->type == __LORA_WAN_PARAM_CAL_RXWIN_EXTENSION) {
            lw_rxwin_calibration_set_time_extension(
                p_param->param.cal_time_extension);
        } else {
            __log_error("unknown lorawan parameter : %d", p_param->type);
        }
    }
    #ifdef CONFIG_LORA_LCT_CONTROL_API
    else if( ioctl == __LORA_IOCTL_LCT_MODE_GET )
    {
        __log_info("ioctl -> get lorawan lct mode");
        bool * p_lct_state = (bool*)arg;
        *p_lct_state = lora_proto_compliance_get_state();
    }
    else if( ioctl == __LORA_IOCTL_LCT_MODE_SET )
    {
        bool lct_state = *(bool*)arg;
        bool curr_state = lora_proto_compliance_get_state();
        __log_info("ioctl -> lorawan lct mode %s ( %s )",
                lct_state != curr_state ? "changes to" : "no change",
                g_on_off[ lct_state == true]);
        lora_wan_process_request(lct_state
            ? __LORA_WAN_PROCESS_LCT_MODE_ENTER
            : __LORA_WAN_PROCESS_LCT_MODE_EXIT,
            NULL);
    }
    #endif /* CONFIG_LORA_LCT_CONTROL_API */
    else if( ioctl == __LORA_IOCTL_TOGGLE_RXWIN_VERBOSITY )
    {
        lm_rxwin_toggle_debug_verbosity();
    }
    else
    {
        __log_info("ioctl -> "__red__"unknown lora wan ioctl"__default__);
    }

    return ret;
}

static lora_mode_interface_t lora_wan_instance = {
    .mode_ctor  = lora_wan_ctor,
    .mode_dtor  = lora_wan_dtor,
    .mode_stats = lora_wan_stats,
    .mode_tx    = lora_wan_tx,
    .mode_rx    = lora_wan_rx,
    .mode_ioctl = lora_wan_ioctl
};

lora_mode_interface_t* lora_wan_get_interface(void)
{
    return & lora_wan_instance;
}

/* --- end of file ---------------------------------------------------------- */
