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
#include "Task.h"

#include <esp_heap_caps.h>

/**
 * @brief Struttura che raccoglie le 
 * informazioni dei pacchetti ascoltati necessari per l'elaborazione
 */
struct Record{
    // SIZE:
    // 6+4+1+2+20+33 = 66
    // but it says 72 -> MEMORY ALIGNMENT
    uint8_t sender_mac[6]; //char sender_mac[17+1]; //6 BYTE
    uint32_t timestamp; //4 BYTE
    int8_t rssi; //1 BYTE
    int16_t seq_num; //2 BYTE
    uint8_t hashed_pkt[20]; //char hashed_pkt[28+1]; //20 BYTE
    char ssid[32+1]; //33 BYTE

    // Record(){
    //     ESP_LOGI("Record", "SIZEOF:");
    //     ESP_LOGI("Record", "sender_mac: %d", sizeof(sender_mac));
    //     ESP_LOGI("Record", "timestamp: %d", sizeof(timestamp));
    //     ESP_LOGI("Record", "rssi: %d", sizeof(rssi));
    //     ESP_LOGI("Record", "seq_num: %d", sizeof(seq_num));
    //     ESP_LOGI("Record", "hashed_pkt: %d", sizeof(hashed_pkt));
    //     ESP_LOGI("Record", "ssid: %d", sizeof(ssid));
    //     ESP_LOGI("Record", "Record: %d", sizeof(Record));
    // }
};

/**
 * CIRCULAR BUFFER
 */
template <class T>
class CircularBuffer
{
private:
    std::map<int,T> buffer;
    int max_size;
    int index_first; //POP
    int index_next; //PUSH
    int m_size;

public:
    CircularBuffer(): 
    index_first(0), index_next(0), m_size(0){

        int bytes_to_leave = 8192;
        // CALCULATE MAX SIZE
        // https://esp32.com/viewtopic.php?t=3802
        // https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/include/heap/esp_heap_caps.h
        // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/mem_alloc.html
        int free_size_bytes = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
        ESP_LOGI("CircularBuffer", "BYTES FREE MALLOC_CAP_DEFAULT: %d", free_size_bytes);
        ESP_LOGI("CircularBuffer", "BYTES FREE MALLOC_CAP_8BIT: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT));
        ESP_LOGI("CircularBuffer", "BYTES FREE MALLOC_CAP_32BIT: %d", heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT));
        int max_elem_size = sizeof(T);
        ESP_LOGI("CircularBuffer", "SIZE OF TYPE: %d", max_elem_size);

        if(free_size_bytes < bytes_to_leave){
            throw std::bad_alloc();
        }
        
        int size_allocable = (free_size_bytes - bytes_to_leave);  

        max_size = size_allocable / max_elem_size;
        ESP_LOGI("CircularBuffer", "LEAVE %d BYTES FREE", bytes_to_leave);
        ESP_LOGI("CircularBuffer", "CIRCULAR BUFFER OF MAX %d ELEMENTS CONSTRUCTED", max_size);
    }

    int size() const{
        ESP_LOGI("CircularBuffer", "CURRENT SIZE=%d", m_size);
        return m_size < max_size ? m_size : max_size;
    }

    void increaseSize(){
        if(m_size < max_size){
            m_size++;
        }
    }

    void decreaseSize(){
        if(m_size > 0){
            m_size--;
        }
    }

    bool isPoppable() const{
        return !(index_next == index_first);
    }

    int increaseWithModule(int val) const{
        int new_val = (val + 1) % max_size;
        ESP_LOGI("CircularBuffer", "INCREASE INDEX FROM %d TO %d", val, new_val);
        return new_val;
    }

    void clear(){
        index_next = index_first;
        m_size = 0;
    }

    void push(const T& t){
        ESP_LOGI("CircularBuffer", "PUSH RECORD");
        increaseSize();
        buffer[index_next] = t;
        ESP_LOGI("CircularBuffer", "FOR INDEX_NEXT:");
        index_next = increaseWithModule(index_next);
        // index_next non deve mai superare index_first
        // da dietro
        if (!isPoppable()){
            ESP_LOGI("CircularBuffer", "FOR INDEX_FIRST:");
            index_first = increaseWithModule(index_first);
        }
        size();
    }    

    T pop(){
        if(isPoppable()){
            ESP_LOGI("CircularBuffer", "POP RECORD");
            decreaseSize();
            T& t = buffer[index_first];
            ESP_LOGI("CircularBuffer", "FOR INDEX_FIRST:");
            index_first = increaseWithModule(index_first);
            size();
            return t;
        }else{
            throw std::bad_typeid();
        }
    }

};

/**
 * @brief Gestisce la raccolta e l'invio dei Record usando l'interfaccia
 *  offerta da Server
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