/** MIT licence

 Copyright (C) 2019 by Vu Nam https://github.com/vunam https://studiokoda.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions: The above copyright notice and this
 permission notice shall be included in all copies or substantial portions of
 the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.

 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2s.h"
#include "ws2812_i2s.h"

static const char *TAG = "ws2812_i2s";

i2s_pin_config_t pin_config = { .bck_o_en = 0, .ws_o_en = 0, .data_out_en = 1, .data_in_en = 0 };

static uint8_t *out_buffer;
static uint8_t off_buffer[ZERO_BUFFER] = { 0 };
static uint16_t led_number = 0;
static uint16_t size_buffer = 0;

static const uint16_t bitpatterns[4] = { 0x88, 0x8e, 0xe8, 0xee };

static QueueHandle_t q_ws2812;

void ws2812_init(uint16_t led_cnt) {
  i2s_config_t i2s_config = {
      .mode = I2S_MODE_MASTER | I2S_MODE_TX,
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = 16,
      .communication_format = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .dma_buf_len = led_cnt*PIXEL_SIZE,
      /*.intr_alloc_flags = 0,*/
      .dma_buf_count = 2,
      /*.use_apll = false,*/
  };

  led_number = led_cnt;
  size_buffer = led_number * PIXEL_SIZE;
  out_buffer = malloc(size_buffer);

  for (int i = 0; i < ZERO_BUFFER; ++i) {
    off_buffer[i] = 0;
  }

  i2s_driver_install(I2S_NUM, &i2s_config, 1, &q_ws2812);
  i2s_set_pin(I2S_NUM, &pin_config);
}

void ws2812_log_event(i2s_event_t* i2s_event){
  switch (i2s_event->type) {

  case I2S_EVENT_DMA_ERROR:{
    ESP_LOGE(TAG, "dma error");
    break;
  }
  case I2S_EVENT_TX_DONE:{
    /*!< I2S DMA finish sent 1 buffer*/
    ESP_LOGD(TAG, "tx done");
    break;
  }

  case I2S_EVENT_RX_DONE: {
    /*!< I2S DMA finish received 1 buffer*/
    ESP_LOGD(TAG, "rx done");
    break;
  }

  default:
    break;
  }
}


void ws2812_update(ws2812_pixel_t *pixels) {
  i2s_event_t i2s_event;
  size_t bytes_written = 0;

  for (uint16_t i = 0; i < led_number; i++) {
    int loc = i * PIXEL_SIZE;

    for (uint8_t j = 0; j < 3; j++) {
      out_buffer[loc++] = bitpatterns[pixels[i].color[j] >> 6 & 0x03];
      out_buffer[loc++] = bitpatterns[pixels[i].color[j] >> 4 & 0x03];
      out_buffer[loc++] = bitpatterns[pixels[i].color[j] >> 2 & 0x03];
      out_buffer[loc++] = bitpatterns[pixels[i].color[j] & 0x03];
    }
  }

  i2s_start(I2S_NUM);
  i2s_write(I2S_NUM, out_buffer, size_buffer, &bytes_written, portMAX_DELAY);
  i2s_write(I2S_NUM, off_buffer, ZERO_BUFFER, &bytes_written, portMAX_DELAY);
  xQueueReceive(q_ws2812, (void *)&i2s_event, portMAX_DELAY);
  ws2812_log_event(&i2s_event);
  i2s_zero_dma_buffer(I2S_NUM);
  xQueueReceive(q_ws2812, (void *)&i2s_event, portMAX_DELAY);
  ws2812_log_event(&i2s_event);

  i2s_stop(I2S_NUM);
}
