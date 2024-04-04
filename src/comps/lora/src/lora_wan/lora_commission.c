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
 * @brief   lora-wan commission parameters handler. it do the following things:
 *          - commission parameters storage management (setting, retrieval)
 *          - load the lora-mac layer with the commission parameters
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include <string.h>
#define __log_subsystem     lora
#define __log_component     wan_comision
#include "log_lib.h"

#include "LoRaMac.h"
#include "Commissioning.h"

#include "lora_commission.h"
#include "lora.h"
#include "lora_nvm.h"

/* --- types ---------------------------------------------------------------- */

typedef struct {
    commission_type_t type;
    lora_wan_version_t version;
    union{
        struct {
            uint8_t app_key[16];
            uint8_t nwk_key[16];
            uint8_t dev_eui[8];
            uint8_t join_eui[8];
        } otaa;
        struct {
            uint8_t app_s_key[16];
            uint8_t nwk_s_key[16];
            uint8_t dev_eui[8];
            uint32_t dev_addr;
        } abp;
    };
    lora_nvm_record_tail_t nvm_record_tail;
} lora_commissioning_data_t;

static void lora_commission_handle_data_change(void);

/** -------------------------------------------------------------------------- *
 * ctor()/dtor()
 * --------------------------------------------------------------------------- *
 */
void lora_commission_ctor(void)
{
    lora_commission_handle_data_change();
}
void lora_commission_dtor(void)
{
    lora_commission_handle_data_change();
}

/** -------------------------------------------------------------------------- *
 * commisioning parameters NvM handling
 * --------------------------------------------------------------------------- *
 */

static lora_commissioning_data_t s_lora_commissioning_params;

static const char* s_lora_commission_nvm_key = "lora-commission";


static void lora_commission_load_defaults( void* p_record_mem, uint32_t size )
{
    memset(p_record_mem, 0, sizeof(size));
    s_lora_commissioning_params.type = __LORA_COMMISSION_OTAA;
}
static void lora_commission_handle_data_change(void)
{
    __log_info("handle commissioning params NvM");

    lora_nvm_handle_change(
        s_lora_commission_nvm_key,
        &s_lora_commissioning_params,
        sizeof(s_lora_commissioning_params),
        lora_commission_load_defaults
    );
}

/** -------------------------------------------------------------------------- *
 * commisioning parameters APIs
 * --------------------------------------------------------------------------- *
 */

void lora_commission_set(lora_commission_params_t * p_params)
{
    lora_commissioning_data_t* ptr = & s_lora_commissioning_params;
    if(p_params->type == __LORA_COMMISSION_OTAA)
    {
        __log_info("set otaa commissioning");
        memcpy(ptr->otaa.dev_eui,  p_params->otaa.dev_eui,  8);
        memcpy(ptr->otaa.join_eui, p_params->otaa.join_eui, 8);
        memcpy(ptr->otaa.app_key,  p_params->otaa.app_key, 16);
        if(p_params->version == __LORA_WAN_VERSION_1_1_X)
        {
            memcpy(ptr->otaa.nwk_key,  p_params->otaa.nwk_key, 16);
        }
    }
    else if(p_params->type == __LORA_COMMISSION_ABP)
    {
        __log_info("set abp commissioning");
        ptr->abp.dev_addr = p_params->abp.dev_addr;
        memcpy(ptr->abp.app_s_key, p_params->abp.app_s_key, 16);
        memcpy(ptr->abp.nwk_s_key, p_params->abp.nwk_s_key, 16);
        memcpy(ptr->abp.dev_eui, p_params->abp.dev_eui, 8);
    }
    else
    {
        __log_error("wrong commissioning type ( %d )", p_params->type);
    }
    s_lora_commissioning_params.type = p_params->type;
    s_lora_commissioning_params.version = p_params->version;

    lora_commission_handle_data_change();
}

void lora_commission_update_mac_layer( void )
{
    MibRequestConfirm_t mib;

    if( s_lora_commissioning_params.type == __LORA_COMMISSION_OTAA )
    {

        __log_info("update LoRaMac layer with OTAA provisioning");

        mib.Type = MIB_NETWORK_ACTIVATION;
        mib.Param.NetworkActivation = ACTIVATION_TYPE_OTAA;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_DEV_EUI;
        mib.Param.DevEui = s_lora_commissioning_params.otaa.dev_eui;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_JOIN_EUI;
        mib.Param.JoinEui = s_lora_commissioning_params.otaa.join_eui;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_APP_KEY;
        mib.Param.AppKey = s_lora_commissioning_params.otaa.app_key;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_NWK_KEY;
        if(s_lora_commissioning_params.version == __LORA_WAN_VERSION_1_1_X)
        {
            // lora-wan version 1.1.x uses different app and nwk keys
            mib.Param.NwkKey = s_lora_commissioning_params.otaa.nwk_key;
        }
        else
        {
            // lora-wan version 1.0.x uses only a general app key
            // hence, we set the nwk key with app key as well here
            mib.Param.NwkKey = s_lora_commissioning_params.otaa.app_key;
        }
        LoRaMacMibSetRequestConfirm( &mib );
    }
    else if (s_lora_commissioning_params.type == __LORA_COMMISSION_ABP )
    {

        __log_info("update LoRaMac layer with ABP provisioning");

        mib.Type = MIB_NET_ID;
        mib.Param.NetID = LORAWAN_NETWORK_ID;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_DEV_ADDR;
        mib.Param.DevAddr = s_lora_commissioning_params.abp.dev_addr;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_DEV_EUI;
        mib.Param.DevEui = s_lora_commissioning_params.abp.dev_eui;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_APP_S_KEY;
        mib.Param.AppSKey = s_lora_commissioning_params.abp.app_s_key;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_F_NWK_S_INT_KEY;
        mib.Param.FNwkSIntKey = s_lora_commissioning_params.abp.nwk_s_key;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_S_NWK_S_INT_KEY;
        mib.Param.SNwkSIntKey = s_lora_commissioning_params.abp.nwk_s_key;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_NWK_S_ENC_KEY;
        mib.Param.NwkSEncKey = s_lora_commissioning_params.abp.nwk_s_key;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_ABP_LORAWAN_VERSION;
        mib.Param.AbpLrWanVersion.Value = 
            s_lora_commissioning_params.version == __LORA_WAN_VERSION_1_1_X
                ? ABP_ACTIVATION_LRWAN_VERSION_V11x
                : ABP_ACTIVATION_LRWAN_VERSION_V10x;
        LoRaMacMibSetRequestConfirm( &mib );

        mib.Type = MIB_NETWORK_ACTIVATION;
        mib.Param.NetworkActivation = ACTIVATION_TYPE_ABP;
        LoRaMacMibSetRequestConfirm( &mib );
    }
}

uint8_t* lora_commission_get_dev_eui_ref(void)
{
    return s_lora_commissioning_params.otaa.dev_eui;
}
uint8_t* lora_commission_get_join_eui_ref(void)
{
    return s_lora_commissioning_params.otaa.join_eui;
}
uint32_t lora_commission_get_dev_addr(void)
{
    return s_lora_commissioning_params.abp.dev_addr;
}
commission_type_t lora_commission_get_type(void)
{
    return s_lora_commissioning_params.type;
}

/* --- end of file ---------------------------------------------------------- */
