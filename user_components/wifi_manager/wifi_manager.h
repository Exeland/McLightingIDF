/*
 * wifi_manger.c
 *
 *  Created on: Dec 15, 2021
 *      Author: exeland
 */

#ifndef _WIFI_MANGER_H_
#define _WIFI_MANGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_wifi.h"
/**
 * @brief Defines the maximum size of a SSID name. 32 is IEEE standard.
 * @warning limit is also hard coded in wifi_config_t. Never extend this value.
 */
#define MAX_SSID_SIZE           32

/**
 * @brief Defines the maximum size of a WPA2 passkey. 64 is IEEE standard.
 * @warning limit is also hard coded in wifi_config_t. Never extend this value.
 */
#define MAX_PASSWORD_SIZE         64


/**
 * @brief Defines the maximum number of access points that can be scanned.
 *
 * To save memory and avoid nasty out of memory errors,
 * we can limit the number of APs detected in a wifi scan.
 */
#define MAX_AP_NUM              15

/** @brief Defines visibility of the access point. 0: visible AP. 1: hidden */
#define DEFAULT_AP_SSID_HIDDEN        0

/** @brief Defines the hostname broadcasted by mDNS */
#define DEFAULT_HOSTNAME          "esp32"

/** @brief Defines access point's bandwidth.
 *  Value: WIFI_BW_HT20 for 20 MHz  or  WIFI_BW_HT40 for 40 MHz
 *  20 MHz minimize channel interference but is not suitable for
 *  applications with high data speeds
 */
#define DEFAULT_AP_BANDWIDTH          WIFI_BW_HT20

/** @brief Defines if esp32 shall run both AP + STA when connected to another AP.
 *  Value: 0 will have the own AP always on (APSTA mode)
 *  Value: 1 will turn off own AP when connected to another AP (STA only mode when connected)
 *  Turning off own AP when connected to another AP minimize channel interference and increase throughput
 */
#define DEFAULT_STA_ONLY          1

/** @brief Defines if wifi power save shall be enabled.
 *  Value: WIFI_PS_NONE for full power (wifi modem always on)
 *  Value: WIFI_PS_MODEM for power save (wifi modem sleep periodically)
 *  Note: Power save is only effective when in STA only mode
 */
#define DEFAULT_STA_POWER_SAVE        WIFI_PS_NONE
/**
 * @brief defines the minimum length of an access point password running on WPA2
 */
#define WPA2_MINIMUM_PASSWORD_LENGTH    8

/**
 * @brief Defines the complete list of all messages that the wifi_manager can process.
 *
 * Some of these message are events ("EVENT"), and some of them are action ("ORDER")
 * Each of these messages can trigger a callback function and each callback function is stored
 * in a function pointer array for convenience. Because of this behavior, it is extremely important
 * to maintain a strict sequence and the top level special element 'MESSAGE_CODE_COUNT'
 *
 * @see wifi_manager_set_callback
 */
typedef enum message_code_t {
  NONE = 0,
  WM_ORDER_START_HTTP_SERVER = 1,
  WM_ORDER_STOP_HTTP_SERVER = 2,
  WM_ORDER_START_DNS_SERVICE = 3,
  WM_ORDER_STOP_DNS_SERVICE = 4,
  WM_ORDER_START_WIFI_SCAN = 5,
  WM_ORDER_LOAD_AND_RESTORE_STA = 6,
  WM_ORDER_CONNECT_STA = 7,
  WM_ORDER_DISCONNECT_STA = 8,
  WM_ORDER_START_AP = 9,
  WM_EVENT_STA_DISCONNECTED = 10,
  WM_EVENT_SCAN_DONE = 11,
  WM_EVENT_STA_GOT_IP = 12,
  WM_ORDER_STOP_AP = 13,
  WM_MESSAGE_CODE_COUNT = 14 /* important for the callback array */

}message_code_t;

typedef enum connection_request_made_by_code_t{
  CONNECTION_REQUEST_NONE = 0,
  CONNECTION_REQUEST_USER = 1,
  CONNECTION_REQUEST_AUTO_RECONNECT = 2,
  CONNECTION_REQUEST_RESTORE_CONNECTION = 3,
  CONNECTION_REQUEST_MAX = 0x7fffffff /*force the creation of this enum as a 32 bit int */
}connection_request_made_by_code_t;

typedef struct{
  message_code_t code;
  void *param;
} queue_message;

/**
 * The actual WiFi settings in use
 */
struct wifi_manager_settings_t{
  uint8_t ap_ssid[MAX_SSID_SIZE];
  uint8_t ap_pwd[MAX_PASSWORD_SIZE];
  uint8_t ap_channel;
  uint8_t ap_ssid_hidden;
  wifi_bandwidth_t ap_bandwidth;
  bool sta_only;
  wifi_ps_type_t sta_power_save;
  bool sta_static_ip;
  tcpip_adapter_ip_info_t sta_static_ip_config; // esp_netif_ip_info_t sta_static_ip_config
};
extern struct wifi_manager_settings_t wifi_manager_settings;
/**
 * @brief Start the mDNS service
 */
void wifi_manager_initialise_mdns();


bool wifi_manager_lock_sta_ip_string(TickType_t xTicksToWait);
void wifi_manager_unlock_sta_ip_string();
/**
 * @brief gets the string representation of the STA IP address, e.g.: "192.168.1.69"
 */
char* wifi_manager_get_sta_ip_string();
/**
 * @brief thread safe char representation of the STA IP update
 */
void wifi_manager_safe_update_sta_ip_string(uint32_t ip);

BaseType_t wifi_manager_send_message(message_code_t code, void *param);
BaseType_t wifi_manager_send_message_to_front(message_code_t code, void *param);

void wifi_manager_get_esp_ip_info_ap(tcpip_adapter_ip_info_t* ip);
void wifi_manager_get_esp_ip_info_sta(tcpip_adapter_ip_info_t* ip);

/**
 * Allocate heap memory for the wifi manager and start the wifi_manager RTOS task
 */
void wifi_manager_start();

/**
 * Frees up all memory allocated by the wifi_manager and kill the task.
 */
void wifi_manager_destroy();

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_MANGER_H_ */
