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
 * @brief   Specifies the lora-mac handler interface component.
 *          the original lora-mac handler component exists at:
 *              ext/LoRaMac-node/src/apps/LoRaMac/common/LmHandler/
 *          This component here in this file is an interface between the
 *          lora-stack and the Semtech LmHandler component
 * --------------------------------------------------------------------------- *
 */
/* --- include -------------------------------------------------------------- */

#include <stdint.h>

#define __log_subsystem  lora
#define __log_component  wan_lmh
#include "log_lib.h"

#include "lora.h"
#include "lora_wan_nvm.h"
#include "lora_event_handler.h"
#include "lora_mac_utils.h"
#include "lora_commission.h"
#include "LmHandler.h"
#include "LmhpRemoteMcastSetup.h"
#include "LmhpClockSync.h"
#include "Commissioning.h"
#include "RegionCommon.h"
#include "secure-element.h"
#include "radio.h"
#include "system/systime.h"
#include "stub_timers.h"
#include "lora_wan_process.h"
#include "lora_mac_handler.h"
#include "lora_nvm.h"
#include "lora_common_utils.h"

/** -------------------------------------------------------------------------- *
 * loramac handler initial configurations
 * --------------------------------------------------------------------------- *
 */
#define __config_app_tx_duty_cycle              5000
#define __config_app_tx_durt_cycle_random       1000
#define __config_lorawan_adr_state              LORAMAC_HANDLER_ADR_OFF
#define __config_lorawan_default_datarate       DR_0
#define __config_lorawan_public_network         true
#define __config_default_message_confirmation   LORAMAC_HANDLER_UNCONFIRMED_MSG
#define __config_app_data_buffer_max_size       242
#define __config_lorawan_duty_cycle_on          false
#define __config_lorawan_port                   1
#define __config_lora_active_region             LORAMAC_REGION_EU868

/** -------------------------------------------------------------------------- *
 * lora-wan nvm application parameters
 * --------------------------------------------------------------------------- *
 */

typedef struct {
    LoRaMacRegion_t         region;
    lora_wan_class_t        class;
    lora_nvm_record_tail_t  nvm_record_tail;
} lora_wan_app_nvm_data_t;

static lora_wan_app_nvm_data_t s_lora_wan_app_nvm_data;

static const char* s_lora_wan_app_nvm_data_key = "lorawan-app";

static bool s_is_class_c_temp_session = false;

static void lora_wan_app_data_defaults( void* p_record_mem, uint32_t size )
{
    memset(p_record_mem, 0, sizeof(size));
    s_lora_wan_app_nvm_data.region = __config_lora_active_region;
    s_lora_wan_app_nvm_data.class = __LORA_WAN_CLASS_A;
}
static void lora_wan_handle_app_nvm_data_change(void)
{
    __log_info("handle lora-wan app data NvM");

    lora_nvm_handle_change(
        s_lora_wan_app_nvm_data_key,
        &s_lora_wan_app_nvm_data,
        sizeof(lora_wan_app_nvm_data_t),
        lora_wan_app_data_defaults
    );
}

/** -------------------------------------------------------------------------- *
 * loramac handler init proc
 * --------------------------------------------------------------------------- *
 */
static uint8_t cb_GetBatteryLevel( void );
static float cb_GetTemperature( void );
static uint32_t cb_GetRandomSeed( void );
static void cb_OnMacProcess( void );
static void cb_OnNvmDataChange(
    LmHandlerNvmContextStates_t state, uint16_t size);
static void cb_OnNetworkParametersChange( CommissioningParams_t *params );
static void cb_OnMacMcpsRequest(
    LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxDelay );
static void cb_OnMacMlmeRequest(
    LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxDelay );
static void cb_OnJoinRequest( LmHandlerJoinParams_t *params );
static void cb_OnTxData( LmHandlerTxParams_t *params );
static void cb_OnRxData(
    LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);
static void cb_OnClassChange( DeviceClass_t deviceClass );
static void cb_OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t *params );
static void cb_OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection );

