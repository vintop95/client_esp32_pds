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
#include "Record.h"
#include "CircularBuffer.h"

#include <esp_heap_caps.h>


/**
 * Gestisce la raccolta e l'invio dei Record usando l'interfaccia
 * offerta da Server
 */
class Sender {
private:
    //Period of time in which ESP32 sniffs packets expressed in milliseconds
    int msListenPeriod;
    CircularBuffer<Record> records;
    FreeRTOSTimer timer;

    bool sendRecordsToServer();
    friend void callback(FreeRTOSTimer *);
    friend void to_json(json& j, const Record& r);
public:
    Server* server;
    Sender(Server* srv, int ms);

    void push_back(const Record& r);
    void startSendingTimer();
};

#endif 