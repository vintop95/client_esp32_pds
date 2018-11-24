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

#include "lwip/apps/sntp.h"

//Tag used for ESP32 log functions 
static const char *LOG_TAG = "main";

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static int boot_count = 0;

WiFi* pWifi;
time_t boot_time;

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

	// TODO: LIMITARE IL NUMERO DI TENTATIVI DI RICONNESSIONE
	virtual esp_err_t staDisconnected(system_event_sta_disconnected_t info){
		//xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
		ESP_LOGD(LOG_TAG, "disconnected! Retrying to reconnect...");
		while(!pWifi->isConnectedToAP()){
			pWifi->connectAP(WIFI_SSID, WIFI_PASS);
			vTaskDelay(5000 / portTICK_RATE_MS);
		}
		
    	return ESP_OK;
	}
};


///////////////////////BLOCCO SNTP
/**
 * @TODO: spostare in classe apposita
 * 
 */
static void initialize_sntp(void)
{
    ESP_LOGI(LOG_TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

/**
 * @TODO: spostare in classe apposita
 * 
 */
static void obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
        ESP_LOGI(LOG_TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

void set_time(){
	time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(LOG_TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
	
	char timeString[100];
	struct tm *ptm = gmtime(&now);
	strftime(timeString, sizeof(timeString), "%FT%TZ", ptm);
	printf("time set to: %s", timeString);


/* 	struct tm tm;
    tm.tm_year = 2018 - 1900;
    tm.tm_mon = 10;
    tm.tm_mday = 15;
    tm.tm_hour = 14;
    tm.tm_min = 10;
    tm.tm_sec = 10; 
	time_t t = mktime(&tm);
    printf("Setting time: %s", asctime(&tm));
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);
	*/
}

/**
 * @TODO: aggiungere sincronizzazione tempo con server
 * 
 */
void app_main(void)
{
	++boot_count;
	ESP_LOGI(LOG_TAG, "Boot count: %d", boot_count);

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
		Sender sender(&server, LISTEN_PERIOD_MS);
		sender.initTimestamp();
		Sniffer sniffer(&sender);
		sniffer.init();
	}
}