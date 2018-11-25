/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#include "Sender.h"
#include <time.h>
#include <sys/time.h>

static const char* LOG_TAG = "Sender";

/**
 * @brief Prototype of callback defined at the end of the file
 */
void callback(FreeRTOSTimer *pTimer);

/**
 * @brief Sender constructor
 */
Sender::Sender(Server* srv, int ms): msListenPeriod(ms), 
    timer((char*)"listenTimer", pdMS_TO_TICKS(msListenPeriod),
    pdTRUE, this, &callback),
    server(srv)
{
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
 * It sends the records to the server
 * This function works together with 'ClientHandler::readyRead()' function
 * in the server
 * 
 * @return true if all went well, false otherwise
 */
bool Sender::sendRecordsToServer(){

    bool recordsAreSent = false;

    if(records.size() != 0){
        ESP_LOGI(LOG_TAG, "SENDING ACCUMULATED RECORDS TO SERVER");

        json j = records;
        recordsAreSent = server->send_records(j);
        records.clear();
    }

    return recordsAreSent;
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

/**
 * @brief Starts the timer for sending periodically the records.
 */
void Sender::startSendingTimer(){
	timer.start();
}

/**
 * @brief Callback called by the Timer.
 *
 * @param Pointer to the Timer Object
 * 
 * @return N/A.
 */
void callback(FreeRTOSTimer *pTimer) {
    // TODO: Ho commentato le linee set_promiscous
    // perché causavano malfunzionamenti

    // esp_wifi_set_promiscuous(false);

    if(IS_WIFI_CONNECTED){
        Sender* pSender = (Sender *)pTimer->getData();
        pSender->sendRecordsToServer();
    }

    // esp_wifi_set_promiscuous(true);
}

