/*
 * http_app.c
 *
 *  Created on: Jul 2, 2021
 *      Author: exeland
 */
/*
 Copyright (c) 2017-2020 Tony Pottier

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 @file http_app.c
 @author Tony Pottier
 @brief Defines all functions necessary for the HTTP server to run.

 Contains the freeRTOS task for the HTTP listener and all necessary support
 function to process requests, decode URLs, serve files, etc. etc.

 @note http_server task cannot run without the wifi_manager task!
 @see https://idyl.io
 @see https://github.com/tonyp7/esp32-wifi-manager
 */
/* An HTTP GET handler */

#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include "../user_components/esp_http_server/include/esp_http_server.h"
#include "../user_components/json_maker/json_maker.h"
#include "../user_components/wifi_manager/wifi_manager.h"
#include "../user_components/shell/shell.h"
#include "../user_components/settings/settings.h"
#include "../user_components/ota_http_updater/ota_http_updater.h"

#include "urlencode.h"
#include "webfiles.h"
#include "../main/led_animator.h"

#include "http_app.h"

static const char *TAG = "http_server";

/* @brief the HTTP server handle */
static httpd_handle_t httpd_handle = NULL;

/* function pointers to URI handlers that can be user made */
esp_err_t
(*custom_get_httpd_uri_handler) (httpd_req_t *r) = NULL;
esp_err_t
(*custom_post_httpd_uri_handler) (httpd_req_t *r) = NULL;

const char http_redirect_url[] = "http://" CONFIG_DEFAULT_AP_IP;
const char http_root_url[] = CONFIG_WEBAPP_LOCATION;
const char http_index_url[] = CONFIG_WEBAPP_LOCATION "index.html";
const char http_modes_url[] = CONFIG_WEBAPP_LOCATION "modes.html";
const char http_wheel_url[] = CONFIG_WEBAPP_LOCATION "wheel.html";
const char http_about_url[] = CONFIG_WEBAPP_LOCATION "about.html";
const char http_settings_url[] = CONFIG_WEBAPP_LOCATION "settings.html";
const char http_upgrade_url[] = CONFIG_WEBAPP_LOCATION "upgrade.html";
const char http_css_url[] = CONFIG_WEBAPP_LOCATION "style.css";

const char http_modesjs_url[] = CONFIG_WEBAPP_LOCATION "js/modes.js";
const char http_settingsjs_url[] = CONFIG_WEBAPP_LOCATION "js/settings.js";
const char http_sitejs_url[] = CONFIG_WEBAPP_LOCATION "js/site.js";
const char http_upgradejs_url[] = CONFIG_WEBAPP_LOCATION "js/upgrade.js";
const char http_wheeljs_url[] = CONFIG_WEBAPP_LOCATION "js/wheel.js";
const char http_contoljs_url[] = CONFIG_WEBAPP_LOCATION "js/control.js";

const char http_modesjson_url[] = CONFIG_WEBAPP_LOCATION "modes.json";
const char http_settingsjson_url[] = CONFIG_WEBAPP_LOCATION "settings.json";
const char http_statusjson_url[] = CONFIG_WEBAPP_LOCATION "state.json";
const char http_defaultlang_url[] = CONFIG_WEBAPP_LOCATION "lang/default.lang";
const char http_run_cmd_url[] = CONFIG_WEBAPP_LOCATION "run?cmd=";

