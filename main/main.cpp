/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
/**
 * LOG FUNCTIONS:
 * ESP_LOGE() -> ERROR
 * ESP_LOGW() -> WARNING
 * ESP_LOGI() -> INFO
 * ESP_LOGD() -> DEBUG (NORMALLY DOES NOT PRINT)
 */

#include "main.h"

#include "Sniffer.h"
#include "Server.h"
#include "Sender.h"

#include "driver/gpio.h"
#define BLINK_GPIO (gpio_num_t) 13

// Tag used for ESP32 log functions 
static const char *LOG_TAG = "main";

/** Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

void retry_reconnect(WiFi* pWifi){
	int attempts = 1;
	while(!pWifi->isConnectedToAP()){
		ESP_LOGI(LOG_TAG, "CONNECT TO WIFI: ATTEMPT #%d", attempts);
		pWifi->connectAP(WIFI_SSID, WIFI_PASS);
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay( pdMS_TO_TICKS(RETRY_PERIOD_MS) );
		gpio_set_level(BLINK_GPIO, 1);
	}
}

/**
 * Class used to define callback to call along with specific WiFi events
 */
class MyEventHandler: public WiFiEventHandler {	
public:
	MyEventHandler(WiFi* p){
		pWifi = p;
	}
private:
	WiFi* pWifi;

	/* // The event handler provides over-rides for:
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

	// TODO: LIMITARE IL NUMERO DI TENTATIVI DI RICONNESSIONE
	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info){
		//xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGE(LOG_TAG, "disconnected! Retrying to reconnect...");
		retry_reconnect(pWifi);		
    	return ESP_OK;
	}
};

void setup_client(void *pvParameter){

	// SET THE LEVEL OF THE LOGGER
	// If you don't set the level to DEBUG LEVEL, it does not print
	// the debug log but just the ERROR and INFO log messages
	// TODO: NOT WORKING 
	esp_log_level_set("*", ESP_LOG_DEBUG);

	ESP_LOGW(LOG_TAG, "+++++ WELCOME TO THE ESP32 SNIFFER 4 INDOOR LOCALIZATION +++++");
	ESP_LOGD(LOG_TAG, "LEVEL OF LOG SET TO DEBUG");

	++boot_count;
	ESP_LOGI(LOG_TAG, ">>> BOOT COUNT: %d <<<", boot_count);

	// SETUP WIFI STRUCTURE
	WiFi wifi = WiFi();
	wifi.setWifiEventHandler(new MyEventHandler(&wifi));
		
	Server server;
	server.setIpPort(SERVER_IP, SERVER_PORT);
	while(1){
		wifi.connectAP(WIFI_SSID, WIFI_PASS);
		retry_reconnect(&wifi);
		Sender sender(&server, LISTEN_PERIOD_MS);
		sender.initTimestamp();
		Sniffer sniffer(&sender);
		sniffer.init();
	}
}

/**
 * Incredibly this is the start point of the program
 */
void app_main(void)
{
	gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	// TODO: check if the stack (4096) is enough
	xTaskCreate(&setup_client, "setup_client", 4096, NULL, 5, NULL );
}