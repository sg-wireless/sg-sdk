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
 * @brief   lora-raw mode radio access sub-component.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define __log_subsystem  lora
#define __log_component  raw_radio_if
#include "log_lib.h"

#include "radio.h"
#include "lora.h"
#include "stub_system.h"
#include "lora_nvm.h"
#include "radio_ext.h"
#include "system/timer.h"
#include "lora_raw_process.h"
#include "lora_common_utils.h"

/** -------------------------------------------------------------------------- *
 * lora raw radio default configurations
 * --------------------------------------------------------------------------- *
 */
#define __lora_raw_default_region       __LORA_REGION_EU868
#define __lora_raw_default_freq         __freq_mhz(868)
#define __lora_raw_default_tx_power     14
#define __lora_raw_default_sf           __LORA_SF_7
#define __lora_raw_default_cr           __LORA_CR_4_5
#define __lora_raw_default_preamble     8
#define __lora_raw_default_bw           __LORA_BW_125_KHZ
#define __lora_raw_default_payload      64
#define __lora_raw_default_tx_inv_iq    false
#define __lora_raw_default_rx_inv_iq    false
#define __lora_raw_default_crc_on       true
#define __lora_raw_default_symb_timeout 5
#define __lora_raw_default_tx_timeout   6000
#define __lora_raw_default_rx_timeout   6000

/** -------------------------------------------------------------------------- *
 * static declarations
 * --------------------------------------------------------------------------- *
 */
static void reset_region_params(lora_region_t region);

/** -------------------------------------------------------------------------- *
 * radio internal nvm params and handling
 * --------------------------------------------------------------------------- *
 */
static const char* s_lora_raw_radio_params_nvm_key = "raw-radio-if";

static struct {
    lora_region_t region;
    uint32_t    freq;
    int8_t      tx_power;   // -- tx power in dBm
    float       max_eirp;   // -- max Effective Isotropic Radiated Power(EIRP)
    float       antenna_gain; // -- antenna gain
    uint8_t     sf;         // -- spreading factor
    uint8_t     cr;         // -- coding rate
    uint8_t     preamble;   // -- preamble length
    uint8_t     bw;         // -- bandwidth
    uint8_t     payload;    // -- max payload length
    bool        tx_inv_iq;  // -- invertied IQ
    bool        rx_inv_iq;  // -- invertied IQ
    bool        crc_on;     // -- CRC enable
    uint8_t     symb_timeout;
    uint32_t    tx_timeout;
    uint32_t    rx_timeout;
    uint32_t    time_on_air;
    lora_nvm_record_tail_t record_tail; // -- needed by the lora_nvm.h
} s_radio_lora_params;

static void radio_if_nvm_load_defaults_callback(void* ptr, uint32_t size)
{
    (void) ptr; (void) size;
    reset_region_params(__lora_raw_default_region);
    s_radio_lora_params.cr         = __lora_raw_default_cr;
    s_radio_lora_params.preamble   = __lora_raw_default_preamble;
    s_radio_lora_params.tx_inv_iq  = __lora_raw_default_tx_inv_iq;
    s_radio_lora_params.rx_inv_iq  = __lora_raw_default_rx_inv_iq;
    s_radio_lora_params.crc_on     = __lora_raw_default_crc_on;
    s_radio_lora_params.symb_timeout = __lora_raw_default_symb_timeout;
    s_radio_lora_params.tx_timeout = __lora_raw_default_tx_timeout;
    s_radio_lora_params.rx_timeout = __lora_raw_default_rx_timeout;
    s_radio_lora_params.bw         = __lora_raw_default_bw;

    s_radio_lora_params.time_on_air = Radio.TimeOnAir(MODEM_LORA,
        s_radio_lora_params.bw, s_radio_lora_params.sf,
        s_radio_lora_params.cr, s_radio_lora_params.preamble, false,
        s_radio_lora_params.payload, s_radio_lora_params.crc_on) << 1;
    __log_info("setting default values for lora raw radio params, toa:%d msec",
        s_radio_lora_params.time_on_air);
}

