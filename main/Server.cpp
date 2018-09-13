//Di Vincenzo Topazio
#include "Server.h"

int Server::connect(std::string ipStr, int port){
    const char* ip = ipStr.c_str();


	// create a new socket
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0) {
		printf("Unable to allocate a new socket\n");
		return s;
	}
	printf("Socket allocated, id=%d\n", s);

    struct sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);

    if(inet_aton(ip, &dest.sin_addr.s_addr ) <= 0)
    {
        printf("\n inet_pton error occured\n");
        return -1;
    } 
    printf("Ip %s converted in sin_addr\n", ip);

	// connect to the specified server
	int result = ::connect(s, (struct sockaddr*)&dest, sizeof(dest));
	if(result != 0) {
		printf("Unable to connect to the target\n");
		::close(s);
		return result;
	}
	printf("Connected to the target\n");

    return 0;
}

int Server::send(std::string str){

    const char* str_to_send = str.c_str();
	//receive buffer
    char recv_buf[100];

	// send the request
	printf("SENDING:\n%s\n", str_to_send);

	int result = write(s, str_to_send, strlen(str_to_send)+1 );
	if(result < 0) {
		printf("Unable to send records\n");
		::close(s);
		return result;
	}
	printf("Records sent\n");
	
   
	//response
    //IMPLEMENT A OK/ERR MESSAGE TO KNOW IF ALL WENT WELL
	printf("Response:\n");
	int r;
	do {
		bzero(recv_buf, sizeof(recv_buf));
	    r = read(s, recv_buf, sizeof(recv_buf) - 1);	
		for(int i = 0; i < r; i++) {
			putchar(recv_buf[i]);
		}
	} while(r > 0);	
	printf("\n");
	return 0;
}

void Server::close(){
    ::close(s);
	printf("Socket closed\n");
}