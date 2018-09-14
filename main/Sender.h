//Di Vincenzo Topazio
#ifndef SENDER_H_
#define SENDER_H_

#include "main.h"

#include "Server.h"
#include "Task.h"

struct Record{
    std::string sender_mac;
    uint32_t timestamp;
    int8_t rssi;
    std::string hashed_pkt;
    std::string ssid;
};

class SenderTask;

class Sender {
private:
    unique_ptr<SenderTask> t;
    int msListenPeriod;
    std::vector<Record> records;
public:
    Server* server;
    Sender(Server* srv, int ms);
    int sendRecordsToServer();

    void push_back(Record r);
    friend void to_json(json& j, const Record& r);
};

class SenderTask: public Task{
public:
    void run(void *data){
        Sender* sender = (Sender *)data; 
        sender->sendRecordsToServer();
    }
};

#endif 