//Di Vincenzo Topazio
#include "Sender.h"

static const char* LOG_TAG = "Sender";

Sender::Sender(Server* srv, int ms): msListenPeriod(ms), server(srv){
    //maybe using a task is not the best way
        //t->setName("Sender");
        //t->delay(msListenPeriod);
        //t->start(this);
}

void to_json(json& j, const Record& r) {
        j = json{{"sender_mac", r.sender_mac},
                 {"timestamp", r.timestamp},
                 {"rssi", r.rssi}, 
                 {"hashed_pkt", r.hashed_pkt}, 
                 {"ssid", r.ssid}};
}

//Function to call each listenPeriod
int Sender::sendRecordsToServer(){
    json j;
    int res = 0;
    
    ESP_LOGI(LOG_TAG, "SENDING ACCUMULATED RECORDS TO SERVER");

    res = server->connect();

    if (res != 0){
        return -1;
    }

    for(auto r: records){
        j = r;
        server->sendData(j);
    }
    //res = server->waitAck();

    server->sendEnd();
    server->close();

    return res;
}

void Sender::push_back(Record r){
    records.push_back(r);
}