static void lora_raw_handle_nvm_change(void)
{
    static bool initialized = false;

    if( initialized )
    {
        s_radio_lora_params.time_on_air = Radio.TimeOnAir(MODEM_LORA,
            s_radio_lora_params.bw, s_radio_lora_params.sf,
            s_radio_lora_params.cr, s_radio_lora_params.preamble, false,
            s_radio_lora_params.payload, s_radio_lora_params.crc_on) << 1;

        __log_debug("-> time on air -> %d msec",
            s_radio_lora_params.time_on_air);

    }

    initialized = true;

    lora_nvm_handle_change(
        s_lora_raw_radio_params_nvm_key,
        &s_radio_lora_params,
        sizeof(s_radio_lora_params),
        radio_if_nvm_load_defaults_callback
        );
}

/** -------------------------------------------------------------------------- *
 * tx power calculations
 * --------------------------------------------------------------------------- *
 */
#define __tx_power_effective()  (int8_t)floor(  \
    (float)(s_radio_lora_params.tx_power) - s_radio_lora_params.antenna_gain)

#define __tx_power_chip_max() (int8_t)floor(    \
    (float)lora_radio_ext_get_max_tx_power() + s_radio_lora_params.antenna_gain)

#define __tx_power_chip_min() (int8_t)floor(\
    (float)lora_radio_ext_get_min_tx_power() + s_radio_lora_params.antenna_gain)

static int8_t tx_power_index_to_dbm(int8_t index, float max_eirp)
{
    float v = max_eirp - index * 2;

    return (int8_t)floor(v);
}

#if 0
static int8_t tx_power_dbm_to_index(int8_t dbm)
{
    float v = (s_radio_lora_params.max_eirp - dbm) / 2.0f;
    int8_t idx;
    if(v >= 0)
        idx = ceill(v);
    else
        idx = -ceill(-v);
    return idx;
}
#endif

static int8_t tx_power_get_region_max(lora_region_t region)
{
    GetPhyParams_t req_param = {.Attribute = PHY_MAX_TX_POWER};
    PhyParam_t ret_param = RegionGetPhyParam(
        get_lora_mac_region_enum_value(region), &req_param);
    return tx_power_index_to_dbm(ret_param.Value, s_radio_lora_params.max_eirp);
}

static int8_t tx_power_correct_specific(int8_t tx_power, lora_region_t region)
{
    {
        int8_t max_chip_power = __tx_power_chip_max();
        if(tx_power > max_chip_power)
        {
            tx_power = max_chip_power;
        }
    }

    {
        int8_t min_chip_power = __tx_power_chip_min();
        if(tx_power < min_chip_power)
        {
            tx_power = min_chip_power;
        }
    }

    {
        int8_t max_region_power = tx_power_get_region_max(region);
        if(tx_power > max_region_power)
            tx_power = max_region_power;
    }

    return tx_power;
}

static void tx_power_correct(void)
{
    s_radio_lora_params.tx_power = tx_power_correct_specific(
            s_radio_lora_params.tx_power, s_radio_lora_params.region);
}

/** -------------------------------------------------------------------------- *
 * LoRaMac radio interface connecting
 * --------------------------------------------------------------------------- *
 */

#define __log_callback(__cb_str) \
    __log_info(__yellow__"callback"__default__" --> "__cyan__ __cb_str);

static void cb_on_tx_done( void )
{
    __log_callback("on tx done");
    lora_raw_process_event(__LORA_RAW_PROCESS_TX_DONE, NULL, false);
}
static void cb_on_rx_done( uint8_t *payload, uint16_t size,
                         int16_t rssi, int8_t snr )
{
    __log_callback("on rx done");
    __log_debug("received signal (rssi: %d) (snr: %d)", rssi, snr);
    __log_debug("received buffer size: %d", size);
    __log_dump(payload, size, 8, __log_dump_flag_disp_char |
        __log_dump_flag_disp_char_on_rhs | __log_dump_flag_hide_address,
        __word_len_8);
    lora_raw_process_event_payload_t msg = {
        .type = __PROCESS_MSG_PAYLOAD_RX_DONE,
        .rx_done_payload = {
            .buf = payload,
            .len = size,
            .rssi = rssi,
            .snr = snr
        }
    };
    lora_raw_process_event(__LORA_RAW_PROCESS_RX_DONE, &msg, false);
}
static void cb_on_tx_timeout( void )
{
    __log_callback("on tx timeout");
    lora_raw_process_event(__LORA_RAW_PROCESS_TX_TIMEOUT, NULL, false);
}
static void cb_on_rx_timeout( void )
{
    __log_callback("on rx timeout");
    lora_raw_process_event(__LORA_RAW_PROCESS_RX_TIMEOUT, NULL, false);
}
static void cb_on_rx_error( void )
{
    __log_callback("on rx error");
    lora_raw_process_event(__LORA_RAW_PROCESS_RX_ERROR, NULL, false);
}
static void cb_on_cad_done( bool channelActivityDetected )
{
    __log_callback("on cad done");
    lora_raw_process_event(__LORA_RAW_PROCESS_CAD_DONE, NULL, false);
}

