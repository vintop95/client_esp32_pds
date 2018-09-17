#ifndef MAIN_H_
#define MAIN_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include <iostream>
#include <string>
#include <string.h>
#include <vector>
#include <memory>

#include "WiFi.h"
#include "FreeRTOSTimer.h"
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

#define DEVICE_NAME "ESP0";
//VT: compilation switch in order to enable/disable
//code for enabling connection to WIFI_SSID network
#define WIFI_ENABLE_CONNECT 1
//VT: needed to connect in a wifi network
//modify Knofig.projbuild to add config parameters like this
//to set them use make menuconfig or modify sdkconfig
// #define WIFI_SSID "CUTRE-TPS" //CONFIG_ESP_WIFI_SSID
// #define WIFI_PASS "Einaudi1935!" //CONFIG_ESP_WIFI_PASSWORD
// #define SERVER_IP "172.16.139.251" //"192.168.43.5"

#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define SERVER_IP "192.168.43.5"

#define SERVER_PORT 7856

//VT: necessary in order to use c++
extern "C" {
	void app_main(void);
}

//VT: Event group, useful to handle wifi events and signal when we are connected
//static EventGroupHandle_t wifi_event_group;
//const int CONNECTED_BIT = BIT0;

#endif
