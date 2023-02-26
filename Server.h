#ifndef SERVER_H
#define SERVER_H 
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
#include "ThreadPool.h"

using namespace std;

class Server {
private:
    unique_ptr<Socket> listenSocketPtr;
    shared_ptr<SafeLog> logFilePtr;
    fd_set readFds;
    int fdMax;
    ThreadPool *threadPool;
public:
    Server(struct addrinfo * address);
    ~Server();
    unique_ptr<Socket>&  getListenSocketPtr();
    void run();
}; 
#endif