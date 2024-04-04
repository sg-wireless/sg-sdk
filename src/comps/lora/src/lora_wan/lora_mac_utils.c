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
 * @brief   lora-wan mode common utils
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include "stdio.h"

#define __log_subsystem     lora
#define __log_component     wan_utils
#include "log_lib.h"

#include "LoRaMac.h"
#include "lora_mac_utils.h"
#include "lora_mac_handler.h"
#include "lora_common_utils.h"

#include "Region.h"
#include "RegionNvm.h"

/** -------------------------------------------------------------------------- *
 * APIs definitions
 * --------------------------------------------------------------------------- *
 */
static lm_active_status_t get_lm_active_type(ActivationType_t type)
{
    switch( type ) {
        case ACTIVATION_TYPE_NONE: return __lm_status_active_none;
        case ACTIVATION_TYPE_OTAA: return __lm_status_active_otaa;
        case ACTIVATION_TYPE_ABP:  return __lm_status_active_abp;
    }
    return __lm_status_active_error;
}
lm_active_status_t lm_get_active_status( void )
{
    MibRequestConfirm_t mibReq;
    LoRaMacStatus_t status;

    mibReq.Type = MIB_NETWORK_ACTIVATION;
    status = LoRaMacMibGetRequestConfirm( &mibReq );

    if( status == LORAMAC_STATUS_OK )
    {
        return get_lm_active_type(mibReq.Param.NetworkActivation);
    }

    return __lm_status_active_error;
}

bool lm_is_joined( void )
{
    lm_active_status_t status = lm_get_active_status();

    return ( status == __lm_status_active_otaa || 
             status == __lm_status_active_abp );
}


static const char* s_active_status_str[] = {
    [ __lm_status_active_otaa  ] = __green__"OTAA" __default__,
    [ __lm_status_active_abp   ] = __green__"ABP"  __default__,
    [ __lm_status_active_none  ] = __blue__ "NONE" __default__,
    [ __lm_status_active_error ] = __red__  "ERROR"__default__
};
const char* lm_get_active_status_string( void )
{
    return s_active_status_str[lm_get_active_status()];
}

#define __lm_log_name_space       20
#define __lm_log_info(name, fmt, args...) \
    __log_output("\n\t- %-"__stringify(__lm_log_name_space)"s : " \
        __yellow__ fmt, name, args)
static void log_credits(const char* name, uint8_t* buf, uint32_t len)
{
    __lm_log_info(name, "%s","");
    if(buf)
    {
        __log_output_hex_lower(buf, len);
    }
}

void lora_utils_stats(void)
{
    MibRequestConfirm_t mib;

    __lm_log_info("region", __blue__"%s",
        get_lora_region_string(lmh_get_region()));

    mib.Type = MIB_DEVICE_CLASS;
    LoRaMacMibGetRequestConfirm( &mib );
    __lm_log_info("class", "class-%c", mib.Param.Class+'A');

    mib.Type = MIB_DEV_EUI;
    LoRaMacMibGetRequestConfirm( &mib );
    log_credits("dev eui", mib.Param.DevEui, 8);

    mib.Type = MIB_JOIN_EUI;
    LoRaMacMibGetRequestConfirm( &mib );
    log_credits("join eui", mib.Param.JoinEui, 8);

    mib.Type = MIB_DEV_ADDR;
    LoRaMacMibGetRequestConfirm( &mib );
    uint8_t * p = (void*)&mib.Param.DevAddr;
    uint8_t arr[] = { p[3], p[2], p[1], p[0] };
    log_credits("dev addr", arr, 4);

    mib.Type = MIB_LORAWAN_VERSION;
    LoRaMacMibGetRequestConfirm( &mib );
    __lm_log_info("lorawan version", "%d.%d.%d.%d",
        mib.Param.LrWanVersion.LoRaWan.Fields.Major,
        mib.Param.LrWanVersion.LoRaWan.Fields.Minor,
        mib.Param.LrWanVersion.LoRaWan.Fields.Patch,
        mib.Param.LrWanVersion.LoRaWan.Fields.Revision );

    const char* active_status_str = lm_get_active_status_string();
    __lm_log_info("activation", "%s", active_status_str);

    __log_output("\n");
}

