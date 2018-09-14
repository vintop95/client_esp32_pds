//Di Vincenzo Topazio
#include "Sender.h"

static const char* LOG_TAG = "Sender";

Sender::Sender(Server* srv, int ms): t(new SenderTask()), msListenPeriod(ms), server(srv){
        t->setName("Sender");
        t->delay(msListenPeriod);
        t->start(this);
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

    ESP_LOGI(LOG_TAG, "SENDING ACCUMULATED RECORDS TO SERVER");

    server->send(string("DATA "));

    for(auto r: records){
        j = r;
        std::cout << j << std::endl;
        server->send(j);
    }

    server->sendEnd();

    return server->waitAck();
}

void Sender::push_back(Record r){
    records.push_back(r);
}
