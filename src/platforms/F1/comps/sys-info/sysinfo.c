/** -------------------------------------------------------------------------- *
 * Copyright (c) 2023-2026 SG Wireless - All Rights Reserved
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
 * @maintainer  Christian Ehlers (SG Wireless)
 * 
 * @brief   System info display
 * --------------------------------------------------------------------------- *
 */

/* --- include -------------------------------------------------------------- */

#include "esp_flash.h"
#include "esp_psram.h"
#include "esp_heap_caps.h"
#include "esp_partition.h"

#include "log_lib.h"
#include "utils_units.h"
#include "fw_version.h"
#include "efuse_if.h"

#ifdef MICROPYTHON_BUILD
#include "genhdr/mpversion.h"
#include "mpconfigboard.h"
#include "mpconfigport.h"
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
    case ESP_PARTITION_TYPE_BOOTLOADER: return "bootloader";
    case ESP_PARTITION_TYPE_PARTITION_TABLE: return "partition_table";
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
    case ESP_PARTITION_SUBTYPE_DATA_LITTLEFS:   return "littlefs";
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

    uint32_t ram_size = esp_psram_get_size();

    __log_output_field(" - ram size", __name_w, ' ', __left__, false);
    __log_output(
            __yellow__"%d "__default__"Bytes"
            " ~= "__yellow__"%d "__default__"MB\n",
            ram_size,
            __bytes_to_mb(ram_size)
            );

    __log_output_fill(__total_w, '=', true);
}

void sysinfo_memory_stats(void)
{
    __log_output_header(" comprehensive memory stats ", __total_w, '=');

    // Helper function to format memory sizes
    #define __format_memory(name, size) do { \
        __log_output_field(" - " name, __name_w, ' ', __left__, false); \
        __log_output(__yellow__"%d "__default__"bytes " \
                    "~= "__yellow__"%.1f "__default__"KB " \
                    "~= "__yellow__"%.1f "__default__"MB\n", \
                    (size), (size)/1024.0f, (size)/(1024.0f*1024.0f)); \
    } while(0)

    // 1. Total heap memory (all types)
    size_t total_heap = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    size_t used_heap = total_heap - free_heap;
    
    __format_memory("Total Heap", total_heap);
    __format_memory("Used Heap", used_heap);
    __format_memory("Free Heap", free_heap);

    __log_output("\n");

    // 2. Internal RAM (DRAM/IRAM) - critical for DMA, ISRs, etc.
    size_t total_internal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t used_internal = total_internal - free_internal;
    
    __format_memory("Total Internal RAM", total_internal);
    __format_memory("Used Internal RAM", used_internal);
    __format_memory("Free Internal RAM", free_internal);

    __log_output("\n");

    // 3. External RAM (PSRAM) - for application data
    size_t total_spiram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_spiram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t used_spiram = total_spiram - free_spiram;
    
    __format_memory("Total PSRAM", total_spiram);
    __format_memory("Used PSRAM", used_spiram);
    __format_memory("Free PSRAM", free_spiram);

    __log_output("\n");

    // 4. DMA-capable memory (subset of internal RAM)
    size_t total_dma = heap_caps_get_total_size(MALLOC_CAP_DMA);
    size_t free_dma = heap_caps_get_free_size(MALLOC_CAP_DMA);
    size_t used_dma = total_dma - free_dma;
    
    __format_memory("Total DMA-capable", total_dma);
    __format_memory("Used DMA-capable", used_dma);
    __format_memory("Free DMA-capable", free_dma);

    __log_output("\n");

    // 5. 32-byte aligned memory (some hardware requires this)
    size_t total_32bit = heap_caps_get_total_size(MALLOC_CAP_32BIT);
    size_t free_32bit = heap_caps_get_free_size(MALLOC_CAP_32BIT);
    size_t used_32bit = total_32bit - free_32bit;
    
    __format_memory("Total 32-bit aligned", total_32bit);
    __format_memory("Used 32-bit aligned", used_32bit);
    __format_memory("Free 32-bit aligned", free_32bit);

    __log_output("\n");

    // 6. Largest free block sizes (fragmentation indicator)
    size_t largest_free_heap = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    size_t largest_free_internal = heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL);
    size_t largest_free_spiram = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    size_t largest_free_dma = heap_caps_get_largest_free_block(MALLOC_CAP_DMA);
    
    __format_memory("Largest free block (Heap)", largest_free_heap);
    __format_memory("Largest free block (Internal)", largest_free_internal);
    __format_memory("Largest free block (PSRAM)", largest_free_spiram);
    __format_memory("Largest free block (DMA)", largest_free_dma);

    __log_output("\n");

    // 7. Memory pressure indicators
    float internal_usage = (used_internal * 100.0f) / total_internal;
    float spiram_usage = total_spiram > 0 ? (used_spiram * 100.0f) / total_spiram : 0.0f;
    float dma_usage = (used_dma * 100.0f) / total_dma;

    __log_output_field(" - Internal RAM usage", __name_w, ' ', __left__, false);
    __log_output(__yellow__"%.1f%%" __default__ "\n", internal_usage);
    
    __log_output_field(" - PSRAM usage", __name_w, ' ', __left__, false);
    __log_output(__yellow__"%.1f%%" __default__ "\n", spiram_usage);
    
    __log_output_field(" - DMA RAM usage", __name_w, ' ', __left__, false);
    __log_output(__yellow__"%.1f%%" __default__ "\n", dma_usage);

    // 8. Critical warnings
    __log_output("\n");
    if (free_internal < 20480) {  // Less than 20KB internal RAM
        __log_output(__red__" WARNING: Very low internal RAM! System may be unstable\n" __default__);
    }
    if (free_dma < 10240) {  // Less than 10KB DMA RAM
        __log_output(__red__" WARNING: Low DMA-capable RAM! May cause UART/SPI issues\n" __default__);
    }
    if (largest_free_internal < 8192) {  // Largest block < 8KB
        __log_output(__red__" WARNING: Internal RAM fragmentation detected\n" __default__);
    }

    #undef __format_memory
    __log_output_fill(__total_w, '=', true);
}

