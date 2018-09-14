//Di Vincenzo Topazio
#ifndef SERVER_H_
#define SERVER_H_

#include "main.h"

class Server {
private:
    int s;//socket
public:
    int connect(std::string ip, int port);
    int send(std::string str);
    int send(json); 
    int sendInit(json j);
    int sendData(json j);
    int sendEnd();
    int waitAck();
    void close();
};

#endif 