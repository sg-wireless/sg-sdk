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

#define __fw_version        0x01030000

/** -------------------------------------------------------------------------- *
 * Implementation
 * --------------------------------------------------------------------------- *
 */

static void OnTxPeriodicityChanged( uint32_t periodicity );
static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed );
static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity );

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

static void OnTxPeriodicityChanged( uint32_t periodicity )
{
    __log_callback("on tx periodicity changed");
    lora_wan_duty_conformance_tx_periodicity(periodicity);
}

static void OnTxFrameCtrlChanged( LmHandlerMsgTypes_t isTxConfirmed )
{
    __log_callback("on tx frame ctrl changed");
}

static void OnPingSlotPeriodicityChanged( uint8_t pingSlotPeriodicity )
{
    __log_callback("on ping slot periodicity changeed");
}

void lora_proto_compliance_init(void)
{
    LmHandlerPackageRegister( PACKAGE_ID_COMPLIANCE, &s_LmhpComplianceParams );
}

/* --- end of file ---------------------------------------------------------- */
