/*
 * settings.c
 *
 *  Created on: Dec 21, 2021
 *      Author: exeland
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "nvs_sync.h"

#include <freertos/FreeRTOS.h>

#include "../cJSON/cJSON.h"

#include "settings.h"


#define BLOB_NAME_VERS       "s1"
#define BLOB_NAME_NW_ENBL    "s2"
#define BLOB_NAME_AUTH_PSWD  "s3"
#define BLOB_NAME_AP_SSID    "s4"
#define BLOB_NAME_AP_PSWD    "s5"
#define BLOB_NAME_NW_SSID    "s6"
#define BLOB_NAME_NW_PSWD    "s7"
#define BLOB_NAME_MQTT_HST   "s8"
#define BLOB_NAME_MQTT_PRT   "s9"
#define BLOB_NAME_MQTT_CLNT  "s10"
#define BLOB_NAME_MQTT_USR   "s11"
#define BLOB_NAME_MQTT_PSWD  "s12"
#define BLOB_NAME_MQTT_TPC   "s13"
#define BLOB_NAME_MQTT_ENBL  "s14"
#define BLOB_NAME_LANG       "s15"
#define BLOB_NAME_LEDS_CNT   "s16"
#define BLOB_NAME_LEDS_ORDER "s17"

static const char *SETTINGS_TAG = "SETT";
const char *settings_nvs_namespace = "SETT";

#define DEF_AUTH_PASS "12345678"

//#undef ESP_LOGD
//#define ESP_LOGD ESP_LOGI


const t_settings default_set = {
    .vers = 2,
//    .auth = {
//        .pswd = DEF_AUTH_PASS
//    },
    .ap = {
        .ssid = CONFIG_DEFAULT_AP_SSID,
        .pswd = CONFIG_DEFAULT_AP_PASSWORD
    },
    .nw = {
        .ssid = CONFIG_DEFAULT_AP_SSID,
        .pswd = CONFIG_DEFAULT_AP_PASSWORD
    },
    .nw_enable = false,
//    .mqtt_enable = false,
//    .mqtt = {
//        .host = "Host",
//        .port = 1883,
//        .client = "WTM",
//        .user = "WTM_USER",
//        .pswd = "12345678",
//        .topic = "home/WTM/meas",
//    },
    .leds = {
        .cnt = 10,
        .order = "GRB",
    },
    .lang = {
        .str = "ru",
    }
};

//const char *LEDs_order[] =
//{
//    STR_RGB,
//    STR_GBR,
//    STR_BRG,
//    STR_RBG,
//    STR_GRB,
//    STR_BGR
//};

static t_settings *settings;

#define check_and_free_memory(_mem_) do{ \
if (_mem_) { \
  free(_mem_); \
  (_mem_) = NULL; \
} \
} while(0)

t_settings* get_settings(void){
  return settings;
}

bool settings_update_string(char* set_str, char* str, size_t max_sz){
  if(set_str == NULL){
    return false;
  }

  ESP_LOGD(SETTINGS_TAG, "old_str: %s", set_str);
  ESP_LOGD(SETTINGS_TAG, "update string: %s, max sz: %d", str, max_sz);

  size_t len = strlen(str) + 1;
  if (len > max_sz) {
    len = max_sz;
  }

  memset(set_str, 0, len);
  memcpy(set_str, str, len);
  ESP_LOGD(SETTINGS_TAG, "string_copy: %s, %d", str, max_sz);
  return true;
}

//bool settings_update_auth_pswd(char* pswd){
//  return settings_update_string(settings->auth.pswd, pswd, SETTINGS_AUTH_PW_SZ);
//}

bool settings_update_ap_ssid(char* ssid){
  return settings_update_string(settings->ap.ssid, ssid, SETTINGS_SSID_MAX_SZ);
}

bool settings_update_ap_pswd(char* pswd){
  return settings_update_string(settings->ap.pswd, pswd, SETTINGS_PW_MAX_SZ);
}

bool settings_update_nw_ssid(char* ssid){
  return settings_update_string(settings->nw.ssid, ssid, SETTINGS_SSID_MAX_SZ);
}

bool settings_update_nw_pswd(char* pswd){
  return settings_update_string(settings->nw.pswd, pswd, SETTINGS_PW_MAX_SZ);
}

//bool settings_update_mqtt_host(char* host){
//  return settings_update_string(settings->mqtt.host, host, SETTINGS_MQTT_HOST_SZ);
//}
//
//bool settings_update_mqtt_client(char* client){
//  return settings_update_string(settings->mqtt.client, client, SETTINGS_MQTT_CLIENT_SZ);
//}
//
//bool settings_update_mqtt_user(char* user){
//  return settings_update_string(settings->mqtt.user, user, SETTINGS_MQTT_USER_SZ);
//}
//
//bool settings_update_mqtt_pswd(char* pswd){
//  return settings_update_string(settings->mqtt.pswd, pswd, SETTINGS_MQTT_PW_SZ);
//}
//
//bool settings_update_mqtt_topic(char* topic){
//  return settings_update_string(settings->mqtt.topic, topic, SETTINGS_MQTT_TOPIC_SZ);
//}

bool settings_update_lang(char* lang){
  return settings_update_string(settings->lang.str, lang, SETTINGS_LANG_SZ);
}

bool settings_update_leds_order(char* order){
  return settings_update_string(settings->leds.order, order, SETTINGS_LEDS_ORDER_SZ);
}

bool settings_update_leds_cnt(int cnt){
  settings->leds.cnt = cnt;
  return true;
}

bool settings_update_nw_enable(bool nw_enable){
  settings->nw_enable = nw_enable;
  return true;
}

//bool settings_update_mqtt_enable(bool mqtt_enable){
//  settings->mqtt_enable = mqtt_enable;
//  return true;
//}
//
//bool settings_update_mqtt_port(int port){
//  settings->mqtt.port = port;
//  return true;
//}

bool get_default_settings(void) {
  uint8_t baseMac[6];
  uint8_t xor1, xor2;
  ESP_LOGD(SETTINGS_TAG, "Load default settings");
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  xor1 = baseMac[0] ^ baseMac[2] ^ baseMac[4];
  xor2 = baseMac[1] ^ baseMac[3] ^ baseMac[5];
//  settings_update_auth_pswd((char*) default_set.auth.pswd);

  char *temp_str;
  temp_str = malloc(128);
  if (temp_str) {
    sprintf(temp_str, "%s_%02X%02X", (char*) default_set.ap.ssid, xor1, xor2);
  } else {
    temp_str = (char*) default_set.ap.ssid;
  }
  // Write unique name into temp_str
  settings_update_ap_ssid(temp_str);
  settings_update_ap_pswd((char*) default_set.ap.pswd);

  settings_update_nw_enable(default_set.nw_enable);
  settings_update_nw_ssid((char*) default_set.nw.ssid);
  settings_update_nw_pswd((char*) default_set.nw.pswd);

//  settings_update_mqtt_enable(default_set.mqtt_enable);
//  settings_update_mqtt_host((char*) default_set.mqtt.host);
//  settings_update_mqtt_port(default_set.mqtt.port);
//  settings_update_mqtt_client((char*) temp_str);
//  settings_update_mqtt_user((char*) default_set.mqtt.user);
//  settings_update_mqtt_pswd((char*) default_set.mqtt.pswd);

//  if (temp_str) {
//    sprintf(temp_str, "home/%s_%02X%02X/meas", (char*) default_set.ap.ssid, xor1, xor2);
//  } else {
//    temp_str = (char*) default_set.mqtt.topic;
//  }
//
//  settings_update_mqtt_topic(temp_str);

  settings_update_lang((char*) default_set.lang.str);


  settings_update_leds_order((char*) default_set.leds.order);
  settings_update_leds_cnt(default_set.leds.cnt);

  settings->vers = default_set.vers;

  if (temp_str) {
    free(temp_str);
  }

  return true;
}


bool nvs_read_str_with_check(
    nvs_handle handle,
    char*      buff,
    size_t     sz,
    char*      blob_name,
    char*      get_str,
    size_t     max_sz){
  esp_err_t esp_err;


  ESP_LOGD(SETTINGS_TAG, "Read blob name: %s", blob_name);

  buff[0] = 0;
  esp_err = nvs_get_blob(handle, blob_name, buff, &sz);
  if (esp_err != ESP_OK) {
    ESP_LOGE(SETTINGS_TAG, "Error nvs_read_str_with_check: %s, %d", blob_name, esp_err);
    return false;
  }

  get_str[0] = 0;
  strncpy(get_str, buff, max_sz);
  ESP_LOGD(SETTINGS_TAG, "value: %s", get_str);

  return true;
}

bool nvs_read_u32_with_check(
    nvs_handle handle,
    char*      blob_name,
    uint32_t*  get_val){

  uint32_t val = 0;
  ESP_LOGD(SETTINGS_TAG, "Read blob name: %s", blob_name);

  if (nvs_get_u32(handle, blob_name, &val) != ESP_OK) {
    ESP_LOGE(SETTINGS_TAG, "Error nvs_read_u32_with_check: %s", blob_name);
    return false;
  }

  ESP_LOGD(SETTINGS_TAG, "value: %d", val);
  *get_val = val;

  return true;
}

bool nvs_read_bool_with_check(
    nvs_handle handle,
    char*      blob_name,
    bool*      get_val){

	uint32_t val = 0;
	ESP_LOGD(SETTINGS_TAG, "Read blob name: %s", blob_name);

	if (nvs_get_u32(handle, blob_name, &val) != ESP_OK) {
		ESP_LOGE(SETTINGS_TAG, "Error nvs_read_bool_with_check: %s", blob_name);
		return false;
	}

	ESP_LOGD(SETTINGS_TAG, "value: %d", val);
	*get_val = (val != 0);

	return true;
}


bool nvs_write_str_with_check(
    nvs_handle handle,
    char*   buff,
    size_t  sz,
    char*   blob_name,
    char*   set_str,
    bool   *change){
  esp_err_t esp_err;

  buff[0] = 0;
  esp_err = nvs_get_blob(handle, blob_name, buff, &sz);

  if ( (esp_err == ESP_OK  || esp_err == ESP_ERR_NVS_NOT_FOUND) &&
      (strcmp(set_str, buff) != 0)) {
    strncpy(buff, set_str, sz);
    if (nvs_set_blob(handle, blob_name, buff, sz) != ESP_OK) {
      ESP_LOGE(SETTINGS_TAG, "Error write blob:%s", blob_name);
      return false;
    }
    *change = true;
  }

  return true;
}

bool nvs_write_u32_with_check(
    nvs_handle handle,
    char*      blob_name,
    uint32_t   set_val,
    bool      *change){
  uint32_t read_val;
  esp_err_t esp_err = nvs_get_u32(handle, blob_name, &read_val);

  if ( (esp_err == ESP_OK  || esp_err == ESP_ERR_NVS_NOT_FOUND) &&
      read_val != set_val) {
    if (nvs_set_u32(handle, blob_name, set_val) != ESP_OK) {
      ESP_LOGE(SETTINGS_TAG, "Error write blob:%s", blob_name);
      return false;
    }
    *change = true;
  }

  return true;
}


bool nvs_write_bool_with_check(
    nvs_handle handle,
    char*      blob_name,
    bool   set_val,
    bool *change)
{
	uint32_t temp = (set_val ? 0xFFFF : 0);
	return nvs_write_u32_with_check( handle, blob_name, (uint32_t) temp, change);
}


bool settings_auth_read_nvs(nvs_handle handle, t_settings_auth *set_auth){
  bool ret = false;
  char* buff;

  size_t sz = SETTINGS_AUTH_PW_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"auth\"");
    return false;
  }

  if(!nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_AUTH_PSWD, set_auth->pswd, SETTINGS_AUTH_PW_SZ)){
    goto Exit;
  }

  ret = true;
Exit:
  free(buff);

  return ret;
}

bool settings_ap_read_nvs(nvs_handle handle, t_settings_AP *set_ap){
  bool ret = false;
  char* buff;

  size_t sz = (SETTINGS_PW_MAX_SZ > SETTINGS_SSID_MAX_SZ) ? SETTINGS_PW_MAX_SZ : SETTINGS_SSID_MAX_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"ssid\"");
    return false;
  }

  if(!nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_AP_SSID, set_ap->ssid, SETTINGS_SSID_MAX_SZ)||
     !nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_AP_PSWD, set_ap->pswd, SETTINGS_PW_MAX_SZ)){
    goto Exit;
  }

  ret = true;
Exit:
  free(buff);

  return ret;
}

bool settings_nw_read_nvs(nvs_handle handle, t_settings_NW *set_nw){
  bool ret = false;
  char* buff;

  size_t sz = (SETTINGS_PW_MAX_SZ > SETTINGS_SSID_MAX_SZ) ? SETTINGS_PW_MAX_SZ : SETTINGS_SSID_MAX_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"ssid\"");
    return false;
  }

  if(!nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_NW_SSID, set_nw->ssid, SETTINGS_SSID_MAX_SZ)||
     !nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_NW_PSWD, set_nw->pswd, SETTINGS_PW_MAX_SZ)){
    goto Exit;
  }

  ret = true;
Exit:
  free(buff);

  return ret;
}

bool settings_mqtt_read_nvs(nvs_handle handle, t_settings_mqtt *set_mqtt){
  bool ret = false;
  char* buff;

  size_t sz = SETTINGS_MQTT_FTOPIC_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"ssid\"");
    return false;
  }

  if(!nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_MQTT_HST, set_mqtt->host, SETTINGS_MQTT_HOST_SZ) ||
     !nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_MQTT_CLNT, set_mqtt->client, SETTINGS_MQTT_CLIENT_SZ) ||
     !nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_MQTT_USR, set_mqtt->user, SETTINGS_MQTT_USER_SZ) ||
     !nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_MQTT_PSWD, set_mqtt->pswd, SETTINGS_MQTT_PW_SZ) ||
     !nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_MQTT_TPC, set_mqtt->topic, SETTINGS_MQTT_TOPIC_SZ) ||
     !nvs_read_u32_with_check(handle, BLOB_NAME_MQTT_PRT, (uint32_t*) &set_mqtt->port)){
    goto Exit;
  }

  ret = true;
Exit:
  free(buff);

  return ret;
}

bool settings_leds_read_nvs(nvs_handle handle, t_settings_leds *set_leds){
  char buff[SETTINGS_LEDS_ORDER_SZ];
  bool ret = false;

  if(!nvs_read_u32_with_check(handle, BLOB_NAME_LEDS_CNT, (uint32_t*) &set_leds->cnt) ||
     !nvs_read_str_with_check(handle, buff, SETTINGS_LEDS_ORDER_SZ, BLOB_NAME_LEDS_ORDER, set_leds->order, SETTINGS_LEDS_ORDER_SZ)){
    goto Exit;
  }

  ret = true;
Exit:
  return ret;
}


bool settings_lang_read_nvs(nvs_handle handle, t_settings_lang *set_lang){
  char buff[SETTINGS_LANG_SZ];
  size_t sz = SETTINGS_LANG_SZ;
  return nvs_read_str_with_check(handle, buff, sz, BLOB_NAME_LANG, set_lang->str, SETTINGS_LANG_SZ);
}


bool settings_auth_write_nvs(nvs_handle handle, t_settings_auth *set_auth, bool *change){
  char* buff;
  bool ret = false;

  size_t sz = SETTINGS_AUTH_PW_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"settings_auth_write_nvs\"");
    return false;
  }

  if (!nvs_write_str_with_check( handle, buff, sz, BLOB_NAME_AUTH_PSWD, set_auth->pswd, change)){
    ESP_LOGE(SETTINGS_TAG, "Error nvs_write_str \"settings_auth_write_nvs\"");
    goto Exit;
  }

  ret = true;

Exit:
  free(buff);
  return ret;
}

bool settings_ap_write_nvs(nvs_handle handle, t_settings_AP *set_ap, bool *change){
  char* buff;
  bool ret = false;

  size_t sz = (SETTINGS_PW_MAX_SZ > SETTINGS_SSID_MAX_SZ) ? SETTINGS_PW_MAX_SZ : SETTINGS_SSID_MAX_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"settings_ap_write_nvs\"");
    return false;
  }

  if (!nvs_write_str_with_check( handle, buff, SETTINGS_SSID_MAX_SZ, BLOB_NAME_AP_SSID, set_ap->ssid, change) ||
      !nvs_write_str_with_check( handle, buff, SETTINGS_PW_MAX_SZ, BLOB_NAME_AP_PSWD, set_ap->pswd, change)){
    ESP_LOGE(SETTINGS_TAG, "Error nvs_write_str \"settings_ap_write_nvs\"");
    goto Exit;
  }

  ret = true;

Exit:
  free(buff);
  return ret;
}

bool settings_nw_write_nvs(nvs_handle handle, t_settings_NW *set_nw, bool *change){
  char* buff;
  bool ret = false;

  size_t sz = (SETTINGS_PW_MAX_SZ > SETTINGS_SSID_MAX_SZ) ? SETTINGS_PW_MAX_SZ : SETTINGS_SSID_MAX_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"settings_nw_write_nvs\"");
    return false;
  }

  if (!nvs_write_str_with_check( handle, buff, SETTINGS_SSID_MAX_SZ, BLOB_NAME_NW_SSID, set_nw->ssid, change) ||
      !nvs_write_str_with_check( handle, buff, SETTINGS_PW_MAX_SZ, BLOB_NAME_NW_PSWD, set_nw->pswd, change)){
    ESP_LOGE(SETTINGS_TAG, "Error nvs_write_str \"settings_nw_write_nvs\"");
    goto Exit;
  }

  ret = true;

Exit:
  free(buff);
  return ret;
}

bool settings_mqtt_write_nvs(nvs_handle handle, t_settings_mqtt *set_mqtt, bool *change){
  char* buff;
  bool ret = false;

  size_t sz = SETTINGS_MQTT_FTOPIC_SZ;
  if ((buff = malloc(sz)) == NULL) {
    ESP_LOGE(SETTINGS_TAG, "Error alloc \"settings_mqtt_write_nvs\"");
    return false;
  }

  if (!nvs_write_str_with_check( handle, buff, SETTINGS_MQTT_HOST_SZ, BLOB_NAME_MQTT_HST, set_mqtt->host, change) ||
      !nvs_write_str_with_check( handle, buff, SETTINGS_MQTT_CLIENT_SZ, BLOB_NAME_MQTT_CLNT, set_mqtt->client, change) ||
      !nvs_write_str_with_check( handle, buff, SETTINGS_MQTT_USER_SZ, BLOB_NAME_MQTT_USR, set_mqtt->user, change) ||
      !nvs_write_str_with_check( handle, buff, SETTINGS_MQTT_PW_SZ, BLOB_NAME_MQTT_PSWD, set_mqtt->pswd, change) ||
      !nvs_write_str_with_check( handle, buff, SETTINGS_MQTT_TOPIC_SZ, BLOB_NAME_MQTT_TPC, set_mqtt->topic, change) ||
      !nvs_write_u32_with_check( handle, BLOB_NAME_MQTT_PRT, set_mqtt->port, change)){
    ESP_LOGE(SETTINGS_TAG, "Error nvs_write_str \"settings_mqtt_write_nvs\"");
    goto Exit;
  }

  ret = true;

Exit:
  free(buff);
  return ret;
}

bool settings_leds_write_nvs(nvs_handle handle, t_settings_leds *set_leds, bool *change){
  char buff[SETTINGS_LEDS_ORDER_SZ];
  bool ret = false;

  if (!nvs_write_str_with_check( handle, buff, SETTINGS_LEDS_ORDER_SZ, BLOB_NAME_LEDS_ORDER, set_leds->order, change) ||
      !nvs_write_u32_with_check( handle, BLOB_NAME_LEDS_CNT, set_leds->cnt, change)){
    ESP_LOGE(SETTINGS_TAG, "Error nvs_write_str \"settings_mqtt_write_nvs\"");
    goto Exit;
  }

  ret = true;

Exit:
  return ret;
}

bool settings_lang_write_nvs(nvs_handle handle, char *set_lang, bool *change){
  size_t sz = SETTINGS_LANG_SZ;
  char buff[SETTINGS_LANG_SZ];
  return nvs_write_str_with_check( handle, buff, sz, BLOB_NAME_LANG, set_lang, change);
}

//bool settings_mqqt_enable_write_nvs(nvs_handle handle, bool mqqt_en, bool *change)
//{
//  uint32_t temp = (mqqt_en ? 0xFFFF : 0);
//  return nvs_write_u32_with_check( handle, BLOB_NAME_MQTT_ENBL, temp, change);
//}


bool settings_vers_write_nvs(nvs_handle handle, int vers, bool *change)
{
  return nvs_write_u32_with_check( handle, BLOB_NAME_VERS, vers, change);
}


bool settings_read_nvs(t_settings *set) {
  bool ret = false;
  nvs_handle handle;

  if (nvs_sync_lock( portMAX_DELAY)) {
    if (nvs_open(settings_nvs_namespace, NVS_READONLY, &handle) != ESP_OK) {
      nvs_sync_unlock();
      ESP_LOGE(SETTINGS_TAG, "%s not opened", settings_nvs_namespace);
      return false;
    }
    uint32_t vers = 0;
    if (!nvs_read_u32_with_check(handle, BLOB_NAME_VERS, &vers)
        || !(vers == default_set.vers)
//        || !settings_auth_read_nvs(handle, &set->auth)
        || !settings_ap_read_nvs(handle, &set->ap)
        || !settings_nw_read_nvs(handle, &set->nw)
//        || !settings_mqtt_read_nvs(handle, &set->mqtt)
        || !settings_leds_read_nvs(handle, &set->leds)
        || !settings_lang_read_nvs(handle, &set->lang)
        || !nvs_read_bool_with_check(handle, BLOB_NAME_NW_ENBL, &set->nw_enable)
//        || !nvs_read_bool_with_check(handle, BLOB_NAME_MQTT_ENBL, &set->mqtt_enable)
        ){
      goto Exit;
    }

    ret = true;
Exit:
    nvs_close(handle);
    nvs_sync_unlock();
  }

  return ret;
}

bool settings_write_nvs(t_settings *set) {
  bool change = false;
  bool ret = false;
  nvs_handle handle;

  if (nvs_sync_lock( portMAX_DELAY)) {
    if (nvs_open(settings_nvs_namespace, NVS_READWRITE, &handle) != ESP_OK) {
      nvs_sync_unlock();
      ESP_LOGE(SETTINGS_TAG, "%s not opened for write", settings_nvs_namespace);
      return false;
    }

    if( /*!settings_auth_write_nvs(handle, &set->auth, &change) ||*/ \
       !settings_lang_write_nvs(handle, set->lang.str, &change) ||
