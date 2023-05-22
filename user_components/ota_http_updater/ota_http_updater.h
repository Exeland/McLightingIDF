/*
 * ota_http_updater.h
 *
 *  Created on: Jan 13, 2022
 *      Author: exeland
 */

#ifndef _OTA_HTTP_UPDATER_H_
#define _OTA_HTTP_UPDATER_H_

#ifdef __cplusplus
extern "C" {
#endif
void CheckOTAUpdate(void);
esp_err_t ota_http_updater_start(void);
esp_err_t ota_http_updater_write(char *buf, size_t sz);
esp_err_t ota_http_updater_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* _OTA_HTTP_UPDATER_H_ */
