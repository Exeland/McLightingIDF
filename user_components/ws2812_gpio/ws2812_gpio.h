/*
 * ws2812_gpio.h
 *
 *  Created on: Jul 21, 2022
 *      Author: exeland
 */

#ifndef _WS2812_GPIO_H_
#define _WS2812_GPIO_H_

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct{
  uint8_t color[3];
} ws2812_pixel_t;

void ws2812_init(uint16_t cnt);

void ws2812_update(ws2812_pixel_t *pixels) ;

#ifdef  __cplusplus
}
#endif


#endif /* USER_COMPONENTS_WS2812_GPIO_WS2812_GPIO_H_ */
