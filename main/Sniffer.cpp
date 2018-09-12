//Di Vincenzo Topazio
#include "Sniffer.h"


void Sniffer::init(){
    esp_wifi_set_promiscuous(true); //VT: attiva modalità promiscua (sniffing)
	//VT: assegna callback da chiamare per ogni pacchetto ricevuto
	esp_wifi_set_promiscuous_rx_cb(&Sniffer::wifi_sniffer_packet_handler);
}

void Sniffer::wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
	//if (type != WIFI_PKT_MGMT)
	//	return;

	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

	/**< timestamp. The local time when this packet is received. 
 	* It is precise only if modem sleep or light sleep is not enabled.
	  unit: microsecond */

    const char* type_str = "";

    switch(type) {
        case WIFI_PKT_MGMT: type_str = "MGMT";
        case WIFI_PKT_DATA: type_str =  "DATA";
        default:	
        case WIFI_PKT_MISC: type_str =  "MISC";
	}

	printf("PACKET TYPE=%s, CHAN=%02d, RSSI=%02d, timestamp=%d"
		" SENDER_ADDR=%02x:%02x:%02x:%02x:%02x:%02x\n",
		type_str,
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

void 
wifi_sniffer_loop_channels(){
	uint8_t channel = 1;

	while (true) {
		//VT: loop all channels
		vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
		channel = (channel % WIFI_CHANNEL_MAX) + 1;
    }
}