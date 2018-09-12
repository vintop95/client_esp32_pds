/**
 * WiFi Sniffer.
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

//VT: compilation switch in order to enable/disable
//code for enabling connection to WIFI_SSID network
#define WIFI_ENABLE_CONNECT (1)
//VT: needed to connect in a wifi network
#define WIFI_SSID "xd"
#define WIFI_PASS "ciao1234"

#define	WIFI_CHANNEL_MAX		(13)
#define	WIFI_CHANNEL_SWITCH_INTERVAL	(500)

#define MAX_APs 20

//static wifi_country_t wifi_country = {.cc="CN", .schan=1, .nchan=13, .policy=WIFI_COUNTRY_POLICY_AUTO};

//VT: Event group, useful to handle wifi events
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

typedef struct {
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

static esp_err_t event_handler(void *ctx, system_event_t *event);
static void wifi_sniffer_set_channel(uint8_t channel);
static const char *wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);


//VT: Empty infinite task -> callback for xTaskCreate api function
void loop_task(void *pvParameter)
{
    while(1) { 
		vTaskDelay(1000 / portTICK_RATE_MS);		
    }
}

//VT: The esp-idf wifi stack requires a method (= event handler) that is called 
//every time an event related to the wifi interface is triggered (for example
//the connection to a new network…). 
esp_err_t
event_handler(void *ctx, system_event_t *event)
{
	#if WIFI_ENABLE_CONNECT

		switch(event->event_id) {
			
		case SYSTEM_EVENT_STA_START:
			esp_wifi_connect();
			break;
		
		case SYSTEM_EVENT_STA_GOT_IP:
			xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
			break;
		
		case SYSTEM_EVENT_STA_DISCONNECTED:
			xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
			break;
		
		default:
			break;
		}

	#endif

	return ESP_OK;
}

//VT: initialize wifi/tcp-ip stack
void
wifi_init(void)
{
	ESP_ERROR_CHECK(nvs_flash_init()); //VT: inizializza non volatile storage

	// create the event group to handle wifi events in handler
	wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init(); //VT: inizializza stack tcp-ip
	//VT: assegna callback a wifi driver
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
	//VT: configura, inizializza e avvia il wifi stack
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	//ESP_ERROR_CHECK( esp_wifi_set_country(&wifi_country) ); /* set country for channel range [1, 13] */
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA));


	//VT: configure the wifi connection to connect to a wifi net
	wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    ESP_ERROR_CHECK( esp_wifi_start() );

}

//VT: initialize only the sniffer part
void 
wifi_sniffer_init(void){
	esp_wifi_set_promiscuous(true); //VT: attiva modalità promiscua (sniffing)
	//VT: assegna callback da chiamare per ogni pacchetto ricevuto
	esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
}

void
wifi_sniffer_set_channel(uint8_t channel)
{
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

//VT
void 
wifi_sniffer_loop_channels(void){
	uint8_t channel = 1;

	while (true) {
		//VT: loop all channels
		vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
		wifi_sniffer_set_channel(channel);
		channel = (channel % WIFI_CHANNEL_MAX) + 1;
    }
}

const char *
wifi_sniffer_packet_type2str(wifi_promiscuous_pkt_type_t type)
{
	switch(type) {
	case WIFI_PKT_MGMT: return "MGMT";
	case WIFI_PKT_DATA: return "DATA";
	default:	
	case WIFI_PKT_MISC: return "MISC";
	}
}

void
wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
	if (type != WIFI_PKT_MGMT)
		return;

	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;


	/**< timestamp. The local time when this packet is received. 
 	* It is precise only if modem sleep or light sleep is not enabled.
	  unit: microsecond */

	printf("PACKET TYPE=%s, CHAN=%02d, RSSI=%02d, timestamp=%d"
		" SENDER_ADDR=%02x:%02x:%02x:%02x:%02x:%02x\n",
		wifi_sniffer_packet_type2str(type),
		ppkt->rx_ctrl.channel,
		ppkt->rx_ctrl.rssi,
		ppkt->rx_ctrl.timestamp,
		/* ADDR1: RECEIVE_ADDR */
		//hdr->addr1[0],hdr->addr1[1],hdr->addr1[2],
		//hdr->addr1[3],hdr->addr1[4],hdr->addr1[5],
		/* ADDR3: FILTERING_ADDR */
		//hdr->addr3[0],hdr->addr3[1],hdr->addr3[2],
		//hdr->addr3[3],hdr->addr3[4],hdr->addr3[5],
		/* ADDR2: SENDER_ADDR */
		hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],
		hdr->addr2[3],hdr->addr2[4],hdr->addr2[5]
	);
}


// From auth_mode code to string
static char* getAuthModeName(wifi_auth_mode_t auth_mode) {
	char *names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
	return names[auth_mode];
}

void 
wifi_scan(void){
	// configure and run the scan process in blocking mode
	wifi_scan_config_t scan_config = {
		.ssid = 0,
		.bssid = 0,
		.channel = 0,
        .show_hidden = true
    };
	printf("Start scanning...");
	ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));
	printf(" completed!\n");
	printf("\n");

	// get the list of APs found in the last scan
	uint16_t ap_num = MAX_APs;
	wifi_ap_record_t ap_records[MAX_APs];
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

	// print the list 
	printf("Found %d access points:\n", ap_num);
	printf("\n");
	printf("               SSID              | Channel | RSSI |   Auth Mode \n");
	printf("----------------------------------------------------------------\n");
	for(int i = 0; i < ap_num; i++)
		printf("%32s | %7d | %4d | %12s\n", (char *)ap_records[i].ssid, ap_records[i].primary, ap_records[i].rssi, getAuthModeName(ap_records[i].authmode));
	printf("----------------------------------------------------------------\n");

	// infinite loop
	xTaskCreate(&loop_task, "loop_task", 2048, NULL, 5, NULL);
}

void
wifi_show_network_details(void *pvParameter){
	// wait for connection
	printf("Main task: waiting for connection to the wifi network... ");
	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
	printf("connected!\n");
	
	// print the local IP address
	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));
	printf("IP Address:  %s\n", ip4addr_ntoa(&ip_info.ip));
	printf("Subnet mask: %s\n", ip4addr_ntoa(&ip_info.netmask));
	printf("Gateway:     %s\n", ip4addr_ntoa(&ip_info.gw));
	
	while(1) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void
app_main(void)
{
	/* setup */
	wifi_init();
	//printf("INITIALIZATION DONE.\n");

	//VT: connect to WIFI_SSID network
	#if WIFI_ENABLE_CONNECT
		xTaskCreate(&wifi_show_network_details, 
		"wifi_show_network_details", 2048, NULL, 5, NULL);
	#endif

	//VT: SNIFFER MODE
	//wifi_sniffer_init();
	//wifi_sniffer_loop_channels();

	//VT: AP SCAN MODE
	//wifi_scan();
}