static RadioEvents_t RadioEvents = {
    .TxDone     = cb_on_tx_done,
    .RxDone     = cb_on_rx_done,
    .TxTimeout  = cb_on_tx_timeout,
    .RxTimeout  = cb_on_rx_timeout,
    .RxError    = cb_on_rx_error,
    .CadDone    = cb_on_cad_done
};

static void loramac_radio_Setup(void)
{
    Radio.SetTxConfig(MODEM_LORA, __tx_power_effective(), 0,
        s_radio_lora_params.bw, s_radio_lora_params.sf, s_radio_lora_params.cr,
        s_radio_lora_params.preamble, false, s_radio_lora_params.crc_on, false,
        0, s_radio_lora_params.tx_inv_iq, s_radio_lora_params.tx_timeout);
    
    Radio.SetRxConfig(MODEM_LORA, s_radio_lora_params.bw,
        s_radio_lora_params.sf, s_radio_lora_params.cr, 0,
        s_radio_lora_params.preamble, s_radio_lora_params.symb_timeout, false,
        s_radio_lora_params.payload, s_radio_lora_params.crc_on, false, 0,
        s_radio_lora_params.rx_inv_iq, true);
}

static void loramac_radio_ctor(void)
{
    Radio.Init( &RadioEvents );

    Radio.SetChannel( s_radio_lora_params.freq );

    loramac_radio_Setup();
}

static void loramac_radio_dtor(void)
{
    Radio.Sleep();
}

static void loramac_radio_process_irqs(void)
{
    if( Radio.IrqProcess != NULL )
        Radio.IrqProcess( );
}

/** -------------------------------------------------------------------------- *
 * lora raw radio populated APIs implementations
 * --------------------------------------------------------------------------- *
 */
void lora_raw_radio_ctor(void)
{
    lora_raw_handle_nvm_change();

    loramac_radio_ctor();
}

void lora_raw_radio_dtor(void)
{
    lora_raw_handle_nvm_change();
    loramac_radio_dtor();
}

void lora_raw_radio_send(uint8_t* buf, uint8_t len)
{
    __log_debug("transmitted data size: %d", len);
    Radio.Send(buf, len);
}

void lora_raw_radio_recv(void)
{
    __log_debug("send rx request");
    Radio.Rx(0);
}

lora_error_t lora_raw_radio_reset_params(void)
{
    radio_if_nvm_load_defaults_callback(NULL, 0);
    return __LORA_OK;
}