static LmHandlerCallbacks_t s_lmh_callbacks = {
    #define __init_cb(__cb) .__cb = cb_ ## __cb
    __init_cb(GetBatteryLevel),
    __init_cb(GetTemperature),
    __init_cb(GetRandomSeed),
    __init_cb(OnMacProcess),
    __init_cb(OnNvmDataChange),
    __init_cb(OnNetworkParametersChange),
    __init_cb(OnMacMcpsRequest),
    __init_cb(OnMacMlmeRequest),
    __init_cb(OnJoinRequest),
    __init_cb(OnTxData),
    __init_cb(OnRxData),
    __init_cb(OnClassChange),
    __init_cb(OnBeaconStatusChange),
    __init_cb(OnSysTimeUpdate),
    #undef __init_cb
};

static uint8_t s_package_data_buffer[__config_app_data_buffer_max_size];

static LmHandlerParams_t s_lmh_params = {
    .Region                 = __config_lora_active_region,
    .AdrEnable              = __config_lorawan_adr_state,
    .IsTxConfirmed          = __config_default_message_confirmation,
    .TxDatarate             = __config_lorawan_default_datarate,
    .PublicNetworkEnable    = __config_lorawan_public_network,
    .DutyCycleEnabled       = __config_lorawan_duty_cycle_on,
    .DataBufferMaxSize      = __config_app_data_buffer_max_size,
    .DataBuffer             = s_package_data_buffer,
    .PingSlotPeriodicity    = REGION_COMMON_DEFAULT_PING_SLOT_PERIODICITY,
};

void lmh_init( void )
{
    __log_info("start lmh re-init");

    lora_wan_handle_app_nvm_data_change();
    s_lmh_params.Region = s_lora_wan_app_nvm_data.region;

    if(LmHandlerInit( &s_lmh_callbacks, &s_lmh_params ) !=
            LORAMAC_HANDLER_SUCCESS) {
        __log_error("loramac handler init failed");
        return;
    }

    #define __max_rx_error      100
    LmHandlerSetSystemMaxRxError( __max_rx_error );

    extern void lora_proto_compliance_init(void);
    lora_proto_compliance_init();

    LmHandlerPackageRegister( PACKAGE_ID_CLOCK_SYNC, NULL );
    LmHandlerPackageRegister( PACKAGE_ID_REMOTE_MCAST_SETUP, NULL );

    extern void lora_proto_fragmentation_init(void);
    lora_proto_fragmentation_init();

    // -- important to synchronize the device time with the network
    LmHandlerDeviceTimeReq();

    __log_info("-- lora mac handler init done --");
}

void lmh_reset(bool purge_nvm)
{
    lora_stub_timers_stop_all();

    if( LoRaMacDeInitialization() != LORAMAC_STATUS_OK ) {
        __log_error("lora mac deinit failed");
    }

    if( purge_nvm )
    {
        // purge the NVM for the lora mac stuff
        lora_nvm_clear_all();
    }

    lmh_init();
}

void lmh_process(void)
{
    LmHandlerProcess();
}

bool lmh_is_busy(void)
{
    return LmHandlerIsBusy();
}

void lmh_join( void )
{
    LmHandlerJoin();
}

static lmh_cb_on_mac_tx_t * on_mac_tx;
static lmh_cb_on_mac_rx_t * on_mac_rx;

void lmh_callbacks(lmh_callbacks_t * p_callbacks)
{
    on_mac_tx = p_callbacks->on_mac_tx;
    on_mac_rx = p_callbacks->on_mac_rx;
}

void lmh_set_region(lora_region_t region)
{
    LoRaMacRegion_t reg = get_lora_mac_region_enum_value(region);
    if( reg != __lora_common_utils_invalid_mac_region ) {
        s_lora_wan_app_nvm_data.region = reg;
        lora_wan_handle_app_nvm_data_change();
    }
    __log_error("invalid region %d", region);
}

