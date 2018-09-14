/**
 * PDS Project - ESP32
 * ESP_LOGI -> INFO
 * ESP_LOGD -> DEBUG (NORMALLY DOESN'T PRINT)
 * ESP_LOGE -> ERROR
 */
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#include "WiFi.h"
#include "Sniffer.h"
#include "Server.h"
#include "Sender.h"

#include <iostream>
#include <string>

using namespace std;

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
using namespace std;

//VT: Event group, useful to handle wifi events and signal when we are connected
//static EventGroupHandle_t wifi_event_group;
//const int CONNECTED_BIT = BIT0;

//VT: name of the device appearing in the log
static const char *LOG_TAG = "ESP_VT";

//VT: Empty infinite task -> callback for xTaskCreate api function
void loop_task(void *pvParameter)
{
    while(1) { 
		vTaskDelay(1000 / portTICK_RATE_MS);		
    }
}


void wifi_scan(WiFi wifi){
	auto ap_records = wifi.scan();
	int ap_num = ap_records.size();

	// print the list 
	printf("Found %d access points:\n", ap_num);
	printf("\n");
	printf("----------------------------------------------------------------\n");
	for(int i = 0; i < ap_num; i++)
		ap_records[i].toString();
	printf("----------------------------------------------------------------\n");

	// infinite loop
	xTaskCreate(&loop_task, "loop_task", 2048, NULL, 5, NULL);
}


class MyEventHandler: public WiFiEventHandler {
	/* The event handler provides over-rides for:
	virtual esp_err_t apStaConnected(system_event_ap_staconnected_t info);
	virtual esp_err_t apStaDisconnected(system_event_ap_stadisconnected_t info);
	virtual esp_err_t apStart();
	virtual esp_err_t apStop();
	system_event_cb_t getEventHandler();
	virtual esp_err_t staConnected(system_event_sta_connected_t info);
	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info);
	virtual esp_err_t staGotIp(system_event_sta_got_ip_t info);
	virtual esp_err_t staScanDone(system_event_sta_scan_done_t info);
	virtual esp_err_t staAuthChange(system_event_sta_authmode_change_t info);
	virtual esp_err_t staStart();
	virtual esp_err_t staStop();
	virtual esp_err_t wifiReady();
	*/

	virtual esp_err_t staGotIp(system_event_sta_got_ip_t e){
		//xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGD(LOG_TAG, "got IP! Connected bit set");
    	return ESP_OK;
	}

	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info){
		//xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGD(LOG_TAG, "disconnected! Connected bit cleared");
    	return ESP_OK;
	}
};



#define RESOURCE "/demo/aphorisms.php"
#define WEBSITE "www.lucadentella.it"

// HTTP request
static const char *REQUEST = "GET " RESOURCE " HTTP/1.1\n"
	"Host: " WEBSITE "\n"
	"User-Agent: ESP32\n"
	"\n";

void tcp_test(void){
	// define connection parameters
	struct addrinfo hints = { };
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	// address info struct and receive buffer
    struct addrinfo *res;
	char recv_buf[100];
	
	// resolve the IP of the target website
	int result = getaddrinfo(WEBSITE, "80", &hints, &res);
	if((result != 0) || (res == NULL)) {
		printf("Unable to resolve IP for target website %s\n", WEBSITE);
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("Target website's IP resolved\n");


	// create a new socket
	//possibile farlo con wrapper, ma sembra più complicato
	//Socket sock = Socket();
	//int s = sock.createSocket();
	int s = socket(res->ai_family, res->ai_socktype, 0);
	if(s < 0) {
		printf("Unable to allocate a new socket\n");
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("Socket allocated, id=%d\n", s);

	// connect to the specified server
	result = connect(s, res->ai_addr, res->ai_addrlen);
	if(result != 0) {
		printf("Unable to connect to the target website\n");
		close(s);
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("Connected to the target website\n");

	// send the request
	printf("SENDING:\n%s", REQUEST);


	result = write(s, REQUEST, strlen(REQUEST));
		if(result < 0) {
		printf("Unable to send the HTTP request\n");
		close(s);
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("HTTP request sent\n");
	
	// print the response
	printf("HTTP response:\n");
	printf("--------------------------------------------------------------------------------\n");
	int r;
	do {
		bzero(recv_buf, sizeof(recv_buf));
		r = read(s, recv_buf, sizeof(recv_buf) - 1);
		for(int i = 0; i < r; i++) {
			putchar(recv_buf[i]);
		}
	} while(r > 0);	
	printf("--------------------------------------------------------------------------------\n");

	close(s);
	printf("Socket closed\n");
	
	while(1) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void app_main(void)
{
	//PRINT DEBUG LOG
	esp_log_level_set("*", ESP_LOG_DEBUG);

	/* setup */
	WiFi wifi = WiFi(); //calling init inside the constructor (RAII)
	wifi.setWifiEventHandler(new MyEventHandler());

	ESP_LOGD(LOG_TAG,"INITIALIZATION DONE.");

	//VT: connect to WIFI_SSID network
	#if WIFI_ENABLE_CONNECT
		wifi.connectAP(WIFI_SSID, WIFI_PASS);	
		//std::cout << "IP AP: " << wifi.getApIp() << std::endl;

		Server server;
		server.connect(SERVER_IP, SERVER_PORT);

		json j;
		j["pi"] = 3.141;
		server.send(j);

		Sender sender(&server, 1000);
		Sniffer sniffer(&sender);
		sniffer.init();
	#endif

	

	//VT: AP SCAN MODE
	//wifi_scan();
	#if WIFI_ENABLE_CONNECT
		server.close();
	#endif
}