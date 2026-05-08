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

#include "ws2812_control.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "ws2812_control";

// Use reasonable defaults if CONFIG values not defined
#ifndef CONFIG_WS2812_LED_RMT_TX_GPIO
#define CONFIG_WS2812_LED_RMT_TX_GPIO 21
#endif

#ifndef CONFIG_WS2812_NUM_LEDS
#define CONFIG_WS2812_NUM_LEDS 1
#endif

// RMT configuration
#define WS2812_RMT_RESOLUTION_HZ    10000000    // 10MHz resolution, 1 tick = 0.1us
#define WS2812_GPIO_NUM             CONFIG_WS2812_LED_RMT_TX_GPIO

// WS2812 timing (in RMT ticks, where 1 tick = 0.1us)
#define WS2812_T0H_TICKS           3   // 0.3us
#define WS2812_T0L_TICKS           9   // 0.9us  
#define WS2812_T1H_TICKS           9   // 0.9us
#define WS2812_T1L_TICKS           3   // 0.3us
#define WS2812_RESET_TICKS         500 // 50us reset time

// LED strip encoder structure
typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} ws2812_encoder_t;

// Global handles
static rmt_channel_handle_t s_rmt_channel = NULL;
static rmt_encoder_handle_t s_led_encoder = NULL;

/**
 * @brief WS2812 LED strip encoder implementation
 */
static size_t ws2812_encode_led_strip(rmt_encoder_t *encoder, rmt_channel_handle_t channel, 
                                     const void *primary_data, size_t data_size, 
                                     rmt_encode_state_t *ret_state)
{
    ws2812_encoder_t *led_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = led_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = led_encoder->copy_encoder;
    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;
    
    switch (led_encoder->state) {
    case RMT_ENCODING_RESET:
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
        // fall through
    case RMT_ENCODING_COMPLETE:
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &led_encoder->reset_code,
                                              sizeof(led_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            led_encoder->state = RMT_ENCODING_RESET;
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

/**
 * @brief Delete WS2812 encoder
 */
static esp_err_t ws2812_del_encoder(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *led_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_del_encoder(led_encoder->bytes_encoder);
    rmt_del_encoder(led_encoder->copy_encoder);
    free(led_encoder);
    return ESP_OK;
}

/**
 * @brief Reset WS2812 encoder
 */
static esp_err_t ws2812_reset_encoder(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *led_encoder = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encoder_reset(led_encoder->bytes_encoder);
    rmt_encoder_reset(led_encoder->copy_encoder);
    led_encoder->state = RMT_ENCODING_RESET;
    return ESP_OK;
}

/**
 * @brief Create WS2812 encoder
 */
static esp_err_t ws2812_new_encoder(rmt_encoder_handle_t *ret_encoder)
{
    esp_err_t ret = ESP_OK;
    ws2812_encoder_t *led_encoder = NULL;
    
    led_encoder = calloc(1, sizeof(ws2812_encoder_t));
    ESP_GOTO_ON_FALSE(led_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for led strip encoder");
    
    led_encoder->base.encode = ws2812_encode_led_strip;
    led_encoder->base.del = ws2812_del_encoder;
    led_encoder->base.reset = ws2812_reset_encoder;
    
    // Create bytes encoder for WS2812 protocol
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = WS2812_T0H_TICKS,
            .level1 = 0,
            .duration1 = WS2812_T0L_TICKS,
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = WS2812_T1H_TICKS,
            .level1 = 0,
            .duration1 = WS2812_T1L_TICKS,
        },
        .flags.msb_first = 1 // WS2812 expects MSB first
    };
    ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &led_encoder->bytes_encoder), err, TAG, "create bytes encoder failed");
    
    // Create copy encoder for reset code
    rmt_copy_encoder_config_t copy_encoder_config = {};
    ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &led_encoder->copy_encoder), err, TAG, "create copy encoder failed");
    
    // Setup reset code
    led_encoder->reset_code = (rmt_symbol_word_t){
        .level0 = 0,
        .duration0 = WS2812_RESET_TICKS,
        .level1 = 0,
        .duration1 = WS2812_RESET_TICKS,
    };
    
    *ret_encoder = &led_encoder->base;
    return ESP_OK;
    
err:
    if (led_encoder) {
        if (led_encoder->bytes_encoder) {
            rmt_del_encoder(led_encoder->bytes_encoder);
        }
        if (led_encoder->copy_encoder) {
            rmt_del_encoder(led_encoder->copy_encoder);
        }
        free(led_encoder);
    }
    return ret;
}

esp_err_t ws2812_control_init(void)
{
    esp_err_t ret = ESP_OK;
    
    ESP_LOGI(TAG, "Initializing WS2812 control with %d LEDs on GPIO %d", NUM_LEDS, WS2812_GPIO_NUM);
    
    // Create RMT TX channel
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = WS2812_GPIO_NUM,
        .mem_block_symbols = 64,
        .resolution_hz = WS2812_RMT_RESOLUTION_HZ,
        .trans_queue_depth = 4,
    };
    ESP_GOTO_ON_ERROR(rmt_new_tx_channel(&tx_chan_config, &s_rmt_channel), err, TAG, "create RMT TX channel failed");
    
    // Create LED strip encoder
    ESP_GOTO_ON_ERROR(ws2812_new_encoder(&s_led_encoder), err, TAG, "create led strip encoder failed");
    
    // Enable RMT TX channel
    ESP_GOTO_ON_ERROR(rmt_enable(s_rmt_channel), err, TAG, "enable RMT TX channel failed");
    
    ESP_LOGI(TAG, "WS2812 control initialized successfully");
    return ESP_OK;
    
err:
    if (s_rmt_channel) {
        rmt_del_channel(s_rmt_channel);
        s_rmt_channel = NULL;
    }
    if (s_led_encoder) {
        rmt_del_encoder(s_led_encoder);
        s_led_encoder = NULL;
    }
    return ret;
}

esp_err_t ws2812_write_leds(struct led_state new_state)
{
    if (!s_rmt_channel || !s_led_encoder) {
        ESP_LOGE(TAG, "WS2812 not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Convert RGB data to GRB format expected by WS2812
    uint8_t led_strip_buf[NUM_LEDS * 3];
    for (int i = 0; i < NUM_LEDS; i++) {
        uint32_t rgb = new_state.leds[i];
        led_strip_buf[i * 3 + 0] = (rgb >> 8) & 0xFF;  // Green
        led_strip_buf[i * 3 + 1] = (rgb >> 16) & 0xFF; // Red  
        led_strip_buf[i * 3 + 2] = rgb & 0xFF;         // Blue
    }
    
    // Transmit data
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no loop
    };
    
    esp_err_t ret = rmt_transmit(s_rmt_channel, s_led_encoder, led_strip_buf, sizeof(led_strip_buf), &tx_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "transmit pixels failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Wait for transmission to complete
    ret = rmt_tx_wait_all_done(s_rmt_channel, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "transmit timeout: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}