/* all the pages */
//const char page_css[] = "style.css";
//const char page_modes[] = "modes.html";
//const char page_settings[] = "settings.html";
//const char page_about[] = "about.html";
//const char page_upgrade[] = "upgrade.html";
//const char page_pswd[] = "passwrd.html";
//const char page_wheel[] = "wheel.html";
//const char page_modes_js[] = "js/modes.js";
//const char page_wheel_js[] = "js/wheel.js";
//const char page_http_site_js[] = "js/site.js";
//const char page_http_upgrade_js[] = "js/upgrade.js";
//const char page_http_settings_js[] = "js/settings.js";
//const char page_http_modes_json[] = "modes.json";
//const char page_http_settings_json[] = "settings.json";
//const char page_http_defaultlang_url[] = "lang/default.lang";
//const char page_http_run_cmd_url[] = "run?cmd=";
//const char page_http_ota_url[] = "run?cmd=ota";
/* const httpd related values stored in ROM */
const static char http_200_hdr[] = "200 OK";
const static char http_302_hdr[] = "302 Found";
//const static char http_400_hdr[] = "400 Bad Request";
const static char http_404_hdr[] = "404 Not Found";
const static char http_500_hdr[] = "500 Internal Server Error";
const static char http_503_hdr[] = "503 Service Unavailable";
const static char http_location_hdr[] = "Location";
const static char http_content_type_html[] = "text/html";
const static char http_content_type_js[] = "text/javascript";
const static char http_content_type_css[] = "text/css";
const static char http_content_type_json[] = "application/json";
const static char http_content_type_text[] = "text/plain";
const static char http_cache_control_hdr[] = "Cache-Control";
const static char http_content_encodin_hdr[] = "Content-Encoding";
const static char http_content_encodin_gzip[] = "gzip";
const static char http_cache_control_no_cache[] = "no-store, no-cache, must-revalidate, max-age=0";
const static char http_cache_control_cache[] = "public, max-age=31536000";
const static char http_pragma_hdr[] = "Pragma";
const static char http_pragma_no_cache[] = "no-cache";
const static char http_pragma_ok[] = "OK";
const static char http_load_succes[] = "File uploaded successfully - reboot in 5s!";
const static char  Mess_Error_Allocmem[] = "Could not allocate buffer memory!";

//static bool login_flg = false;
//static bool login_flg = true;
//
//static bool check_login(char* pswd){
//  t_settings* sett = get_settings();
//  return (strcmp(sett->auth.pswd, pswd) == 0);
//}

esp_err_t
http_app_set_handler_hook (httpd_method_t method, esp_err_t
(*handler) (httpd_req_t *r))
{

  if (method == HTTP_GET)
    {
      custom_get_httpd_uri_handler = handler;
      return ESP_OK;
    }
  else if (method == HTTP_POST)
    {
      custom_post_httpd_uri_handler = handler;
      return ESP_OK;
    }
  else
    {
      return ESP_ERR_INVALID_ARG;
    }

}

void
get_json_lang (const char *lang[], size_t *sz)
{
  t_settings *set = get_settings ();

  if (strcmp (set->lang.str, "en") == 0)
    {
      *lang = enlang;
      *sz = sizeof(enlang);
    }
  else
    {
      *lang = rulang;
      *sz = sizeof(rulang);
    }
}

static esp_err_t
http_server_delete_handler (httpd_req_t *req)
{

  ESP_LOGD(TAG, "DELETE %s", req->uri);

  /* DELETE /connect.json */
//    if(strcmp(req->uri, http_connect_url) == 0){
////        wifi_manager_disconnect_async();
//
//        httpd_resp_set_status(req, http_200_hdr);
//        httpd_resp_set_type(req, http_content_type_json);
//        httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
//        httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
//        httpd_resp_send(req, NULL, 0);
//    }
//    if(strcmp(req->uri, http_connect_url) == 0){
//
//    }
//    else{
  httpd_resp_set_status (req, http_404_hdr);
  httpd_resp_send (req, NULL, 0);
//    }

  return ESP_OK;
}