static void reset_region_params(lora_region_t region)
{
    s_radio_lora_params.region = region;
    LoRaMacRegion_t reg = get_lora_mac_region_enum_value(region);

    GetPhyParams_t req_param;
    PhyParam_t ret_param;

    // -- load region default params
    s_radio_lora_params.freq = get_lora_region_default_frequency(region);

    // antenna gain
    req_param.Attribute = PHY_DEF_ANTENNA_GAIN;
    ret_param = RegionGetPhyParam(reg, &req_param);
    s_radio_lora_params.antenna_gain = ret_param.fValue;

    // max eirp
    req_param.Attribute = PHY_DEF_MAX_EIRP;
    ret_param = RegionGetPhyParam(reg, &req_param);
    s_radio_lora_params.max_eirp = ret_param.fValue;

    // This is a tweak for the region US915 because the antenna gain is added
    // to the max EIRP in the getter function.
    // other regions returns the max EIRP directly
    if(region == __LORA_REGION_US915)
        s_radio_lora_params.max_eirp -= 2.15f;

    // tx-power
    req_param.Attribute = PHY_DEF_TX_POWER;
    ret_param = RegionGetPhyParam(reg, &req_param);
    s_radio_lora_params.tx_power =
        tx_power_index_to_dbm(ret_param.Value, s_radio_lora_params.max_eirp);

    // DR -- data rate index
    req_param.Attribute = PHY_DEF_TX_DR;
    ret_param = RegionGetPhyParam(reg, &req_param);
    uint32_t default_dr = ret_param.Value;

    // sf -- spreading factor
    req_param.Attribute = PHY_SF_FROM_DR;
    req_param.Datarate = default_dr;
    ret_param = RegionGetPhyParam(reg, &req_param);
    s_radio_lora_params.sf = ret_param.Value;

    // BW -- bandwidth
    req_param.Attribute = PHY_BW_FROM_DR;
    ret_param = RegionGetPhyParam(reg, &req_param);
    s_radio_lora_params.bw = ret_param.Value;

    // payload
    req_param.Attribute = PHY_MAX_PAYLOAD;
    ret_param = RegionGetPhyParam(reg, &req_param);
    s_radio_lora_params.payload = ret_param.Value;

    // set others to default values
    s_radio_lora_params.cr         = __lora_raw_default_cr;
    s_radio_lora_params.preamble   = __lora_raw_default_preamble;
    s_radio_lora_params.tx_inv_iq  = __lora_raw_default_tx_inv_iq;
    s_radio_lora_params.rx_inv_iq  = __lora_raw_default_rx_inv_iq;
    s_radio_lora_params.crc_on     = __lora_raw_default_crc_on;
    s_radio_lora_params.symb_timeout = __lora_raw_default_symb_timeout;
    s_radio_lora_params.tx_timeout = __lora_raw_default_tx_timeout;
    s_radio_lora_params.rx_timeout = __lora_raw_default_rx_timeout;

    tx_power_correct();
}

static const char* lora_raw_params_names[] = {
    [__LORA_RAW_PARAM_REGION       ] = "region", 
    [__LORA_RAW_PARAM_FREQ         ] = "frequency", 
    [__LORA_RAW_PARAM_TX_POWER     ] = "tx_power", 
    [__LORA_RAW_PARAM_SF           ] = "sf", 
    [__LORA_RAW_PARAM_CR           ] = "coding_rate", 
    [__LORA_RAW_PARAM_PREAMBLE     ] = "preamble", 
    [__LORA_RAW_PARAM_BW           ] = "bandwidth", 
    [__LORA_RAW_PARAM_PAYLOAD      ] = "payload", 
    [__LORA_RAW_PARAM_TX_INV_IQ    ] = "tx_iq", 
    [__LORA_RAW_PARAM_RX_INV_IQ    ] = "rx_iq", 
    [__LORA_RAW_PARAM_CRC_ON       ] = "crc_on", 
    [__LORA_RAW_PARAM_SYMB_TIMEOUT ] = "symbol_timeout", 
    [__LORA_RAW_PARAM_TX_TIMEOUT   ] = "tx_timeout",
    [__LORA_RAW_PARAM_RX_TIMEOUT   ] = "rx_timeout", 
};

static const char* get_param_string(lora_raw_param_type_t type)
{
    if(type < sizeof(lora_raw_params_names)/sizeof(lora_raw_params_names[0]))
    {
        return lora_raw_params_names[type];
    }

    __log_error("try get wrong param");

    return NULL;
}

