/*
 * json_maker.c
 *
 *  Created on: Dec 22, 2021
 *      Author: exeland
 */

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "esp_log.h"

#include "../cJSON/cJSON.h"
#include "../../main/led_animator.h"
#include "../settings/settings.h"
#include "json_maker.h"

#define JSON_MAKER_BUFF_LEN   512

static const char *TAG = "JSONMAKER";

SemaphoreHandle_t json_mutex = NULL;

//typedef struct s_meas_jsons{
//  cJSON *root;
//  cJSON *dev_type;
//  cJSON *dev_mode;
//  cJSON *num_starts;
//  cJSON *t_common;
//  cJSON *t_current;
//  cJSON *t_last;
//  cJSON *voltage;
//  cJSON *freq;
//}t_meas_jsons;

//typedef struct s_sett_auth_jsons{
//  cJSON *root;
//  cJSON *pw;
//}t_sett_auth_jsons;

typedef struct s_sett_ap_jsons{
  cJSON *root;
  cJSON *ssid;
  cJSON *pw;
}t_sett_ap_jsons;

typedef struct s_sett_nw_jsons{
  cJSON *root;
  cJSON *enable;
  cJSON *ssid;
  cJSON *pw;
}t_sett_nw_jsons;

typedef struct s_sett_mqtt_jsons{
  cJSON *root;
  cJSON *enable;
  cJSON  *host;
  cJSON  *port;
  cJSON  *client;
  cJSON  *user;
  cJSON  *pswd;
  cJSON  *topic;
  cJSON  *full_topic;
}t_sett_mqtt_jsons;

typedef struct s_sett_lang_jsons{
  cJSON *root;
  cJSON *value;
}t_sett_lang_jsons;

typedef struct s_sett_leds_jsons{
  cJSON *root;
  cJSON *cnt;
  cJSON *order;
}t_sett_leds_jsons;

typedef struct s_sett_jsons{
  cJSON *root;
//  t_sett_auth_jsons auth;
  t_sett_ap_jsons   ap;
  t_sett_nw_jsons   nw;
  t_sett_leds_jsons leds;
//  t_sett_mqtt_jsons mqtt;
  t_sett_lang_jsons lang;
}t_sett_jsons;

/*
//{
//    "AP":{
//        "ssid": "PE_WTM",
//        "pswd": "12345678"
//    },
//    "NW":{
//        "enbl": false,
//        "ssid": "AP",
//        "pswd": "12345678"
//    },
//    "MQTT":{
//        "enbl": false,
//        "hst": "",
//        "prt": "1883",
//        "clnt": "WTM_%06X",
//        "usr": "WTM_USER",
//        "pswd": "12345678",
//        "tpc" : "wtm",
//        "f_tpc" : "%prefix%/%topic%/"
//    },
//    "lang":{
//        "val" : "ru"
//    }
// }
*/

typedef struct s_status_jsons{
  cJSON *root;
  cJSON *color_r;
  cJSON *color_g;
  cJSON *color_b;
  cJSON *mode;
  cJSON *speed;
  cJSON *brghtns;
}t_status_jsons;

static char *json_print_buff = NULL;
//static char json_print_buff[JSON_MAKER_BUFF_LEN];

//static t_meas_jsons *meas_jsons = NULL;
static t_sett_jsons *sett_jsons = NULL;
static t_status_jsons *status_jsons = NULL;

bool lock_json_buffer(TickType_t xTicksToWait){
  if(json_mutex){
    if( xSemaphoreTake(json_mutex, xTicksToWait ) == pdTRUE ) {
      return true;
    }
    else{
      return false;
    }
  }
  else{
    return false;
  }

}

void unlock_json_buffer(void){
  xSemaphoreGive(json_mutex );
}

//char *get_wtm_json_data(void){
//  t_wtm_data* wtm_data = get_wtm_data();
//  cJSON_SetIntValue(meas_jsons->dev_type, wtm_data->device_type);
//  cJSON_SetIntValue(meas_jsons->dev_mode, wtm_data->mode);
//  cJSON_SetIntValue(meas_jsons->num_starts, wtm_data->start_count);
//  cJSON_SetIntValue(meas_jsons->t_common, wtm_data->common_time);
//  cJSON_SetIntValue(meas_jsons->t_current, wtm_data->current_time);
//  cJSON_SetIntValue(meas_jsons->t_last, wtm_data->power_time);
//  cJSON_SetIntValue(meas_jsons->voltage, wtm_data->voltage);
//  cJSON_SetIntValue(meas_jsons->freq, wtm_data->freq);
//  cJSON_PrintPreallocated(meas_jsons->root, json_print_buff, JSON_MAKER_BUFF_LEN, 0);
//  return json_print_buff;
//}

