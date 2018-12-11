/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#ifndef RECORD_H_
#define RECORD_H_

/**
 * Struct that contains the details about the packet sniffed
 * for Indoor Positioning
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


#endif