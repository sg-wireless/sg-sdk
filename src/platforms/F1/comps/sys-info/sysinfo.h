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
 * @brief   This file represents the system information interface
 * --------------------------------------------------------------------------- *
 */

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -- APIs ------------------------------------------------------------------ */

/**
 * @brief   it shows the firmware board info.
 */
void sysinfo_board(void);

/**
 * @brief   it shows the firmware version and version string that are provided
 *          in the build command using these definitions
 *              FW_VERSION=<fw-version>
 *              FW_VERSION_STRING=<fw-version-string>
 */
void sysinfo_version(void);

/**
 * @brief   it shows the current flash configurations such as SPI frequency
 *          and size
 */
void sysinfo_flash_stats(void);

/**
 * @brief   it shows the SPI RAM specs and info
 */
void sysinfo_spiram_stats(void);

void sysinfo_efuses(void);

/* -- end of file ----------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __SYSINFO_H__ */
