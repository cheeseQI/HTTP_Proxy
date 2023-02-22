#include "Server.h"
#include "fcntl.h"
#include "functional"
#define PENDINGQUEUE 512  


Server::Server(struct addrinfo * address) {
    listenSocketPtr = make_unique<Socket>(address);
    int listenFd = listenSocketPtr->getFd();
    FD_ZERO(&readFds);
    FD_SET(listenFd, &readFds);
    fdMax = listenFd;
    threadPool = new ThreadPool(THREAD_NUM, &readFds);
    if (bind(listenFd, address->ai_addr, address->ai_addrlen) == -1) {
        throw ServerBindException();
    }
    // start listen, kernel will reject the new connection if pending connections exceed PENDINGQUEUE
    if (listen(listenFd, PENDINGQUEUE) == -1) {
        throw ServerListenException();
    }
    cout << "cool, the server start listening" << endl;
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
    // todo: only used for time-debugging, finally delete and also the select call tv param change to NULL
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    int listenFd = listenSocketPtr->getFd();
    while (true) {
        // copy on write
        fd_set cpFds = readFds;
        // start blocking select, any changes will be updated to read_fds
        int state;
        if ((state = select(fdMax + 1, &cpFds, NULL, NULL, &tv)) == -1) {
            throw SelectException();
        } else if (state == 0) {
            std::cout << "Loop ended after time out" << std::endl;
            break;
        }
        if (FD_ISSET(listenFd, &cpFds)) {
        // if the change is from listen socket, create a service socket for acccepting
            struct sockaddr_storage clientAddr;
            socklen_t addrLen = sizeof(clientAddr);
            memset(&clientAddr, 0, addrLen);
            int serviceFd = accept(listenFd, (struct sockaddr *)&clientAddr,  &addrLen);
            if (serviceFd == -1) {
                throw AcceptException();
            } else {
                struct sockaddr_in *s = (struct sockaddr_in*) &clientAddr;
                int port = ntohs(s->sin_port);
                char ip[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET, &(s->sin_addr), ip, INET_ADDRSTRLEN);
                cout << "successful connect to client ip: " << ip << " with port: " << port << endl;
                FD_SET(serviceFd, &readFds);
                fdMax = fdMax > serviceFd ? fdMax : serviceFd;
                // non-blocking socket
                // int flags = fcntl(serviceFd, F_GETFL, 0);
                // fcntl(serviceFd, F_SETFL, flags | O_NONBLOCK);
                threadPool->submit(serviceFd);
            }
        }
    }
}