void sysinfo_efuses(void)
{
    __log_output_header(" efuses for user data ", __total_w, '=');

    {
        efuse_layout_version_t ver;
        efuse_if_read_layout_version(&ver);
        __log_output_field(" - Layout Version", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&ver, sizeof(ver));
        __log_output("\n");
    }

    {
        efuse_serial_number_t serial_number;
        efuse_if_read_serial_number(&serial_number);
        __log_output_field(" - Serial Number", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&serial_number, sizeof(serial_number));
        __log_output("\n");
    }

    {
        efuse_hw_id_t hw_id;
        efuse_if_read_hw_id(&hw_id);
        __log_output_field(" - HW ID", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&hw_id, sizeof(hw_id));
        __log_output("\n");
    }

    {
        efuse_project_id_t project_id;
        efuse_if_read_project_id(&project_id);
        __log_output_field(" - Project ID", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&project_id, sizeof(project_id));
        __log_output("\n");
    }

    {
        efuse_wifi_mac_t wifi_mac;
        efuse_if_read_wifi_mac(&wifi_mac);
        __log_output_field(" - WiFi MAC", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&wifi_mac, sizeof(wifi_mac));
        __log_output("\n");
    }

    #ifdef __feature_lora
    {
        efuse_lora_mac_t lora_mac;
        efuse_if_read_lora_mac(&lora_mac);
        __log_output_field(" - LoRa DevEUI", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&lora_mac, sizeof(lora_mac));
        __log_output("\n");
    }
    #endif

    #ifdef __efuse_lora_keys_enable
    {
        efuse_lora_app_key_t lora_app_key;
        efuse_if_read_lora_app_key(&lora_app_key);
        __log_output_field(" - LoRa AppKey", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&lora_app_key, sizeof(lora_app_key));
        __log_output("\n");
    }
    {
        efuse_lora_nwk_key_t lora_nwk_key;
        efuse_if_read_lora_nwk_key(&lora_nwk_key);
        __log_output_field(" - LoRa NwkKey", __name_w, ' ', __left__, false);
        __log_output_hex_lower(&lora_nwk_key, sizeof(lora_nwk_key));
        __log_output("\n");
    }
    #endif

    __log_output_fill(__total_w, '=', true);
}

/* --- end of file ---------------------------------------------------------- */
