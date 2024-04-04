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
 * @brief   Semtech LoRaMac stack NvM handling sub-component.
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#define __log_subsystem     lora
#define __log_component     wan_nvm
#include "log_lib.h"

#include "LoRaMac.h"
#include "stub_nvm.h"
#include "lora_commission.h"
#include "lora_wan_nvm.h"

/** -------------------------------------------------------------------------- *
 * Implementation
 * --------------------------------------------------------------------------- *
 */
static LoRaMacNvmData_t s_nvm;

static bool s_is_initialized = false;

#define __check_init(__ret)                             \
    do{                                                 \
        if( !s_is_initialized ) {                       \
            __log_error("lora_nvm is not initialized"); \
            return __ret;                               \
        }                                               \
    }while(0)

typedef struct {
    uint16_t flag;
    uint16_t length;
    uint16_t offset;
    uint16_t is_init;
    const char* nvm_key;
} lora_nvm_record_meta_info_t;

static lora_nvm_record_meta_info_t s_nvm_records_info[] = {
    #define __offset_of(__member) \
        ((uint16_t)(uint32_t)&(((LoRaMacNvmData_t*)0)->__member))
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_CRYPTO,
        .length = sizeof(LoRaMacCryptoNvmData_t),
        .offset = __offset_of(Crypto),
        .nvm_key = "lora-crypto"
    },
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1,
        .length = sizeof(LoRaMacNvmDataGroup1_t),
        .offset = __offset_of(MacGroup1),
        .nvm_key = "loramac-group1"
    },
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2,
        .length = sizeof(LoRaMacNvmDataGroup2_t),
        .offset = __offset_of(MacGroup2),
        .nvm_key = "loramac-group2"
    },
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT,
        .length = sizeof(SecureElementNvmData_t),
        .offset = __offset_of(SecureElement),
        .nvm_key = "lora-SE"
    },
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1,
        .length = sizeof(RegionNvmDataGroup1_t),
        .offset = __offset_of(RegionGroup1),
        .nvm_key = "lora-region-1"
    },
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2,
        .length = sizeof(RegionNvmDataGroup2_t),
        .offset = __offset_of(RegionGroup2),
        .nvm_key = "lora-region-2"
    },
    {
        .flag   = LORAMAC_NVM_NOTIFY_FLAG_CLASS_B,
        .length = sizeof(LoRaMacClassBNvmData_t),
        .offset = __offset_of(ClassB),
        .nvm_key = "lora-class-B"
    }
    #undef __offset_of
};

#define __lora_records_number  \
    (sizeof(s_nvm_records_info)/sizeof(s_nvm_records_info[0]))

bool lora_nvm_restore(void)
{
    s_is_initialized = true;
    bool is_restored = false;

    lora_nvm_record_meta_info_t * ptr = s_nvm_records_info;
    uint32_t records = __lora_records_number;
    uint32_t found_records = 0;
    uint8_t* base = (void*)&s_nvm;
    while( records -- ) {
        if( lora_stub_nvm_check(ptr->nvm_key) ) {
            __log_info("-- load %s", ptr->nvm_key);
            ++ found_records;
            lora_stub_nvm_load(ptr->nvm_key, base + ptr->offset, ptr->length);
        }
        ++ ptr;
    }

    if(found_records == __lora_records_number) {
        MibRequestConfirm_t mib;
        mib.Type = MIB_NVM_CTXS;
        mib.Param.Contexts = (void*)base;
        if( LoRaMacMibSetRequestConfirm( &mib ) != LORAMAC_STATUS_OK ) {
            __log_error("lora nvm restore Failed");
        } else {
            is_restored = true;
            __log_info("lora nvm restore "__green__"OK");
        }
    } else {
        memset(base, 0, sizeof(s_nvm));
        ptr = s_nvm_records_info;
        records = __lora_records_number;
        while( records -- ) {
            lora_stub_nvm_store(ptr->nvm_key, base + ptr->offset, ptr->length);
            ++ ptr;
        }

        lora_stub_nvm_sync();

        __log_info("lora nvm init");
    }
    return is_restored;
}

void lora_nvm_clear_all(void)
{
    lora_nvm_record_meta_info_t * ptr = s_nvm_records_info;
    uint32_t records = __lora_records_number;
    while( records -- ) {
        __log_info("-- clear %s", ptr->nvm_key);
        lora_stub_nvm_clear(ptr->nvm_key);
        ++ ptr;
    }
    memset(&s_nvm, 0, sizeof(s_nvm));
    s_is_initialized = false;
    lora_stub_nvm_sync();
}

void lora_nvm_handle_data_change(uint16_t flags)
{
    __log_debug("");

    __check_init();

    if(flags == LORAMAC_NVM_NOTIFY_FLAG_CRYPTO) {
        return;
    }

    MibRequestConfirm_t mib;

    mib.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm( &mib );

    uint8_t * p_src_base = (void*)mib.Param.Contexts;
    uint8_t * p_dst_base = (void*)&s_nvm;

    lora_nvm_record_meta_info_t * ptr = s_nvm_records_info;
    uint32_t records = __lora_records_number;
    uint16_t offset;
    uint16_t length;
    while( records -- ) {
        if(ptr->flag & flags) {
            __log_debug("change in %s", ptr->nvm_key);
            offset = ptr->offset;
            length = ptr->length;
            memcpy(p_dst_base + offset, p_src_base + offset, length);
            lora_stub_nvm_store(ptr->nvm_key, p_dst_base + offset, length);
        }
        ++ ptr;
    }
    lora_stub_nvm_sync();
}

LoRaMacNvmData_t* lora_nvm_get_ref(void)
{
    __check_init( NULL );

    return &s_nvm;
}

void lora_nvm_get_join_accept_delays(uint32_t * p_rx_1, uint32_t * p_rx_2)
{
    __check_init();

    *p_rx_1 = s_nvm.MacGroup2.MacParamsDefaults.JoinAcceptDelay1;
    *p_rx_2 = s_nvm.MacGroup2.MacParamsDefaults.JoinAcceptDelay2;
}

void lora_nvm_get_rx_delays(uint32_t * p_rx_1, uint32_t * p_rx_2)
{
    __check_init();

    *p_rx_1 = s_nvm.MacGroup2.MacParamsDefaults.ReceiveDelay1;
    *p_rx_2 = s_nvm.MacGroup2.MacParamsDefaults.ReceiveDelay2;
}

static struct {
    uint32_t  number;
    char      string[50];
} test_nvm_data = {
    .number = 1000,
    .string = "hello test string"
};

void lora_nvm_test(uint32_t number, const char* str)
{
    const char* test_key = "test_key";
    if(lora_stub_nvm_check(test_key)) {
        lora_stub_nvm_load(test_key, (void*)&test_nvm_data,
            sizeof(test_nvm_data));
        __log_output(" -- @test_key: number "__yellow__"%d"__default__
            ", string "__green__"%s"__default__"\n",
            test_nvm_data.number, test_nvm_data.string);
    } else {
        __log_output(" -- no previous record -- \n");
    }
    __log_output(" -- storing new data num: %d, string: %s", number, str);
    test_nvm_data.number = number,
    strncpy(test_nvm_data.string, str, 49);
    lora_stub_nvm_store(test_key, (void*)&test_nvm_data, sizeof(test_nvm_data));
    lora_stub_nvm_sync();
}

/* --- end of file ---------------------------------------------------------- */
