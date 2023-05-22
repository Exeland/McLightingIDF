/*
 * json_maker.h
 *
 *  Created on: Dec 22, 2021
 *      Author: exeland
 */

#ifndef _JSON_MAKER_H_
#define _JSON_MAKER_H_

#ifdef __cplusplus
extern "C" {
#endif

bool lock_json_buffer(TickType_t xTicksToWait);
void unlock_json_buffer(void);

char *get_status_json_data(void);
char *get_settings_json_data(void);

void json_maker_init(void);

#ifdef __cplusplus
}
#endif

#endif /* USER_COMPONENTS_JSON_MAKER_JSON_MAKER_H_ */
