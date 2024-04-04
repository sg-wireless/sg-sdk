/** -------------------------------------------------------------------------- *
 * @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
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
 * @brief   uart interface sub-component header
 * --------------------------------------------------------------------------- *
 */
#ifndef __LTE_UART_H__
#define __LTE_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * includes
 * --------------------------------------------------------------------------- *
 */
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @brief   initialises the uart driver for the LTE GM02s chip
 * 
 * @param   baudrate intended baudrate
 */
void lte_uart_init(uint32_t baudrate);

/**
 * @brief   de-initialises the uart driver for the LTE GM02s chip
 */
void lte_uart_deinit(void);

/**
 * @brief   read incoming data over the uart
 * 
 * @param   buf a memory buffer to fill in the read data
 * @param   len the maximum length of the buffer, to not exceed it while reading
 * @param   timeout_ms required timeout to wait until the required read data
 *              is available. Zero no wait time, then just pick up the required
 *              data length if immediately available in the received buffer.
 * 
 * @return  the actual read data length.
 *          if some error occur inside or the interface is not initialized,
 *              Zero will be returned.
 */
int  lte_uart_read(uint8_t*buf, uint32_t len, uint32_t timeout_ms);

/**
 * @brief   write data over the uart
 * 
 * @param   buf a memory buffer carrying the data to be transmitted over uart
 * @param   len the data length in the given buffer
 * 
 * @return  the actual written data length.
 *          if some error occur inside or the interface is not initialized,
 *              Zero will be returned.
 */
int  lte_uart_write(const uint8_t*buf, uint32_t len);

/**
 * @brief   get the current buffered rx data length
 * 
 * @return  size of the current buffered rx data
 */
int  lte_uart_any(void);

int lte_uart_flush(void);

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LTE_UART_H__ */
