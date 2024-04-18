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
 * @brief   System info display
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include "esp_flash.h"
#include "spiram.h"
#include "esp_heap_caps.h"
#include "esp_partition.h"

#include "log_lib.h"
#include "utils_units.h"
#include "fw_version.h"
#include "efuse_if.h"

#ifdef MICROPYTHON_BUILD
#include "genhdr/mpversion.h"
#include "mpconfigboard.h"
#endif

/* --- macros --------------------------------------------------------------- */

#define __total_w   80
#define __name_w    30

/* --- APIs ----------------------------------------------------------------- */

void sysinfo_board(void)
{
    __log_output_header(" board info ", __total_w, '=');

    __log_output_field(" - board full name", __name_w, ' ', __left__, false);
    __log_output(__cyan__ SDK_BOARD __default__"\n");

    __log_output_field(" - platform", __name_w, ' ', __left__, false);
    __log_output(__cyan__ SDK_PLATFORM __default__"\n");
    __log_output_field(" - board name", __name_w, ' ', __left__, false);
    __log_output(__cyan__ SDK_BOARD_NAME __default__"\n");
    __log_output_field(" - board number", __name_w, ' ', __left__, false);
    __log_output(__cyan__ SDK_BOARD_NUMBER __default__"\n");
    __log_output_field(" - board shield", __name_w, ' ', __left__, false);
    __log_output(__cyan__ SDK_BOARD_SHIELD __default__"\n");

    #ifdef MICROPYTHON_BUILD
    __log_output("\n");
    __log_output_field(" - micropython board name", __name_w, ' ',
        __left__, false);
    __log_output(__cyan__ MICROPY_HW_BOARD_NAME __default__"\n");
    __log_output_field(" - micropython MCU name", __name_w, ' ',
        __left__, false);
    __log_output(__cyan__ MICROPY_HW_MCU_NAME __default__"\n");
    __log_output_field(" - micropython system name", __name_w, ' ',
        __left__, false);
    __log_output(__cyan__ MICROPY_PY_SYS_PLATFORM __default__"\n");
    #endif /* MICROPYTHON_BUILD */
}

void sysinfo_version(void)
{
    __log_output_header(" firmware version ", __total_w, '=');

    __log_output_field(" - firmware version", __name_w, ' ', __left__, false);
    __log_output(__cyan__"%s"__default__"\n", fw_version_string());

    __log_output_field(" - firmware base release",
        __name_w, ' ', __left__, false);
    __log_output(__cyan__"%s\n"__default__, fw_version_release_str());
    __log_output_field(" - custom version string",
        __name_w, ' ', __left__, false);
    __log_output(__cyan__"%s\n"__default__, fw_version_custom_str());

    __log_output("\n");
    __log_output_field(" - build date and time",
        __name_w, ' ', __left__, false);
    __log_output(__cyan__"%s - %s\n"__default__,
        fw_version_date_str(), fw_version_time_str());

    __log_output("\n");
    __log_output_field(" - git hash long", __name_w, ' ', __left__, false);
    __log_output(__cyan__"%s\n"__default__, fw_version_git_tag_full_str());
    __log_output_field(" - git hash short", __name_w, ' ', __left__, false);
    __log_output(__cyan__"%s\n"__default__, fw_version_git_tag_short_str());
    __log_output_field(" - git delta", __name_w, ' ', __left__, false);
    __log_output(__cyan__"%d\n"__default__, fw_version_git_delta());

    #ifdef MICROPYTHON_BUILD
    __log_output("\n");

    __log_output_field(" - micropython build", __name_w, ' ', __left__, false);
    __log_output(__cyan__ MICROPY_GIT_TAG __default__"\n");

    __log_output_field(" - micropython build date", __name_w, ' ',
        __left__, false);
    __log_output(__cyan__ MICROPY_BUILD_DATE __default__"\n");
    #endif /* MICROPYTHON_BUILD */

    __log_output_fill(__total_w, '=', true);
}

static const char* flash_get_part_type_name(esp_partition_type_t type)
{
    switch(type)
    {
    case ESP_PARTITION_TYPE_APP:    return "app";
    case ESP_PARTITION_TYPE_DATA:   return "data";
    case ESP_PARTITION_TYPE_ANY:    return "any";
    }

    return __red__"unknown"__default__;
}

static const char* flash_get_part_subtype_name(esp_partition_subtype_t type)
{
    switch(type)
    {
    case ESP_PARTITION_SUBTYPE_APP_FACTORY:     return "factory";
    case ESP_PARTITION_SUBTYPE_APP_OTA_MIN
         ...ESP_PARTITION_SUBTYPE_APP_OTA_MAX:  return "ota";
    case ESP_PARTITION_SUBTYPE_DATA_PHY:        return "phy";
    case ESP_PARTITION_SUBTYPE_DATA_NVS:        return "nvs";
    case ESP_PARTITION_SUBTYPE_DATA_COREDUMP:   return "coredump";
    case ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS:   return "nvs_keys";
    case ESP_PARTITION_SUBTYPE_DATA_EFUSE_EM:   return "efuse_em";
    case ESP_PARTITION_SUBTYPE_DATA_UNDEFINED:  return "data-undef";
    case ESP_PARTITION_SUBTYPE_DATA_ESPHTTPD:   return "esp-httpd";
    case ESP_PARTITION_SUBTYPE_DATA_FAT:        return "fat";
    case ESP_PARTITION_SUBTYPE_DATA_SPIFFS:     return "spi-ffs";
    case ESP_PARTITION_SUBTYPE_ANY:             return "any";
    }

    return __red__"unknown"__default__;
}


