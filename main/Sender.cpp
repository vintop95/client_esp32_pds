//Di Vincenzo Topazio
#include "Sender.h"

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
    for(auto r: records){
        j = r;
        std::cout << j << std::endl;
        server->send(j);
    }

    j = "END";
    server->send(j);

    return server->waitAck();
}

void Sender::push_back(Record r){
    printf("\nRECORD PUSHED BACK?\n");
    vector<Record> r_test;
    r_test.push_back(r);
    //records.push_back(r);
    printf("it seems so\n");
}
