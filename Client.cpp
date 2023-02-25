#include "Client.h"

/// @brief 
/// @param fd service fd to user client
/// @param address remote server address
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
    int status;
    int connectFd = connectSocketPtr->getFd();
    if ((status = send(connectFd, request.c_str(), request.length(), 0)) == -1) {
            throw SendException(gai_strerror(status));
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
    // cout << "Received first/part response from server " << " : \n";
    cout << "header:" << httpResponse.getHeader() << endl;
    int headerLen = httpResponse.getHeader().length();
    if (!httpResponse.getIsChuncked()) {
        int contentLen = httpResponse.getContentLength();
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
    int status;
    if ((status = send(serviceFd, &sendBuffer.data()[0], sendBuffer.size(), 0)) == -1) {
        throw SendException(gai_strerror(status));
    }
    return;
}


// tunnel that forward info between client(by serviceFd) and server(connectFd)
void Client::contactInTunnel() {
    int connectFd = connectSocketPtr->getFd();
    string confirm = "HTTP/1.1 200 OK\r\n\r\n";
    int status;
    if ((status = send(serviceFd, confirm.c_str(), confirm.length(), 0)) == -1) {
        throw SendException(gai_strerror(status));
    }
    fcntl(serviceFd, F_SETFL, O_NONBLOCK);
    fcntl(connectFd, F_SETFL, O_NONBLOCK);
    fd_set readFds;
    FD_ZERO(&readFds);
    int fdMax = max(connectFd, serviceFd);
    FD_SET(connectFd, &readFds);
    FD_SET(serviceFd, &readFds);
    while (true) {
        fd_set cpFds = readFds;
        //int state;
        if ((status = select(fdMax + 1, &cpFds, NULL, NULL, NULL)) == -1) {
            throw SelectException();
        }
        vector<char> forwardBuffer(65536);
        int dataLen = 0;
        if (FD_ISSET(serviceFd, &cpFds)) {
            cout << "statr receive from client (will not show the encrypted msg)" << endl;
            if ((dataLen = recv(serviceFd, &forwardBuffer.data()[0], forwardBuffer.size(), 0)) == -1) {
                throw RecvException();
            }
            if (dataLen <= 0) {
                break;
            }
            //string str(forwardBuffer.begin(), forwardBuffer.begin() + dataLen);
            //cout << str << endl;
            if ((status = send(connectFd, &forwardBuffer.data()[0], dataLen, 0)) == -1) {
                throw SendException(gai_strerror(status));
            }
        }
        if (FD_ISSET(connectFd, &cpFds)) {
            cout << "statr receive from server (will not show the encrypted msg)" << endl;
            if ((dataLen = recv(connectFd, &forwardBuffer.data()[0], forwardBuffer.size(), 0)) == -1) {
                throw RecvException();
            }
            if (dataLen <= 0) {
                break;
            }
            //string str(forwardBuffer.begin(), forwardBuffer.begin() + dataLen);
            //cout << str << endl;
            if ((status = send(serviceFd, &forwardBuffer.data()[0], dataLen, 0)) == -1) {
                throw SendException(gai_strerror(status));
            }
        }

    }
}