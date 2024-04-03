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
 * @brief   This file implements the a cross interface between the bootloader
 *          and the main firmware application.
 * --------------------------------------------------------------------------- *
 */

/* --- includes ------------------------------------------------------------- */

#include "sdkconfig.h"
#include "boot_if.h"
#ifdef CONFIG_SAFEBOOT_FEATURE_ENABLE
#include "esp_err.h"
#include "bootloader_common.h"

#ifdef BOOTLOADER_BUILD
    extern const char* g_safeboot_tag;
    #include "esp_log.h"
    #define __bootif_log(fmt, args...) ESP_LOGI(g_safeboot_tag, fmt, args)
#else
    #include "esp_event.h"  // needed for safeboot soft reset mechanism
    #include "esp_system.h" // for actual esp32 reset function
    #define __log_component bootif
    #include "log_lib.h"
    __log_component_def(default, bootif, default, 1, 0)
    #define __bootif_log(fmt, args...) __log_info(fmt, args)
#endif

/* --- local methods declarations ------------------------------------------- */

#ifdef CONFIG_BOOTLOADER_CUSTOM_RESERVE_RTC
static const char* bootif_state_str(bootif_state_t state);
#endif

/* --- APIs Definitions ----------------------------------------------------- */

void bootif_state_set(bootif_state_t state)
{
    #ifdef CONFIG_BOOTLOADER_CUSTOM_RESERVE_RTC

    bootloader_common_update_rtc_retain_mem(NULL, true);

    rtc_retain_mem_t* mem = bootloader_common_get_rtc_retain_mem();
    uint32_t* _rtc_vars = (uint32_t*) mem->custom;
    _rtc_vars[0] = (uint32_t)state;
    __bootif_log("set rtc mem bootloader state: [%08x] %d: %s\n",
        (uint32_t)_rtc_vars, state, bootif_state_str(state));

    #endif
}

bootif_state_t bootif_state_get(void)
{
    bootif_state_t state = __BOOTIF_STATE_NORMAL_MODE;

    #ifdef CONFIG_BOOTLOADER_CUSTOM_RESERVE_RTC

    extern rtc_retain_mem_t* bootloader_common_get_rtc_retain_mem(void);        
    rtc_retain_mem_t* mem = bootloader_common_get_rtc_retain_mem();
    uint32_t* _rtc_vars = (uint32_t*) mem->custom;
    state = _rtc_vars[0];
    __bootif_log("get rtc mem bootloader state: [%08x] %d: %s\n",
        (uint32_t)_rtc_vars, state, bootif_state_str(state));

    #endif

    return state;
}

#ifndef BOOTLOADER_BUILD

#ifdef CONFIG_SAFEBOOT_ENABLE_SOFT_RESET
static const char* s_bootif_event_base = "bootif";
static bool s_soft_reset_init = false;

#define __bootif_event_reset_in_safeboot   (0)

static void bootif_soft_reset_handler(
    void* event_handler_arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data)
{
    __log_info("execute software safeboot reset handler");
    esp_restart();
}
#endif /* CONFIG_SAFEBOOT_ENABLE_SOFT_RESET */

void bootif_safeboot_soft_reset_init(void)
{
    #ifdef CONFIG_SAFEBOOT_ENABLE_SOFT_RESET
    if( !s_soft_reset_init )
    {
        __log_info("register safeboot soft reset handler");
        esp_err_t err = esp_event_handler_register(
            s_bootif_event_base,
            __bootif_event_reset_in_safeboot,
            bootif_soft_reset_handler,
            NULL);
        if(err != ESP_OK )
        {
            __log_error("failed to register safeboot soft reset handler:"
                "err-code: %d (0x%x)", err, err);
        }
        s_soft_reset_init = true;
    }
    #endif
}

void bootif_safeboot_soft_reset(void)
{
    #ifdef CONFIG_SAFEBOOT_ENABLE_SOFT_RESET
    if( !s_soft_reset_init )
    {
        __log_error("safeboot soft reset not initialized");
        return;
    }

    bootif_state_set(__BOOTIF_STATE_SAFEBOOT_MODE);

    esp_event_isr_post(s_bootif_event_base,
        __bootif_event_reset_in_safeboot, NULL, 0, NULL);
    #endif
}

#endif /* #ifndef BOOTLOADER_BUILD */

#ifdef CONFIG_BOOTLOADER_CUSTOM_RESERVE_RTC
static const char* bootif_state_str(bootif_state_t state)
{
    switch(state)
    {
    case __BOOTIF_STATE_NORMAL_MODE:
        return "normal-mode";
    case __BOOTIF_STATE_SAFEBOOT_MODE:
        return "safeboot-mode";
    }
    return "unknown-mode";
}
#endif

#else /* CONFIG_SAFEBOOT_FEATURE_ENABLE */

void bootif_state_set(bootif_state_t state)
{
    (void)state;
}

bootif_state_t bootif_state_get(void)
{
    return __BOOTIF_STATE_NORMAL_MODE;
}

#endif /* CONFIG_SAFEBOOT_FEATURE_ENABLE */

/* -- end of file ----------------------------------------------------------- */
