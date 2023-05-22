/*
 * settings.h
 *
 *  Created on: Dec 21, 2021
 *      Author: exeland
 */


#ifndef USER_COMPONENTS_SETTINGS_SETTINGS_H_
#define USER_COMPONENTS_SETTINGS_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SETTINGS_AUTH_PW_SZ      (32)
#define SETTINGS_SSID_MAX_SZ     (32)
#define SETTINGS_PW_MAX_SZ       (64)
#define SETTINGS_MQTT_HOST_SZ    (64)
#define SETTINGS_MQTT_CLIENT_SZ  (32)
#define SETTINGS_MQTT_USER_SZ    (32)
#define SETTINGS_MQTT_PW_SZ      (32)
#define SETTINGS_MQTT_TOPIC_SZ   (32)
#define SETTINGS_MQTT_FTOPIC_SZ  (64)
#define SETTINGS_LANG_SZ         (4)
#define SETTINGS_LEDS_ORDER_SZ   (4)

//#define STR_RGB  "RGB"
//#define STR_GBR  "GBR"
//#define STR_BRG  "BRG"
//#define STR_RBG  "RBG"
//#define STR_GRB  "GRB"
//#define STR_BGR  "BGR"
//
//typedef enum
//{
//    RGB = 1,
//    GBR = 2,
//    BRG = 3,
//    RBG = 4,
//    GRB = 5,
//    BGR = 6
//} t_color_order;

typedef struct{
  char   ssid[SETTINGS_SSID_MAX_SZ];
  char   pswd[SETTINGS_PW_MAX_SZ];
}t_settings_AP;

typedef struct{
  char   ssid[SETTINGS_SSID_MAX_SZ];
  char   pswd[SETTINGS_PW_MAX_SZ];
}t_settings_NW;

typedef struct{
  int            port;
  char           host[SETTINGS_MQTT_HOST_SZ];
  char           client[SETTINGS_MQTT_CLIENT_SZ];
  char           user[SETTINGS_MQTT_USER_SZ];
  char           pswd[SETTINGS_MQTT_PW_SZ];
  char           topic[SETTINGS_MQTT_TOPIC_SZ];
}t_settings_mqtt;

typedef struct{
  int            cnt;
  char           order[SETTINGS_LEDS_ORDER_SZ];
}t_settings_leds;

typedef struct{
  char  str[SETTINGS_LANG_SZ];
}t_settings_lang;

typedef struct{
  char  pswd[32];
}t_settings_auth;

typedef struct{
  int             vers;
  bool            nw_enable;
  t_settings_AP   ap;
  t_settings_NW   nw;
  t_settings_leds leds;
  t_settings_lang lang;
//  bool            mqtt_enable;
//  t_settings_auth auth;
//  t_settings_mqtt mqtt;
} t_settings;

bool settings_update_auth_pswd(char* ssid);
bool settings_update_ap_ssid(char* ssid);
bool settings_update_ap_pswd(char* pswd);
bool settings_update_nw_ssid(char* ssid);
bool settings_update_nw_pswd(char* pswd);
bool settings_update_mqtt_host(char* host);
bool settings_update_mqtt_client(char* client);
bool settings_update_mqtt_user(char* user);
bool settings_update_mqtt_pswd(char* pswd);
bool settings_update_mqtt_topic(char* topic);
bool settings_update_mqtt_full_topic(char* full_topic);
bool settings_update_lang(char* lang);
bool settings_update_leds_order(char* order);
bool settings_update_leds_cnt(int cnt);
bool settings_update_nw_enable(bool nw_enable);
bool settings_update_mqtt_enable(bool mqtt_enable);
bool settings_update_mqtt_port(int port);
bool settings_save(void);

bool settings_read_nvs(t_settings *set);
bool settings_write_nvs(t_settings *set);
t_settings* get_settings(void);
bool settings_init(void);
bool get_default_settings(void);

#ifdef __cplusplus
}
#endif

#endif /* USER_COMPONENTS_SETTINGS_SETTINGS_H_ */
