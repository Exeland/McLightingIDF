/*
 * ota_http_updater.c
 *
 *  Created on: Jan 13, 2022
 *      Author: exeland
 */
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "esp_image_format.h"
#include "nvs.h"

#include "ota_http_updater.h"

static const char *TAG="ota updater";

#define BUFFSIZE 16384
#define HASH_LEN 32 /* SHA-256 digest length */

static esp_ota_handle_t update_handle = 0 ;
static const esp_partition_t *update_partition = NULL;
bool image_header_was_checked = false;

static bool diagnostic(void)
{
    return true;
}

static void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG, "%s: %s", label, hash_print);
}

void CheckOTAUpdate(void)
{
    ESP_LOGI(TAG, "Start CheckOTAUpdateCheck ...");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);

//    uint8_t sha_256[HASH_LEN] = { 0 };
//    esp_partition_t partition;
//
//    // get sha256 digest for the partition table
//    partition.address   = ESP_PARTITION_TABLE_OFFSET;
//    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
//    partition.type      = ESP_PARTITION_TYPE_DATA;
//    esp_partition_get_sha256(&partition, sha_256);
//    print_sha256(sha_256, "SHA-256 for the partition table: ");
//
//    // get sha256 digest for bootloader
//    partition.address   = ESP_BOOTLOADER_OFFSET;
//    partition.size      = ESP_PARTITION_TABLE_OFFSET;
//    partition.type      = ESP_PARTITION_TYPE_APP;
//    esp_partition_get_sha256(&partition, sha_256);
//    print_sha256(sha_256, "SHA-256 for bootloader: ");
//
//    // get sha256 digest for running partition
//    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
//    print_sha256(sha_256, "SHA-256 for current firmware: ");
//
//    const esp_partition_t *running = esp_ota_get_running_partition();
//    esp_ota_img_states_t ota_state;
//    esp_err_t res_stat_partition = esp_ota_get_state_partition(running, &ota_state);
//    switch (res_stat_partition)
//    {
//        case ESP_OK:
//            ESP_LOGI(TAG,"CheckOTAUpdate Partition: ESP_OK\n");
//            if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
//                if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
//                    // run diagnostic function ...
//                    bool diagnostic_is_ok = diagnostic();
//                    if (diagnostic_is_ok) {
//                        ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
//                        esp_ota_mark_app_valid_cancel_rollback();
//                    } else {
//                        ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
//                        esp_ota_mark_app_invalid_rollback_and_reboot();
//                    }
//                }
//            }
//            break;
//        case ESP_ERR_INVALID_ARG:
//            ESP_LOGE(TAG,"CheckOTAUpdate Partition: ESP_ERR_INVALID_ARG\n");
//            break;
//        case ESP_ERR_NOT_SUPPORTED:
//            ESP_LOGE(TAG,"CheckOTAUpdate Partition: ESP_ERR_NOT_SUPPORTED\n");
//            break;
//        case ESP_ERR_NOT_FOUND:
//            ESP_LOGE(TAG,"CheckOTAUpdate Partition: ESP_ERR_NOT_FOUND\n");
//            break;
//    }
//    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
//        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
//            // run diagnostic function ...
//            bool diagnostic_is_ok = diagnostic();
//            if (diagnostic_is_ok) {
//                ESP_LOGI(TAG, "Diagnostics completed successfully! Continuing execution ...");
//                esp_ota_mark_app_valid_cancel_rollback();
//            } else {
//                ESP_LOGE(TAG, "Diagnostics failed! Start rollback to the previous version ...");
//                esp_ota_mark_app_invalid_rollback_and_reboot();
//            }
//        }
//    }
}

esp_err_t ota_http_updater_start(void){

  /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
  update_handle = 0 ;
  update_partition = NULL;

  ESP_LOGI(TAG, "Starting OTA updater");

  const esp_partition_t *configured = esp_ota_get_boot_partition();
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (configured != running) {
      ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
      ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
  }
  ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);


  update_partition = esp_ota_get_next_update_partition(NULL);
  ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);

  // deal with all receive packet
  image_header_was_checked = false;

  return ESP_OK;
}

#define OTA_HEADER_SZ (sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))

esp_err_t ota_http_updater_write(char *buf, size_t sz) {
  esp_err_t err;
  if (image_header_was_checked == false) {
    esp_app_desc_t new_app_info;
    if (sz > OTA_HEADER_SZ) {
      const esp_partition_t *running = esp_ota_get_running_partition();
      // check current version with downloading
      memset(&new_app_info, 0, sizeof(esp_app_desc_t));
      memcpy(&new_app_info, &buf[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
      ESP_LOGI(TAG, "New firmware version: %s", new_app_info.version);
      esp_app_desc_t running_app_info;
      if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
      }

//      const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
//      esp_app_desc_t invalid_app_info;
//      if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
//        ESP_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
//      }

//      // check current version with last invalid partition
//      if (last_invalid_app != NULL) {
//        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
//          ESP_LOGW(TAG, "New version is the same as invalid version.");
//          ESP_LOGW(TAG, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
//          ESP_LOGW(TAG, "The firmware has been rolled back to the previous version.");
//          return ESP_OK;
//        }
//      }

      image_header_was_checked = true;

      err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return err;
      }
      ESP_LOGI(TAG, "esp_ota_begin succeeded");
    }
    else {
      ESP_LOGE(TAG, "received package is not fit len");
      return err;
    }
  }
  ESP_LOGD(TAG, "write, size: %d", sz);
  err = esp_ota_write( update_handle, (const void *)buf, sz);

  return err;
}

esp_err_t ota_http_updater_stop(void){
  esp_err_t err = esp_ota_end(update_handle);
  if (err != ESP_OK) {
      if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
          ESP_LOGE(TAG, "Image validation failed, image is corrupted");
      }
      ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
  }
  else{
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
    }
  }

  return err;
}

