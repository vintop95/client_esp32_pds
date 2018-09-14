//Di Vincenzo Topazio
#ifndef SERVER_H_
#define SERVER_H_

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

#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class Server {
private:
    int s;//socket
public:
    int connect(std::string ip, int port);
    int send(json); 
    int waitAck();
    void close();
};

#endif 