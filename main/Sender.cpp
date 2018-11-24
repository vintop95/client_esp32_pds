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
    timer((char*)"listenTimer",
    pdMS_TO_TICKS(msListenPeriod),
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
 * @brief Initialize timestamp during the boot phase
 * 
 * @return 0 if all went well, otherwise a number != 0
 */
int Sender::initTimestamp(){
    int res = 0;
    uint32_t timestamp= 0;

    do{
        ESP_LOGI(LOG_TAG, "CONNECTING TO SERVER TO OBTAIN CURRENT TIME");

        //esp_wifi_set_promiscuous(false);
        res = server->connect();
        if (res != 0){
            ESP_LOGE(LOG_TAG, "CANNOT CONNECT TO SERVER. RETRYING...");
            //return -1;
            vTaskDelay( pdMS_TO_TICKS(RETRY_PERIOD_MS) ); // wait 
            continue; //retry
        }

        //send an 'END' to Server just to obtain the timestamp 
        server->sendEnd();
        res = server->waitAck(&timestamp);

        server->close();
        //esp_wifi_set_promiscuous(true);
    }while(res != 0);

    return res;
}

/**
 * @brief It sends the records to the server
 * This function works together with 'ClientHandler::readyRead()' function
 * in the server
 * 
 * @return 0 if all went well, otherwise a number != 0
 */
int Sender::sendRecordsToServer(){
    json j;
    int res = 0;
    
    if(records.size() != 0){
        ESP_LOGI(LOG_TAG, "SENDING ACCUMULATED RECORDS TO SERVER");

        res = server->connect();

        if (res != 0){
            return -1;
        }

        j = records;
        records.clear();

        server->sendData(j);
        res = server->waitAck();

        uint32_t timestamp;
        server->sendEnd();
        res = server->waitAck(&timestamp);

        server->close();
    }

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

/**
 * @brief Starts the timer.
 */
void Sender::start_timer(){
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
    //TODO: Ho commentato le linee set_promiscous
    //perché causavano malfunzionamenti

    //esp_wifi_set_promiscuous(false);

    Sender* pSender = (Sender *)pTimer->getData();
    pSender->sendRecordsToServer();

    //esp_wifi_set_promiscuous(true);
}

