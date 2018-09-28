/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */

/**
 * LOG FUNCTIONS:
 * ESP_LOGI() -> INFO
 * ESP_LOGD() -> DEBUG (NORMALLY DOES NOT PRINT)
 * ESP_LOGE() -> ERROR
 */

//

#include "main.h"

#include "Sniffer.h"
#include "Server.h"
#include "Sender.h"

//Tag used for ESP32 log functions 
static const char *LOG_TAG = "main";

WiFi* pWifi;

//Class used to define callback to call along with specific WiFi events
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
		ESP_LOGD(LOG_TAG, "got IP!");
    	return ESP_OK;
	}

	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info){
		//xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGD(LOG_TAG, "disconnected! Retrying to reconnect...");
		pWifi->connectAP(WIFI_SSID, WIFI_PASS);
    	return ESP_OK;
	}
};

void app_main(void)
{
	//PRINT DEBUG LOG
	esp_log_level_set("*", ESP_LOG_DEBUG);

	//SETUP WIFI
	WiFi wifi = WiFi(); //calling WiFi::init inside the constructor (RAII)
	pWifi = &wifi;
	wifi.setWifiEventHandler(new MyEventHandler());

	//connect to WIFI_SSID network
	wifi.connectAP(WIFI_SSID, WIFI_PASS);	
	//std::cout << "IP AP: " << wifi.getApIp() << std::endl;

	Server server;
	while(1){
		server.setIpPort(SERVER_IP, SERVER_PORT);
		Sender sender(&server, 10000);
		Sniffer sniffer(&sender);
		sniffer.init();
	}
}