static const char* s_mac_event_info_status_str[] = {
    [LORAMAC_EVENT_INFO_STATUS_OK] = "service exec ok",
    [LORAMAC_EVENT_INFO_STATUS_ERROR] = "service exec error",
    [LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT] = "tx timout",
    [LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT] = "rx1 timeout",
    [LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT] = "rx2 timeout",
    [LORAMAC_EVENT_INFO_STATUS_RX1_ERROR] = "rx error on rx win 1",
    [LORAMAC_EVENT_INFO_STATUS_RX2_ERROR] = "rx error on rx win 2",
    [LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL] = "error during join proc",
    [LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED] = "repeated dl counter",
    [LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR] = "tx payload size",
    [LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL] = "adress error",
    [LORAMAC_EVENT_INFO_STATUS_MIC_FAIL] = "MIC fail",
    [LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL] = "multicast fail",
    [LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED] = "beacon locked",
    [LORAMAC_EVENT_INFO_STATUS_BEACON_LOST] = "beacon lost",
    [LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND] = "beacon not found"
};

const char * lora_utils_get_mac_event_info_status_str(
    LoRaMacEventInfoStatus_t status)
{
    if(status < sizeof(s_mac_event_info_status_str)/sizeof(const char*))
    {
        return s_mac_event_info_status_str[ status ];
    }
    else
    {
        return NULL;
    }
}

static const char* s_mac_return_status_str[] = {
    [LORAMAC_STATUS_OK] = "ok",
    [LORAMAC_STATUS_BUSY] = "busy",
    [LORAMAC_STATUS_SERVICE_UNKNOWN] = "service unknown",
    [LORAMAC_STATUS_PARAMETER_INVALID] = "parameter invalid",
    [LORAMAC_STATUS_FREQUENCY_INVALID] = "frequency invalid",
    [LORAMAC_STATUS_DATARATE_INVALID] = "data-rate invalid",
    [LORAMAC_STATUS_FREQ_AND_DR_INVALID] = "freq and dr invalid",
    [LORAMAC_STATUS_NO_NETWORK_JOINED] = "no network joined",
    [LORAMAC_STATUS_LENGTH_ERROR] = "length error",
    [LORAMAC_STATUS_REGION_NOT_SUPPORTED] = "not supported region",
    [LORAMAC_STATUS_SKIPPED_APP_DATA] = "app data skipped",
    [LORAMAC_STATUS_DUTYCYCLE_RESTRICTED] = "duty-cycle restricted",
    [LORAMAC_STATUS_NO_CHANNEL_FOUND] = "no channel found",
    [LORAMAC_STATUS_NO_FREE_CHANNEL_FOUND] = "no free channel found",
    [LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME] = "busy beacon reserved time",
    [LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME] = "busy ping slot window",
    [LORAMAC_STATUS_BUSY_UPLINK_COLLISION] = "busy uplink collision",
    [LORAMAC_STATUS_CRYPTO_ERROR] = "crypto error",
    [LORAMAC_STATUS_FCNT_HANDLER_ERROR] = "fcnt handler error",
    [LORAMAC_STATUS_MAC_COMMAD_ERROR] = "mac command error",
    [LORAMAC_STATUS_CLASS_B_ERROR] = "class b error",
    [LORAMAC_STATUS_CONFIRM_QUEUE_ERROR] = "confirm queue error",
    [LORAMAC_STATUS_MC_GROUP_UNDEFINED] = "mc group undefined",
    [LORAMAC_STATUS_ERROR] = "undefined error",
};

const char * lora_utils_get_mac_return_status_str(LoRaMacStatus_t status)
{
    if(status < sizeof(s_mac_return_status_str)/sizeof(const char*))
    {
        return s_mac_return_status_str[ status ];
    }
    else
    {
        return NULL;
    }
}

void lora_utils_log_mcps_req(McpsReq_t* req)
{
    if(req->Type >= 4)
    {
        __log_error("mcps-req error unknown frame type");
        return;
    }

    const char* f_str[] = {
        [MCPS_UNCONFIRMED] = "unconfirmed",
        [MCPS_CONFIRMED] = "confirmed",
        [MCPS_MULTICAST] = "multicast",
        [MCPS_PROPRIETARY] = "proprietary",
    };

    __log_printf("mcps-req ("__cyan__"%s-frame"__default__")",
        f_str[req->Type]);
    
    if(req->Type == MCPS_UNCONFIRMED || req->Type == MCPS_CONFIRMED)
    {
        __log_printf(" fport:"__blue__"%d"__default__,
            req->Req.Confirmed.fPort);
    }

    if(req->Type != MCPS_MULTICAST)
    {
        __log_printf(" buf-size:"__blue__"%d"__default__,
            req->Req.Confirmed.fBufferSize);
    }

    __log_printf(" ==> ret duty-wait:"__yellow__"%d"__default__" ms",
        req->ReqReturn.DutyCycleWaitTime);

    __log_endl();
}

