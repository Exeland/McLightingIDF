/*
 * shell_cmd.c
 *
 *  Created on: Dec 23, 2021
 *      Author: exeland
 */

/*
 ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

/**
 * @file    shell_cmd.c
 * @brief   Simple CLI shell common commands code.
 *
 * @addtogroup SHELL
 * @{
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"
#include "shell.h"

#include "../../main/led_animator.h"
#include "../settings/settings.h"

static const char *SHELL_TAG = "SHELL_CMND";

void cmnd_set(int argc, char *argv[], char *out, uint16_t *out_sz) {

  if (argc) {
    char *set_val = (argc >= 2) ? argv[1] : "";
    char *p_point = strpbrk(argv[0], ".");
    ESP_LOGI(SHELL_TAG, "\"set\"");

    if (p_point) {
      *p_point = '\0';
//      if (strcmp(argv[0], "auth") == 0) {
//        ESP_LOGI(SHELL_TAG, "auth");
//        p_point++;
//        if (strcmp(p_point, "pswd") == 0) {
//          ESP_LOGI(SHELL_TAG, "pswd:%s", set_val);
//          settings_update_auth_pswd(set_val);
//        }
//      } else
      if (strcmp(argv[0], "AP") == 0) {
        ESP_LOGI(SHELL_TAG, "AP");
        p_point++;
        if (strcmp(p_point, "ssid") == 0) {
          ESP_LOGI(SHELL_TAG, "ssid:%s", set_val);
          settings_update_ap_ssid(set_val);
        } else if (strcmp(p_point, "pswd") == 0) {
          ESP_LOGI(SHELL_TAG, "pswd:%s", set_val);
          settings_update_ap_pswd(set_val);
        }
      } else if (strcmp(argv[0], "NW") == 0) {
        ESP_LOGI(SHELL_TAG, "NW");
        p_point++;
        if (strcmp(p_point, "ssid") == 0) {
          ESP_LOGI(SHELL_TAG, "ssid:%s", set_val);
          settings_update_nw_ssid(set_val);
        } else if (strcmp(p_point, "pswd") == 0) {
          ESP_LOGI(SHELL_TAG, "pswd:%s", set_val);
          settings_update_nw_pswd(set_val);
        } else if (strcmp(p_point, "enbl") == 0) {
          ESP_LOGI(SHELL_TAG, "enbl:%s", set_val);
          settings_update_nw_enable(strcmp(set_val, "true") == 0);
        }
      }
//      else if (strcmp(argv[0], "MQTT") == 0) {
//        ESP_LOGI(SHELL_TAG, "MQTT");
//        p_point++;
//        if (strcmp(p_point, "hst") == 0) {
//          ESP_LOGI(SHELL_TAG, "hst:%s", set_val);
//          settings_update_mqtt_host(set_val);
//        } else if (strcmp(p_point, "prt") == 0) {
//          int port = atoi(set_val);
//          ESP_LOGI(SHELL_TAG, "prt:%d", port);
//          settings_update_mqtt_port(port);
//        } else if (strcmp(p_point, "clnt") == 0) {
//          ESP_LOGI(SHELL_TAG, "clnt:%s", set_val);
//          settings_update_mqtt_client(set_val);
//        } else if (strcmp(p_point, "usr") == 0) {
//          ESP_LOGI(SHELL_TAG, "usr");
//          settings_update_mqtt_user(set_val);
//        } else if (strcmp(p_point, "pswd") == 0) {
//          ESP_LOGI(SHELL_TAG, "pswd:%s", set_val);
//          settings_update_mqtt_pswd(set_val);
//        } else if (strcmp(p_point, "tpc") == 0) {
//          ESP_LOGI(SHELL_TAG, "tpc:%s", set_val);
//          settings_update_mqtt_topic(set_val);
//        } else if (strcmp(p_point, "enbl") == 0) {
//          ESP_LOGI(SHELL_TAG, "enbl:%s", set_val);
//          settings_update_mqtt_enable(strcmp(set_val, "true") == 0);
//        }
//      }
      else if (strcmp(argv[0], "LEDS") == 0) {
        ESP_LOGI(SHELL_TAG, "LEDS");
        p_point++;
        if (strcmp(p_point, "order") == 0) {
          ESP_LOGI(SHELL_TAG, "order:%s", set_val);
          settings_update_leds_order(set_val);
        } else if (strcmp(p_point, "cnt") == 0) {
          int cnt = atoi(set_val);
          ESP_LOGI(SHELL_TAG, "cnt:%d", cnt);
          settings_update_leds_cnt(cnt);
        }
      }
      else if (strcmp(argv[0], "lang") == 0) {
        ESP_LOGI(SHELL_TAG, "lang");
        p_point++;
        if (strcmp(p_point, "val") == 0) {
          ESP_LOGI(SHELL_TAG, "val:%s", set_val);
          settings_update_lang(set_val);
        }
      }
    }
  }

}

void cmnd_save(int argc, char *argv[], char *out, uint16_t *out_sz) {
  if (argc) {
    if (strcmp(argv[0], "settings") == 0) {
      settings_save();
    }
  }
}

void cmnd_reset(int argc, char *argv[], char *out, uint16_t *out_sz) {
  esp_restart();
}

void cmnd_set_mode(int argc, char *argv[], char *out, uint16_t *out_sz) {
  if (argc) {
    char *str_mode = argv[0];
    ESP_LOGI(SHELL_TAG, "set mode:%s", str_mode);
    led_animator_setmode(atoi(str_mode));
  }
}

void cmnd_set_color(int argc, char *argv[], char *out, uint16_t *out_sz) {
  if (argc >= 3) {
    char *str_color_R = argv[0];
    char *str_color_G = argv[1];
    char *str_color_B = argv[2];
    ESP_LOGI(SHELL_TAG, "set color:%s %s %s", str_color_R, str_color_G, str_color_B);
    led_animator_setcolor(atoi(str_color_R), atoi(str_color_G), atoi(str_color_B));
  }
  else{
    ESP_LOGI(SHELL_TAG, "set color does not match argument count: %d", argc);
  }
}

void cmnd_set_speed(int argc, char *argv[], char *out, uint16_t *out_sz) {
  if (argc) {
    char *str_speed = argv[0];
    ESP_LOGI(SHELL_TAG, "set speed:%s", str_speed);
    led_animator_setspeed(atoi(str_speed));
  }
}

void cmnd_set_brightness(int argc, char *argv[], char *out, uint16_t *out_sz) {
  if (argc) {
    char *str_brightness = argv[0];
    ESP_LOGI(SHELL_TAG, "set brightness:%s", str_brightness);
    led_animator_setbrightness(atoi(str_brightness));
  }
}

const shell_Command shell_local_commands[] = {
    { "setmode", cmnd_set_mode },
    { "setcolor", cmnd_set_color },
    { "setspeed", cmnd_set_speed },
    { "setbrghtns", cmnd_set_brightness },
    { "set", cmnd_set },
    { "save", cmnd_save },
    { "reset", cmnd_reset },
    { TERMINATE_CMD, NULL }
};

/** @} */

