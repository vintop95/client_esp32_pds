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

int volatile IS_WIFI_CONNECTED = 0;

// Tag used for ESP32 log functions 
static const char *LOG_TAG = "main";

/** Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

/**
 * Utility function that gives a string of the mac passed
 * 
 * @param Array of 8 bit integers representing the mac address
 * @param String representing the mac address to return
 * 
 * @return N/A
 */
void mac2str(const uint8_t* ptr, char* string)
{
  #ifdef MASKED
  sprintf(string, "XX:XX:XX:%02x:%02x:XX", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
  #else
  sprintf(string, "%02x:%02x:%02x:%02x:%02x:%02x", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5]);
  #endif
  return;
}

/**
 * It blinks the led at a frequency of ms_freq
 * stored in pvParameter
 * 
 * @param pointer to int that represents ms_freq
 */
void led_blink(void *pvParameter){
	int ms_freq_standard = RETRY_PERIOD_MS/2;
	int* ms_freq_ptr = &ms_freq_standard;
	if(pvParameter != NULL){
		ms_freq_ptr = (int*) (pvParameter);
	}
	int ms_freq = *ms_freq_ptr;
	while(1){
		vTaskDelay( pdMS_TO_TICKS(ms_freq/2) );
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay( pdMS_TO_TICKS(ms_freq/2) );
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

	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info){
		//xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGE(LOG_TAG, "WAS_WIFI_CONNECTED?: %d -> NOW IS 0", IS_WIFI_CONNECTED);
		IS_WIFI_CONNECTED = 0;
    	return ESP_OK;
	}
};

void setup_client(void *pvParameter){

	// SET THE GPIO PORT FOR BLINKING THE LED
	gpio_pad_select_gpio(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

	// WHEN THE ESP32 SLEEPS, EVERY SLEEP_SECS the esp32 will be awaken
	esp_sleep_enable_timer_wakeup(SLEEP_SECS*1000000);

	// SET THE LEVEL OF THE LOGGER
	// If you don't set the level to DEBUG LEVEL, it does not print
	// the debug log but just the ERROR and INFO log messages
	// TODO: NOT WORKING 
	esp_log_level_set("*", ESP_LOG_DEBUG);

	ESP_LOGW(LOG_TAG, "+++++ WELCOME TO THE ESP32 SNIFFER 4 INDOOR LOCALIZATION +++++");
	ESP_LOGD(LOG_TAG, "LEVEL OF LOG SET TO DEBUG");

	ESP_LOGI(LOG_TAG, ">>> BOOT COUNT: %d <<<", ++boot_count);

	// SETUP WIFI STRUCTURE
	WiFi wifi = WiFi();
	wifi.setWifiEventHandler(new MyEventHandler(&wifi));

	Server server(&wifi);
	server.set_ip_port(SERVER_IP, SERVER_PORT);

	Sender sender(&server, LISTEN_PERIOD_MS);
	Sniffer sniffer(&sender);

	while(1){
		bool connected = server.wifi_connect();
		if(!connected){
			//esp_restart();
			esp_deep_sleep_start();
		}

		bool got_timestamp = server.init_timestamp();
		if(!got_timestamp){
			if(!IS_WIFI_CONNECTED){
				continue;
			}else{
				esp_deep_sleep_start();
			}
		}

		// It is in an infinite loop that breaks only when the WiFi 
		// connection is not available anymore
		sniffer.init();
		sniffer.stop();
	}
}

/**
 * Incredibly this is the start point of the program
 */
void app_main(void)
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	xReturned = xTaskCreate(&setup_client, "setup_client", 4096, NULL, 5, &xHandle );

	if( xReturned != pdPASS ){
        ESP_LOGE(LOG_TAG, "Cannot create task");
    }
}