static const char* s_mac_mlme_req_type_str[] = {
    [MLME_UNKNOWN] = "unknown",
    [MLME_JOIN] = "join",
    [MLME_REJOIN_0] = "rejoin-0",
    [MLME_REJOIN_1] = "rejoin-1",
    [MLME_REJOIN_2] = "rejoin-2",
    [MLME_LINK_CHECK] = "link-check",
    [MLME_TXCW] = "tx-cont-wave",
    [MLME_DERIVE_MC_KE_KEY] = "derive McKEKey from AppKey",
    [MLME_DERIVE_MC_KEY_PAIR] = "derive McAppSKey,McNwkSKey from McKey",
    [MLME_DEVICE_TIME] = "init->DeviceTimeReq",
    [MLME_BEACON] = "beacon-rx-status",
    [MLME_BEACON_ACQUISITION] = "init beacon-accquisition",
    [MLME_PING_SLOT_INFO] = "init PingSlotInfoReq",
    [MLME_BEACON_TIMING] = "init BeaconTimingReq",
    [MLME_BEACON_LOST] = "ind beacon-lost",
    [MLME_REVERT_JOIN] = "trigger-join-again",
};
void lora_utils_log_mlme_req(MlmeReq_t* req)
{
    if(req->Type >= 16)
    {
        __log_error("mlme-req error unknown mlme type");
        return;
    }

    __log_printf("mlme-req ("__cyan__"%s"__default__")",
        s_mac_mlme_req_type_str[req->Type]);

    if(req->Type == MLME_JOIN)
    {
        MlmeReqJoin_t* p = &req->Req.Join;
        __log_printf(" activation: %s, dr: "__blue__"%d"__default__,
            s_active_status_str[ get_lm_active_type(p->NetworkActivation) ],
            p->Datarate);
    }
    else if(req->Type == MLME_TXCW)
    {
        MlmeReqTxCw_t* p = &req->Req.TxCw;
        __log_printf(" timeout: %d, frequency: %d, power:%+d",
            p->Timeout, p->Frequency, p->Power);
    }
    else if(req->Type == MLME_PING_SLOT_INFO)
    {
        PingSlotInfo_t* p = &req->Req.PingSlotInfo.PingSlot;
        __log_printf(" value: %d, periodicity: %d",
            p->Value, p->Fields.Periodicity);
    }
    else if(req->Type == MLME_DERIVE_MC_KE_KEY)
    {
        MlmeReqDeriveMcKEKey_t* p = &req->Req.DeriveMcKEKey;
        __log_printf(" key-id: %d, nonce: %d, dev-eui: ",
            p->KeyID, p->Nonce);
        __log_printf_hex_lower(p->DevEUI, 8);
    }
    else if(req->Type == MLME_DERIVE_MC_KEY_PAIR)
    {
        __log_printf(" group-id: %d", req->Req.DeriveMcSessionKeyPair.GroupID);
    }

    __log_printf(" ==> ret duty-wait:"__yellow__"%d"__default__" ms",
        req->ReqReturn.DutyCycleWaitTime);

    __log_endl();
}

static void display_region_bands_list(
    lora_region_t region, Band_t* p_bands, int len);
static void display_region_channels_list(
    lora_region_t region,
    ChannelParams_t* p_ch,
    uint16_t* p_def_masks,
    uint16_t* p_masks);

static RegionNvmDataGroup1_t s_grp1;
static RegionNvmDataGroup2_t s_grp2;
static Band_t s_bands[REGION_NVM_MAX_NB_BANDS] = {0};
void lora_list_region_params(lora_region_t region)
{
    LoRaMacRegion_t mac_region = get_lora_mac_region_enum_value(region);

    InitDefaultsParams_t params = {
        .NvmGroup1 = &s_grp1,
        .NvmGroup2 = &s_grp2,
        .Bands = s_bands,
        .Type = INIT_TYPE_DEFAULTS
    };

    memset(&s_grp1, 0, sizeof(s_grp1));
    memset(&s_grp2, 0, sizeof(s_grp2));
    memset(&s_bands, 0, sizeof(s_bands));

    __log_output(" # Channels Max : "__blue__"%d\n",
        REGION_NVM_MAX_NB_CHANNELS);
    __log_output(" # Masks Max    : "__blue__"%d\n",
        REGION_NVM_CHANNELS_MASK_SIZE);

    RegionInitDefaults(mac_region, &params);

    display_region_bands_list(region, s_bands, REGION_NVM_MAX_NB_BANDS);

    display_region_channels_list(region, s_grp2.Channels,
        s_grp2.ChannelsDefaultMask, s_grp2.ChannelsMask);
}

