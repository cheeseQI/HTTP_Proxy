#ifndef CLIENT_H
#define CLIENT_H 
#include "Socket.h"
#include "memory"
#include "iostream"
#include "sys/socket.h"
#include "unistd.h"
#include "netdb.h"
#include "fcntl.h"
#include "ExceptionHandler.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "arpa/inet.h"
#include "vector"
#include "string.h"
#include "uuid/uuid.h"
#include "fstream"
#include "mutex"
#include <chrono>
#include <ctime>
#include "SafeLog.h"
using namespace std;

class Client {
private:
    int serviceFd;
    unique_ptr<Socket> connectSocketPtr;
    string uuidStr;
    shared_ptr<SafeLog>& logFile;
public:
    Client(int fd, struct addrinfo * address, string uuidStr, shared_ptr<SafeLog>& logFile);
    void contactWithRemoteServer(string request);
    void contactWithRemoteClient(vector<char> sendBuffer);
    void contactInTunnel(string requestStr);
}; 
#endif