void sysinfo_flash_stats(void)
{
    __log_output_header(" flash stats ", __total_w, '=');

    __log_output_field(" - flash frequency",__name_w, ' ', __left__, false);
    int flash_freq = 0;
    #if CONFIG_ESPTOOLPY_FLASHFREQ_20M
    flash_freq = 20;
    #elif CONFIG_ESPTOOLPY_FLASHFREQ_40M
    flash_freq = 40;
    #elif CONFIG_ESPTOOLPY_FLASHFREQ_80M
    flash_freq = 80;
    #elif CONFIG_ESPTOOLPY_FLASHFREQ_120M
    flash_freq = 120;
    #endif
    __log_output(__yellow__"%d"__default__" MHz\n", flash_freq);

    __log_output_field(" - flash size", __name_w, ' ', __left__, false);
    
    __log_output(__yellow__"%d "__default__" Bytes"
                 " ~= "__yellow__"%d"__default__" MB\n",
        esp_flash_default_chip->size,
        __bytes_to_mb(esp_flash_default_chip->size));
    
    __log_output(" - partition table:\n");

    #define __part_table_w  (__total_w - 2*4)

    #define __w_label           10
    #define __w_type             6
    #define __w_subtype         12
    #define __w_enc              4
    #define __w_start           10
    #define __w_end             10
    #define __w_size_b          10
    #define __w_size_kb          8
    #define __w_size_mb          8

    __log_output("    ");
    __log_col_header_l(label);
    __log_col_header_l(type);
    __log_col_header_l(subtype);
    __log_col_header_l(enc);
    __log_col_header_c(start);
    __log_col_header_c(end);
    __log_col_header_c(size_b);
    __log_col_header_c(size_kb);
    __log_col_header_c(size_mb);
    __log_output("\n");

    esp_partition_iterator_t it = esp_partition_find(
        ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

    do {
        const esp_partition_t* part = esp_partition_get(it);
        __log_output("    ");
        __log_col_str_val_color_l(label, __cyan__, part->label);

        __log_col_str_val_l(type, flash_get_part_type_name(part->type));
        __log_col_str_val_l(subtype, flash_get_part_subtype_name(part->subtype));
        __log_col_str_val_l(enc, g_yes_no[part->encrypted]);

        __log_col_hex8_val(start, part->address);
        __log_col_hex8_val(end, part->address + part->size - 1);

        __log_col_int_val(size_b, 8, part->size);

        __log_col_float_val(size_kb, 7, 1, (float)(part->size)/1024.0f);
        __log_col_float_val(size_mb, 5, 1, (float)(part->size)/1024.0f/1024.0f);

        __log_output("\n");

        it = esp_partition_next(it);
    } while(it);

    __log_output_fill(__total_w, '=', true);
}

void sysinfo_spiram_stats(void)
{
    __log_output_header(" spiram stats ", __total_w, '=');

    uint32_t ram_size = esp_spiram_get_size();

    __log_output_field(" - ram size", __name_w, ' ', __left__, false);
    __log_output(
            __yellow__"%d "__default__"Bytes"
            " ~= "__yellow__"%d "__default__"MB\n",
            ram_size,
            __bytes_to_mb(ram_size)
            );

    __log_output_fill(__total_w, '=', true);
}

void sysinfo_efuses(void)
{
    __log_output_header(" efuses for user data ", __total_w, '=');

    efuse_layout_version_t ver;
    efuse_if_read_layout_version(&ver);
    __log_output_field(" - Layout Version", __name_w, ' ', __left__, false);
    __log_output_hex_lower(&ver, sizeof(ver));
    __log_output("\n");

    efuse_lora_mac_t lora_mac;
    efuse_if_read_lora_mac(&lora_mac);
    __log_output_field(" - LoRa MAC", __name_w, ' ', __left__, false);
    __log_output_hex_lower(&lora_mac, sizeof(lora_mac));
    __log_output("\n");

    efuse_serial_number_t serial_number;
    efuse_if_read_serial_number(&serial_number);
    __log_output_field(" - Serial Number", __name_w, ' ', __left__, false);
    __log_output_hex_lower(&serial_number, sizeof(serial_number));
    __log_output("\n");

    efuse_hw_id_t hw_id;
    efuse_if_read_hw_id(&hw_id);
    __log_output_field(" - HW ID", __name_w, ' ', __left__, false);
    __log_output_hex_lower(&hw_id, sizeof(hw_id));
    __log_output("\n");

    efuse_project_id_t project_id;
    efuse_if_read_project_id(&project_id);
    __log_output_field(" - Project ID", __name_w, ' ', __left__, false);
    __log_output_hex_lower(&project_id, sizeof(project_id));
    __log_output("\n");

    efuse_wifi_mac_t wifi_mac;
    efuse_if_read_wifi_mac(&wifi_mac);
    __log_output_field(" - WiFi MAC", __name_w, ' ', __left__, false);
    __log_output_hex_lower(&wifi_mac, sizeof(wifi_mac));
    __log_output("\n");

    __log_output_fill(__total_w, '=', true);
}

/* --- end of file ---------------------------------------------------------- */
