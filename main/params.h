/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo - 2
 * Salvatore Di Cara - 1
 * Giorgio Pizzuto - 3
 * Vincenzo Topazio - 0
 */
#ifndef PARAMS_H_
    #define PARAMS_H_

    #define DEVICE_NAME "ESP0" 
    #define CONF_CHOSEN 0

    #if CONF_CHOSEN==0
        #define WIFI_SSID "xd"
        #define WIFI_PASS "ciao1234"
        #define SERVER_IP "192.168.43.5"
    #elif CONF_CHOSEN==1
        // #define WIFI_SSID "PCDITOTÒ 7614"
        // #define WIFI_PASS "Padrepio"
        // #define SERVER_IP "192.168.137.1"
        /////
        #define WIFI_SSID "xd"
        #define WIFI_PASS "ciao1234"
        #define SERVER_IP "192.168.43.211"
    #elif CONF_CHOSEN==2
        #define WIFI_SSID "My ASUS G"
        #define WIFI_PASS "gianluca"
        #define SERVER_IP "192.168.43.33"
    #elif CONF_CHOSEN==3
    #endif
#endif