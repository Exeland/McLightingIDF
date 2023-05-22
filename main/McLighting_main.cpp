/*
 * McLighting_main.cpp
 *
 *  Created on: Oct 19, 2021
 *      Author: exeland
 */

#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_log.h"
//#include "esp_spiffs.h"
#include "nvs_flash.h"

#include "../user_components/settings/settings.h"
#include "../user_components/json_maker/json_maker.h"
#include "../user_components/wifi_manager/wifi_manager.h"
#include "../user_components/shell/shell.h"
#include "../user_components/ota_http_updater/ota_http_updater.h"

#include "driver/gpio.h"
#include "definitions.h"

#include "led_animator.h"

void gpio_setup() {
  printf("GPIO setup");
  gpio_config_t io_conf;
#if defined(LED_BUILTIN)
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = LED_BUILTIN;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
#endif
  // button pin setup
#if defined(ENABLE_BUTTON)
  printf("Enabled Button Mode on PIN: %d\r\n", ENABLE_BUTTON);
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = ENABLE_BUTTON;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
#endif

#if defined(ENABLE_BUTTON_GY33)
  printf("Enabled GY-33 Button Mode on PIN: %d\r\n", ENABLE_BUTTON_GY33);
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = ENABLE_BUTTON_GY33;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
#endif

#if defined(POWER_SUPPLY)
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = POWER_SUPPLY;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
#endif

}


