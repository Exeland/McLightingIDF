/*
 * led_animator.cpp
 *
 *  Created on: Jun 28, 2022
 *      Author: exeland
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "driver/gpio.h"

//#include "../user_components/esp8266_ws2812_i2s/src/ws2812_i2s.h"
//#include "../user_components/ws2812_i2s/ws2812_i2s.h"
#include "../user_components/ws2812_gpio/ws2812_gpio.h"

#include "FX.h"

#include "led_animator.h"

#if 1
#undef  ESP_LOGD
#define ESP_LOGD( tag, format, ... )       {}
#endif


static const char *TAG = "LED Animator";

#define LEDS_ORDER_CNT  6
#define STR_RGB  "RGB"
#define STR_GBR  "GBR"
#define STR_BRG  "BRG"
#define STR_RBG  "RBG"
#define STR_GRB  "GRB"
#define STR_BGR  "BGR"

//typedef enum
//{
//    RGB = 0,
//    GBR = 1,
//    BRG = 2,
//    RBG = 3,
//    GRB = 4,
//    BGR = 5
//} t_color_order;

const char *str_LEDs_order[LEDS_ORDER_CNT] =
{
    STR_RGB,
    STR_GBR,
    STR_BRG,
    STR_RBG,
    STR_GRB,
    STR_BGR
};

CRGBPalette16 currentPalette;
TBlendType currentBlending;

#include "palettes.h"

//#define NUM_LEDS 50

static CRGB*           leds;
static ws2812_pixel_t* ws2812_pixel;

//static WS2812 ledstrip;
//static Pixel_t pixels[NUM_LEDS];

/* test using the FX unit
 **
 */

void hal_setBrightness(uint8_t scale);
void hal_show(void);

/* test specific patterns so we know FX is working right
 **
 */
static WS2812FX ws2812fx;

typedef struct {
  uint16_t        cnt;
  uint8_t         col_indx[3];
  uint8_t         mode;
  uint32_t        color;
  uint8_t         speed;
  uint8_t         brightness;
} animMode_t;

static animMode_t animMode;

//static void led_anim_fill_color(uint8_t c1, uint8_t c2, uint8_t c3) {
//  for (int i = 0; i < NUM_LEDS; i++) {
//    ws2812_pixel[i].color[0] = c1;
//    ws2812_pixel[i].color[1] = c2;
//    ws2812_pixel[i].color[2] = c3;
//  }
//}

static void led_anim_task(void *pvParameters) {
  WS2812FX::Segment *segments = ws2812fx.getSegments();

  ws2812fx.init(animMode.cnt, leds, false); // type was configured before
  ws2812fx.setBrightness(animMode.brightness);
  ws2812fx.show();

  ws2812fx.setMode(0 , animMode.mode);
  ws2812fx.setColor(0, animMode.color);
  ws2812fx.setSpeed(0, animMode.speed);
  segments[0].speed = animMode.speed;

  ESP_LOGI(TAG, "Led anim task start");

//  uint32_t cntr = 0;
//
//  led_anim_fill_color(0xf0,0x00,0x00);

  while (true) {
    ws2812fx.service();
//    cntr++;
//
//    if(cntr == 10){
//      led_anim_fill_color(0x00,0xf0,0x00);
//    }
//
//    if(cntr == 20){
//      led_anim_fill_color(0x00,0x00,0xf0);
//    }
//
//    if(cntr == 30){
//      led_anim_fill_color(0xf0,0x00,0x00);
//      cntr = 0;
//    }

//    ws2812_update(ws2812_pixel);
    vTaskDelay(1); /*10ms*/
  }
}

void hal_setBrightness(uint8_t scale) {
  ESP_LOGD(TAG, "hal_setBrightness: %d\n", scale);
}


uint8_t inline cnv_brightness(uint8_t c){
  uint32_t ret = (animMode.brightness*c)/255;
  if((c > 0)&&(ret == 0)){
    ret = 1;
  }
  else if(ret > 255){
    ret = 255;
  }
  return ret;
}


void hal_show(void) {
  ESP_LOGD(TAG, "RGB:\n");
  uint8_t r_oder = animMode.col_indx[0];
  uint8_t g_oder = animMode.col_indx[1];
  uint8_t b_oder = animMode.col_indx[2];

  for (int i = 0; i < animMode.cnt; i++) {
    ws2812_pixel[i].color[r_oder] = cnv_brightness(leds[i].r);
    ws2812_pixel[i].color[g_oder] = cnv_brightness(leds[i].g);
    ws2812_pixel[i].color[b_oder] = cnv_brightness(leds[i].b);
    ESP_LOGD(TAG, "[%d, %d, %d], ", ws2812_pixel[i].color[0], ws2812_pixel[i].color[1], ws2812_pixel[i].color[2]);
  }
  ESP_LOGD(TAG, "\n");
  ws2812_update(ws2812_pixel);
}

const char* led_animator_get_JSON_modes_names() {
  return JSON_mode_names;
}

void led_animator_setmode(int mode) {
  ESP_LOGD(TAG, "set mode:%d\n", mode);
  animMode.mode = mode;
  ws2812fx.setMode(0 /*segid*/, animMode.mode);
}

void led_animator_setcolor(int r, int g, int b) {
  animMode.color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  ESP_LOGD(TAG, "set color:%d\n", animMode.color);
  ws2812fx.setColor(0, animMode.color);
}

void led_animator_setbrightness(int br) {
  animMode.brightness = br;
  ESP_LOGD(TAG, "set brightness:%d\n", animMode.brightness);
  ws2812fx.setBrightness(animMode.brightness);
}

void led_animator_setspeed(int sp) {
  animMode.speed = sp;
  ESP_LOGD(TAG, "set speed:%d\n", animMode.speed);
  ws2812fx.setSpeed(0, animMode.speed);
}

int led_animator_getmode(void){
  return animMode.mode;
}

void led_animator_getcolor(int *r, int *g, int *b){
  *r = 0xFF & (animMode.color)>>16;
  *g = 0xFF & (animMode.color)>>8;
  *b = 0xFF & (animMode.color);
}

int led_animator_getbrightness(void){
  return animMode.brightness;
}

int led_animator_getspeed(void){
  return animMode.speed;
}

void led_animator_init(int cnt, char* order) {
  ESP_LOGI(TAG, "ws2812_init");
//  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
  leds = new CRGB[cnt];
  ws2812_pixel = new ws2812_pixel_t[cnt];
  ws2812_init(cnt);
  ESP_LOGI(TAG, "OK\n");

//  animMode.order = GRB;
  for (int i = 0; i < 3; ++i) {
    if(order[i] == 'R'){
      animMode.col_indx[0] = i;
    }
    if(order[i] == 'G'){
      animMode.col_indx[1] = i;
    }
    if(order[i] == 'B'){
      animMode.col_indx[2] = i;
    }
  }
  animMode.cnt   = cnt;
  animMode.color = DEFAULT_COLOR;
  animMode.mode = DEFAULT_MODE;
  animMode.speed = DEFAULT_SPEED;
  animMode.brightness = DEFAULT_BRIGHTNESS;

  xTaskCreatePinnedToCore(&led_anim_task, "led_anim_task", 4000, NULL, 5, NULL, 0);
}