static esp_err_t
http_server_post_handler (httpd_req_t *req)
{

  esp_err_t ret = ESP_OK;

  ESP_LOGD(TAG, "POST %s", req->uri);

//    /* POST /connect.json */
//    if(strcmp(req->uri, http_connect_url) == 0){
//
//
//        /* buffers for the headers */
//        size_t ssid_len = 0, password_len = 0;
//        char *ssid = NULL, *password = NULL;
//
//        /* len of values provided */
//        ssid_len = httpd_req_get_hdr_value_len(req, "X-Custom-ssid");
//        password_len = httpd_req_get_hdr_value_len(req, "X-Custom-pwd");
//
//
//        if(ssid_len && ssid_len <= 32 && password_len && password_len <= 64){
//
//            /* get the actual value of the headers */
//            ssid = malloc(sizeof(char) * (ssid_len + 1));
//            password = malloc(sizeof(char) * (password_len + 1));
//            httpd_req_get_hdr_value_str(req, "X-Custom-ssid", ssid, ssid_len+1);
//            httpd_req_get_hdr_value_str(req, "X-Custom-pwd", password, password_len+1);
//
//            wifi_config_t* config;
//          config = wifi_manager_get_wifi_sta_config();
//            memset(config, 0x00, sizeof(wifi_config_t));
//            memcpy(config->sta.ssid, ssid, ssid_len);
//            memcpy(config->sta.password, password, password_len);
//            ESP_LOGD(TAG, "ssid: %s, password: %s", ssid, password);
//            ESP_LOGD(TAG, "http_server_post_handler: wifi_manager_connect_async() call");
//            wifi_manager_connect_async();
//
//            /* free memory */
//            free(ssid);
//            free(password);
//
//            httpd_resp_set_status(req, http_200_hdr);
//            httpd_resp_set_type(req, http_content_type_json);
//            httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
//            httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
//            httpd_resp_send(req, NULL, 0);
//
//        }
//        else{
//            /* bad request the authentification header is not complete/not the correct format */
//            httpd_resp_set_status(req, http_400_hdr);
//            httpd_resp_send(req, NULL, 0);
//        }
  if ((strstr(req->uri, http_run_cmd_url) == req->uri)) {
    const char *cmnd = &req->uri[9];

    if (strstr(cmnd, "ota") == cmnd) {
#define BUFFSIZE 0x2000

      int remaining = req->content_len;
      int binary_file_length = 0;
      ESP_LOGD(TAG, "File size : %d bytes", remaining);

      char *buf = (char*) malloc(BUFFSIZE);
      if (!buf) {
        ESP_LOGE(TAG, "Could not allocate memory!");
        /* Respond with 400 Bad Request */
        httpd_resp_set_status(req, http_404_hdr);
        httpd_resp_send(req, Mess_Error_Allocmem, sizeof(Mess_Error_Allocmem));
        return ESP_FAIL;
      }

      ota_http_updater_start();

      size_t data_read;

      ESP_LOGD(TAG, "Receiving file : ...");

      while (remaining > 0) {
        ESP_LOGD(TAG, "Remaining size : %d", remaining);
        if ((data_read = httpd_req_recv(req, buf, MIN(remaining, BUFFSIZE))) <= 0) {
          if (data_read == HTTPD_SOCK_ERR_TIMEOUT) {
            /* Retry if timeout occurred */
            continue;
          }

          ESP_LOGE(TAG, "File reception failed!");
          /* Respond with 500 Internal Server Error */
          httpd_resp_set_status(req, http_500_hdr);
          httpd_resp_send(req, NULL, 0);
          return ESP_FAIL;
        }

        if (ota_http_updater_write(buf, data_read) != ESP_OK) {
          /* Respond with 500 Internal Server Error */
          ESP_LOGE(TAG, "File reception failed!");
          httpd_resp_set_status(req, http_500_hdr);
          httpd_resp_send(req, NULL, 0);
          return ESP_FAIL;
        }

        binary_file_length += data_read;
        ESP_LOGD(TAG, "Written image length %d", binary_file_length);

        /* Keep track of remaining size of
         * the file left to be uploaded */
        remaining -= data_read;
      }

      ESP_LOGD(TAG, "Total Write binary data length: %d", binary_file_length);

      ota_http_updater_stop();
      httpd_resp_set_status(req, http_200_hdr);
      httpd_resp_set_type(req, http_content_type_text);
      httpd_resp_send(req, http_load_succes, sizeof(http_load_succes));
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      ESP_LOGD(TAG, "Restart");
      esp_restart();
    }
//    else if (strstr(cmnd, "pswd") == cmnd) {
//      char line[64];
//      url_decode(line, &cmnd[7]);
//      ESP_LOGD(TAG, "Pass: %s", line);
//      login_flg = check_login(line);
//      if(login_flg){
//        ESP_LOGD(TAG, "Success entered");
//        httpd_resp_set_status(req, http_200_hdr);
//        httpd_resp_set_type(req, http_content_type_text);
//        httpd_resp_send(req, http_pragma_ok, sizeof(http_pragma_ok));
//      }
//      else{
//        httpd_resp_set_status(req, http_404_hdr);
//        httpd_resp_send(req, NULL, 0);
//      }
//    }
    }
  else
    {
      if (custom_post_httpd_uri_handler == NULL)
        {
          httpd_resp_set_status (req, http_404_hdr);
          httpd_resp_send (req, NULL, 0);
        }
      else
        {

          /* if there's a hook, run it */
          ret = (*custom_post_httpd_uri_handler) (req);
        }
    }

  return ret;
}

static void
http_server_req_redirect (httpd_req_t *req, const char *http_redirect_url)
{
  /* Captive Portal functionality */
  /* 302 Redirect to IP of the access point */
  httpd_resp_set_status (req, http_302_hdr);
  httpd_resp_set_hdr (req, http_location_hdr, http_redirect_url);
  httpd_resp_send (req, NULL, 0);
}

