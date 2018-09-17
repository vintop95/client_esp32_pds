#include "main.h"

/////////tcp_test
#define RESOURCE "/demo/aphorisms.php"
#define WEBSITE "www.lucadentella.it"

// HTTP request
static const char *REQUEST = "GET " RESOURCE " HTTP/1.1\n"
	"Host: " WEBSITE "\n"
	"User-Agent: ESP32\n"
	"\n";

void tcp_test(void){
	// define connection parameters
	struct addrinfo hints = { };
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	// address info struct and receive buffer
    struct addrinfo *res;
	char recv_buf[100];
	
	// resolve the IP of the target website
	int result = getaddrinfo(WEBSITE, "80", &hints, &res);
	if((result != 0) || (res == NULL)) {
		printf("Unable to resolve IP for target website %s\n", WEBSITE);
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("Target website's IP resolved\n");


	// create a new socket
	//possibile farlo con wrapper, ma sembra più complicato
	//Socket sock = Socket();
	//int s = sock.createSocket();
	int s = socket(res->ai_family, res->ai_socktype, 0);
	if(s < 0) {
		printf("Unable to allocate a new socket\n");
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("Socket allocated, id=%d\n", s);

	// connect to the specified server
	result = connect(s, res->ai_addr, res->ai_addrlen);
	if(result != 0) {
		printf("Unable to connect to the target website\n");
		close(s);
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("Connected to the target website\n");

	// send the request
	printf("SENDING:\n%s", REQUEST);


	result = write(s, REQUEST, strlen(REQUEST));
		if(result < 0) {
		printf("Unable to send the HTTP request\n");
		close(s);
		while(1) vTaskDelay(1000 / portTICK_RATE_MS);
	}
	printf("HTTP request sent\n");
	
	// print the response
	printf("HTTP response:\n");
	printf("--------------------------------------------------------------------------------\n");
	int r;
	do {
		bzero(recv_buf, sizeof(recv_buf));
		r = read(s, recv_buf, sizeof(recv_buf) - 1);
		for(int i = 0; i < r; i++) {
			putchar(recv_buf[i]);
		}
	} while(r > 0);	
	printf("--------------------------------------------------------------------------------\n");

	close(s);
	printf("Socket closed\n");
	
	while(1) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void wifi_scan(WiFi wifi){
	auto ap_records = wifi.scan();
	int ap_num = ap_records.size();

	// print the list 
	printf("Found %d access points:\n", ap_num);
	printf("\n");
	printf("----------------------------------------------------------------\n");
	for(int i = 0; i < ap_num; i++)
		ap_records[i].toString();
	printf("----------------------------------------------------------------\n");

	// infinite loop
	xTaskCreate(&loop_task, "loop_task", 2048, NULL, 5, NULL);
}
