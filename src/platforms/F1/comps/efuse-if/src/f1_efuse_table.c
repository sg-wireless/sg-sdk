/*
 * SPDX-FileCopyrightText: 2017-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "sdkconfig.h"
#include "esp_efuse.h"
#include <assert.h>
#include "f1_efuse_table.h"

// md5_digest_table 40d4db4b56b855572278d907b4235a54
// This file was generated from the file f1_efuse_table.csv. DO NOT CHANGE THIS FILE MANUALLY.
// If you want to change some fields, you need to change f1_efuse_table.csv file
// then run `efuse_common_table` or `efuse_custom_table` command it will generate this file.
// To show efuse_table run the command 'show_efuse_table'.

static const esp_efuse_desc_t USER_DATA_LPWAN_MAC[] = {
    {EFUSE_BLK3, 0, 64}, 	 // LoRa-WAN DevEUI,
};

static const esp_efuse_desc_t USER_DATA_SERIAL_NUMBER[] = {
    {EFUSE_BLK3, 64, 48}, 	 // Board Serial Number,
};

static const esp_efuse_desc_t USER_DATA_HW_ID[] = {
    {EFUSE_BLK3, 112, 24}, 	 // HW ID,
};

static const esp_efuse_desc_t USER_DATA_PROJECT_ID[] = {
    {EFUSE_BLK3, 136, 24}, 	 // Specific Project ID,
};

static const esp_efuse_desc_t USER_DATA_LAYOUT_VERSION[] = {
    {EFUSE_BLK3, 192, 8}, 	 // Version of this layout,
};

static const esp_efuse_desc_t USER_DATA_WIFI_MAC[] = {
    {EFUSE_BLK3, 200, 48}, 	 // WiFi MAC Addr,
};

static const esp_efuse_desc_t USER_DATA_CRC8[] = {
    {EFUSE_BLK3, 248, 8}, 	 // USER_DATA CRC8,
};





const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_LPWAN_MAC[] = {
    &USER_DATA_LPWAN_MAC[0],    		// LoRa-WAN DevEUI
    NULL
};

const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_SERIAL_NUMBER[] = {
    &USER_DATA_SERIAL_NUMBER[0],    		// Board Serial Number
    NULL
};

const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_HW_ID[] = {
    &USER_DATA_HW_ID[0],    		// HW ID
    NULL
};

const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_PROJECT_ID[] = {
    &USER_DATA_PROJECT_ID[0],    		// Specific Project ID
    NULL
};

const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_LAYOUT_VERSION[] = {
    &USER_DATA_LAYOUT_VERSION[0],    		// Version of this layout
    NULL
};

const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_WIFI_MAC[] = {
    &USER_DATA_WIFI_MAC[0],    		// WiFi MAC Addr
    NULL
};

const esp_efuse_desc_t* ESP_EFUSE_USER_DATA_CRC8[] = {
    &USER_DATA_CRC8[0],    		// USER_DATA CRC8
    NULL
};
