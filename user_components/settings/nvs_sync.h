/*
 * nvs_sync.h
 *
 *  Created on: Dec 16, 2021
 *      Author: exeland
 */

#ifndef WIFI_MANAGER_NVS_SYNC_H_INCLUDED
#define WIFI_MANAGER_NVS_SYNC_H_INCLUDED

#include <stdbool.h> /* for type bool */
#include <freertos/FreeRTOS.h> /* for TickType_t */
#include <esp_err.h> /* for esp_err_t */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Attempts to get hold of the NVS semaphore for a set amount of ticks.
 * @note If you are uncertain about the number of ticks to wait use portMAX_DELAY.
 * @return true on a succesful lock, false otherwise
 */
bool nvs_sync_lock(TickType_t xTicksToWait);


/**
 * @brief Releases the NVS semaphore
 */
void nvs_sync_unlock();


/**
 * @brief Create the NVS semaphore
 * @return      ESP_OK: success or if the semaphore already exists
 *              ESP_FAIL: failure
 */
esp_err_t nvs_sync_create();

/**
 * @brief Frees memory associated with the NVS semaphore
 * @warning Do not delete a semaphore that has tasks blocked on it (tasks that are in the Blocked state waiting for the semaphore to become available).
 */
void nvs_sync_free();


#ifdef __cplusplus
}
#endif

#endif /* WIFI_MANAGER_NVS_SYNC_H_INCLUDED */
