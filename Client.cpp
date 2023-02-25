#include "Client.h"

Client::Client(int fd, struct addrinfo * address) {
    connectSocketPtr = make_unique<Socket>(address);
    int connectFd = connectSocketPtr->getFd();
    serviceFd = fd;
    if (connect(connectFd, address->ai_addr, address->ai_addrlen) == -1) {
        throw ConnectException();
    }
    freeaddrinfo(address);
}

void Client::contactWithRemoteServer(string request) {
    int connectFd = connectSocketPtr->getFd();
    if (send(connectFd, request.c_str(), request.length(), 0) == -1) {
            throw SendException();
    }
    // read and send msg to client;
    vector<char> receiveBuffer(65536);
    //fill_n(receiveBuffer.begin(), receiveBuffer.size(), 0);
    int dataLen = recv(connectFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0);
    if (dataLen == -1) {
        throw RecvException();
    }
    int dataIdx = dataLen;
    string responseStr(receiveBuffer.begin(), receiveBuffer.begin() + dataLen);
    HttpResponse httpResponse(responseStr);
    // cout << "Received first/part response from server " << " : \n";
    cout << "header:" << httpResponse.getHeader() << endl;
    int headerLen = httpResponse.getHeader().length();
    if (!httpResponse.getIsChuncked()) {
        int contentLen = httpResponse.getContentLength();
        // no length info
        if (contentLen == 0) {
            // todo: may need resize(double when half) here
            while ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) > 0) {
                dataIdx += dataLen;
            }
            if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
                throw RecvException();
            }
            receiveBuffer.resize(dataIdx);
            contactWithRemoteClient(receiveBuffer);
            return;
        }
        int totalLen = headerLen + contentLen + 4; // addition for /r/n/r/n
        receiveBuffer.resize(totalLen); 
        // if (dataIdx < totalLen) {
        //     fill_n(receiveBuffer.end() - (totalLen - dataIdx), totalLen - dataIdx, 0);
        // }
        while (dataIdx < contentLen) {
            if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
                throw RecvException();
            }
            dataIdx += dataLen;
        }
        // cout << "Received full response from server " << " : \n";
        // string full_response(receiveBuffer.begin(), receiveBuffer.end());
        // cout << full_response << endl;
        contactWithRemoteClient(receiveBuffer);
        return;
    }
    // deal with chunked blocks
    receiveBuffer.resize(dataIdx);
    contactWithRemoteClient(receiveBuffer);
    while ((dataLen = recv(connectFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0)) > 0) {
        receiveBuffer.resize(dataLen);
        contactWithRemoteClient(receiveBuffer);
    }
}

void Client::contactWithRemoteClient(vector<char> sendBuffer) {
    if (send(serviceFd, &sendBuffer.data()[0], sendBuffer.size(), 0) == -1) {
        throw SendException();
    }
    return;
}