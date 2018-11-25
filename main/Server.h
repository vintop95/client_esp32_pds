/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef SERVER_H_
#define SERVER_H_

#include "main.h"

/**
 * Gestisce l'interfaccia di comunicazione con il server che ha il compito di
 * elaborare i Record inviati
 * L'invio avviene mediante il protocollo definito di seguito:
 * Client:
 * - "INIT [jSON dei dati di inizializzazione]": primo messaggio inviato al
 *   server per comunicare il nome del dispositivo esp32
 * - "DATA [jSON dei Record raccolti]": messaggio contenente i Record raccolti
 * Server:
 * - "END": messaggio per chiusura commessiome 
 * - "OK [timestamp]": comunicazione ricevuta correttamente
 *      con eventuale timestamp (uint32_t)
 * - "ERR": errore di ricezione
 * 
 * Ogni listenPeriod la sezione del sistema operativo FreeRTOS dedicata al 
 * timer chiamerà una callback contenente la funzione che gestisce l'invio 
 * dei Record accumulati durante il listenPeriod
 * Sembra più comodo avviare una nuova connessione TCP ogni listenPeriod 
 * anziché avviarla una volta sola e lasciarla attiva, ispirandosi al 
 * principio di protocollo stateless HTML
 */
class Server {
private:
    WiFi* pWifi;
    int s;//socket
    string ipStr;
    int port;
    TaskHandle_t ledBlinkTask = NULL;

    int send(std::string str);
    int send(json); 
public:
    Server(WiFi* p):pWifi(p){}
    void setIpPort(std::string, int);

    int wifi_connect();
    int connect();

    int init_timestamp();
    
    int sendInit(json j);
    int sendData(json j);
    int sendEnd();
    int waitAck(uint32_t* time_ptr = nullptr);

    void close();
};

#endif 