void setup() {
  static const char *TAG = "setup";

  ESP_LOGI(TAG, "Starting...Main Setup");

  gpio_setup();

  esp_err_t nvs_ret = nvs_flash_init();
  if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES
      || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    nvs_ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(nvs_ret);


  settings_init();

  json_maker_init();

  shell_init();

  wifi_manager_start();


//  // ***************************************************************************
//  // Setup: WiFiManager
//  // ***************************************************************************
//  // The extra parameters to be configured (can be either global or just in the setup)
//  // After connecting, parameter.getValue() will get you the configured value
//  // id/name placeholder/prompt default length
//
//#if defined(ENABLE_STATE_SAVE)
//  //Strip Config
//  (readConfigFS()) ? printf("WiFiManager config FS read success!"): printf("WiFiManager config FS Read failure!");
//  delay(250);
//  (readStateFS()) ? printf("Strip state config FS read Success!") : printf("Strip state config FS read failure!");
//  char _stripSize[6], _fx_options[5], _rgbOrder[5]; //needed tempararily for WiFiManager Settings
//  WiFiManagerParameter custom_hostname("hostname", "Hostname", HOSTNAME, 64, " maxlength=64");
//  #if defined(ENABLE_MQTT)
//    char _mqtt_port[6]; //needed tempararily for WiFiManager Settings
//    WiFiManagerParameter custom_mqtt_host("host", "MQTT hostname", mqtt_host, 64, " maxlength=64");
//    sprintf(_mqtt_port, "%d", mqtt_port);
//    WiFiManagerParameter custom_mqtt_port("port", "MQTT port", _mqtt_port, 5, " maxlength=5 type=\"number\"");
//    WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 32, " maxlength=32");
//    WiFiManagerParameter custom_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32, " maxlength=32 type=\"password\"");
//  #endif
//  sprintf(_stripSize, "%d", Config.stripSize);
//  WiFiManagerParameter custom_strip_size("strip_size", "Number of LEDs", _stripSize, 4, " maxlength=4 type=\"number\"");
//  #if !defined(USE_WS2812FX_DMA)
//    char tmp_led_pin[3];
//    sprintf(tmp_led_pin, "%d", Config.pin);
//    WiFiManagerParameter custom_led_pin("led_pin", "LED GPIO", tmp_led_pin, 2, " maxlength=2 type=\"number\"");
//  #endif
//  sprintf(_rgbOrder, "%s", Config.RGBOrder);
//  WiFiManagerParameter custom_rgbOrder("rgbOrder", "RGBOrder", _rgbOrder, 4, " maxlength=4");
//  sprintf(_fx_options, "%d", segState.options);
//  WiFiManagerParameter custom_fxoptions("fxoptions", "fxOptions", _fx_options, 3, " maxlength=3");
//#endif
//
//
//  //Local intialization. Once its business is done, there is no need to keep it around
//  wifi_station_set_hostname(const_cast<char*>(HOSTNAME));
//  WiFiManager wifiManager;
//  //reset settings - for testing
//  //wifiManager.resetSettings();
//
//  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
//  wifiManager.setAPCallback(configModeCallback);
//  //set config save notify callback
//  wifiManager.setSaveConfigCallback(saveConfigCallback);
//
//  wifiManager.addParameter(&custom_hostname);
//  #if defined(ENABLE_MQTT)
//    //add all your parameters here
//    wifiManager.addParameter(&custom_mqtt_host);
//    wifiManager.addParameter(&custom_mqtt_port);
//    wifiManager.addParameter(&custom_mqtt_user);
//    wifiManager.addParameter(&custom_mqtt_pass);
//  #endif
//  wifiManager.addParameter(&custom_strip_size);
//  #if !defined(USE_WS2812FX_DMA)
//     wifiManager.addParameter(&custom_led_pin);
//  #endif
//  wifiManager.addParameter(&custom_rgbOrder);
//  wifiManager.addParameter(&custom_fxoptions);
//
//  WiFi.setSleepMode(WIFI_NONE_SLEEP);
//
//  // Uncomment if you want to restart ESP8266 if it cannot connect to WiFi.
//  // Value in brackets is in seconds that WiFiManger waits until restart
//#if defined(WIFIMGR_PORTAL_TIMEOUT)
//  wifiManager.setConfigPortalTimeout(WIFIMGR_PORTAL_TIMEOUT);
//#endif
//
//  // Order is: IP, Gateway and Subnet
//#if defined(WIFIMGR_SET_MANUAL_IP)
//  wifiManager.setSTAStaticIPConfig(IPAddress(_ip[0], _ip[1], _ip[2], _ip[3]), IPAddress(_gw[0], _gw[1], _gw[2], _gw[3]), IPAddress(_sn[0], _sn[1], _sn[2], _sn[3]));
//#endif
//
//  //fetches ssid and pass and tries to connect
//  //if it does not connect it starts an access point with the specified name
//  //here  "AutoConnectAP"
//  //and goes into a blocking loop awaiting configuration
//  if (!wifiManager.autoConnect(HOSTNAME)) {
//    printf("failed to connect and hit timeout");
//    //reset and try again, or maybe put it to deep sleep
//    //ESP.reset();  //Will be removed when upgrading to standalone offline McLightingUI version
//    //delay(1000);  //Will be removed when upgrading to standalone offline McLightingUI version
//  }
//
//  //save the custom parameters to FS/EEPROM
//  #if defined(ENABLE_STATE_SAVE)
//    strcpy(HOSTNAME, custom_hostname.getValue());
//    #if defined(ENABLE_MQTT)
//      //read updated parameters
//      strcpy(mqtt_host, custom_mqtt_host.getValue());
//      mqtt_port = atoi(custom_mqtt_port.getValue());
//      strcpy(mqtt_user, custom_mqtt_user.getValue());
//      strcpy(mqtt_pass, custom_mqtt_pass.getValue());
//    #endif
//    strcpy(_stripSize, custom_strip_size.getValue());
//    Config.stripSize = constrain(atoi(custom_strip_size.getValue()), 1, MAXLEDS);
//    #if !defined(USE_WS2812FX_DMA)
//      checkPin(atoi(custom_led_pin.getValue()));
//    #endif
//    strcpy(_rgbOrder, custom_rgbOrder.getValue());
//    checkRGBOrder(_rgbOrder);
//    segState.options = atoi(custom_fxoptions.getValue());
//    if (updateConfig) {
//      (writeConfigFS(updateConfig)) ? printf("WiFiManager config FS Save success!"): printf("WiFiManager config FS Save failure!");
//    }
//  #endif
//
//  //if you get here you have connected to the WiFi
//  printf("connected...yeey :)");
//  ticker.detach();
//  //keep LED on
//  digitalWrite(LED_BUILTIN, LOW);
//  //switch LED off
//  //digitalWrite(LED_BUILTIN, HIGH);
//
//#if defined(ENABLE_OTA)
//  #if ENABLE_OTA == 0
//  // ***************************************************************************
//  // Configure Arduino OTA
//  // ***************************************************************************
//    printf("Arduino OTA activated.");
//
//    // Port defaults to 8266
//    ArduinoOTA.setPort(8266);
//
//    // Hostname defaults to esp8266-[ChipID]
//    ArduinoOTA.setHostname(HOSTNAME);
//
//    // No authentication by default
//    // ArduinoOTA.setPassword("admin");
//
//    // Password can be set with it's md5 value as well
//    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
//    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
//
//    ArduinoOTA.onStart([]() {
//      printf("Arduino OTA: Start updating");
//    });
//    ArduinoOTA.onEnd([]() {
//      printf("Arduino OTA: End");
//    });
//    ArduinoOTA.onProgress([](uint16_t progress, uint16_t total) {
//      DBG_OUTPUT_PORT.printf("Arduino OTA Progress: %u%%\r", (progress / (total / 100)));
//    });
//    ArduinoOTA.onError([](ota_error_t error) {
//      DBG_OUTPUT_PORT.printf("Arduino OTA Error[%u]: ", error);
//      if (error == OTA_AUTH_ERROR) printf("Arduino OTA: Auth Failed");
//      else if (error == OTA_BEGIN_ERROR) printf("Arduino OTA: Begin Failed");
//      else if (error == OTA_CONNECT_ERROR) printf("Arduino OTA: Connect Failed");
//      else if (error == OTA_RECEIVE_ERROR) printf("Arduino OTA: Receive Failed");
//      else if (error == OTA_END_ERROR) printf("Arduino OTA: End Failed");
//    });
//
//    ArduinoOTA.begin();
//    printf("");
//  #endif
//  #if ENABLE_OTA == 1
//    httpUpdater.setup(&server, "/update");
//  #endif
//#endif
//
//#if defined(ENABLE_MQTT)
//  initMqtt();
//#endif
//
//#if ENABLE_MQTT == 1
//  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
//  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
//#endif
//
//  // ***************************************************************************
//  // Setup: MDNS responder
//  // ***************************************************************************
//  bool mdns_result = MDNS.begin(HOSTNAME);
//
//  DBG_OUTPUT_PORT.print("Open http://");
//  DBG_OUTPUT_PORT.print(WiFi.localIP());
//  printf("/ to open McLighting.");
//
//  DBG_OUTPUT_PORT.print("Use http://");
//  DBG_OUTPUT_PORT.print(HOSTNAME);
//  printf(".local/ when you have Bonjour installed.");
//#if !defined(USE_HTML_MIN_GZ)
//  DBG_OUTPUT_PORT.print("New users: Open http://");
//  DBG_OUTPUT_PORT.print(WiFi.localIP());
//  printf("/upload to upload the webpages first.");
//#endif
//  printf("");
//
//  // ***************************************************************************
//  // Setup: WebSocket server
//  // ***************************************************************************
//  webSocket.begin();
//  webSocket.onEvent(webSocketEvent);
//
//#include "rest_api.h"
//
//  server.begin();
//
//  // Start MDNS service
//  if (mdns_result) {
//    MDNS.addService("http", "tcp", 80);
//  }
//
//  #if defined(ENABLE_BUTTON_GY33)
//    tcs.setConfig(MCU_LED_06, MCU_WHITE_ON);
////    delay(2000);
////    tcs.setConfig(MCU_LED_OFF, MCU_WHITE_OFF);
//  #endif
//  #if defined(ENABLE_REMOTE)
//    irrecv.enableIRIn();  // Start the receiver
//  #endif
//  fx_speed = segState.speed[State.segment];
//  brightness_trans = State.brightness;
//  initStrip();
//  strip->setBrightness(0);
//  printf("finished Main Setup!");
}


extern "C" void app_main()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ", chip_info.cores);
    printf("silicon revision %d, ", chip_info.revision);
    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024), (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    CheckOTAUpdate();
    printf("Starting...Main Setup\n");
    setup();

    {
      t_settings* sett = get_settings();
      led_animator_init(sett->leds.cnt, sett->leds.order);
    }
}



