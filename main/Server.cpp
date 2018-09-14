//Di Vincenzo Topazio
#include "Server.h"

static const char* LOG_TAG = "Server";

int Server::connect(std::string ipStr, int port){
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
	int result = ::connect(s, (struct sockaddr*)&dest, sizeof(dest));
	if(result != 0) {
		ESP_LOGE(LOG_TAG, "Unable to connect to the target");
		::close(s);
		return result;
	}
	ESP_LOGI(LOG_TAG, "Connected to the target");

    return 0;
}

//TODO: create buffer for more elements at once?
int Server::send(json j){

	std::cout << "DEBUG: ipStr da inviare " << j.dump() << std::endl;

	printf("\n");

	const char *str_to_send = j.dump().c_str();

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

int Server::waitAck(){
	//receive buffer
    char recv_buf[100];

    //IMPLEMENT A OK/ERR MESSAGE TO KNOW IF ALL WENT WELL
	ESP_LOGI(LOG_TAG, "RESPONSE:");
	int r;
	do {
		memset(recv_buf, 0, sizeof(recv_buf));
	    r = read(s, recv_buf, sizeof(recv_buf) - 1);	
		for(int i = 0; i < r; i++) {
			putchar(recv_buf[i]);
		}
	} while(r > 0);	
	printf("\n");

	if(strcmp(recv_buf,"OK")==0){
		return 0;
	}else{
		return -1;
	}
}

void Server::close(){
    ::close(s);
	ESP_LOGI(LOG_TAG, "Socket closed");
}