lora_region_t lmh_get_region(void)
{
    return get_lora_api_region_enum_value(s_lora_wan_app_nvm_data.region);
}

void lmh_set_class(lora_wan_class_t class)
{
    s_lora_wan_app_nvm_data.class = class;
    lora_wan_handle_app_nvm_data_change();

    lora_wan_process_request(__LORA_WAN_PROCESS_REQ_CLASS, NULL);
}

void lmh_start_class_c_temp_session(void)
{
    s_is_class_c_temp_session = true;

    lora_wan_process_request(__LORA_WAN_PROCESS_REQ_CLASS, NULL);
}

void lmh_stop_class_c_temp_session(void)
{
    s_is_class_c_temp_session = false;

    lora_wan_process_request(__LORA_WAN_PROCESS_REQ_CLASS, NULL);
}

lora_wan_class_t lmh_get_class(void)
{
    return s_lora_wan_app_nvm_data.class;
}

void lmh_change_class(void)
{
    lora_wan_class_t requested_class;
    if( s_is_class_c_temp_session )
    {
        requested_class = __LORA_WAN_CLASS_C;
    }
    else
    {
        requested_class = s_lora_wan_app_nvm_data.class;
    }
    /* compare the current operating class with the requested class */
    if((int)LmHandlerGetCurrentClass() ==  (int)requested_class)
    {
        __log_info("req class is already active -- no need to change");
        lora_wan_process_request(__LORA_WAN_PROCESS_CLASS_CHANGED, NULL);
    }
    else
    {
        if(LmHandlerGetCurrentClass() > 0 && requested_class > 0)
        {
            /* if requested class is C while in B or vice-versa
             * change to class A first */
            __log_info("change to class A first");
            LmHandlerRequestClass(CLASS_A);

        }
        __log_info("change to class %c", 'A' + requested_class);
        LmHandlerRequestClass(requested_class);
    }
}

/** -------------------------------------------------------------------------- *
 * loramac handler tx and rx APIs
 * --------------------------------------------------------------------------- *
 */
static uint8_t s_rx_data_buf[__config_app_data_buffer_max_size];
static LmHandlerAppData_t s_app_data =
{
    .Buffer = s_rx_data_buf,
    .BufferSize = 0,
    .Port = 0,
};

static bool s_compliance_enabled = false;
static bool s_compliance_tx_confirm = false;
void lmh_compliance_frame_ctrl(bool isTxConfirmed)
{
    s_compliance_enabled = true;
    s_compliance_tx_confirm = isTxConfirmed;
}

lora_error_t lmh_send(uint8_t * buf, uint8_t len, uint8_t port, bool confirm)
{
    if( len <= __config_app_data_buffer_max_size ) {

        memcpy(s_app_data.Buffer, buf, len);
        s_app_data.BufferSize = len;
        s_app_data.Port = port;

        #define __app_confirm               (confirm)
        #define __compliance_enable         (s_compliance_enabled)
        #define __compliance_confirm        (s_compliance_tx_confirm)
        #define __confirm_condition         \
            ( (__app_confirm && !(__compliance_enable) ) || \
              (__compliance_enable && __compliance_confirm) )

        if(LmHandlerSend( &s_app_data, __confirm_condition ) ==
            LORAMAC_HANDLER_SUCCESS) {
            return __LORA_OK;
        }
    }
    return __LORA_ERROR;
}

uint8_t lmh_get_tx_payload_size(void)
{
    LoRaMacTxInfo_t tx_info;
    LoRaMacStatus_t status;

    status = LoRaMacQueryTxPossible( 0, &tx_info );

    if(LORAMAC_STATUS_OK != status)
    {
        return 0;
    }

    return tx_info.MaxPossibleApplicationDataSize;
}

/** -------------------------------------------------------------------------- *
 * loramac handler callbacks
 * --------------------------------------------------------------------------- *
 */

#define __log_callback(__cb_str) \
    __log_info(__yellow__"callback"__default__" --> "__cyan__ __cb_str);

