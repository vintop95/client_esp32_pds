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
 * - "OK": comunicazione ricevuta correttamente (per adesso solo dopo l'invio
 *   del messaggio INIT)
 * - "ERR": errore di ricezione
 * 
 * Ogni listenPeriod la sezione del sistema operativo FreeRTOS dedicata al 
 * timer chiamerà una callback contenente la funzione che gestisce l'invio 
 * dei Record accumulati durante il listenPeriod
 * Sembra più comodo avviare una nuova connessione TCP ogni listenPeriod 
 * anziché avviarla una volta sola e lasciarla attiva, ispirandosi al 
 * principio di protocollo stateless HTML
 * Il protocollo è da perfezionare, in particolare:
 * - Considerare l'introduzione di un messaggio END per 
 *   terminare la comunicazione
 * - Eventuali modifiche per il miglioramento delle prestazioni
 */
class Server {
private:
    int s;//socket
    string ipStr;
    int port;
public:
    void setIpPort(std::string, int);
    int connect();
    int send(std::string str);
    int send(json); 
    int sendInit(json j);
    int sendData(json j);
    int sendEnd();
    int waitAck(time_t* time_ptr = nullptr);
    void close();
};

#endif 