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

#include "Sender.h"
#include "FrameStructures.h"

#define	WIFI_CHANNEL_MAX		     (13)
#define	WIFI_CHANNEL_SWITCH_INTERVAL	(500)

extern Sender* pSender;

/**
 * Gestisce l'ascolto di pacchetti in modalità promiscua e usa 
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
    void stop();
};

#endif /* SNIFFER_H_ */