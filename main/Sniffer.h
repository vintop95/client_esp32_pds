//Di Vincenzo Topazio
#ifndef SNIFFER_H_
#define SNIFFER_H_

#include "main.h"

#include "Sender.h"

#define	WIFI_CHANNEL_MAX		(13)
#define	WIFI_CHANNEL_SWITCH_INTERVAL	(500)

//pointer to sender for sending
extern void* pObject;

class Sniffer {
private:
    void wifi_sniffer_loop_channels();
    friend void wifi_sniffer_packet_handler(void*, wifi_promiscuous_pkt_type_t);
public:
    Sniffer(Sender* sndr);
    void init();
};

//per qualche arcano motivo questa struttura appare di 4 byte
//invece che di 2, quindi ho dovuto rimuovere duration_id dal
//wifi_ieee80211_mac_hdr_t, visto che non serve
typedef struct {
     unsigned protocol:2;
     unsigned type:2;
     unsigned subtype:4;
     unsigned to_ds:1;
     unsigned from_ds:1;
     unsigned more_frag:1;
     unsigned retry:1;
     unsigned pwr_mgmt:1;
     unsigned more_data:1;
     unsigned wep:1;
     unsigned strict:1;
} wifi_header_frame_control_t;

typedef struct {
	wifi_header_frame_control_t frame_ctrl;
	//unsigned duration_id:16;  //ugly hack!!!!
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	//uint8_t addr4[6]; /* a quanto pare non deve apparire */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;



typedef enum {
     ASSOCIATION_REQ,
     ASSOCIATION_RES,
     REASSOCIATION_REQ,
     REASSOCIATION_RES,
     PROBE_REQ,
     PROBE_RES,
     NU1, /* ......................*/
     NU2, /* 0110, 0111 not used */
     BEACON,
     ATIM,
     DISASSOCIATION,
     AUTHENTICATION,
     DEAUTHENTICATION,
     ACTION,
     ACTION_NACK,
} wifi_mgmt_subtypes_t;

typedef struct{
     unsigned interval:16;
     unsigned capability:16;
     unsigned tag_number:8;
     unsigned tag_length:8;
     char ssid[0];
     uint8_t rates[1];
} wifi_mgmt_beacon_t;

typedef struct{
    unsigned element_id:8;
    unsigned length:8;
    char ssid[0];
} wifi_mgmt_probe_req_t;



#endif /* SNIFFER_H_ */