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
 * Pointer to a Sender defined out here in order to use it in callbacks.
 * If i had designed it in the Sniffer class it would make impossible 
 * to call it from the callback
 */
Sender* pSender;

/**
 * Sniffer constructor
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
 * Initialize the Sniffer, enabling promiscuous mode
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
 * Deinitialize the Sniffer, 
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
 * Utility function that gives a string of the packet type
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
 * Called by json object to convert Record object in json
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

void printf_date(time_t timestamp){
	char buffer[24];
    struct tm* tm_info;

    tm_info = localtime(&timestamp);

    strftime(buffer, 24, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[\033[0;32m%s\033[0m]", buffer);
}

/**
 * Callback that handles the sniffed packet
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
    // char addr1[] = "00:00:00:00:00:00\0";
    char addr2[] = "00:00:00:00:00:00\0";
    // char addr3[] = "00:00:00:00:00:00\0";

    // mac2str(hdr->addr1, addr1);//RECEIVER
    mac2str(hdr->addr2, addr2);//SENDER
    // mac2str(hdr->addr3, addr3);//FILTERING

    ///// DEBUG: FILTER MAC ADDRESS SENDER
    // if(strcmp(addr2, "c0:ee:fb:02:b8:fa") != 0 ){
    //     return;
    // }

    printf("\033[1;33m");
    printf("===============================================================================\n");
    printf("\033[0m");

    // 0) Size of the packet
    /**< length of packet including Frame Check Sequence(FCS) - 12 bit field*/
    unsigned int pkt_size = ppkt->rx_ctrl.sig_len;
    printf("\033[1;33m");
    printf("PKT_SIZE");
    printf("\033[0m");
    printf(": %u B, ", ppkt->rx_ctrl.sig_len);
    unsigned int payload_size = pkt_size - (sizeof(wifi_ieee80211_mac_hdr_t) + 4);
    printf("\033[1;33m");
    printf("PAYLOAD_SIZE_WITHOUT_CRC32");
    printf("\033[0m");
    printf(": %u B, ", payload_size);

    // JSON TO SEND
    Record r;

    // 1) sender_mac
    memcpy(r.sender_mac, hdr->addr2, sizeof(hdr->addr2));

    // 2) timestamp
    //// CALCOLO TIMESTAMP CON GETTIME
    struct timeval tv;
    gettimeofday(&tv, NULL); 
    r.timestamp = tv.tv_sec;

    // 3) rssi
    r.rssi = ppkt->rx_ctrl.rssi;

    // 4) ssid - TAKE THE SSID FROM THE PKT IF ACTUALLY THERE IS ONE
    const wifi_mgmt_probe_req_t *probe_req_frame = (wifi_mgmt_probe_req_t*) ipkt->payload;
    if(probe_req_frame->length > 0){
        strncpy(r.ssid, probe_req_frame->ssid, probe_req_frame->length);
        r.ssid[probe_req_frame->length] = '\0';
    }else{
        r.ssid[0] = '\0';
    }
    
    printf("\033[1;33m");
    printf("PACKET_CONTENT:\n");
    printf("\033[0m");
    for(int i=0; i<pkt_size; ++i){
        //printf("%02x ", (unsigned char)ipkt->payload[i]);
        printf("%02x ", ieee80211_pkt_binary[i]);

        if((i+1) % 24 == 0){
            printf("\n");
        }
    }
    printf("\n");


    // 5) hashed_pkt - USE A HASH FUNCTION IN ORDER TO HAVE A STRING TO PUT IN hashed_pkt
    esp_sha(SHA1, (const unsigned char*)ieee80211_pkt_binary, 24, r.hashed_pkt); //"ipkt->payload, pkt_size" al posto di "(const unsigned char*)ieee80211_pkt_binary, 24"

    // 6) seq_num
    r.seq_num = hdr->sequence_ctrl >> 4; //4 bits are the fragment number

    //aggiungi il record alla lista di record da inviare
    pSender->push_back(r);

    // PER STAMPARE HASH IN BASE64
    //uint8_t shaData[20]; //dimensione fissa di output di 20B
    //unsigned char shaBase64[100];
    //size_t outputLen;
    //mbedtls_base64_encode(shaBase64, 100, (size_t*)&outputLen, (const unsigned char*)shaData, (size_t)20 );
    //std::string str((const char *)shaBase64);
    //r.hashed_pkt = str;

    ///// stampa il pkt ricevuto
    printf_date(r.timestamp);

    printf("\033[0;36m");
    printf(" %u", r.timestamp);
    printf("\033[0m");

    printf(" %s", wifi_pkt_type2str((wifi_promiscuous_pkt_type_t)frame_ctrl->type, (wifi_mgmt_subtypes_t)frame_ctrl->subtype));

    printf(" CHAN=");
    printf("\033[1;32m");
    printf("%02d", ppkt->rx_ctrl.channel);
    printf("\033[0m");

    printf(", SEQ=");
    printf("\033[0;32m");
    printf("%d", r.seq_num);
    printf("\033[0m");

    printf(", RSSI=");
    printf("\033[1;32m");
    printf("%02d", ppkt->rx_ctrl.rssi);
    printf("\033[0m");

    printf(", \n>> SNDR=");
    printf("\033[1;32m");
    printf("%s", addr2);
    printf("\033[0m");

    printf(", SSID=\"");
    printf("\033[1;32m");
    printf("%s", r.ssid);
    printf("\033[0m");

    printf("\", HASH=\"");
    printf("\033[1;32m");
    printf("%s", r.hashed_pkt);
    printf("\033[0m");

    printf("\"\n");
    /////

    gpio_set_level(BLINK_GPIO, 1);
}

/**
 * NECESSARY BUSY WAITING FUNCTION
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

