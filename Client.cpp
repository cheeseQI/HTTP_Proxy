#include "Client.h"

/// @brief 
/// @param fd service fd to user client
/// @param address remote server address
Client::Client(int fd, struct addrinfo * address, string uuidStr, shared_ptr<SafeLog>& logFile) 
: serviceFd(fd), uuidStr(uuidStr), logFile(logFile) {
    connectSocketPtr = make_unique<Socket>(address);
    int connectFd = connectSocketPtr->getFd();
    if (connect(connectFd, address->ai_addr, address->ai_addrlen) == -1) {
        throw ConnectException();
    }
    freeaddrinfo(address);
}

void Client::contactWithRemoteServer(string request) { 
    int status;
    int connectFd = connectSocketPtr->getFd(); // todo:如果是GET 先查缓存，缓存有效直接返回；否则走下面逻辑
    HttpRequest httpRequest(request);
    /**cache ops**/
    if ((status = send(connectFd, request.c_str(), request.length(), 0)) == -1) {
        throw SendException(gai_strerror(status));
    }
    logFile->writeRequestToLog(uuidStr, httpRequest.getFirstLine(), httpRequest.getHost());

    vector<char> receiveBuffer(65536);
    int dataLen = recv(connectFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0);
    if (dataLen == -1) {
        throw RecvException();
    }
    // server close the connection
    if (dataLen == 0) {
        return;
    }
    int dataIdx = dataLen;
    string responseStr(receiveBuffer.begin(), receiveBuffer.begin() + dataLen);
    HttpResponse httpResponse(responseStr);
    logFile->writeServerResponseToLog(uuidStr, httpResponse.getFirstLine(), httpRequest.getHost());
    cout << "header:" << httpResponse.getHeader() << endl;

    cout << "expire time:" << httpResponse.getExpire() << endl;
    int headerLen = httpResponse.getHeader().length(); // 如果响应支持缓存（can cache）&&GET&&200 则缓存
    /**cache ops**/
    // not chunked response
    if (!httpResponse.getIsChuncked()) {
        int contentLen = httpResponse.getContentLength();
        // protocol does not support content-length
        if (contentLen == -1) {
            // todo: may need resize(double when half) here
            while ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) > 0) {
                dataIdx += dataLen;
            }
            if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
                throw RecvException();
            }
            
            receiveBuffer.resize(dataIdx);
            logFile->writeResponseToClientToLog(uuidStr, httpResponse.getFirstLine());
            contactWithRemoteClient(receiveBuffer);
            return;
        }
        // pass remaining data
        int totalLen = headerLen + contentLen + 4; // addition for /r/n/r/n
        receiveBuffer.resize(totalLen); 
        while (dataIdx < totalLen) {
            if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
                throw RecvException();
            }
            dataIdx += dataLen;
        }
        contactWithRemoteClient(receiveBuffer);
        return;
    }
    // deal with chunked blocks
    receiveBuffer.resize(dataIdx);
    logFile->writeResponseToClientToLog(uuidStr, httpResponse.getFirstLine());
    contactWithRemoteClient(receiveBuffer);
    // send remaining data;
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
void Client::contactInTunnel(string requestStr) {
    HttpRequest httpRequest(requestStr);
    logFile->writeServerResponseToLog(uuidStr, httpRequest.getFirstLine(), httpRequest.getHost());
    int connectFd = connectSocketPtr->getFd();
    string confirm = "HTTP/1.1 200 OK\r\n\r\n";
    logFile->writeResponseToClientToLog(uuidStr, "HTTP/1.1 200 OK");
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
        // int state;
        if ((status = select(fdMax + 1, &cpFds, NULL, NULL, NULL)) == -1) {
            throw SelectException();
        }
        vector<char> forwardBuffer(65536);
        int dataLen = 0;
        if (FD_ISSET(serviceFd, &cpFds)) {
            cout << "receive from client (will not show the encrypted msg)" << endl;
            if ((dataLen = recv(serviceFd, &forwardBuffer.data()[0], forwardBuffer.size(), 0)) == -1) {
                throw RecvException();
            }
            if (dataLen <= 0) {
                logFile->writeTunnelClosedLog(uuidStr);
                break;
            }
            string str(forwardBuffer.begin(), forwardBuffer.begin() + dataLen);
            cout << str << endl;
            cout << "send to server (will not show the encrypted msg)" << endl;
            if ((status = send(connectFd, &forwardBuffer.data()[0], dataLen, 0)) == -1) {
                throw SendException(gai_strerror(status));
            }
        }
        if (FD_ISSET(connectFd, &cpFds)) {
            cout << "receive from server (will not show the encrypted msg)" << endl;
            if ((dataLen = recv(connectFd, &forwardBuffer.data()[0], forwardBuffer.size(), 0)) == -1) {
                throw RecvException();
            }
            if (dataLen <= 0) {
                logFile->writeTunnelClosedLog(uuidStr);
                break;
            }
            string str(forwardBuffer.begin(), forwardBuffer.begin() + dataLen);
            cout << str << endl;
            cout << "send to client (will not show the encrypted msg)" << endl;
            if ((status = send(serviceFd, &forwardBuffer.data()[0], dataLen, 0)) == -1) {
                //todo: sometimes crush here, only break may help but that's not a perfect solution
                throw SendException(gai_strerror(status));
                //break;
            }
        }
    }
}