//char *get_wtm_json_data(void){
//  t_wtm_data* wtm_data = get_wtm_data();
//  cJSON_SetIntValue(meas_jsons->dev_type, wtm_data->device_type);
//  cJSON_SetIntValue(meas_jsons->dev_mode, wtm_data->mode);
//  cJSON_SetIntValue(meas_jsons->num_starts, wtm_data->start_count);
//  cJSON_SetIntValue(meas_jsons->t_common, wtm_data->common_time);
//  cJSON_SetIntValue(meas_jsons->t_current, wtm_data->current_time);
//  cJSON_SetIntValue(meas_jsons->t_last, wtm_data->power_time);
//  cJSON_SetIntValue(meas_jsons->voltage, wtm_data->voltage);
//  cJSON_SetIntValue(meas_jsons->freq, wtm_data->freq);
//  cJSON_PrintPreallocated(meas_jsons->root, json_print_buff, JSON_MAKER_BUFF_LEN, 0);
//  return json_print_buff;
//}

char *get_settings_json_data(void){
  t_settings* sett = get_settings();

//  cJSON_SetValuestring(sett_jsons->auth.pw, sett->auth.pswd);
  cJSON_SetValuestring(sett_jsons->ap.ssid, sett->ap.ssid);
  cJSON_SetValuestring(sett_jsons->ap.pw, sett->ap.pswd);

  cJSON_SetBoolValue(sett_jsons->nw.enable, sett->nw_enable);
  cJSON_SetValuestring(sett_jsons->nw.ssid, sett->nw.ssid);
  cJSON_SetValuestring(sett_jsons->nw.pw, sett->nw.pswd);

  cJSON_SetIntValue(sett_jsons->leds.cnt, sett->leds.cnt);
  cJSON_SetValuestring(sett_jsons->leds.order, sett->leds.order);

//  cJSON_SetBoolValue(sett_jsons->mqtt.enable, sett->mqtt_enable);
//  cJSON_SetValuestring(sett_jsons->mqtt.host, sett->mqtt.host);
//  cJSON_SetIntValue(sett_jsons->mqtt.port, sett->mqtt.port);
//  cJSON_SetValuestring(sett_jsons->mqtt.client, sett->mqtt.client);
//  cJSON_SetValuestring(sett_jsons->mqtt.user, sett->mqtt.user);
//  cJSON_SetValuestring(sett_jsons->mqtt.pswd, sett->mqtt.pswd);
//  cJSON_SetValuestring(sett_jsons->mqtt.topic, sett->mqtt.topic);
//  cJSON_SetValuestring(sett_jsons->lang.value, sett->lang.str);

  if (!cJSON_PrintPreallocated(sett_jsons->root, json_print_buff, JSON_MAKER_BUFF_LEN, 0)) {
    ESP_LOGE(TAG, "cJSON_PrintPreallocated failed!\n");
  }
  return json_print_buff;
}

char *get_status_json_data(void){
  int r, g, b;
  led_animator_getcolor(&r, &g, &b);

  cJSON_SetIntValue(status_jsons->brghtns, led_animator_getbrightness());
  cJSON_SetIntValue(status_jsons->mode, led_animator_getmode());
  cJSON_SetIntValue(status_jsons->speed, led_animator_getspeed());
  cJSON_SetIntValue(status_jsons->color_r, r);
  cJSON_SetIntValue(status_jsons->color_g, g);
  cJSON_SetIntValue(status_jsons->color_b, b);

  if (!cJSON_PrintPreallocated(status_jsons->root, json_print_buff, JSON_MAKER_BUFF_LEN, 1)) {
    ESP_LOGE(TAG, "cJSON_PrintPreallocated failed!\n");
  }
  return json_print_buff;
}

