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
 * @brief   This file is a cross interface between the bootloader and the main
 *          firmware application.
 * --------------------------------------------------------------------------- *
 */
#ifndef __BOOT_IF_H__
#define __BOOT_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * @details
 * § The interaction between the firmware and the bootloader can be done using
 *   the methods declared in this file.
 * 
 * § The system boots in two modes:
 *   - normal mode   : the normal system running mode.
 *   - safeboot mode : a special mode dedicated for system diagnosis purposes.
 *     In micropython builds, any start-up python scripts (such as boot.py and
 *     main.py) will be skipped during system start-up.
 * 
 * § In safeboot mode the user can select temporarily a different firmware image
 *   to boot from. The selection can be done from either of the following images
 * 
 *   | Firmware-Image   | Hold-Time Config(*)                     | default |
 *   | :--------------: | :-------------------------------------- | :-----: |
 *   | latest firmware  | SAFEBOOT_HOLD_TIME_FOR_LATEST_FW        |    0    |
 *   | previous OTA     | SAFEBOOT_HOLD_TIME_FOR_PREV_OTA         |    3    |
 *   | factory firmware | SAFEBOOT_HOLD_TIME_FOR_FACTORY_FIRMWARE |    7    |
 *   (*) shall be defined to the sdkconfig.h
 *  
 * § The following sequence diagram describe the different use cases and system
 *   expected respons in each use-case
 *   @startuml
 *   !include boot_if.puml
 *   @enduml
 * --------------------------------------------------------------------------- *
 */

/* --- typedefs ------------------------------------------------------------- */

/**
 * @typedef bootif_state_t
 * @brief   the system boot state during startup
 */
typedef enum {
    __BOOTIF_STATE_NORMAL_MODE,   /**< system boots in normal mode */
    __BOOTIF_STATE_SAFEBOOT_MODE, /**< system boots in safeboot mode */
} bootif_state_t;

/* --- APIs Declaration ----------------------------------------------------- */

/**
 * @brief   this function sets the current boot state of the system
 *          it is used as a ping-pong value between the bootloader and the
 *          firmware in the following way:
 *          
 *          If it set by the firmware, the boot loader will start the safeboot
 *          mode automatically.
 * 
 *          If it is set by the firmare, it means the safeboot button is pressed
 *          and according to the hold-time the intended image will be loaded.
 * 
 *          the responsibility to reset the state to normal, is for the firmware
 *          after reading it, every time for both triggering modes whether
 *          by software reset of by the hardware safeboot button.
 * 
 * @param   state the current start-up boot state.
 */
void bootif_state_set(bootif_state_t state);

/**
 * @brief   this function gets the current boot state of the system, it is
 *          usually called from the application to know whether the system boots
 *          in normal or safeboot to take the proper action
 * 
 * @return  the current system startup booting state.
 *          __BOOTIF_STATE_NORMAL_MODE    normal boot mode
 *          __BOOTIF_STATE_SAFEBOOT_MODE  safe boot mode
 */
bootif_state_t bootif_state_get(void);

/**
 * @brief   this function hard resets the system in safeboot state.
 *          it is called from the application side to make a software safeboot.
 */
#ifndef BOOTLOADER_BUILD
void bootif_safeboot_soft_reset(void);
#endif

/**
 * @brief   init the software reset mechanism.
 */
#ifndef BOOTLOADER_BUILD
void bootif_safeboot_soft_reset_init(void);
#endif

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __BOOT_IF_H__ */
