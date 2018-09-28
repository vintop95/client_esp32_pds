/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#include "Sniffer.h"

static const char* LOG_TAG = "Sniffer";

/**
 * @brief Pointer to a Sender defined out here in order
 * to use it in callbacks, defining it in the Sniffer class
 * make it impossible to be called by the callback
 */
Sender* pSender;

/**
 * @brief Sniffer constructor
 * 
 * @param Sender object used by the sniffer to send Records
 * 
 * @return String of packet type.
 */
Sniffer::Sniffer(Sender* sndr)
{  
    pSender = sndr;
}

/**
 * @brief Utility function that gives a string of the mac passed
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
 * @brief Utility function that gives a string of the packet type
 * 
 * @param Type of the packet received
 * @param Subtype of the management packet received
 * 
 * @return String of packet type.
 */
const char* wifi_pkt_type2str(wifi_promiscuous_pkt_type_t type, wifi_mgmt_subtypes_t subtype)
{
  switch(type)
  {
    case WIFI_PKT_MGMT:
    switch(subtype)
    {
        case ASSOCIATION_REQ:
        return "Mgmt: Association request";
        case ASSOCIATION_RES:
        return "Mgmt: Association response";
        case REASSOCIATION_REQ:
        return "Mgmt: Reassociation request";
        case REASSOCIATION_RES:
        return "Mgmt: Reassociation response";
        case PROBE_REQ:
        return "MGMT:PROBE_REQ";
        case PROBE_RES:
        return "Mgmt: Probe response";
        case BEACON:
        return "Mgmt: Beacon frame";
        case ATIM:
        return "Mgmt: ATIM";
        case DISASSOCIATION:
        return "Mgmt: Dissasociation";
        case AUTHENTICATION:
        return "Mgmt: Authentication";
        case DEAUTHENTICATION:
        return "Mgmt: Deauthentication";
        case ACTION:
        return "Mgmt: Action";
        case ACTION_NACK:
        return "Mgmt: Action no ack";
    	default:
        return "Mgmt: Unsupported/error";
    }

    case WIFI_PKT_CTRL:
    return "Control";

    case WIFI_PKT_DATA:
    return "Data";

    default:
      return "Unsupported/error";
  }
}

/**
 * @brief Called by json object to convert Record object in json
 * 
 * @param Reference of json element
 * @param Const ref to the Record to convert
 * 
 * @return N/A.
 */
extern void to_json(json& j, const Record& r);

// Task to be created.
 void vTaskCode( void * pvParameters )
 {
     pSender->sendRecordsToServer();
     vTaskDelete(NULL);
     while(true){

     }
 }

/**
 * @brief Callback that handles the sniffed packet
 * It must push back to the list of records the following fields:
 * - l’indirizzo MAC del mittente
 * - l’SSID richiesto(se presente)
 * - una marca temporale
 * - l’hash del pacchetto
 * - il livello del segnale ricevuto
 * 
 * @param Pointer to the packet received
 * @param Type of the packet received
 * 
 * @return N/A.
 */
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type)
{
	if (type != WIFI_PKT_MGMT)
		return;

    // Type cast the received buffer into our generic SDK structure
	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    // Pointer to where the actual 802.11 packet is within the structure
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    // Define pointers to the 802.11 packet header and payload
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
    //const uint8_t *data = ipkt->payload;
    // Pointer to the frame control section within the packet header
    const wifi_header_frame_control_t *frame_ctrl = (wifi_header_frame_control_t *)&hdr->frame_ctrl;
	
    if(frame_ctrl->subtype != PROBE_REQ){
        return;
    }
    
    ////DEBUG: stampa contenuto della variabile ipkt
    //printf("SIZEOF wifi_header_frame_control_t: %d\n", sizeof(wifi_header_frame_control_t));
    // unsigned char *p = (unsigned char *)ipkt;
    // for(int i=0; i<1000; i++){
    //     printf("%02x ", p[i]);
    // }
    // printf("\n");

    // Parse MAC addresses contained in packet header into human-readable strings
    char addr1[] = "00:00:00:00:00:00\0";
    char addr2[] = "00:00:00:00:00:00\0";
    char addr3[] = "00:00:00:00:00:00\0";

    mac2str(hdr->addr1, addr1);//RECEIVER
    mac2str(hdr->addr2, addr2);//SENDER
    mac2str(hdr->addr3, addr3);//FILTERING

    //json structure to send
    //TODO: da correggere
    Record r;
    r.sender_mac = addr2;
    r.timestamp = ppkt->rx_ctrl.timestamp;
    r.rssi = ppkt->rx_ctrl.rssi;
    //TODO: inserire correttamente payload del pacchetto
    //r.hashed_pkt = ipkt->payload;
    r.ssid = "";

	printf("%d %s CHAN=%02d, SEQ=%d, RSSI=%02d, SNDR=%s",
        /**< timestamp. The local time when this packet is received. 
        * It is precise only if modem sleep or light sleep is not enabled.
        unit: microsecond */
        ppkt->rx_ctrl.timestamp,
		wifi_pkt_type2str((wifi_promiscuous_pkt_type_t)frame_ctrl->type, (wifi_mgmt_subtypes_t)frame_ctrl->subtype),
        ppkt->rx_ctrl.channel,
        hdr->sequence_ctrl,
		ppkt->rx_ctrl.rssi,
        addr2
	);

    if (frame_ctrl->type == WIFI_PKT_MGMT && frame_ctrl->subtype == PROBE_REQ)
    {
        const wifi_mgmt_probe_req_t *probe_req_frame = (wifi_mgmt_probe_req_t*) ipkt->payload;
        char ssid[32] = {0};

        strncpy(ssid, probe_req_frame->ssid, probe_req_frame->length);

        r.ssid = ssid;

        printf(", SSID=%s", ssid);
    }
    printf("\n");

    //aggiungi il record alla lista di record da inviare
    pSender->push_back(r);
    
    //pSender->server->sendData(j);

    TaskHandle_t xHandle;
    xTaskCreate( vTaskCode, "NAME", 10000, (void*) 1, tskIDLE_PRIORITY, &xHandle );

    printf("\n");
}

/**
 * @brief Initialize the Sniffer, enabling promiscuous mode
 * and setting the callback
 * 
 * @return N/A.
 */
void Sniffer::init(){
    esp_wifi_set_promiscuous(true);
    ESP_LOGI(LOG_TAG, "initialized: setting sniffed packet handler callback\n");
    //TODO: commentare per testare l'invio di pacchetti
    //senza poter riceverne
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
    wifi_sniffer_loop_channels();
}

/**
 * @brief It loops wifi channels.
 * The assignment says to listen to just one channel, but listen
 * to more channels could be useful.
 * 
 * @return N/A.
 */
void Sniffer::wifi_sniffer_loop_channels(){
	uint8_t channel = 1;

    pSender->start_timer();
    
    while (true) {
        //VT: loop all channels
        vTaskDelay(WIFI_CHANNEL_SWITCH_INTERVAL / portTICK_PERIOD_MS);
        //esp_wifi_set_promiscuous(false);
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        //esp_wifi_set_promiscuous(true);
        channel = (channel % WIFI_CHANNEL_MAX) + 1;
    }

}

