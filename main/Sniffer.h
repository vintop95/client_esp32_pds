/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef SNIFFER_H_
#define SNIFFER_H_

#include "main.h"
#include "Task.h"

#include "Sender.h"

#include <hwcrypto/sha.h>
#include <mbedtls/base64.h>
#include <sys/time.h>

#define	WIFI_CHANNEL_MAX		        (13)
#define	WIFI_CHANNEL_SWITCH_INTERVAL	(500)

/**
 * @brief Pointer to a Sender defined out here in order
 * to use it in callbacks, defining it in the Sniffer class
 * make it impossible to be called by the callback
 */
extern Sender* pSender;

/**
 * @brief Gestisce l'ascolto di pacchetti in modalità promiscua e usa 
 * l'interfaccia offerta da Sender per accumulare i Record da inviare 
 * al server
 */
class Sniffer {
private:
    void wifi_sniffer_loop_channels();
    friend void wifi_sniffer_packet_handler(void*, 
                wifi_promiscuous_pkt_type_t);
public:
    Sniffer(Sender* sndr);
    ~Sniffer();
    void init();
};

/**
 * Structures that represents the bytes of the packet received by WiFi interface
 */

/**
 * @brief This describes the header frame control of packet
 * WARNING: It should be 2 bytes, but C++ makes it of 4 byte
 * so I had to remove duration_id from wifi_ieee80211_mac_hdr_t
 * but no worry because we do not need it
 */
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

/**
 * @brief This describes the header of a wifi packet
 * WARNING: duration_id is removed, look up wifi_header_frame_control_t
 * to know why
 */
typedef struct {
	wifi_header_frame_control_t frame_ctrl;
	//unsigned duration_id:16;  //WARNING: look wifi_header_frame_control_t
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */
	unsigned sequence_ctrl:16;
	//uint8_t addr4[6]; /* It must not appear in PROBE REQ packets */
} wifi_ieee80211_mac_hdr_t;

/**
 * @brief This describes the whole wifi packet
 */
typedef struct {
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) - originally uint8_t*/
} wifi_ieee80211_packet_t;

/**
 * @brief This describes the beginning of the payload of a wifi packet
 * Specifically of a PROBE REQ packet
 * The structure allows us to pick the ssid sent, if there is any
 */
typedef struct{
    unsigned element_id:8;
    unsigned length:8;
    char ssid[0];
} wifi_mgmt_probe_req_t;

/**
 * @brief This describes the beginning of the payload of a wifi packet
 * Specifically of a BEACON packet (not needed by us)
 */
typedef struct{
     unsigned interval:16;
     unsigned capability:16;
     unsigned tag_number:8;
     unsigned tag_length:8;
     char ssid[0];
     uint8_t rates[1];
} wifi_mgmt_beacon_t;

/**
 * @brief Enumerator of all possible subtypes of a WiFi packet
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

#endif /* SNIFFER_H_ */