#include "Client.h"
#include <limits.h>
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
    if (httpRequest.getFirstLine() == "GET http://thumbnail1.360doc.com/videoimg/2023/02/27/186759_1677489485262.jpg HTTP/1.1") {
        cout << "catch!" << endl;
    }
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
    //cout << "msg:" << responseStr << endl;
    //cout << "expire time:" << httpResponse.getExpire() << endl;
    int headerLen = httpResponse.getHeader().length(); // 如果响应支持缓存（can cache）&&GET&&200 则缓存
    /**cache ops**/
    // not chunked response
    //cout << "test before chun check" << endl;
    if (!httpResponse.getIsChuncked()) {
        int contentLen = httpResponse.getContentLength();
        // protocol does not support content-length
        //cout << "test after chun check" << endl;
        if (contentLen == -1) {
            //cout << "test after contentLen check fail" << endl;
            // while ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) > 0) {
            //     dataIdx += dataLen;
            // }
            // if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
            //     throw RecvException();
            // }
            //cout << "test after while end" << endl;
            receiveBuffer.resize(dataIdx);
            logFile->writeResponseToClientToLog(uuidStr, httpResponse.getFirstLine());
            contactWithRemoteClient(receiveBuffer);
            return;
        }
        // pass remaining data
        //cout << "test after contentLen check success" << endl;
        int totalLen = headerLen + contentLen + 4; // addition for /r/n/r/n
        receiveBuffer.resize(totalLen); 
        while (dataIdx < totalLen) {
            if ((dataLen = recv(connectFd, &receiveBuffer.data()[dataIdx], receiveBuffer.size() - dataIdx, 0)) == -1) {
                throw RecvException();
            }
            dataIdx += dataLen;
        }
        //cout << "test after contentLen check success and while end" << endl;
        contactWithRemoteClient(receiveBuffer);
        return;
    }
    //cout << "test after chun check success" << endl;
    // deal with chunked blocks
    receiveBuffer.resize(dataIdx);
    logFile->writeResponseToClientToLog(uuidStr, httpResponse.getFirstLine());
    contactWithRemoteClient(receiveBuffer);
    // send remaining data;
    //cout << "ready for chunk loop" << endl;
    // maintain last 4 digit
    vector<char> trailerMaintainer = {'h','o','l', 'd'};
    while ((dataLen = recv(connectFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0)) > 0) {
        receiveBuffer.resize(dataLen);
        //cout << "chunk rec success with len: " << dataLen << endl;
        //contactWithRemoteClient(receiveBuffer);
        if (send(serviceFd, &receiveBuffer.data()[0], receiveBuffer.size(), 0) <= 0) {
            cout << "end chunk" << endl;
            return;
        }
        // update trailer(actually implement a queue)
        for (int i = min(0, dataLen - 4); i < dataLen; i ++) {
            // 0<-1, 1<-2, 2<-3, 3<-new
            for (int j = 0; j < 3; j ++) {
                trailerMaintainer[j] = trailerMaintainer[j + 1];
            }
            trailerMaintainer[3] = receiveBuffer[i];
        }
        //cout << "data len: " << dataLen << endl;
        string trailer = string(trailerMaintainer.begin(), trailerMaintainer.end());
        if (trailer.compare("\r\n\r\n") == 0) {
            cout << "end chunk" << endl;
            return;
        }
        //cout << "chunk send success with len: " << dataLen << endl;
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
    FD_ZERO(&readFds); // put down？
    int fdMax = max(connectFd, serviceFd);
    FD_SET(connectFd, &readFds);
    FD_SET(serviceFd, &readFds);
    // struct timeval tv;
    // tv.tv_sec = 8;
    // tv.tv_usec = 0;
    while (true) {
        //fd_set readFds;
        //FD_ZERO(&readFds); // put down？
        fd_set cpFds = readFds;
        // int state;
        if ((status = select(fdMax + 1, &cpFds, NULL, NULL, NULL)) == -1) {
            throw SelectException();
        } else if (status == 0) {
            std::cout << "Loop ended after time out" << std::endl;
            break;
        }
        vector<char> forwardBuffer(65536);
        int dataLen = 0;
        if (FD_ISSET(serviceFd, &cpFds)) {
            cout << "receive from client (will not show the encrypted msg)" << endl;
            if ((dataLen = recv(serviceFd, &forwardBuffer.data()[0], forwardBuffer.size(), 0)) <= 0) {
                //throw RecvException();
                logFile->writeTunnelClosedLog(uuidStr);
                cout << "Tunnel closed by client" << endl;
                break;
            }
            //string str(forwardBuffer.begin(), forwardBuffer.begin() + dataLen);
            //cout << str << endl;
            cout << "send to server (will not show the encrypted msg) with len: " << dataLen << endl;
            if ((status = send(connectFd, &forwardBuffer.data()[0], dataLen, 0)) <= 0) {
                logFile->writeTunnelClosedLog(uuidStr);
                cout << "Tunnel closed by server" << endl;
                break;
                //throw SendException(gai_strerror(status));
            }
            cout << "after send to server (will not show the encrypted msg)" << endl;
        } else if (FD_ISSET(connectFd, &cpFds)) {
            cout << "receive from server (will not show the encrypted msg)" << endl;
            if ((dataLen = recv(connectFd, &forwardBuffer.data()[0], forwardBuffer.size(), 0)) <= 0) {
                logFile->writeTunnelClosedLog(uuidStr);
                cout << "Tunnel closed by server" << endl;
                break;
            }
            // string str(forwardBuffer.begin(), forwardBuffer.begin() + dataLen);
            // cout << str << endl;
            cout << "send to client (will not show the encrypted msg)" << endl;
            if ((status = send(serviceFd, &forwardBuffer.data()[0], dataLen, 0)) <= 0) {
                //todo: sometimes crush here, only break may help but that's not a perfect solution
                logFile->writeTunnelClosedLog(uuidStr);
                cout << "Tunnel closed by client" << endl;
                break;
            }
            cout << "after to client (will not show the encrypted msg)" << endl;
        }
    }
}