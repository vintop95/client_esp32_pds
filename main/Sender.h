/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef SENDER_H_
#define SENDER_H_

#include "main.h"

#include "Server.h"
#include "Task.h"

/**
 * @brief Struttura che raccoglie le 
 * informazioni dei pacchetti ascoltati necessari per l'elaborazione
 */
struct Record{
    std::string sender_mac;
    uint32_t timestamp;
    int8_t rssi;
    std::string hashed_pkt;
    std::string ssid;
};

/**
 * @brief Gestisce la raccolta e l'invio dei Record usando l'interfaccia
 *  offerta da Server
 */
class Sender {
private:
    //Period of time in which ESP32 sniffs packets expressed in milliseconds
    int msListenPeriod;
    std::vector<Record> records;
    FreeRTOSTimer timer;
public:
    Server* server;
    Sender(Server* srv, int ms);
    int sendRecordsToServer();
    int initTimestamp();
    int getListenPeriod(){
        return msListenPeriod;
    }
    void push_back(Record r);
    void start_timer();
    friend void to_json(json& j, const Record& r);

};

#endif 