static uint8_t cb_GetBatteryLevel( void )
{
    __log_callback("get battery level");
    return 0;
}
static float cb_GetTemperature( void )
{
    __log_callback("get temprature");
    return 0.0f;
}
static uint32_t cb_GetRandomSeed( void )
{
    __log_callback("get random seed");
    return TimerGetCurrentTime();
}
static void cb_OnMacProcess( void )
{
    __log_callback("on mac process notify");
    lora_wan_process_request(__LORA_WAN_PROCESS_PROCESS_MAC, NULL);
}
static void cb_OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size)
{
    __log_callback("on nvm data change -- update LoRaMac and LmHandler");
    MibRequestConfirm_t mib;

    mib.Type = MIB_CHANNELS_DEFAULT_DATARATE;
    mib.Param.ChannelsDefaultDatarate = __config_lorawan_default_datarate;
    LoRaMacMibSetRequestConfirm( &mib );

    mib.Type = MIB_CHANNELS_DATARATE;
    mib.Param.ChannelsDatarate = __config_lorawan_default_datarate;
    LoRaMacMibSetRequestConfirm( &mib );

    lora_commission_update_mac_layer();
}
static void cb_OnNetworkParametersChange( CommissioningParams_t *params )
{
    __log_callback("on network params change");

    commission_type_t type = lora_commission_get_type();

    params->IsOtaaActivation = (type == __LORA_COMMISSION_OTAA);

    if( type == __LORA_COMMISSION_OTAA ) {
        memcpy(params->DevEui, lora_commission_get_dev_eui_ref(), 8);
        memcpy(params->JoinEui, lora_commission_get_join_eui_ref(), 8);
    } else {
        params->DevAddr = lora_commission_get_dev_addr();
    }

    __log_printf("\tactivation: "__blue__"%s\n",
        params->IsOtaaActivation ? "OTAA" : "ABP");
    __log_printf("\tdev_eui:    "__yellow__);
    __log_printf_hex_upper(params->DevEui, 8);
    __log_endl();
    __log_printf("\tjoin_eui:   "__yellow__);
    __log_printf_hex_upper(params->JoinEui, 8);
    __log_endl();
    __log_printf("\tdev_addr    "__yellow__);
    uint8_t* p = (void*)&params->DevAddr;
    uint8_t arr[4] = {p[3], p[2], p[1], p[0]};
    __log_printf_hex_upper(arr, 4);
    __log_endl();
    __log_printf("\tnetword_id: "__green__"%d\n", params->NetworkId);
    __log_printf("\tse_pin:     "__cyan__);
    __log_printf_hex_upper(params->SePin, 4);
    __log_endl();
}

static void cb_OnMacMcpsRequest(
    LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxDelay )
{
    __log_callback("on mac mcps request");
    __log_info("mac status: (%d) %s", status,
        lora_utils_get_mac_return_status_str(status));
    lora_utils_log_mcps_req(mcpsReq);
    __log_info("next-tx-delay: "__yellow__"%d"__default__" ms", nextTxDelay);
}

static void cb_OnMacMlmeRequest(
    LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxDelay )
{
    __log_callback("on mac mlme request");
    __log_info("mac status: (%d) %s", status,
        lora_utils_get_mac_return_status_str(status));
    lora_utils_log_mlme_req(mlmeReq);
    __log_info("next-tx-delay: "__yellow__"%d"__default__" ms", nextTxDelay);
}

static void cb_OnJoinRequest( LmHandlerJoinParams_t *params )
{
    __log_callback("on join request");
    __log_printf("\tstatus:       %s",
        params->Status == LORAMAC_HANDLER_SUCCESS ?
            __green__"success" : __red__"failed");
    __log_printf("\tjoin status : %s\n", lm_get_active_status_string());
    if(params->Status == LORAMAC_HANDLER_SUCCESS)
    {
        __log_info("-- join success --");
        lora_wan_process_request(__LORA_WAN_PROCESS_JOIN_DONE, NULL);
    }
    else
    {
        __log_info("-- join failed --");
        lora_wan_process_request(__LORA_WAN_PROCESS_JOIN_FAIL, NULL);
    }
}

