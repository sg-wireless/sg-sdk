/*
 * SPDX-FileCopyrightText: 2024 SG Wireless
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Modern ESP-IDF 5.4 compatible WS2812 driver using RMT peripheral
 * Based on official ESP-IDF led_strip examples
 * 
 * API design inspired by JSchaenzle's ESP32-NeoPixel-WS2812-RMT component
 * Original: https://github.com/JSchaenzle/ESP32-NeoPixel-WS2812-RMT (Unlicense)
 */

#ifndef WS2812_CONTROL_H
#define WS2812_CONTROL_H

#include <stdint.h>
#include "sdkconfig.h"
#include "esp_err.h"

#pragma once

#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Use reasonable defaults if CONFIG values not defined
#ifndef CONFIG_WS2812_NUM_LEDS
#define CONFIG_WS2812_NUM_LEDS 1
#endif

#define NUM_LEDS        CONFIG_WS2812_NUM_LEDS

#ifdef __cplusplus
extern "C" {
#endif

// This structure is used for indicating what the colors of each LED should be set to.
// There is a 32bit value for each LED. Only the lower 3 bytes are used and they hold the
// Red (byte 2), Green (byte 1), and Blue (byte 0) values to be set.
struct led_state {
    uint32_t leds[NUM_LEDS];
};

// Setup the hardware peripheral. Only call this once.
esp_err_t ws2812_control_init(void);

// Update the LEDs to the new state. Call as needed.
// This function will block the current task until the RMT peripheral is finished sending 
// the entire sequence.
esp_err_t ws2812_write_leds(struct led_state new_state);

#ifdef __cplusplus
}
#endif

#endif