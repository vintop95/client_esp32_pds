/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#include "Sniffer.h"

static const char* LOG_TAG = "Sniffer";
void wifi_sniffer_packet_handler(void* buff, wifi_promiscuous_pkt_type_t type);

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

Sniffer::~Sniffer()
{
    this->stop();
}

/**
 * @brief Initialize the Sniffer, enabling promiscuous mode
 * and setting the callback
 * 
 * @return N/A.
 */
void Sniffer::init(){
    esp_wifi_set_promiscuous(true);
    ESP_LOGI(LOG_TAG, "Start sniffing\n");
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);

    wifi_sniffer_loop_channels();
}

/**
 * @brief Deinitialize the Sniffer, 
 * disabling the promiscous mode and unsetting the callback
 * 
 * @return N/A.
 */
void Sniffer::stop(){
    ESP_LOGE(LOG_TAG, "Stop sniffing...\n");
    esp_wifi_set_promiscuous(false);
    esp_wifi_set_promiscuous_rx_cb(NULL);
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

// printing bits of number for debugging
void printBits(unsigned num)
{
   for(int bit=0;bit<(sizeof(unsigned) * 8); bit++)
   {
      printf("%i ", num & 0x01);
      num = num >> 1;
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
    /**< timestamp. The local time when this packet is received. 
        * It is precise only if modem sleep or light sleep is not enabled.
        unit: microsecond */

	if (type != WIFI_PKT_MGMT)
		return;

    // Type cast the received buffer into our generic SDK structure
	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
    // Pointer to where the actual 802.11 packet is within the structure
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
    // Define pointers to the 802.11 packet header and payload
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
    const unsigned char* ieee80211_pkt_binary = (unsigned char*)hdr;
    //const uint8_t *data = ipkt->payload;
    // Pointer to the frame control section within the packet header
    const wifi_header_frame_control_t *frame_ctrl = (wifi_header_frame_control_t *)&hdr->frame_ctrl;
	
    if(frame_ctrl->subtype != PROBE_REQ){
        return;
    }
    
    gpio_set_level(BLINK_GPIO, 0);

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

    ///// DEBUG: FILTER MAC ADDRESS SENDER
    // if(strcmp(addr2, "c0:ee:fb:02:b8:fa") != 0 ){
    //     return;
    // }

    // Size of the packet
    /**< length of packet including Frame Check Sequence(FCS) - 12 bit field*/
        
    // printf("PACKET SIZE IN BITS: \n");
    // printBits(ppkt->rx_ctrl.sig_len);
    // printf("\n");

    //// e se non ci fosse bisogno di cio?
    // unsigned short pkt_size_short = (ppkt->rx_ctrl.sig_len << 4) | (ppkt->rx_ctrl.sig_len >> 8);
    // int pkt_size = (int)pkt_size_short;
    ////
    
    unsigned int pkt_size = ppkt->rx_ctrl.sig_len;
    printf("PKT SIZE: %u B, ", ppkt->rx_ctrl.sig_len);
    
    unsigned int payload_size = pkt_size - (sizeof(wifi_ieee80211_mac_hdr_t) + 4);
    printf("PAYLOAD SIZE WITHOUT CRC32: %u B, ", payload_size);

    //json structure to send
    Record r;
    r.sender_mac = addr2;

    //// CALCOLO TIMESTAMP CON GETTIME
    struct timeval tv;
    gettimeofday(&tv, NULL); 
    r.timestamp = tv.tv_sec;
    //// 

    // printf("NOW: %u SEC\n", (unsigned)tv.tv_sec); 
    // printf("TIMESTAMP TO SEND: %u SEC\n", r.timestamp); 

    r.rssi = ppkt->rx_ctrl.rssi;
    r.ssid = "";

    printf("payload:");
    for(int i=0; i<pkt_size; ++i){
        //printf("%02x ", (unsigned char)ipkt->payload[i]);
        printf("%02x ", ieee80211_pkt_binary[i]);
    }
    printf("\n");

    // USE A HASH FUNCTION IN ORDER TO HAVE A STRING TO PUT IN hashed_pkt
    // TODO: sto facendo l'hash dell'intero header, non del contenuto del payload
    // perché non mi convince il contenuto, proviamo innanzitutto a vedere se 
    // due board riescono a sniffare lo stesso pacchetto (con stesso hash), se sì
    // vediamo di estendere l'hash all'intero pacchetto.
    uint8_t shaData[pkt_size];
    esp_sha(SHA1, ipkt->payload, pkt_size, shaData); //"ipkt->payload, pkt_size" al posto di "(const unsigned char*)ieee80211_pkt_binary, 24"
    unsigned char shaBase64[100];
    size_t outputLen;
    mbedtls_base64_encode(shaBase64, 100, (size_t*)&outputLen, (const unsigned char*)shaData, (size_t)20 );
    std::string str((const char *)shaBase64);
    r.hashed_pkt = str;

    // printf("\nTIME ELAPSED: %u SEC\n", ppkt->rx_ctrl.timestamp/1000000);  
	printf("%u %s CHAN=%02d, SEQ=%d, RSSI=%02d, SNDR=%s",
        
        r.timestamp,
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
    printf(", HASH=%s", shaBase64);
    printf("\n\n");

    //aggiungi il record alla lista di record da inviare
    pSender->push_back(r);

    gpio_set_level(BLINK_GPIO, 1);
}

/**
 * @brief NECESSARY BUSY WAITING FUNCTION
 * 
 * Originally it looped the wifi channels.
 * The assignment says to listen to just one channel, but listen
 * to more channels could be useful.
 * 
 * @return N/A.
 */
void Sniffer::wifi_sniffer_loop_channels(){
	uint8_t channel = WIFI_LISTEN_CHANNEL;
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    pSender->startSendingTimer();
    
    while (IS_WIFI_CONNECTED) {
        // loop all channels
        vTaskDelay( pdMS_TO_TICKS(WIFI_CHANNEL_SWITCH_INTERVAL) );
        
        // IT CHANGES CHANNEL, DISABLED BECAUSE OF CONFLICTS WITH SENDING TO THE SERVER
        // esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        // channel = (channel % WIFI_CHANNEL_MAX) + 1;
    }

}

