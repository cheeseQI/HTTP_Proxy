#ifndef CLIENT_H
#define CLIENT_H 
#include "Socket.h"
#include "memory"
#include "iostream"
#include "sys/socket.h"
#include "unistd.h"
#include "netdb.h"
#include "ExceptionHandler.h"
#include <arpa/inet.h>
#include "vector"
#include "string.h"

using namespace std;

class Client {
private:
    unique_ptr<Socket> connectSocketPtr;
public:
    Client(struct addrinfo * address);
    void contactWithRemoteServer(string request);
}; 
#endif