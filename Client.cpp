#include "Client.h"

Client::Client(struct addrinfo * address) {
    connectSocketPtr = make_unique<Socket>(address);
    int connectFd = connectSocketPtr->getFd();
    if (connect(connectFd, address->ai_addr, address->ai_addrlen) == -1) {
        throw ConnectException();
    }
    freeaddrinfo(address);
}

void Client::contactWithRemoteServer(string request) {
    char sendBuffer[2048];
    int connectFd = connectSocketPtr->getFd();
    strncpy(sendBuffer, request.c_str(), request.length());
    if (send(connectFd, sendBuffer, request.length(), 0) == -1) {
            throw SendException();
    }
    // read and send msg to client;
    vector<char> buffer(2048);
    int dataIdx = 0;
    int dataLen = 0;
    while ((dataLen = recv(connectFd, &buffer.data()[dataIdx], buffer.size() - dataIdx, 0)) > 0) {
        dataIdx += dataLen;
        // enlarge when meet half
        if (dataIdx >= (int)buffer.size() / 2) {
            buffer.resize(buffer.size() * 2);
        }
        cout << "Received message from server " << " : \n";
        string requestStr(buffer[0], buffer[dataIdx]);
        for (int i = 0; i < dataIdx; i ++) {
            cout << buffer[i];
        } 
    }
    if (dataLen == -1) {
        throw RecvException();
    }

    close(connectFd);
}