//       !nvs_write_bool_with_check(handle, BLOB_NAME_MQTT_ENBL, set->mqtt_enable, &change) ||
       !nvs_write_bool_with_check(handle, BLOB_NAME_NW_ENBL, set->nw_enable, &change) ||
//       !settings_mqtt_write_nvs(handle, &set->mqtt, &change) ||
       !settings_leds_write_nvs(handle, &set->leds, &change) ||
       !settings_ap_write_nvs(handle, &set->ap, &change) ||
       !settings_nw_write_nvs(handle, &set->nw, &change) ||
       !settings_vers_write_nvs(handle, set->vers, &change)){
      goto Exit;
    }

    if(change){
      ESP_LOGD(SETTINGS_TAG, "nvs_commit");
      nvs_commit(handle);
    }
    else{
      ESP_LOGD(SETTINGS_TAG, "Wifi config was not saved to flash because no change has been detected.");
    }

    ret = true;

Exit:
    nvs_close(handle);
    nvs_sync_unlock();
  }
  return ret;
}

bool settings_save(void){
  return settings_write_nvs(settings);
//  return true;
}

bool settings_init(void) {
  bool ret = false;

  ESP_LOGI(SETTINGS_TAG, "settings initialization");

  {
    nvs_stats_t nvs_stats;
    nvs_get_stats(NULL, &nvs_stats);
    ESP_LOGD(SETTINGS_TAG, "Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n", nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries);
  }

  ESP_ERROR_CHECK(nvs_sync_create()); /* semaphore for thread synchronization on NVS memory */

  if (!(settings = malloc(sizeof(t_settings)))) {
    goto Exit;
  }

  settings->vers = 0;
//  settings->auth.pswd[0] = 0;
  settings->ap.ssid[0] = 0;
  settings->ap.pswd[0] = 0;
  settings->nw.ssid[0] = 0;
  settings->nw.pswd[0] = 0;
//  settings->mqtt.host[0] = 0;
//  settings->mqtt.port = 0;
//  settings->mqtt.client[0] = 0;
//  settings->mqtt.user[0] = 0;
//  settings->mqtt.pswd[0] = 0;
//  settings->mqtt.topic[0] = 0;
  settings->lang.str[0] = 0;
  settings->leds.order[0] = 0;

  if (!get_default_settings()) {
    ESP_LOGE(SETTINGS_TAG, "Default settings no loaded");
    goto Exit;
  }

  if (settings_read_nvs(settings)) {
    ESP_LOGI(SETTINGS_TAG,"Loaded from nvs");
  }
  else {
    if(!settings_write_nvs(settings)){
      ESP_LOGE(SETTINGS_TAG, "NVS write error");
    }
  }

  ESP_LOGI(SETTINGS_TAG, "Sett. loaded");
//  ESP_LOGI(SETTINGS_TAG, "auth.pswd:%s", settings->auth.pswd);
  ESP_LOGI(SETTINGS_TAG, "ap.ssid:%s", settings->ap.ssid);
  ESP_LOGI(SETTINGS_TAG, "ap.pswd:%s", settings->ap.pswd);
  ESP_LOGI(SETTINGS_TAG, "nw.ssid:%s", settings->nw.ssid);
  ESP_LOGI(SETTINGS_TAG, "nw.pswd:%s", settings->nw.pswd);
  ESP_LOGI(SETTINGS_TAG, "leds.order:%s", settings->leds.order);
  ESP_LOGI(SETTINGS_TAG, "leds.cnt:%d", settings->leds.cnt);

//  ESP_LOGD(SETTINGS_TAG, "mqtt.host:%s", settings->mqtt.host);
//  ESP_LOGD(SETTINGS_TAG, "mqtt.port:%d", settings->mqtt.port);
//  ESP_LOGD(SETTINGS_TAG, "mqtt.client:%s", settings->mqtt.client);
//  ESP_LOGD(SETTINGS_TAG, "mqtt.user:%s", settings->mqtt.user);
//  ESP_LOGD(SETTINGS_TAG, "mqtt.pswd:%s", settings->mqtt.pswd);
//  ESP_LOGD(SETTINGS_TAG, "mqtt.topic:%s", settings->mqtt.topic);
  ret = true;

  Exit:
  if(!ret && settings){
    ESP_LOGE(SETTINGS_TAG, "no free memory");
//    free_setting_struct();
    check_and_free_memory(settings);
  }
  return ret;
}
