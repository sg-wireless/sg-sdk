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
 * @brief   lora nvm handling interface
 * --------------------------------------------------------------------------- *
 */
#ifndef __LORA_NVM_H__
#define __LORA_NVM_H__

#ifdef __cplusplus
extern "C" {
#endif

/** -------------------------------------------------------------------------- *
 * include
 * --------------------------------------------------------------------------- *
 */
#include <stdbool.h>
#include <stdint.h>

/** -------------------------------------------------------------------------- *
 * typedefs
 * --------------------------------------------------------------------------- *
 */
typedef void lora_nvm_load_defaults_t( void* p_record_mem, uint32_t size );

/**
 * a tail structure that should be placed at the end of any data structure
 * intended to be saved using this nvm handling mechanism and shall be used only
 * by the handling mechanism.
 * 
 * Example:
 *      in a component ABC, the following data type carry data to be stored in
 *      nvm memory:
 * 
 *      // in abc.c
 * 
 *      // define the nvm data data-type
 *      typedef struct{
 *          // ABC specific data definition ..
 *          // ABC specific data definition ..
 * 
 *          lora_nvm_record_tail_t tail; // used by the nvm handler only
 *      } abc_nvm_data_t;
 * 
 *      // define the nvm data object
 *      static abc_nvm_data_t s_abc_nvm_data;
 * 
 *      // an optional lead defaults function can be fed to the nvm handler
 *      // to be called if the data is corrupt or if it is first time storing.
 *      static void abc_load_default_fun( void* p_record_mem, uint32_t size )
 *      {
 *          abc_nvm_data_t* p_nvm_data = p_record_mem;
 *          // .. load default values for all data
 *          // .. and don't touch the tail member.
 *      }
 * 
 *      // at any point of time, the nvm handler can be called to store the data
 *      {
 *          // ..
 *          lora_nvm_handle_change(
 *              "abc-nvm",
 *              &s_abc_nvm_data,
 *              sizeof(s_abc_nvm_data),
 *              abc_load_default_fun
 *              );
 *          // ..
 *      }
 * 
 *      // the nvm handler will perform the below mechanism and store a clone of
 *      // the most recent data safely.
 */
typedef struct {
    uint32_t    magic;
    uint32_t    crc32;
} lora_nvm_record_tail_t;

/** -------------------------------------------------------------------------- *
 * APIs
 * --------------------------------------------------------------------------- *
 */

/**
 * @details It performs the required nvm handling for a data record in the
 *          following procedure:
 *              Do these checks:
 *                  CH-A these is no previous clone for this record in the NvM
 *                  CH-B if it has a wrong loading magic number
 *                  CH-C if it has a correct crc32
 *              IF CH-A failed:
 *                  [ LABEL-1 ]
 *                  - WRITE the correct magic number
 *                  - CALL the load_default_method() if specified
 *                  - CALC the new CRC32 value and write it
 *                  - STORE the record
 *              ELSE IF CH-B failed:
 *                  - LOAD the clone from the NvM into the record memory
 *                  - Do CH-B and CH-C again
 *                  IF CH-B failed || CH-C failed:
 *                      GOTO [ LABEL-1 ]
 *              ELSE IF CH-C failed:
 *                  - CALC the CRC32 value and write it in the record memory
 *                  - STORE a record clone into the NvM
 *              ELSE
 *                  - Do nothing as no change
 *              END
 * 
 * @param   key_name a unique name for the record
 * @param   p_record_mem a reference to the record memory
 * @param   load_default_method an optional calllback method to restore the
 *              default values of the record before saving.
 */
void lora_nvm_handle_change(
    const char* key_name,
    void*       p_record_mem,
    uint32_t    record_size,
    lora_nvm_load_defaults_t* load_default_method
    );

/* --- end of file ---------------------------------------------------------- */
#ifdef __cplusplus
}
#endif
#endif /* __LORA_NVM_H__ */