static void
http_server_req_gzip_html (httpd_req_t *req, const char *p, size_t len)
{
  httpd_resp_set_status (req, http_200_hdr);
  httpd_resp_set_type (req, http_content_type_html);
  httpd_resp_set_hdr (req, http_content_encodin_hdr, http_content_encodin_gzip);
  httpd_resp_set_hdr (req, http_cache_control_hdr, http_cache_control_cache);
  httpd_resp_send (req, p, len);
}

static void
http_server_req_gzip_js (httpd_req_t *req, const char *p, size_t len)
{
  httpd_resp_set_status (req, http_200_hdr);
  httpd_resp_set_type (req, http_content_type_js);
  httpd_resp_set_hdr (req, http_content_encodin_hdr, http_content_encodin_gzip);
  httpd_resp_set_hdr (req, http_cache_control_hdr, http_cache_control_cache);
  httpd_resp_send (req, p, len);
}

static void
http_server_req_gzip_css (httpd_req_t *req, const char *p, size_t len)
{
  httpd_resp_set_status (req, http_200_hdr);
  httpd_resp_set_type (req, http_content_type_css);
  httpd_resp_set_hdr (req, http_content_encodin_hdr, http_content_encodin_gzip);
  httpd_resp_set_hdr (req, http_cache_control_hdr, http_cache_control_cache);
  httpd_resp_send (req, p, len);
}

static void
http_server_req_json (httpd_req_t *req, const char *p, size_t len)
{
  httpd_resp_set_status (req, http_200_hdr);
  httpd_resp_set_type (req, http_content_type_json);
  httpd_resp_set_hdr (req, http_cache_control_hdr, http_cache_control_no_cache);
  httpd_resp_set_hdr (req, http_pragma_hdr, http_pragma_no_cache);
  httpd_resp_send (req, p, len);
}

static void
http_server_req_default_lang (httpd_req_t *req)
{
  const char *str_lang_json;
  size_t json_sz;
  get_json_lang (&str_lang_json, &json_sz);

  if (str_lang_json)
    {
      ESP_LOGD(TAG, "Serving page /default.lang");
      httpd_resp_set_status (req, http_200_hdr);
      httpd_resp_set_type (req, http_content_type_json);
      httpd_resp_set_hdr (req, http_content_encodin_hdr,
                          http_content_encodin_gzip);
      httpd_resp_set_hdr (req, http_cache_control_hdr,
                          http_cache_control_cache);
      httpd_resp_send (req, str_lang_json, json_sz);
    }
  else
    {
      httpd_resp_set_status (req, http_503_hdr);
      httpd_resp_send (req, NULL, 0);
      ESP_LOGE(
          TAG,
          "http_server_netconn_serve: GET /status.json failed to obtain mutex");
    }
}

