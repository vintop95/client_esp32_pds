/**
 * PDS Project - ESP32
 */

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

#include "Sniffer.h"
#include "WiFi.h"

//VT: compilation switch in order to enable/disable
//code for enabling connection to WIFI_SSID network
#define WIFI_ENABLE_CONNECT (1)
//VT: needed to connect in a wifi network
//modify Knofig.projbuild to add config parameters like this
//to set them use make menuconfig or modify sdkconfig
#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD

//VT: necessary in order to use c++
extern "C" {
	void app_main(void);
}
using namespace std;

//VT: Event group, useful to handle wifi events and signal when we are connected
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

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

void app_main(void)
{
	/* setup */
	WiFi wifi = WiFi();
	ESP_LOGI(LOG_TAG,"INITIALIZATION DONE.");


	//VT: connect to WIFI_SSID network
	#if WIFI_ENABLE_CONNECT
		wifi.setWifiEventHandler(new MyEventHandler());
		wifi.connectAP(WIFI_SSID, WIFI_PASS);		
	#endif

	//VT: SNIFFER MODE
	//Sniffer sniffer;
	//sniffer.init();
	//wifi_sniffer_loop_channels();

	//VT: AP SCAN MODE
	//wifi_scan();
}