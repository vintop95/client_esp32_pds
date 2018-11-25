/**
 * PDS Project - Client ESP32
 * Gianluca D'Alleo
 * Salvatore Di Cara
 * Giorgio Pizzuto
 * Vincenzo Topazio
 */
#include "Server.h"


static const char* LOG_TAG = "Server";

/**
 * @brief Wrapper of strncmp
 * 
 * @param First string to compare
 * @param Second string to compare
 * 
 * @return true if they are equal
 */
bool eqStr(const char* str1, const char* str2){
    return (strncmp(str1,str2,strlen(str2)) == 0);
}

/**
 * @brief It sets the ip and port of the server
 * 
 * @param IP of the server
 * @param Port of the server
 * 
 * @return N/A
 */
void Server::setIpPort(std::string _ip, int _port){
	ipStr = _ip;
	port = _port;
}

int Server::wifi_connect(){
	int attempts = 0;
	while(!pWifi->isConnectedToAP() && attempts < MAX_ATTEMPTS){
		++attempts;
		ESP_LOGI(LOG_TAG, "CONNECT TO WIFI: ATTEMPT #%d", attempts);
		pWifi->connectAP(WIFI_SSID, WIFI_PASS);

		vTaskDelay( pdMS_TO_TICKS(RETRY_PERIOD_MS/2) );
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay( pdMS_TO_TICKS(RETRY_PERIOD_MS/2) );
		gpio_set_level(BLINK_GPIO, 1);
	}

	if(!pWifi->isConnectedToAP()){
		IS_WIFI_CONNECTED = 0;
		return -1;
	}else{
		IS_WIFI_CONNECTED = 1;
		return 0;
	}
}

void led_blink(void *pvParameter){
	int ms_freq_standard = RETRY_PERIOD_MS/4;
	int* ms_freq = &ms_freq_standard;
	if(pvParameter != NULL){
		ms_freq = (int*) (pvParameter);
	}
	while(1){
		vTaskDelay( pdMS_TO_TICKS(*ms_freq) );
		gpio_set_level(BLINK_GPIO, 0);
		vTaskDelay( pdMS_TO_TICKS(*ms_freq) );
		gpio_set_level(BLINK_GPIO, 1);
	}
}

/**
 * @brief It connects to the server
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::connect(){
	
    const char* ip = ipStr.c_str();
	
	// create a new socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0) {
		ESP_LOGE(LOG_TAG, "Unable to allocate a new socket");
		return s;
	}
	ESP_LOGI(LOG_TAG, "Socket allocated, id=%d", s);

    struct sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    if(inet_aton(ip, &dest.sin_addr.s_addr ) <= 0)
    {
        ESP_LOGE(LOG_TAG, "inet_pton error occured");
        return -1;
    }

	// connect to the specified server
	xTaskCreate(&led_blink, "led_blink", 512, NULL, 5, &ledBlinkTask );
	int result = ::connect(s, (struct sockaddr*)&dest, sizeof(dest));
	if(result != 0) {
		ESP_LOGE(LOG_TAG, "Unable to connect to the target %s:%d", ip, port);
		::close(s);
		return result;
	}else{
		gpio_set_level(BLINK_GPIO, 1);
		vTaskDelete(ledBlinkTask);
	}
	ESP_LOGI(LOG_TAG, "Connected to the target %s:%d", ip, port);

	//send info about initializing the connection
	json j;
	j["name"] = DEVICE_NAME;
	
	this->sendInit(j);
	this->waitAck();
    return 0;
}

/**
 * @brief Initialize timestamp during the boot phase
 * 
 * @return 0 if all went well, otherwise a number != 0
 */
