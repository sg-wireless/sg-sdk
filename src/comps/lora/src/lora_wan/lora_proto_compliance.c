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
 * @brief   interfacing component to the semtech compliance protocol component.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

#define __log_subsystem     lora_pkg
#define __log_component     wan_compli
#include "log_lib.h"

#include "LoRaMac.h"
#include "LmHandler.h"
#include "LmHandlerTypes.h"
#include "LmhPackage.h"
#include "LmhpCompliance.h"
#include "lora_wan_duty.h"
#include "lora_mac_handler.h"
#include "lora_nvm.h"
#include "lora_wan_process.h"

#define __fw_version        0x01030000

/** -------------------------------------------------------------------------- *
 * Implementation
 * --------------------------------------------------------------------------- *
 */

static void OnTxPeriodicityChanged( uint32_t periodicity );
static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed );
static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity );

static bool s_is_confirmed = false;

static LmhpComplianceParams_t s_LmhpComplianceParams =
{
    .FwVersion.Value = __fw_version,
    .OnTxPeriodicityChanged = OnTxPeriodicityChanged,
    .OnTxFrameCtrlChanged = OnTxFrameCtrlChanged,
    .OnPingSlotPeriodicityChanged = OnPingSlotPeriodicityChanged,
};

#define __log_callback(__cb_str) \
    __log_info(__yellow__"compliance callback"__default__" --> " \
                __cyan__ __cb_str);

/** -------------------------------------------------------------------------- *
 * LCTT reset request state saving, so that the device can detect that it is
 * powered up after DutResetReq command and can start re-join again.
 * --------------------------------------------------------------------------- *
 */
static struct {
    bool dut_reset_requested;
    lora_nvm_record_tail_t record_tail; // -- needed by the lora_nvm.h
} s_compliance_settings;

static const char* s_lora_compliance_nvm_key = "lora-lct";

static void lora_compliance_nvm_load_defaults_callback(void* ptr, uint32_t size)
{
    s_compliance_settings.dut_reset_requested = false;
}
static void lora_compliance_handle_nvm(void)
{
    lora_nvm_handle_change(
        s_lora_compliance_nvm_key,
        &s_compliance_settings,
        sizeof(s_compliance_settings),
        lora_compliance_nvm_load_defaults_callback
        );
}

void lora_compliance_reset_dut(void)
{
    s_compliance_settings.dut_reset_requested = true;
    lora_compliance_handle_nvm();
}

bool lora_compliance_reset_state(void)
{
    bool ret = s_compliance_settings.dut_reset_requested;
    s_compliance_settings.dut_reset_requested = false;
    lora_compliance_handle_nvm();
    return ret;
}

bool lora_proto_compliance_get_state(void)
{
    MibRequestConfirm_t mib_req;

    mib_req.Type = MIB_IS_CERT_FPORT_ON;
    LoRaMacMibGetRequestConfirm( &mib_req );

    return mib_req.Param.IsCertPortOn;
}

bool lora_proto_compliance_set_state(bool state)
{
    bool curr_state = lora_proto_compliance_get_state();

    if(curr_state != state)
    {
        __log_info("compliance test switch %s", g_on_off[state == true]);
        MibRequestConfirm_t mib_req;
        mib_req.Type = MIB_IS_CERT_FPORT_ON;
        mib_req.Param.IsCertPortOn = state;

        if(LoRaMacMibSetRequestConfirm( &mib_req ) != LORAMAC_STATUS_OK)
        {
            __log_error("failed to switch compliance test from %s to %s",
                g_on_off[curr_state], g_on_off[state]);
            return false;
        }
    }

    return true;
}

void lora_proto_compliance_process_duty_cycle(void)
{
    lmh_send(NULL, 0, 0, s_is_confirmed);
}

static void OnTxPeriodicityChanged( uint32_t periodicity )
{
    __log_callback("on tx periodicity changed");
    lora_wan_duty_compliance_tx_periodicity(periodicity);
}

static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed )
{
    __log_callback("on tx frame ctrl changed");
    s_is_confirmed = isTxConfirmed == LORAMAC_HANDLER_CONFIRMED_MSG;
}

static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity )
{
    __log_callback("on ping slot periodicity changeed - not implemented yet");
}

void lora_proto_compliance_init(void)
{
    lora_compliance_handle_nvm();
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &s_LmhpComplianceParams );
}

void lora_proto_compliance_notify_fport_224_disable(void)
{
    __log_info("on fport 224 disable");
    lora_wan_process_request(__LORA_WAN_PROCESS_LCT_MODE_EXIT, NULL);
}

/* --- end of file ---------------------------------------------------------- */
