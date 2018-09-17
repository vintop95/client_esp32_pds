/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#include "Sender.h"

static const char* LOG_TAG = "Sender";

/**
 * @brief Sender constructor
 */
Sender::Sender(Server* srv, int ms): msListenPeriod(ms), server(srv){
    //maybe using a task is not the best way
    //but if we need to, here is the functions needed
    //t->setName("Sender");
    //t->delay(msListenPeriod);
    //t->start(this);
}

/**
 * @brief Called by json object to convert Record object in json
 * 
 * @param Reference of json element
 * @param Const ref to the Record to convert
 * 
 * @return N/A.
 */
void to_json(json& j, const Record& r) {
        j = json{{"sender_mac", r.sender_mac},
                 {"timestamp", r.timestamp},
                 {"rssi", r.rssi}, 
                 {"hashed_pkt", r.hashed_pkt}, 
                 {"ssid", r.ssid}};
}

/**
 * @brief It sends the records to the server
 * 
 * @return 0 if all went well, otherwise a number != 0
 */
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

/**
 * @brief Push back record object in the vector of records
 * 
 * @param Record to push
 * 
 * @return N/A
 */
void Sender::push_back(Record r){
    records.push_back(r);
}