static bool verify_region(lora_region_t region, void* param)
{
    switch(region) {
        #if defined(REGION_AS923)
        case __LORA_REGION_AS923:
        #endif
        #if defined(REGION_AU915)
        case __LORA_REGION_AU915:
        #endif
        #if defined(REGION_CN470)
        case __LORA_REGION_CN470:
        #endif
        #if defined(REGION_CN779)
        case __LORA_REGION_CN779:
        #endif
        #if defined(REGION_EU433)
        case __LORA_REGION_EU433:
        #endif
        #if defined(REGION_EU868)
        case __LORA_REGION_EU868:
        #endif
        #if defined(REGION_KR920)
        case __LORA_REGION_KR920:
        #endif
        #if defined(REGION_IN865)
        case __LORA_REGION_IN865:
        #endif
        #if defined(REGION_US915)
        case __LORA_REGION_US915:
        #endif
        #if defined(REGION_RU864)
        case __LORA_REGION_RU864:
        #endif
            return true;
    }
    return false;
}
static bool verify_freq(lora_region_t region, void* param)
{
    return RegionVerify(
        get_lora_mac_region_enum_value(region),
        &(VerifyParams_t){.Frequency = *(uint32_t*)param},
        PHY_FREQUENCY );
}
static bool verify_tx_power(lora_region_t region, void* param)
{
    int8_t tx_power_dbm = *(int8_t*)param;

    // checking tx-power w.r.t the chip capabilities

    if(tx_power_dbm > __tx_power_chip_max() ||
       tx_power_dbm < __tx_power_chip_min()) {
        __log_output(__red__"error:"__default__" invalid chip power "
            __red__"%+d"__default__" dBm -- chip "__cyan__"%s"__default__
            " tx power range ( "__green__"%+d ~ %+d"__default__" ) dBm"
            " considering antenna gain "__green__"%.2f"__default__" dBi\n",
            tx_power_dbm, lora_radio_ext_get_chip_name(),
            __tx_power_chip_min(), __tx_power_chip_max(),
            s_radio_lora_params.antenna_gain);
        return false;
    }

    // protecting against max allowed region tx-power

    int8_t max_region_tx_power = tx_power_get_region_max(region);

    if( tx_power_dbm > max_region_tx_power )
    {
        __log_output(__red__"error:"__default__" invalid region power "
            __red__"%+d"__default__" dBm -- max region "
            __cyan__"%s"__default__" available power "
            __green__"%+d"__default__" dBm\n",
            tx_power_dbm, get_lora_region_string(region), max_region_tx_power);
        return false;
    }
    return true;
}
static bool verify_antenna_gain(lora_region_t region, void* param)
{
    #define __max_antenna_gain 2.151f

    float antenna_gain = *(float*)param;
    return antenna_gain > -0.001f && antenna_gain <= __max_antenna_gain;
}
static bool verify_sf(lora_region_t region, void* param)
{
    switch(*(uint8_t*)param)
    {
        case __LORA_SF_6:
        case __LORA_SF_7:
        case __LORA_SF_8:
        case __LORA_SF_9:
        case __LORA_SF_10:
        case __LORA_SF_11:
        case __LORA_SF_12:
            return true;
    }
    return false;
}
static bool verify_cr(lora_region_t region, void* param)
{
    switch(*(uint8_t*)param)
    {
        case __LORA_CR_4_5:
        case __LORA_CR_4_6:
        case __LORA_CR_4_7:
        case __LORA_CR_4_8:
            return true;
    }
    return false;
}
static bool verify_preamble(lora_region_t region, void* param)
{
    return true;
}
static bool verify_bw(lora_region_t region, void* param)
{
    switch(*(uint8_t*)param)
    {
        case __LORA_BW_125_KHZ:
        case __LORA_BW_250_KHZ:
        case __LORA_BW_500_KHZ:
            return true;
    }
    return false;

}
static bool verify_payload(lora_region_t region, void* param)
{
    return true;
}
static bool verify_tx_inv_iq(lora_region_t region, void* param)
{
    return true;
}
static bool verify_rx_inv_iq(lora_region_t region, void* param)
{
    return true;
}
static bool verify_crc_on(lora_region_t region, void* param)
{
    return true;
}
static bool verify_symb_timeout(lora_region_t region, void* param)
{
    return true;
}
static bool verify_tx_timeout(lora_region_t region, void* param)
{
    return true;
}
static bool verify_rx_timeout(lora_region_t region, void* param)
{
    return true;
}

static bool (*lora_raw_param_verification_table [] )(lora_region_t, void*) = {
    [__LORA_RAW_PARAM_REGION       ] = verify_region,
    [__LORA_RAW_PARAM_FREQ         ] = verify_freq,
    [__LORA_RAW_PARAM_TX_POWER     ] = verify_tx_power,
    [__LORA_RAW_PARAM_ANTENNA_GAIN ] = verify_antenna_gain,
    [__LORA_RAW_PARAM_SF           ] = verify_sf,
    [__LORA_RAW_PARAM_CR           ] = verify_cr,
    [__LORA_RAW_PARAM_PREAMBLE     ] = verify_preamble,
    [__LORA_RAW_PARAM_BW           ] = verify_bw,
    [__LORA_RAW_PARAM_PAYLOAD      ] = verify_payload,
    [__LORA_RAW_PARAM_TX_INV_IQ    ] = verify_tx_inv_iq,
    [__LORA_RAW_PARAM_RX_INV_IQ    ] = verify_rx_inv_iq,
    [__LORA_RAW_PARAM_CRC_ON       ] = verify_crc_on,
    [__LORA_RAW_PARAM_SYMB_TIMEOUT ] = verify_symb_timeout,
    [__LORA_RAW_PARAM_TX_TIMEOUT   ] = verify_tx_timeout,
    [__LORA_RAW_PARAM_RX_TIMEOUT   ] = verify_rx_timeout,
};