static esp_err_t
http_server_get_handler (httpd_req_t *req)
{

  char *host = NULL;
  size_t buf_len;
  esp_err_t ret = ESP_OK;

  ESP_LOGD(TAG, "GET %s", req->uri);

  /* Get header value string length and allocate memory for length + 1,
   * extra byte for null termination */
  buf_len = httpd_req_get_hdr_value_len (req, "Host") + 1;
  if (buf_len > 1)
    {
      host = malloc (buf_len);
      if (httpd_req_get_hdr_value_str (req, "Host", host, buf_len) != ESP_OK)
        {
          /* if something is wrong we just 0 the whole memory */
          memset (host, 0x00, buf_len);
        }
    }

  /* determine if Host is from the STA IP address */
  wifi_manager_lock_sta_ip_string (portMAX_DELAY);
  bool access_from_sta_ip =
      host != NULL ? strstr (host, wifi_manager_get_sta_ip_string ()) : false;
  wifi_manager_unlock_sta_ip_string ();

  if ((host != NULL && !strstr (host, CONFIG_DEFAULT_AP_IP)
      && !access_from_sta_ip))
    {
      http_server_req_redirect (req, http_redirect_url);
    }
//  else if(!login_flg){
//    if (strcmp(req->uri, http_root_url) == 0) {
//      http_server_req_gzip_html(req, indexhtml, sizeof(indexhtml));
//    }
//    /* GET /site.js */
//    else if (strcmp(req->uri, http_sitejs_url) == 0) {
//      ESP_LOGD(TAG, "Serving page js/site.js");
//      http_server_req_gzip_js(req, sitejs, sizeof(sitejs));
//    }
//    /* GET /style.css */
//    else if (strcmp(req->uri, http_css_url) == 0) {
//      http_server_req_gzip_css(req, stylecss, sizeof(stylecss));
//    }
//    else if (strcmp(req->uri, http_defaultlang_url) == 0) {
//      http_server_req_default_lang(req);
//    }
//    else {
//      ESP_LOGD(TAG, "Rejected url: %s", req->uri);
//      httpd_resp_set_status(req, http_404_hdr);
//      httpd_resp_send(req, NULL, 0);
//    }
//  }
  else
    {
      /* GET /  */
      ESP_LOGD(TAG, "Serving page: %s", req->uri);
      if (strcmp (req->uri, http_root_url) == 0)
        {
          http_server_req_gzip_html (req, indexhtml, sizeof(indexhtml));
        }
      else if (strcmp (req->uri, http_index_url) == 0)
        {
          http_server_req_gzip_html (req, indexhtml, sizeof(indexhtml));
        }
      else if (strcmp (req->uri, http_modes_url) == 0)
        {
          http_server_req_gzip_html (req, modeshtml, sizeof(modeshtml));
        }
      else if (strcmp (req->uri, http_wheel_url) == 0)
        {
          http_server_req_gzip_html (req, wheelhtml, sizeof(wheelhtml));
        }
      else if (strcmp (req->uri, http_settings_url) == 0)
        {
          http_server_req_gzip_html (req, settingshtml, sizeof(settingshtml));
        }
      else if (strcmp (req->uri, http_about_url) == 0)
        {
          http_server_req_gzip_html (req, abouthtml, sizeof(abouthtml));
        }
      else if (strcmp (req->uri, http_upgrade_url) == 0)
        {
          http_server_req_gzip_html (req, upgradehtml, sizeof(upgradehtml));
        }
      /* GET /modes.js */
      else if (strcmp (req->uri, http_modesjs_url) == 0)
        {
          http_server_req_gzip_js (req, modesjs, sizeof(modesjs));
        }
      /* GET /wheel.js */
      else if (strcmp (req->uri, http_wheeljs_url) == 0)
        {
          http_server_req_gzip_js (req, wheeljs, sizeof(wheeljs));
        }
      /* GET /wheel.js */
      else if (strcmp (req->uri, http_contoljs_url) == 0)
        {
          http_server_req_gzip_js (req, controljs, sizeof(controljs));
        }
      /* GET /settings.js */
      else if (strcmp (req->uri, http_settingsjs_url) == 0)
        {
          http_server_req_gzip_js (req, settingsjs, sizeof(settingsjs));
        }
      /* GET /site.js */
      else if (strcmp (req->uri, http_sitejs_url) == 0)
        {
          http_server_req_gzip_js (req, sitejs, sizeof(sitejs));
        }
      /* GET /settings.js */
      else if (strcmp (req->uri, http_upgradejs_url) == 0)
        {
          http_server_req_gzip_js (req, upgradejs, sizeof(upgradejs));
        }
      /* GET /style.css */
      else if (strcmp (req->uri, http_css_url) == 0)
        {
          http_server_req_gzip_css (req, stylecss, sizeof(stylecss));
        }
      /* GET /modes.json */
      else if (strcmp (req->uri, http_modesjson_url) == 0)
        {
          char *modes = (char*) led_animator_get_JSON_modes_names ();
//      if(lock_json_buffer(( TickType_t ) 10)){
//        char *str_meas_json = get_wtm_json_data();
          http_server_req_json (req, modes, strlen (modes));
//        unlock_json_buffer();
//      }
//      else {
//        httpd_resp_set_status(req, http_503_hdr);
//        httpd_resp_send(req, NULL, 0);
//        ESP_LOGE(TAG, "http_server_netconn_serve: GET /status.json failed to obtain mutex");
//      }
        }
      else if (strcmp (req->uri, http_settingsjson_url) == 0)
        {
          if (lock_json_buffer ((TickType_t) 10))
            {
              char *str_json = get_settings_json_data ();
              ESP_LOGD(TAG, "JSON:%s", str_json);
              http_server_req_json (req, str_json, strlen (str_json));
              unlock_json_buffer ();
            }
          else
            {
              httpd_resp_set_status (req, http_503_hdr);
              httpd_resp_send (req, NULL, 0);
              ESP_LOGE(
                  TAG,
                  "http_server_netconn_serve: GET /settingsjson.json failed to obtain mutex");
            }
        }
      else if (strcmp (req->uri, http_statusjson_url) == 0)
        {
          if (lock_json_buffer ((TickType_t) 10))
            {
              char *str_json = get_status_json_data ();
              ESP_LOGD(TAG, "JSON:%s", str_json);
              http_server_req_json (req, str_json, strlen (str_json));
              unlock_json_buffer ();
            }
          else
            {
              httpd_resp_set_status (req, http_503_hdr);
              httpd_resp_send (req, NULL, 0);
              ESP_LOGE(
                  TAG,
                  "http_server_netconn_serve: GET /settingsjson.json failed to obtain mutex");
            }
        }
      else if (strcmp (req->uri, http_defaultlang_url) == 0)
        {
          http_server_req_default_lang (req);
        }
//        /* GET /status.json */
//        else if(strcmp(req->uri, http_status_url) == 0){
//
//            if(wifi_manager_lock_json_buffer(( TickType_t ) 10)){
//                char *buff = wifi_manager_get_ip_about_json();
//                if(buff){
//                    httpd_resp_set_status(req, http_200_hdr);
//                    httpd_resp_set_type(req, http_content_type_json);
//                    httpd_resp_set_hdr(req, http_cache_control_hdr, http_cache_control_no_cache);
//                    httpd_resp_set_hdr(req, http_pragma_hdr, http_pragma_no_cache);
//                    httpd_resp_send(req, buff, strlen(buff));
//                    wifi_manager_unlock_json_buffer();
//                }
//                else{
//                    httpd_resp_set_status(req, http_503_hdr);
//                    httpd_resp_send(req, NULL, 0);
//                }
//            }
//            else{
//                httpd_resp_set_status(req, http_503_hdr);
//                httpd_resp_send(req, NULL, 0);
//                ESP_LOGE(TAG, "http_server_netconn_serve: GET /status.json failed to obtain mutex");
//            }
//        }

      else if (strstr (req->uri, http_run_cmd_url) == req->uri)
        {
          char *cmnd;
          char line[64];
          url_decode (line, req->uri);
          cmnd = &line[9];
          ESP_LOGD(TAG, "Serving command url: %s", cmnd);

          if (lock_shell ((TickType_t) 10))
            {
              if (shell_req (cmnd, NULL, 0))
                {
                  httpd_resp_set_status (req, http_200_hdr);
                  httpd_resp_set_type (req, http_content_type_text);
                  httpd_resp_send (req, http_pragma_ok,
                                   strlen (http_pragma_ok));
                }
              else
                {
                  httpd_resp_set_status (req, http_404_hdr);
                  httpd_resp_send (req, NULL, 0);
                }
              unlock_shell ();
            }
          else
            {
              httpd_resp_set_status (req, http_503_hdr);
              httpd_resp_send (req, NULL, 0);
              ESP_LOGE(
                  TAG,
                  "http_server_netconn_serve: GET /run?cmd= failed to obtain mutex");
            }

        }
      else
        {
          ESP_LOGW(TAG, "Rejected url: %s", req->uri);

          if (custom_get_httpd_uri_handler == NULL)
            {
              httpd_resp_set_status (req, http_404_hdr);
              httpd_resp_send (req, NULL, 0);
            }
          else
            {
              /* if there's a hook, run it */
              ret = (*custom_get_httpd_uri_handler) (req);
            }
        }

    }

  /* memory clean up */
  if (host != NULL)
    {
      free (host);
    }

  return ret;
}

/* URI wild card for any GET request */
static const httpd_uri_t http_server_get_request =
  { .uri = "*", .method = HTTP_GET, .handler = http_server_get_handler };

static const httpd_uri_t http_server_post_request =
  { .uri = "*", .method = HTTP_POST, .handler = http_server_post_handler };

static const httpd_uri_t http_server_delete_request =
  { .uri = "*", .method = HTTP_DELETE, .handler = http_server_delete_handler };

void
http_app_stop ()
{
  if (httpd_handle != NULL)
    {
      /* stop server */
      httpd_stop (httpd_handle);
      httpd_handle = NULL;
    }
}

void
http_app_start (bool lru_purge_enable) {
  esp_err_t err;

  if (httpd_handle == NULL) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

    /* this is an important option that isn't set up by default.
     * We could register all URLs one by one, but this would not work while the fake DNS is active */
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.lru_purge_enable = lru_purge_enable;

    err = httpd_start (&httpd_handle, &config);
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "Registering URI handlers");
      httpd_register_uri_handler (httpd_handle, &http_server_get_request);
      httpd_register_uri_handler (httpd_handle, &http_server_post_request);
      httpd_register_uri_handler (httpd_handle, &http_server_delete_request);
    }
  }
}

