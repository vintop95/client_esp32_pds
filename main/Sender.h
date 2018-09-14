//Di Vincenzo Topazio
#ifndef SENDER_H_
#define SENDER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"

#include <iostream>
#include <string>
#include <string.h>
#include <vector>

#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "Server.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

struct Record{
    std::string sender_mac;
    uint32_t timestamp;
    int8_t rssi;
    std::string hashed_pkt;
    std::string ssid;
};



class Sender {
private:
    Server* server;
    int msListenPeriod;
    std::vector<Record> records;
public:
    Sender(Server* srv, int ms): server(srv), msListenPeriod(ms){

    }
    int sendRecordsToServer();

    void push_back(Record r);
    friend void to_json(json& j, const Record& r);
};

#endif 