lora_error_t lora_raw_radio_verify_param(lora_raw_param_t* param)
{
    lora_error_t ret = __LORA_ERROR;
    if(param->type < sizeof(lora_raw_param_verification_table) / 
        sizeof(lora_raw_param_verification_table[0]) &&
        lora_raw_param_verification_table[param->type] != NULL)
    {
        if( lora_raw_param_verification_table[param->type](
            param->region, &param->param)) {
            ret = __LORA_OK;
        }
    }
    return ret;
}

lora_error_t lora_raw_radio_set_param(lora_raw_param_t* param)
{
    #define __set_param(_p) \
        __log_info("set "__yellow__ #_p __default__" = %d", param->param._p);\
        s_radio_lora_params._p = param->param._p
    
    bool verified = lora_raw_radio_verify_param(param) == __LORA_OK;
    if( ! verified )
    {
        __log_output("lora-raw: parameter verification failed: "__red__"%s\n",
            get_param_string(param->type));
        return __LORA_ERROR;
    }

    switch(param->type) {
        case __LORA_RAW_PARAM_REGION:
            reset_region_params(param->region);
            break;
        case __LORA_RAW_PARAM_TX_POWER:
            __set_param(tx_power);
            tx_power_correct();
            break;
        case __LORA_RAW_PARAM_ANTENNA_GAIN:
            {
                float gain = param->param.antenna_gain;
                if(gain < 0) // because we added epsilon 0.001 while verifying
                    gain = 0.0f;
                s_radio_lora_params.antenna_gain = gain;
            }
            tx_power_correct();
            break;
        case __LORA_RAW_PARAM_FREQ:         __set_param(freq);      break;
        case __LORA_RAW_PARAM_SF:           __set_param(sf);        break;
        case __LORA_RAW_PARAM_CR:           __set_param(cr);        break;
        case __LORA_RAW_PARAM_PREAMBLE:     __set_param(preamble);  break;
        case __LORA_RAW_PARAM_BW:           __set_param(bw);        break;
        case __LORA_RAW_PARAM_PAYLOAD:      __set_param(payload);   break;
        case __LORA_RAW_PARAM_TX_INV_IQ:    __set_param(tx_inv_iq); break;
        case __LORA_RAW_PARAM_RX_INV_IQ:    __set_param(rx_inv_iq); break;
        case __LORA_RAW_PARAM_CRC_ON:       __set_param(crc_on);    break;
        case __LORA_RAW_PARAM_SYMB_TIMEOUT: __set_param(symb_timeout);break;
        case __LORA_RAW_PARAM_TX_TIMEOUT:   __set_param(tx_timeout);break;
        case __LORA_RAW_PARAM_RX_TIMEOUT:   __set_param(rx_timeout);break;
        default:
            return __LORA_ERROR;
    }
    #undef __set_param
    return __LORA_OK;
}

lora_error_t lora_raw_radio_get_param(lora_raw_param_t* param)
{
    #define __get_param(_p) param->param._p = s_radio_lora_params._p
    switch(param->type) {
        case __LORA_RAW_PARAM_REGION:
            param->region = s_radio_lora_params.region;
            break;
        case __LORA_RAW_PARAM_FREQ:         __get_param(freq);      break;
        case __LORA_RAW_PARAM_ANTENNA_GAIN: __get_param(antenna_gain);break;
        case __LORA_RAW_PARAM_TX_POWER:     __get_param(tx_power);  break;
        case __LORA_RAW_PARAM_SF:           __get_param(sf);        break;
        case __LORA_RAW_PARAM_CR:           __get_param(cr);        break;
        case __LORA_RAW_PARAM_PREAMBLE:     __get_param(preamble);  break;
        case __LORA_RAW_PARAM_BW:           __get_param(bw);        break;
        case __LORA_RAW_PARAM_PAYLOAD:      __get_param(payload);   break;
        case __LORA_RAW_PARAM_TX_INV_IQ:    __get_param(tx_inv_iq); break;
        case __LORA_RAW_PARAM_RX_INV_IQ:    __get_param(rx_inv_iq); break;
        case __LORA_RAW_PARAM_CRC_ON:       __get_param(crc_on);    break;
        case __LORA_RAW_PARAM_SYMB_TIMEOUT: __get_param(symb_timeout);break;
        case __LORA_RAW_PARAM_TX_TIMEOUT:   __get_param(tx_timeout);break;
        case __LORA_RAW_PARAM_RX_TIMEOUT:   __get_param(rx_timeout);break;
        default:
            return __LORA_ERROR;
    }
    #undef __get_param
    return __LORA_OK;
}

lora_error_t lora_raw_radio_get_default_region_param(lora_raw_param_t* param)
{
    lora_region_t region  = param->region;
    LoRaMacRegion_t reg = get_lora_mac_region_enum_value(region);
    GetPhyParams_t req_param;
    PhyParam_t ret_param;

    switch(param->type) {
        case __LORA_RAW_PARAM_FREQ:
            param->param.freq = get_lora_region_default_frequency(region);
            break;
        case __LORA_RAW_PARAM_ANTENNA_GAIN:
            req_param.Attribute = PHY_DEF_ANTENNA_GAIN;
            ret_param = RegionGetPhyParam(reg, &req_param);
            param->param.antenna_gain = ret_param.fValue;
            break;
        case __LORA_RAW_PARAM_TX_POWER:
        {
            // max eirp
            float max_eirp;
            req_param.Attribute = PHY_DEF_MAX_EIRP;
            ret_param = RegionGetPhyParam(reg, &req_param);
            max_eirp = ret_param.fValue;

            // This is a tweak for the region US915 because the antenna gain is
            // added to the max EIRP in the getter function.
            // other regions returns the max EIRP directly
            if(region == __LORA_REGION_US915)
                max_eirp -= 2.15f;
            req_param.Attribute = PHY_DEF_TX_POWER;
            ret_param = RegionGetPhyParam(reg, &req_param);
            int8_t tx_power = tx_power_index_to_dbm(ret_param.Value, max_eirp);
            
            param->param.tx_power = tx_power_correct_specific(tx_power, region);
            break;
        }
        case __LORA_RAW_PARAM_SF:
        {
            // DR -- data rate index
            req_param.Attribute = PHY_DEF_TX_DR;
            ret_param = RegionGetPhyParam(reg, &req_param);
            uint32_t default_dr = ret_param.Value;

            // sf -- spreading factor
            req_param.Attribute = PHY_SF_FROM_DR;
            req_param.Datarate = default_dr;
            ret_param = RegionGetPhyParam(reg, &req_param);
            param->param.sf = ret_param.Value;
            break;
        }
        case __LORA_RAW_PARAM_CR:
            param->param.cr = __lora_raw_default_cr;
            break;
        case __LORA_RAW_PARAM_PREAMBLE:
            param->param.preamble = __lora_raw_default_preamble;
            break;
        case __LORA_RAW_PARAM_BW:
        {
            // DR -- data rate index
            req_param.Attribute = PHY_DEF_TX_DR;
            ret_param = RegionGetPhyParam(reg, &req_param);
            uint32_t default_dr = ret_param.Value;

            // BW -- bandwidth
            req_param.Attribute = PHY_BW_FROM_DR;
            req_param.Datarate = default_dr;
            ret_param = RegionGetPhyParam(reg, &req_param);
            param->param.bw = ret_param.Value;
            break;
        }
        case __LORA_RAW_PARAM_PAYLOAD:
            // payload
            req_param.Attribute = PHY_MAX_PAYLOAD;
            ret_param = RegionGetPhyParam(reg, &req_param);
            param->param.payload = ret_param.Value;
            break;
        case __LORA_RAW_PARAM_TX_INV_IQ:
            param->param.tx_inv_iq = __lora_raw_default_tx_inv_iq;
            break;
        case __LORA_RAW_PARAM_RX_INV_IQ:
            param->param.rx_inv_iq = __lora_raw_default_rx_inv_iq;
            break;
        case __LORA_RAW_PARAM_CRC_ON:
            param->param.crc_on = __lora_raw_default_crc_on;
            break;
        case __LORA_RAW_PARAM_SYMB_TIMEOUT:
            param->param.symb_timeout = __lora_raw_default_symb_timeout;
            break;
        case __LORA_RAW_PARAM_TX_TIMEOUT:
            param->param.tx_timeout = __lora_raw_default_tx_timeout;
            break;
        case __LORA_RAW_PARAM_RX_TIMEOUT:
            param->param.rx_timeout = __lora_raw_default_rx_timeout;
            break;
        default:
            return __LORA_ERROR;
    }
    return __LORA_OK;
}

void lora_raw_radio_process_irqs(void)
{
    loramac_radio_process_irqs();
}

void lora_raw_radio_sleep(void)
{
    Radio.Sleep();
}

void lora_raw_radio_tx_cont_wave(uint32_t freq, int8_t power,
    uint32_t time_msec)
{
    Radio.SetTxContinuousWave(freq, power, time_msec/1000);
}

uint32_t lora_raw_radio_get_time_on_air(void)
{
    return s_radio_lora_params.time_on_air;
}

static const char* s_on_off[] = {
    __red__"False"__default__,
    __green__"True"__default__
};

void lora_raw_radio_stats(void)
{
    #define __temp(item, val_fmt, args...) \
        log_list_item("%-15s: " val_fmt __default__, item, args)

    log_list_start(4);

    log_list_indent();
    log_list_item(__blue__"regional params");
    log_list_indent();
        __temp("region", __yellow__"%s",
            get_lora_region_string(s_radio_lora_params.region));
        __temp("frequency", __yellow__"%d"__default__" Hz",
            s_radio_lora_params.freq);
        __temp("freq_khz", __yellow__"%.3f"__default__" KHz",
            (float)s_radio_lora_params.freq/1000.0f);
        __temp("freq_mhz", __yellow__"%.3f"__default__" MHz",
            (float)s_radio_lora_params.freq/1000000.0f);
    log_list_outdent();
    log_list_item(__blue__"modulation params");
    log_list_indent();
        __temp("sf", __yellow__"%d", s_radio_lora_params.sf);
        {
            __temp("bandwidth", __yellow__"%d"__default__" KHz",
                s_radio_lora_params.bw == __LORA_BW_125_KHZ ? 125u :
                s_radio_lora_params.bw == __LORA_BW_250_KHZ ? 250u :
                s_radio_lora_params.bw == __LORA_BW_500_KHZ ? 500u :
                0 );
        }
        __temp("coding_rate", __yellow__"4_%d", (s_radio_lora_params.cr + 4));
    log_list_outdent();
    log_list_item(__blue__"packet params");
    log_list_indent();
        __temp("preamble", __yellow__"%d", s_radio_lora_params.preamble);
        __temp("payload", __yellow__"%d", s_radio_lora_params.payload);
        __temp("crc_on", "%s", s_on_off[ s_radio_lora_params.crc_on ]);
    log_list_outdent();
    log_list_item(__blue__"lora tranceiver");
    log_list_indent();
        __temp("chip", __yellow__"%s", lora_radio_ext_get_chip_name());
        __temp("max tx_power", __yellow__"%+d"__default__" dBm",
            lora_radio_ext_get_max_tx_power());
    log_list_outdent();
    log_list_item(__blue__"tx params");
    log_list_indent();
        __temp("tx_power", __yellow__"%+d"__default__" dBm",
            (int)s_radio_lora_params.tx_power);
        __temp("antenna_gain", __yellow__"%+.2f"__default__" dBi",
            s_radio_lora_params.antenna_gain);
        __temp("tx_power_eff", __yellow__"%+d"__default__" dBm",
            __tx_power_effective());
        __temp("tx_timeout", __yellow__"%d"__default__" msec",
            s_radio_lora_params.tx_timeout);
        __temp("tx_iq", "%s", s_on_off[ s_radio_lora_params.tx_inv_iq ]);
    log_list_outdent();
    log_list_item(__blue__"rx params");
    log_list_indent();
        __temp("rx_timeout", __yellow__"%d"__default__" msec",
            s_radio_lora_params.rx_timeout);
        __temp("rx_iq", "%s", s_on_off[ s_radio_lora_params.rx_inv_iq ]);
    log_list_end();
}

/* --- end of file ---------------------------------------------------------- */