void lora_list_mac_channels_status(void)
{
    MibRequestConfirm_t mib;

    mib.Type = MIB_CHANNELS;
    LoRaMacMibGetRequestConfirm(&mib);
    ChannelParams_t* p_channels = mib.Param.ChannelList;

    mib.Type = MIB_CHANNELS_DEFAULT_MASK;
    LoRaMacMibGetRequestConfirm(&mib);
    uint16_t* p_def_masks = mib.Param.ChannelsDefaultMask;

    mib.Type = MIB_CHANNELS_MASK;
    LoRaMacMibGetRequestConfirm(&mib);
    uint16_t* p_masks = mib.Param.ChannelsMask;

    display_region_channels_list(lmh_get_region(), p_channels,
        p_def_masks, p_masks);
    
    RxChannelParams_t ch;

    mib.Type = MIB_RX2_CHANNEL;
    LoRaMacMibGetRequestConfirm(&mib);
    ch = mib.Param.Rx2Channel;
    __lm_log_info("rx2 channel",
        "freq:"__blue__"%8.3f"__default__" DR:"__green__"%d",
        (float)ch.Frequency/1000000.0f, ch.Datarate);

    mib.Type = MIB_RX2_DEFAULT_CHANNEL;
    LoRaMacMibGetRequestConfirm(&mib);
    ch = mib.Param.Rx2DefaultChannel;
    __lm_log_info("rx2 default channel",
        "freq:"__blue__"%8.3f"__default__" DR:"__green__"%d",
        (float)ch.Frequency/1000000.0f, ch.Datarate);

    mib.Type = MIB_RXC_CHANNEL;
    LoRaMacMibGetRequestConfirm(&mib);
    ch = mib.Param.RxCChannel;
    __lm_log_info("rx C channel",
        "freq:"__blue__"%8.3f"__default__" DR:"__green__"%d",
        (float)ch.Frequency/1000000.0f, ch.Datarate);

    mib.Type = MIB_RXC_DEFAULT_CHANNEL;
    LoRaMacMibGetRequestConfirm(&mib);
    ch = mib.Param.RxCDefaultChannel;
    __lm_log_info("rx C default channel",
        "freq:"__blue__"%8.3f"__default__" DR:"__green__"%d",
        (float)ch.Frequency/1000000.0f, ch.Datarate);
    __log_output("\n");
}

static void display_region_bands_list(
    lora_region_t region, Band_t* p_bands, int len)
{
    #define __w_c1              (14)
    #define __w_cn              (6)
    #define __max_str_len       (40)

    char str[__max_str_len];

    __log_output("\t");
    snprintf(str, __max_str_len, "Region "__yellow_str("%s")" Bands",
        get_lora_region_string(region));
    __log_output_header(str, __w_c1 + len * __w_cn, '=');

    __log_output("\t");
    __log_output_field(__cyan__"idx", __w_c1, ' ', __left__, false);
    for(int i = 0; i < len; i++) {
        snprintf(str, __max_str_len, "%d", i);
        __log_output_field(str, __w_cn, ' ', __right__, false);
    }
    __log_output("\n\t");

    __log_output_field(__cyan__"duty-cycle", __w_c1, ' ', __left__, false);
    for(int i = 0; i < len; i++) {
        snprintf(str, __max_str_len, "%d", (uint32_t)p_bands[i].DCycle);
        __log_output_field(str, __w_cn, ' ', __right__, false);
    }
    __log_output("\n\t");

    __log_output_field(__cyan__"tx-power-idx", __w_c1, ' ', __left__,false);
    for(int i = 0; i < len; i++) {
        snprintf(str, __max_str_len, "%d", (uint32_t)p_bands[i].TxMaxPower);
        __log_output_field(str, __w_cn, ' ', __right__, false);
    }
    __log_output("\n\t");
    __log_output_fill(__w_c1 + len * __w_cn, '-', true);

    #undef __w_c1
    #undef __w_cn
    #undef __max_str_len
}