static void cb_OnTxData( LmHandlerTxParams_t *params )
{
    __log_printf_header(">>> on tx data >>> ", 80, '#');
    __log_printf("status: %s\n",
        lora_utils_get_mac_event_info_status_str( params->Status ));
    __log_printf("confirmation: %srequired"__default__" -- Ack %sreceived\n",
        params->MsgType == 0 ? __red__"not ": __green__"",
        params->AckReceived ? "" : __red__"not "__default__);

    __log_printf("(port: "__yellow__"%d"__default__") "
        "(ul counter: "__yellow__"%d"__default__") "
        "(channel: "__yellow__"%d"__default__")"
        "(tx power: "__cyan__"%d"__default__")",
        params->AppData.Port, params->UplinkCounter,
        params->Channel, params->TxPower);
    __log_endl();
    if(params->AppData.BufferSize) {
        __log_dump(params->AppData.Buffer, params->AppData.BufferSize, 16,
            __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs |
            __log_dump_flag_hide_address, __word_len_8);
    }
    __log_printf_fill(80, '-', true);

    if(on_mac_tx)
    {
        lmh_tx_status_params_t tx_info = {
            .status = params->Status,
            .ack_received = params->AckReceived,
            .tx_power = params->TxPower,
            .port = params->AppData.Port,
            .ul_counter = params->UplinkCounter,
            .channel = params->Channel,
            .data_rate = params->Datarate
        };
        on_mac_tx(&tx_info);
    }
}

static void cb_OnRxData(
    LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
    __log_printf_header("<<< on rx data <<< ", 80, '#');
    __log_printf("status: %s\n",
        lora_utils_get_mac_event_info_status_str( params->Status ));
    __log_printf("is MCPS indication: %s"__default__"\n",
        params->IsMcpsIndication ? __green__"yes" : __red__"no");
    __log_printf("(port: "__yellow__"%d"__default__") "
        "(dl counter: "__yellow__"%d"__default__") "
        "(rx slot: "__yellow__"%d"__default__")"
        "(datarate: "__yellow__"%d"__default__")"
        "(snr: "__cyan__"%d"__default__") "
        "(rssi: "__cyan__"%d"__default__")",
        appData->Port, params->DownlinkCounter, params->RxSlot,
        params->Datarate, params->Snr, params->Rssi);
    __log_endl();
    if(appData->BufferSize) {
        __log_dump(appData->Buffer, appData->BufferSize, 16,
            __log_dump_flag_disp_char | __log_dump_flag_disp_char_on_rhs |
            __log_dump_flag_hide_address, __word_len_8);
    }
    __log_printf_fill(80, '-', true);

    if(on_mac_rx)
    {
        lmh_rx_status_params_t rx_info = {
            .status = params->Status,
            .rssi = params->Rssi,
            .snr = params->Snr,
            .dl_counter = params->DownlinkCounter,
            .port = appData->Port,
            .buf = appData->Buffer,
            .len = appData->BufferSize,
            .data_rate = params->Datarate,
            .rx_slot = params->RxSlot
        };
        on_mac_rx(&rx_info);
    }
}

static void cb_OnClassChange( DeviceClass_t deviceClass )
{
    __log_callback("on class change");
    __log_output(__green__"class change to %c\n"__default__, 'A' + deviceClass);
    lora_wan_process_request(__LORA_WAN_PROCESS_CLASS_CHANGED, NULL);
}

static void cb_OnBeaconStatusChange( LoRaMacHandlerBeaconParams_t *params )
{
    __log_callback("on beacon status change");
}

static void cb_OnSysTimeUpdate( bool isSynchronized, int32_t timeCorrection )
{
    __log_callback("on sys time update");
}

/* --- end of file ---------------------------------------------------------- */
