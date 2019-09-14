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
 * Prototype of callback defined at the end of the file
 */
void callback(FreeRTOSTimer *pTimer);

/**
 * Sender constructor
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

// /**
//  * Called by json object to convert Record object in json
//  * 
//  * @param Reference of json element
//  * @param Const ref to the Record to convert
//  * 
//  * @return N/A.
//  */
// void to_json(json& j, const Record& r) {
//     // ESP_LOGI(LOG_TAG, "START JSON TRANSFORMATION");

//     char j_sender_mac[] = "00:00:00:00:00:00";
//     mac2str(r.sender_mac, j_sender_mac);
//     // ESP_LOGI(LOG_TAG, "j_sender_mac: %s", j_sender_mac);

//     const int sizeStringSha = 30+1;
//     unsigned char j_hashed_pkt[sizeStringSha];
//     size_t outputLen;
//     mbedtls_base64_encode(j_hashed_pkt, sizeStringSha, (size_t*)&outputLen, (const unsigned char*)r.hashed_pkt, (size_t)20 );
//     // ESP_LOGI(LOG_TAG, "j_hashed_pkt: %s", j_hashed_pkt);

//     j = json{
//                 {"sender_mac", std::string((const char *)j_sender_mac)},
//                 {"timestamp", r.timestamp},
//                 {"rssi", r.rssi}, 
//                 {"hashed_pkt", std::string((const char *)j_hashed_pkt, outputLen)}, 
//                 {"seq_num", r.seq_num}, 
//                 {"ssid", std::string((const char *)r.ssid)}
//             };
// }

/**
 * It sends the records to the server
 * This function works together with 'ClientHandler::readyRead()' function
 * in the server
 * 
 * @return true if all went well, false otherwise
 */
bool Sender::sendRecordsToServer(){

    bool recordsAreSent = false;

    recordsAreSent = server->send_records(records);

    return recordsAreSent;
}

/**
 * Push back record object in the vector of records
 * 
 * @param Record to push
 * 
 * @return N/A
 */
void Sender::push_back(const Record& r){
    records.push(r);
}

/**
 * Starts the timer for sending periodically the records.
 */
void Sender::startSendingTimer(){
	timer.start();
}

/**
 * Callback called by the Timer.
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

