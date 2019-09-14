/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef FRAMES_STRUCTURES_H_
#define FRAMES_STRUCTURES_H_

/**
 * Structures that represents the bytes of the packet
 *  received by WiFi interface
 * 
 *                 wifi_ieee80211_packet_t
 *                            ||| 
 *             |-----------------------------------|
 *   wifi_ieee80211_mac_hdr_t                  [PAYLOAD]
 *            |                                    | 
 *            V                             V--------------|
 * wifi_header_frame_control_t    wifi_mgmt_probe_req_t/   |
 *            |                                            V
 *            V                                   wifi_mgmt_beacon_t
 *    wifi_mgmt_subtypes_t
 */

struct wifi_ieee80211_packet_t;
struct wifi_ieee80211_mac_hdr_t;
struct wifi_header_frame_control_t;
struct wifi_mgmt_probe_req_t;
struct wifi_mgmt_beacon_t;


///////////////////////////////////////////////////////////////////////
/**
 * This describes the header frame control of packet
 * WARNING: It should be 2 bytes, but C++ makes it of 4 byte
 * so I had to remove duration_id from wifi_ieee80211_mac_hdr_t
 * but no worry because we do not need it
 */
struct wifi_header_frame_control_t{
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
};

/**
 * Enumerator of all possible subtypes of a WiFi packet
 */
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

/**
 * This describes the header of a wifi packet
 * WARNING: duration_id is removed, look up wifi_header_frame_control_t
 * to know why
 */
struct wifi_ieee80211_mac_hdr_t{
	wifi_header_frame_control_t frame_ctrl;
	//unsigned duration_id:16;  //WARNING: look wifi_header_frame_control_t
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	//uint8_t addr4[6]; /* It must not appear in PROBE REQ packets */
};

///////////////////////////////////////////////////////////////////////
/**
 * This describes the beginning of the payload of a wifi packet
 * Specifically of a PROBE REQ packet
 * The structure allows us to pick the ssid sent, if there is any
 */
struct wifi_mgmt_probe_req_t{
    unsigned element_id:8;
    unsigned length:8;
    char ssid[0];
};

/**
 * This describes the beginning of the payload of a wifi packet
 * Specifically of a BEACON packet (not needed by us)
 */
struct wifi_mgmt_beacon_t{
     unsigned interval:16;
     unsigned capability:16;
     unsigned tag_number:8;
     unsigned tag_length:8;
     char ssid[0];
     uint8_t rates[1];
};

///////////////////////////////////////////////////////////////////////
/**
 * This describes the WHOLE wifi packet
 */
struct wifi_ieee80211_packet_t{
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) - originally uint8_t*/
};

#endif