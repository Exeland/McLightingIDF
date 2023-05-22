/*
 * led_animator.h
 *
 *  Created on: Jun 28, 2022
 *      Author: exeland
 */

#ifndef MAIN_LED_ANIMATOR_H_
#define MAIN_LED_ANIMATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

const char* led_animator_get_JSON_modes_names();

void led_animator_setmode(int mode);
void led_animator_setcolor(int r, int g, int b) ;
void led_animator_setbrightness(int br);
void led_animator_setspeed(int sp);

int led_animator_getmode(void);
void led_animator_getcolor(int *r, int *g, int *b) ;
int led_animator_getbrightness(void);
int led_animator_getspeed(void);

void led_animator_init(int cnt, char* order);


#ifdef __cplusplus
}
#endif

#endif /* MAIN_LED_ANIMATOR_H_ */
