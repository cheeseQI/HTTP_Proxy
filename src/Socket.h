#ifndef SOCKET_H
#define SOCKET_H 
#include "memory"
#include "iostream"
#include "sys/socket.h"
#include "unistd.h"
#include "netdb.h"
#include "ExceptionHandler.h"
using namespace std;

class Socket {
private:
    int socketFd;
public:
    Socket(struct addrinfo * address);
    ~Socket();
    int getFd();
}; 
#endif