/**
 * PDS Project - ESP32
 * ESP_LOGI -> INFO
 * ESP_LOGD -> DEBUG (NORMALLY DOESN'T PRINT)
 * ESP_LOGE -> ERROR
 */
#include "main.h"

#include "Sniffer.h"
#include "Server.h"
#include "Sender.h"

static const char *LOG_TAG = "main";

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
	//PRINT DEBUG LOG
	esp_log_level_set("*", ESP_LOG_DEBUG);

	/* setup */
	WiFi wifi = WiFi(); //calling init inside the constructor (RAII)
	wifi.setWifiEventHandler(new MyEventHandler());

	//connect to WIFI_SSID network
	wifi.connectAP(WIFI_SSID, WIFI_PASS);	
	//std::cout << "IP AP: " << wifi.getApIp() << std::endl;

	Server server;
	while(1){
		server.connect(SERVER_IP, SERVER_PORT);

		Sender sender(&server, 1000);

		Sniffer sniffer(&sender);
		sniffer.init();

		server.close();
	}
}