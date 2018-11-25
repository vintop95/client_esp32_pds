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
 * Wrapper of strncmp
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
 * It sets the ip and port of the server
 * 
 * @param IP of the server
 * @param Port of the server
 * 
 * @return N/A
 */
void Server::set_ip_port(std::string _ip, int _port){
	ipStr = _ip;
	port = _port;
}

/**
 * It tries to connect to the wifi network saved in the settings
 * until MAX_ATTEMPTS is reached
 * 
 * @return true if all went well, false otherwise
 */
bool Server::wifi_connect(){
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
		return false;
	}else{
		IS_WIFI_CONNECTED = 1;
		return true;
	}
}


/**
 * @brief It connects to the server
 * 
 * @return true if all went well, false otherwise
 */
bool Server::connect(){
	
    const char* ip = ipStr.c_str();
	
	// create a new socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0) {
		ESP_LOGE(LOG_TAG, "Unable to allocate a new socket");
		return false;
	}
	ESP_LOGI(LOG_TAG, "Socket allocated, id=%d", s);

    struct sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    if(inet_aton(ip, &dest.sin_addr.s_addr ) <= 0)
    {
        ESP_LOGE(LOG_TAG, "inet_pton error occured");
        return false;
    }

	// Start fast blinking
	int freq = RETRY_PERIOD_MS/2;
	xTaskCreate(&led_blink, "fast_blink", 512, &freq, 5, &ledBlinkTask );

	// Connect to the specified server
	int notConnected = ::connect(s, (struct sockaddr*)&dest, sizeof(dest));
	if(notConnected) {
		ESP_LOGE(LOG_TAG, "Unable to connect to the target %s:%d", ip, port);
		::close(s);
		return false;
	}else{
		// Close task of fast blinking because we are totally connected
		// and we must leave the led on
		vTaskDelete(ledBlinkTask);
		gpio_set_level(BLINK_GPIO, 1);
	}
	ESP_LOGI(LOG_TAG, "Connected to the target %s:%d", ip, port);

	// Send info to initialize the connection
	json j;
	j["name"] = DEVICE_NAME;
	
	this->send_init(j);
	this->wait_ack();

    return true;
}

/**
 * @brief Send records in the json object to the server
 * with the SCILP protocol 
 * [Sniffer Communication for Indoor Localization Protocol]
 * 
 * @return true if all went well, false otherwise
 */
bool Server::send_records(json j){

	bool isConnected = this->connect();
	if (!isConnected){
		return false;
	}

	gpio_set_level(BLINK_GPIO, 0);
	
	this->send_data(j);

	bool ackOK = this->wait_ack();
	if (!ackOK){
		return false;
	}

	uint32_t timestamp;
	this->send_end();
	bool recordsAreSent = this->wait_ack(&timestamp);

	this->close();

	gpio_set_level(BLINK_GPIO, 1);

	return recordsAreSent;
}

/**
 * @brief Initialize timestamp during the boot phase
 * 
 * @return true if all went well, false otherwise
 */
bool Server::init_timestamp(){
	bool timeProtocolWentWell;
    uint32_t timestamp = 0;

    do{
        ESP_LOGI(LOG_TAG, "CONNECTING TO SERVER TO OBTAIN CURRENT TIME");

        timeProtocolWentWell = this->connect();
        if ( !timeProtocolWentWell ){
            ESP_LOGE(LOG_TAG, "CANNOT CONNECT TO SERVER. RETRYING...");
            vTaskDelay( pdMS_TO_TICKS(RETRY_PERIOD_MS) ); // wait 
            continue; //retry
        }

        this->send_end();
        timeProtocolWentWell = wait_ack(&timestamp);

        this->close();
    }while( IS_WIFI_CONNECTED && !timeProtocolWentWell );

	// TURN OFF THE FAST BLINKING OF SERVER CONNECTING
	// IF WIFI DISCONNECTED BECAUSE NOW WE RETRY TO CONNECT TO WIFI
	// AND WE MUST USE SLOW BLINKING
	if( !IS_WIFI_CONNECTED && ledBlinkTask != NULL ){
		vTaskDelete(ledBlinkTask);
		ledBlinkTask = NULL;
	}

    return timeProtocolWentWell;
}

/**
 * @brief It sends a std::string to the server
 * 
 * @param String to send
 * 
 * @return true if all went well, false otherwise
 */
bool Server::send(std::string str){
	const char *str_to_send = str.c_str();

	ESP_LOGI(LOG_TAG, "SENDING:\n%s", str_to_send);
	int result = write(s, str_to_send, strlen(str_to_send)+1 );
	if(result < 0) {
		ESP_LOGE(LOG_TAG, "Unable to send record");
		::close(s);
		return false;
	}else{
		ESP_LOGI(LOG_TAG, "Record sent");
		return true;
	}
}

/**
 * @brief It sends a INIT message to the server
 * 
 * @param Json to send
 * 
 * @return true if all went well, false otherwise
 */
bool Server::send_init(json j){
	return this->send("INIT " + j.dump());
}

/**
 * @brief It sends a DATA message to the server
 * 
 * @param Json to send
 * 
 * @return true if all went well, false otherwise
 */
bool Server::send_data(json j){
	return this->send("DATA " + j.dump());
}

/**
 * @brief It sends a END message to the server
 * 
 * @return 0 true if all went well, false otherwise
 */
bool Server::send_end(){
	return this->send(string("END"));
}

/**
 * @brief It waits the ack from the server
 * - OK [timestamp]: all went well
 * - ERR: some error occurred
 * 
 * @param if you pass a not null time_ptr, this function
 * checks for the timestamp in the message and tries
 * to set the time to the timestamp found, otherwise
 * if you pass a nullptr it doesn't check the timestamp
 * 
 * @return true if the sending went well, false otherwise
 */
bool Server::wait_ack(uint32_t* time_ptr){
	//receive buffer
    char recv_buf[20];

    //IMPLEMENT A OK/ERR MESSAGE TO KNOW IF ALL WENT WELL
	ESP_LOGI(LOG_TAG, "RESPONSE:");
	int r=0;
	do {
		memset(recv_buf, 0, sizeof(recv_buf));
	    r = read(s, recv_buf, sizeof(recv_buf) - 1);

		// Print the buffer received
		for(int i = 0; i < r; i++) {
			putchar(recv_buf[i]);
		}

		// IF MESSAGE IS OK
		if(eqStr(recv_buf,"OK")){
			ESP_LOGI(LOG_TAG, "SERVER ACK IS OK");

			// Is timestamp requested? If yes, i put it in the pointer
			if(time_ptr != nullptr){
				// Reading timestamp from buffer
				uint32_t timestamp;
				memcpy(&timestamp,recv_buf + strlen("OK ") , sizeof(uint32_t));
				timestamp = ntohl(timestamp);
				printf("TIMESTAMP: %u\n", timestamp);

				//set received time as current time
				struct timeval now;
				now.tv_sec = timestamp;
				int r = settimeofday(&now, NULL);
				if(r != 0){
					ESP_LOGE(LOG_TAG, "CANNOT SET TIME OF DAY");
				}else{
					*time_ptr = timestamp;
					time_t tv;
					time(&tv);
					std::string text = "TIME OF DAY SET TO ";
					text += std::to_string(tv);
					text += " (" + std::to_string(now.tv_sec) + ")\n";
					esp_log_write(ESP_LOG_INFO, LOG_TAG, text.c_str() );
				}
			}

			return true;
		}else{
			ESP_LOGE(LOG_TAG, "THERE WAS A PROBLEM SENDING");
			return false;
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