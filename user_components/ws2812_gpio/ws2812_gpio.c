/*
 * ws2812_gpio.c
 *
 *  Created on: Jul 21, 2022
 *      Author: exeland
 */
#include "driver/gpio.h"

#include "esp8266/eagle_soc.h"

#include "esp_timer.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp8266/gpio_struct.h"
#include "ws2812_gpio.h"

#if 1
#undef  ESP_LOGD
#define ESP_LOGD( tag, format, ... )       {}
#endif

#define GPIO_N            (GPIO_NUM_2)
#define PIN_MUX()         {PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);}
//#define PIN_MUX()         {PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);}
#define SET_GPIO_H()      {GPIO.out_w1ts |= (0x1 << GPIO_N);}
#define SET_GPIO_L()      {GPIO.out_w1tc |= (0x1 << GPIO_N);}

static const char *TAG = "ws2812_gpio";

//static gpio_num_t ws_gpio_num = GPIO_NUM_MAX;
static uint16_t led_cnt = 0;
static int64_t nxt_updt_time;

//I just used a scope to figure out the right time periods.

static inline uint32_t _getCycleCount(void) {
  uint32_t cycles;
  __asm__ __volatile__("rsr %0,ccount":"=a" (cycles));
  return cycles;
}


void ws2812_init(uint16_t cnt){
//  ws_gpio_num = gpio_num;
//  WRITE_PERI_REG( PERIPHS_GPIO_BASEADDR + GPIO_ID_PIN(ws_gpio_num), 0 );
//
  PIN_MUX();
  gpio_set_direction(GPIO_N, GPIO_MODE_OUTPUT);
  led_cnt = cnt;
  nxt_updt_time = esp_timer_get_time();
}


extern int64_t esp_timer_get_time(void);

void IRAM_ATTR  ws2812_update(ws2812_pixel_t *pixels) {
  uint8_t *p, *end, pixel, mask;
  uint32_t t, t0h, t1h, ttot, c, start_time;

//  pin_mask = 1 << pin;
  p =  (uint8_t *) pixels;
  end =  p + led_cnt * sizeof(ws2812_pixel_t);
  pixel = *p++;
  mask = 0x80;
  start_time = 0;

//    GPIO_OUTPUT_SET(GPIO_ID_PIN(WSGPIO), 0);

  t0h  = (1000 * CONFIG_ESP8266_DEFAULT_CPU_FREQ_MHZ) / 3333;  // 0.30us (spec=0.35 +- 0.15)
  t1h  = (1000 * CONFIG_ESP8266_DEFAULT_CPU_FREQ_MHZ) / 1666;  // 0.60us (spec=0.70 +- 0.15)
  ttot = (1000 * CONFIG_ESP8266_DEFAULT_CPU_FREQ_MHZ) /  800;  // 1.25us (MUST be >= 1.25)
  ESP_LOGD(TAG, "refresh t0h:%d, t1h:%d, ttot:%d", t0h, t1h, ttot);

  int64_t time = esp_timer_get_time();

  while(time < nxt_updt_time){
    portYIELD();
    time = esp_timer_get_time();
  }

  taskENTER_CRITICAL();
  while (true) {
    if (pixel & mask) {
        t = t1h;
    } else {
        t = t0h;
    }
    while (((c = _getCycleCount()) - start_time) < ttot); // Wait for the previous bit to finish
    SET_GPIO_H();                                         // Set pin high
    start_time = c;                                       // Save the start time
    while (((c = _getCycleCount()) - start_time) < t);    // Wait for high time to finish
    SET_GPIO_L();                                         // Set pin low
    if (!(mask >>= 1)) {                                  // Next bit/byte
      if (p >= end) {
        break;
      }
      pixel= *p++;
      mask = 0x80;
    }
  }
  taskEXIT_CRITICAL();

  nxt_updt_time = esp_timer_get_time() + 50; // Treset uS
}
