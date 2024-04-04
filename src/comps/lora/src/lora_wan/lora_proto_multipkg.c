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
 * @brief   multi-package lora-wan protocol component.
 * 
 * TODO: not completed yet
 * --------------------------------------------------------------------------- *
 */

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>
#include <stdbool.h>

#define __log_subsystem     lora_pkg
#define __log_component     wan_multipkg
#include "log_lib.h"

#include "LoRaMac.h"
#include "LmHandler.h"
#include "LmHandlerTypes.h"
#include "LmhPackage.h"

/** -------------------------------------------------------------------------- *
 * Package management
 * --------------------------------------------------------------------------- *
 */
#define MULTI_PACKAGE_PORT  (225)
#define MULTI_PACKAGE_ID    (0)

static bool s_is_initialized = false;

typedef struct {
    bool        is_initialized;
    bool        is_tx_pending;
    uint8_t*    data_buf;
    uint8_t     data_buf_max_size;
} lmh_multipkg_state_t;

typedef enum {
    __MULTIPKG_PACKAGE_VERSION_REQ  = 0x00,
    __MULTIPKG_DEV_PKG_REQ          = 0x01,
    __MULTIPKG_BUFFER_REQ           = 0x02,
} multipkg_srv_req_t;

typedef enum {
    __MULTIPKG_PACKAGE_VERSION_ANS  = 0x00,
    __MULTIPKG_DEV_PKG_ANS          = 0x01,
    __MULTIPKG_BUFFER_FRAG          = 0x02,
} multipkg_srv_ans_t;

static void multipkg_init(void *params, uint8_t *dataBuffer,
    uint8_t dataBufferMaxSize);
static bool multipkg_is_initialized( void );
static bool multipkg_is_tx_pending( void );
static void multipkg_process( void );
static void multipkg_on_mcps_confirm_process( McpsConfirm_t *mcpsConfirm );
static void multipkg_on_mcps_indication_process(
    McpsIndication_t *mcpsIndication );
static lmh_multipkg_state_t s_multipkg_state;

static LmhPackage_t LmhpFragmentationPackage =
{
    .Port = MULTI_PACKAGE_PORT,
    .Init = multipkg_init,
    .IsInitialized = multipkg_is_initialized,
    .IsTxPending =  multipkg_is_tx_pending,
    .Process = multipkg_process,
    .OnMcpsConfirmProcess = multipkg_on_mcps_confirm_process,
    .OnMcpsIndicationProcess = multipkg_on_mcps_indication_process,
    .OnMlmeConfirmProcess = NULL,
    .OnMlmeIndicationProcess = NULL,
    .OnMacMcpsRequest = NULL,
    .OnMacMlmeRequest = NULL,
    .OnJoinRequest = NULL,
    .OnDeviceTimeRequest = NULL,
    .OnSysTimeUpdate = NULL,
};

void __mpkg_dummy(void)
{
    (void)LmhpFragmentationPackage;
}


static void multipkg_init(void *params, uint8_t *dataBuffer,
    uint8_t dataBufferMaxSize)
{
    __log_info("initialized");

    s_multipkg_state.data_buf = dataBuffer;
    s_multipkg_state.data_buf_max_size = dataBufferMaxSize;
    s_multipkg_state.is_tx_pending = false;
    s_multipkg_state.is_initialized = true;
}

static bool multipkg_is_initialized( void )
{
    return s_is_initialized;
}

static bool multipkg_is_tx_pending( void )
{
    return s_multipkg_state.is_tx_pending;
}

static void multipkg_process( void )
{
    __log_info("process");
}

static void multipkg_on_mcps_confirm_process( McpsConfirm_t *mcpsConfirm )
{
    __log_info("on MCPS Confirm Process");
}

static void multipkg_on_mcps_indication_process(
    McpsIndication_t *mcpsIndication )
{
    __log_info("on MCPPS Indication Process");
    if( !s_multipkg_state.is_initialized ) {
        return;
    }

    if( mcpsIndication->Port != MULTI_PACKAGE_PORT ) {
        return;
    }
}

/** -------------------------------------------------------------------------- *
 * Package decoder
 * --------------------------------------------------------------------------- *
 */


/* --- end of file ---------------------------------------------------------- */
