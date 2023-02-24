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
    // char sendBuffer[65536];
    int connectFd = connectSocketPtr->getFd();
    // strncpy(sendBuffer, request.c_str(), request.length());
    if (send(connectFd, request.c_str(), request.length(), 0) == -1) {
            throw SendException();
    }
    // read and send msg to client;
    vector<char> receiveBuffer(65536);
    int dataLen = recv(connectFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0);
    if (dataLen == -1) {
        throw RecvException();
    }
    int dataIdx = dataLen;
    string responseStr(receiveBuffer.begin(), receiveBuffer.begin() + dataLen);
    HttpResponse httpResponse(responseStr);
    // cout << "Received response from server " << " : \n";
    cout << "data:" << responseStr << endl;
    cout << "header:" << httpResponse.getHeader() << endl;
    int headerLen = httpResponse.getHeader().length();
    if (!httpResponse.getIsChuncked()) {
        int contentLen = httpResponse.getContentLength();
        receiveBuffer.resize(headerLen + contentLen + 4); // 4  for /r/n/r/n
        while (dataIdx < contentLen) {
            if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
                throw RecvException();
            }
            dataIdx += dataLen;
        }
        cout << "Received response from server " << " : \n";
        string full_response(receiveBuffer.begin(), receiveBuffer.end());
        cout << full_response << endl;
        contactWithRemoteClient(receiveBuffer);
        return;
    }
    // todo: deal with chuncked block
    while ((dataLen = recv(connectFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0)) > 0) {
        string responseStr(receiveBuffer.begin(), receiveBuffer.begin() + dataLen);
        HttpResponse httpResponse(responseStr);
        //contactWithRemoteClient(responseStr);
    }
}

void Client::contactWithRemoteClient(vector<char> sendBuffer) {
    // char sendBuffer[65535];
    //strncpy(sendBuffer, responseStr.c_str(), responseStr.length() + 1);

    // string message = "Hello, world!\n";
    // string response = "HTTP/1.1 200 OK\r\n"
    //                         "Content-Type: text/plain\r\n"
    //                         "Content-Length: " + to_string(message.length()) + "\r\n"
    //                         "\r\n"
    //                         + message;
    // if (send(serviceFd, response.c_str(), response.length(), 0) == -1) {
    //     throw SendException();
    // }
    //cout << responseStr << endl; 
    //cout << response << endl;
    // HttpResponse httpResponse(responseStr);
    //cout << responseStr << endl;
    // cout << "real data len: " << responseStr.length() << endl;
    // strncpy(sendBuffer, responseStr.c_str(), responseStr.length());
    if (send(serviceFd, &sendBuffer.data()[0], sendBuffer.size(), 0) == -1) {
        throw SendException();
    }
    return;
}