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
 * --------------------------------------------------------------------------- *
 * Copyright (c) 2022, Pycom Limited.
 *
 * This software is licensed under the GNU GPL version 3 or any
 * later version, with permitted additional terms. For more information
 * see the Pycom Licence v1.0 document supplied with this file, or
 * available at https://www.pycom.io/opensource/licensing
 * 
 * @author  (Pycom)
 * 
 * @brief   This file represents a uPython c-module interface to the exported
 *          APIs of system information.
 * --------------------------------------------------------------------------- *
 */


#include "esp_sleep.h"
#include "esp_attr.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

#include "mp_lite_if.h"

#include "log_lib.h"

/* --- module functions definitions ----------------------------------------- */

static esp_ota_handle_t esp_ota_handle;

__mp_mod_name(fuota, FUOTA);

__mp_mod_fun_0(fuota, info)(void)
{
	const esp_partition_t *partition;
	partition = esp_ota_get_next_update_partition(NULL);
	__log_output("Next update partition: %s\n", partition->label);
	partition = esp_ota_get_running_partition();
	__log_output("Running partition: %s\n", partition->label);
	partition = esp_ota_get_boot_partition();
	__log_output("Boot partition: %s\n", partition->label);
	return mp_const_none;
}

static void handle_exceptions(esp_err_t result)
{
    const char* err_msg = NULL;

    if(ESP_ERR_NO_MEM == result)
	{
		err_msg = "Cannot allocate memory for OTA operation.";
	}
	else if(ESP_ERR_OTA_PARTITION_CONFLICT == result)
	{
		err_msg = "Partition holds the currently running firmware, "
                    "cannot update in place.";
	}
	else if(ESP_ERR_NOT_FOUND == result)
	{
		err_msg = "Requested resource not found.";
	}
	else if(ESP_ERR_INVALID_SIZE == result)
	{
		err_msg = "Partition doesn't fit in configured flash size.";
	}
	else if(ESP_ERR_FLASH_OP_TIMEOUT == result)
	{
		err_msg = "Flash write failed.";
	}
	else if(ESP_ERR_FLASH_OP_FAIL == result)
	{
		err_msg = "Flash write failed.";
	}
	else if(ESP_ERR_OTA_ROLLBACK_INVALID_STATE == result)
	{
		err_msg = "Before performing an update, the application must be valid.";
	}
	else if (ESP_ERR_OTA_VALIDATE_FAILED == result)
	{
		err_msg = "First byte of image contains invalid app image magic byte."
                "or OTA image is invalid.";
	}
	else if (ESP_ERR_OTA_SELECT_INFO_INVALID == result)
	{
		err_msg = "OTA data partition has invalid contents.";
	}
	else if (ESP_ERR_INVALID_STATE == result)
	{
		err_msg = "Flash write failed.";
	}
	else if (ESP_ERR_OTA_ROLLBACK_FAILED == result)
	{
		err_msg = "The rollback is not possible "
                  "due to flash does not have any apps.";
	}
    else if(result != ESP_OK)
    {
        err_msg = "the requested operation failed";
    }

    if(err_msg) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT(err_msg));
    }
}

__mp_mod_fun_0(fuota, start)(void)
{
    const esp_partition_t *partition_new;
	const esp_partition_t *partition_now;
	esp_err_t result;
	partition_new = esp_ota_get_next_update_partition(NULL);
	partition_now = esp_ota_get_running_partition();
	result = esp_ota_begin(partition_new, 0, &esp_ota_handle);

	__log_output("OTA esp_ota_begin returned %d\n", result);
	__log_output("Writing to partition: %s\n", partition_new->label);
	__log_output("Current boot partition: %s\n", partition_now->label);

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    return mp_const_none;
}

__mp_mod_fun_1(fuota, write)(mp_obj_t data)
{
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(data, &bufinfo, MP_BUFFER_READ);
    esp_err_t result;
    result = esp_ota_write(esp_ota_handle, bufinfo.buf, bufinfo.len);

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    return mp_const_none;
}

__mp_mod_fun_0(fuota, finish)(void)
{
	const esp_partition_t *partition_new;
	const esp_partition_t *partition_now;
	esp_err_t result;
	partition_new = esp_ota_get_boot_partition();
	partition_now = esp_ota_get_running_partition();
	result = esp_ota_begin(partition_new, 0, &esp_ota_handle);
	result = esp_ota_end(esp_ota_handle);
	__log_output("OTA esp_ota_end returned %d\n", result);

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    partition_new = esp_ota_get_next_update_partition(NULL);
	result = esp_ota_set_boot_partition(partition_new);
	__log_output("OTA esp_ota_set_boot_partition returned %d\n", result);
	__log_output("NEXT boot from partition: %s\n", partition_new->label);
	__log_output("Current boot partition: %s\n", partition_now->label);
	if (ESP_OK != result) {
        mp_raise_msg(&mp_type_OSError,
            MP_ERROR_TEXT("Error setting new ota partition"));
	}

    return mp_const_none;
}

__mp_mod_fun_0(fuota, rollback)(void)
{
	esp_err_t result;
	result = esp_ota_mark_app_invalid_rollback_and_reboot();

    if(result != ESP_OK)
    {
        handle_exceptions(result);
    }

    return mp_const_none;
}

__mp_mod_fun_0(fuota, valid)(void)
{
	esp_err_t result;
	result = esp_ota_mark_app_valid_cancel_rollback();

	if (ESP_OK != result)
    {
        handle_exceptions(result);
    }

	return mp_const_none;
}

/* --- end of file ---------------------------------------------------------- */
