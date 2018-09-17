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

class Sender {
private:
    int msListenPeriod;
    std::vector<Record> records;
public:
    Server* server;
    Sender(Server* srv, int ms);
    int sendRecordsToServer();
    int getListenPeriod(){
        return msListenPeriod;
    }
    void push_back(Record r);
    friend void to_json(json& j, const Record& r);
};

#endif 