static void display_region_channels_list(
    lora_region_t region,
    ChannelParams_t* p_ch,
    uint16_t* p_def_masks,
    uint16_t* p_masks)
{
    __log_dump(p_def_masks, REGION_NVM_CHANNELS_MASK_SIZE * 2, 16, 0,
        __word_len_16);

    #define __w_idx              3
    #define __w_def_msk          6
    #define __w_msk              4
    #define __w_freq            10
    #define __w_rx1f            10
    #define __w_dr_min           8
    #define __w_dr_max           8
    #define __w_band_idx        10
    #define __w_total           (__w_idx + __w_def_msk + __w_msk + __w_freq + \
                                __w_rx1f + __w_dr_min + __w_dr_max + \
                                __w_band_idx)
    #define __max_str_len       (40)

    char str[__max_str_len];

    __log_output("\t");
    snprintf(str, __max_str_len, "Region "__yellow_str("%s")" Channels",
        get_lora_region_string(region));
    __log_output_header(str, __w_total, '=');

    __log_output("\t");
    __log_output_field(__cyan__"idx", __w_idx, ' ', __right__, false);
    __log_output_field(__cyan__"d-msk", __w_def_msk, ' ', __right__, false);
    __log_output_field(__cyan__"msk", __w_msk, ' ', __right__, false);
    __log_output_field(__cyan__"freq", __w_freq, ' ', __right__, false);
    __log_output_field(__cyan__"rx1-freq", __w_rx1f, ' ', __right__, false);
    __log_output_field(__cyan__"dr-min", __w_dr_min, ' ', __right__, false);
    __log_output_field(__cyan__"dr-max", __w_dr_max, ' ', __right__, false);
    __log_output_field(__cyan__"band-idx", __w_band_idx, ' ', __right__, true);

    for(int i = 0; i < REGION_NVM_MAX_NB_CHANNELS; i++) {
        // |idx|def-msk|msk|freq|rx1-req|dr-min|dr-max|band-idx|
        if(((p_def_masks[i/16]>>(i%16)) & 1) == 0) {
            continue;
        }
        __log_output("\t");
        snprintf(str, __max_str_len, "%d",i);
        __log_output_field(str, __w_idx, ' ', __right__, false);

        snprintf(str, __max_str_len, "%d", (p_def_masks[i/16]>>(i%16)) & 1);
        __log_output_field(str, __w_def_msk, ' ', __right__, false);

        snprintf(str, __max_str_len, "%d", (p_masks[i/16]>>(i%16)) & 1);
        __log_output_field(str, __w_msk, ' ', __right__, false);

        snprintf(str, __max_str_len, "%.3f",
            (float)p_ch[i].Frequency/1000000.0f);
        __log_output_field(str, __w_freq, ' ', __right__, false);

        snprintf(str, __max_str_len, "%.3f",
            (float)p_ch[i].Rx1Frequency/1000000.0f);
        __log_output_field(str, __w_rx1f, ' ', __right__, false);

        snprintf(str, __max_str_len, "%d", p_ch[i].DrRange.Fields.Min);
        __log_output_field(str, __w_dr_min, ' ', __right__, false);

        snprintf(str, __max_str_len, "%d", p_ch[i].DrRange.Fields.Max);
        __log_output_field(str, __w_dr_max, ' ', __right__, false);

        snprintf(str, __max_str_len, "%d", p_ch[i].Band);
        __log_output_field(str, __w_band_idx, ' ', __right__, true);
    }
    __log_output("\t");
    __log_output_fill(__w_total, '-', true);
}

void __log_lora_key(void* key)
{
    uint8_t * p = key;
    for(int i = 0; i < 16; i++)
        __log_output("%02x", p[i]);
    __log_output("\n");
}

#undef __log_subsystem
#undef __log_component
#define __log_subsystem     default
#define __log_component     default

void __log_debug_dump(uint8_t* buf, uint32_t size)
{
    __log_dump(buf, size, 16, 
        __log_dump_flag_disp_char|__log_dump_flag_disp_char_on_rhs|
        __log_dump_flag_hide_address|__log_dump_flag_hide_offset, __word_len_8);
}

void lora_dr_stats(void)
{
    MibRequestConfirm_t mib;

    mib.Type = MIB_ADR;
    LoRaMacMibGetRequestConfirm(&mib);
    __lm_log_info("ADR Enable", __yellow__"%d", mib.Param.AdrEnable);

    mib.Type = MIB_ADR_ACK_LIMIT;
    LoRaMacMibGetRequestConfirm(&mib);
    __lm_log_info("ADR ACK Limit", __yellow__"%d", mib.Param.AdrAckLimit);

    mib.Type = MIB_ADR_ACK_DELAY;
    LoRaMacMibGetRequestConfirm(&mib);
    __lm_log_info("ADR ACK Delay", __yellow__"%d", mib.Param.AdrAckDelay);

    mib.Type = MIB_CHANNELS_DATARATE;
    LoRaMacMibGetRequestConfirm(&mib);
    __lm_log_info("Channels DR", __yellow__"%d", mib.Param.ChannelsDatarate);

    __log_output("\n");
}

/* --- end of file ---------------------------------------------------------- */
