#include "Server.h"
#include "fcntl.h"
#include "functional"
#define PENDINGQUEUE 512  


Server::Server(struct addrinfo * address) {
    logFilePtr = make_shared<SafeLog>("/var/log/erss/proxy.log");
    FD_ZERO(&readFds);
    listenSocketPtr = make_unique<Socket>(address);
    int listenFd = listenSocketPtr->getFd();
    FD_SET(listenFd, &readFds);
    fdMax = listenFd;
    threadPool = new ThreadPool(THREAD_NUM, logFilePtr);
    if (bind(listenFd, address->ai_addr, address->ai_addrlen) == -1) {
        throw ServerBindException();
    }
    // start listen, kernel will reject the new connection if pending connections exceed PENDINGQUEUE
    if (listen(listenFd, PENDINGQUEUE) == -1) {
        throw ServerListenException();
    }
    cout << "cool, the server start listening" << endl;
    freeaddrinfo(address);
}

Server::~Server() {
    for (int i = 0; i <= fdMax; ++i) {
            if (FD_ISSET(i, &readFds)) {
                FD_CLR(i, &readFds);
                close(i);
            }
    }
    delete threadPool;
    cout << "cool, the server is deleted" << endl; 
}

unique_ptr<Socket>& Server::getListenSocketPtr() {
    return listenSocketPtr;
}

void Server::run() {
    int listenFd = listenSocketPtr->getFd();
    while (true) {
        int state;
        if ((state = select(listenFd + 1, &readFds, NULL, NULL, NULL)) == -1) {
            throw SelectException();
        } 
        // if the change is from listen socket, create a service socket for acccepting
        struct sockaddr_storage clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        memset(&clientAddr, 0, addrLen);
        int serviceFd = accept(listenFd, (struct sockaddr *) &clientAddr,  &addrLen);
        if (serviceFd == -1) {
            throw AcceptException();
        } else {
            struct sockaddr_in *s = (struct sockaddr_in*) &clientAddr;
            int port = ntohs(s->sin_port);
            char ip[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET, &(s->sin_addr), ip, INET_ADDRSTRLEN);
            cout << "successful connect to client ip: " << ip << " with port: " << port << endl;
            threadPool->submit(serviceFd);
        }
    }
}