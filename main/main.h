/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef MAIN_H_
#define MAIN_H_

// ISSUE INTELLISENSE NOT WORKING
// https://github.com/Microsoft/vscode-cpptools/issues/743
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

#include "params.h"
using json = nlohmann::json;
using namespace std;

#define SERVER_PORT 7856

#define LISTEN_PERIOD_MS 60000
#define WIFI_LISTEN_CHANNEL 1
#define RETRY_PERIOD_MS 3000

//VT: necessary in order to use c++
extern "C" {
	void app_main(void);
}

//VT: Event group, useful to handle wifi events and signal when we are connected
//static EventGroupHandle_t wifi_event_group;
//const int CONNECTED_BIT = BIT0;

extern WiFi* pWifi;
extern int BOOT_OK;

#endif