void json_maker_init(void){
//  t_wtm_data* wtm_data = get_wtm_data();
  t_settings* sett = get_settings();
  ESP_LOGI(TAG, "Start");

//  meas_jsons = (t_meas_jsons*) malloc(sizeof(t_meas_jsons));
  sett_jsons = (t_sett_jsons*) malloc(sizeof(t_sett_jsons));
  status_jsons = (t_status_jsons*) malloc(sizeof(t_status_jsons));
  json_print_buff = malloc(JSON_MAKER_BUFF_LEN);

  if((sett_jsons == NULL) \
     ||(status_jsons == NULL) \
     ||(json_print_buff == NULL)){
    ESP_LOGE(TAG, "memory");
    return;
  }

  json_mutex = xSemaphoreCreateMutex();

  status_jsons->root = cJSON_CreateObject();
  status_jsons->mode = cJSON_AddNumberToObject(status_jsons->root, "mode", 1);
  status_jsons->speed = cJSON_AddNumberToObject(status_jsons->root, "speed", 1);
  status_jsons->brghtns = cJSON_AddNumberToObject(status_jsons->root, "brghtns", 1);
  cJSON *color = cJSON_AddArrayToObject(status_jsons->root, "color");
  status_jsons->color_r = cJSON_CreateNumber(1);
  status_jsons->color_g = cJSON_CreateNumber(1);
  status_jsons->color_b = cJSON_CreateNumber(1);
  cJSON_AddItemToArray(color, status_jsons->color_r);
  cJSON_AddItemToArray(color, status_jsons->color_g);
  cJSON_AddItemToArray(color, status_jsons->color_b);

  if((status_jsons->root == NULL) \
     ||(status_jsons->mode == NULL) \
     ||(status_jsons->speed == NULL) \
     ||(status_jsons->color_r == NULL) \
     ||(status_jsons->color_g == NULL) \
     ||(status_jsons->color_b == NULL) \
     ||(color == NULL)){
    ESP_LOGE(TAG, "memory");
    return;
  }

  sett_jsons->root = cJSON_CreateObject();

//  sett_jsons->auth.root = cJSON_AddObjectToObject(sett_jsons->root, "auth");
//  sett_jsons->auth.pw = cJSON_AddStringToObject(sett_jsons->auth.root, "pswd", sett->auth.pswd);

  sett_jsons->ap.root = cJSON_AddObjectToObject(sett_jsons->root, "AP");
  sett_jsons->ap.ssid = cJSON_AddStringToObject(sett_jsons->ap.root, "ssid",sett->ap.ssid);
  sett_jsons->ap.pw = cJSON_AddStringToObject(sett_jsons->ap.root, "pswd", sett->ap.pswd);

  sett_jsons->nw.root = cJSON_AddObjectToObject(sett_jsons->root, "NW");
  sett_jsons->nw.enable = cJSON_AddBoolToObject(sett_jsons->nw.root, "enbl", sett->nw_enable);
  sett_jsons->nw.ssid = cJSON_AddStringToObject(sett_jsons->nw.root, "ssid", sett->nw.ssid);
  sett_jsons->nw.pw = cJSON_AddStringToObject(sett_jsons->nw.root, "pswd", sett->nw.pswd);

  sett_jsons->leds.root = cJSON_AddObjectToObject(sett_jsons->root, "LEDS");
  sett_jsons->leds.cnt = cJSON_AddNumberToObject(sett_jsons->leds.root, "cnt", sett->leds.cnt);
  sett_jsons->leds.order = cJSON_AddStringToObject(sett_jsons->leds.root, "order", sett->leds.order);

//  sett_jsons->mqtt.root = cJSON_AddObjectToObject(sett_jsons->root, "MQTT");
//  sett_jsons->mqtt.enable = cJSON_AddBoolToObject(sett_jsons->mqtt.root, "enbl", sett->mqtt_enable);
//  sett_jsons->mqtt.host = cJSON_AddStringToObject(sett_jsons->mqtt.root, "hst", sett->mqtt.host);
//  sett_jsons->mqtt.port = cJSON_AddNumberToObject(sett_jsons->mqtt.root, "prt", sett->mqtt.port);
//  sett_jsons->mqtt.client = cJSON_AddStringToObject(sett_jsons->mqtt.root, "clnt", sett->mqtt.client);
//  sett_jsons->mqtt.user = cJSON_AddStringToObject(sett_jsons->mqtt.root, "usr", sett->mqtt.user);
//  sett_jsons->mqtt.pswd = cJSON_AddStringToObject(sett_jsons->mqtt.root, "pswd", sett->mqtt.pswd);
//  sett_jsons->mqtt.topic = cJSON_AddStringToObject(sett_jsons->mqtt.root, "tpc", sett->mqtt.topic);

  sett_jsons->lang.root = cJSON_AddObjectToObject(sett_jsons->root, "lang");
  sett_jsons->lang.value = cJSON_AddStringToObject(sett_jsons->lang.root, "val",sett->lang.str);

  ESP_LOGI(TAG, "Done");

}
