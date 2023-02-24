#ifndef CLIENT_H
#define CLIENT_H 
#include "Socket.h"
#include "memory"
#include "iostream"
#include "sys/socket.h"
#include "unistd.h"
#include "netdb.h"
#include "ExceptionHandler.h"
#include "HttpResponse.h"
#include <arpa/inet.h>
#include "vector"
#include "string.h"

using namespace std;

class Client {
private:
    int serviceFd;
    unique_ptr<Socket> connectSocketPtr;
public:
    Client(int fd, struct addrinfo * address);
    void contactWithRemoteServer(string request);
    void contactWithRemoteClient(vector<char> sendBuffer);
}; 
#endif