int Server::init_timestamp(){
	int timeProtocolDidntWentWell;
    uint32_t timestamp = 0;

    do{
        ESP_LOGI(LOG_TAG, "CONNECTING TO SERVER TO OBTAIN CURRENT TIME");

        timeProtocolDidntWentWell = connect();
        if (timeProtocolDidntWentWell){
            ESP_LOGE(LOG_TAG, "CANNOT CONNECT TO SERVER. RETRYING...");
            vTaskDelay( pdMS_TO_TICKS(RETRY_PERIOD_MS) ); // wait 
            continue; //retry
        }

        //send an 'END' to Server just to obtain the timestamp 
        sendEnd();
        timeProtocolDidntWentWell = waitAck(&timestamp);

        close();
    }while(IS_WIFI_CONNECTED && timeProtocolDidntWentWell);

	// TURN OFF THE FAST BLINKING OF SERVER CONNECTING
	// IF WIFI DISCONNECTED BECAUSE NOW WE RETRY TO CONNECT TO WIFI
	// AND WE MUST USE SLOW BLINKING
	if( !IS_WIFI_CONNECTED && ledBlinkTask != NULL ){
		vTaskDelete(ledBlinkTask);
		ledBlinkTask = NULL;
	}
	
    return timeProtocolDidntWentWell;
}

/**
 * @brief It sends a std::string to the server
 * 
 * @param String to send
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::send(std::string str){
	const char *str_to_send = str.c_str();

	// send the request
	ESP_LOGI(LOG_TAG, "SENDING:\n%s", str_to_send);
	int result = write(s, str_to_send, strlen(str_to_send)+1 );
	if(result < 0) {
		ESP_LOGE(LOG_TAG, "Unable to send record");
		::close(s);
		return result;
	}
	ESP_LOGI(LOG_TAG, "Record sent");

	return 0;
}

//TODO: create buffer for more elements at once?
/**
 * @brief It sends a json object to the server
 * 
 * @param Json to send
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::send(json j){
	return this->send(j.dump());
}

/**
 * @brief It sends a INIT message to the server
 * 
 * @param Json to send
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::sendInit(json j){
	return this->send("INIT " + j.dump());
}

/**
 * @brief It sends a DATA message to the server
 * 
 * @param Json to send
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::sendData(json j){
	return this->send("DATA " + j.dump());
}

/**
 * @brief It sends a END message to the server
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::sendEnd(){
	return this->send(string("END"));
}

/**
 * @brief It waits the ack from the server
 * - OK: all went well
 * - ERR: some error occurred
 * 
 * @return 0 if all went well, a number != 0 otherwise
 */
int Server::waitAck(uint32_t* time_ptr){
	//receive buffer
    char recv_buf[20];

    //IMPLEMENT A OK/ERR MESSAGE TO KNOW IF ALL WENT WELL
	ESP_LOGI(LOG_TAG, "RESPONSE:");
	int r=0;
	do {
		memset(recv_buf, 0, sizeof(recv_buf));
	    r = read(s, recv_buf, sizeof(recv_buf) - 1);
		for(int i = 0; i < r; i++) {
			putchar(recv_buf[i]);
		}
		if(eqStr(recv_buf,"OK")){
			ESP_LOGI(LOG_TAG, "SERVER ACK IS OK");

			//e' richiesto il timestamp? se si lo posiziono nel puntatore
			if(time_ptr != nullptr){
				//reading timestamp from buffer
				uint32_t timestamp;
				memcpy(&timestamp,recv_buf + strlen("OK ") , sizeof(uint32_t));//WARNING
				timestamp = ntohl(timestamp);
				printf("TIMESTAMP: %u\n", timestamp);
				*time_ptr = timestamp;

				//set received time as current time
				struct timeval now;
				now.tv_sec = timestamp;
				int r = settimeofday(&now, NULL);
				if(r != 0){
					ESP_LOGE(LOG_TAG, "CANNOT SET TIME OF DAY");
					return -1;
				}else{
					time_t tv;
					time(&tv);
					std::string text = "TIME OF DAY SET TO ";
					text += std::to_string(tv);
					text += " (" + std::to_string(now.tv_sec) + ")\n";
					esp_log_write(ESP_LOG_INFO, LOG_TAG, text.c_str() );
				}
			}

			return 0;
		}else{
			ESP_LOGE(LOG_TAG, "THERE WAS A PROBLEM IN THE SENDING");
			return -1;
		}
	} while(r > 0);	
	printf("\n");

}

/**
 * @brief Close the socket with the server
 */
void Server::close(){
    ::close(s);
	ESP_LOGI(LOG_TAG, "Socket closed");
}