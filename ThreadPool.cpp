#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadNum, fd_set * readFds) : m_shutdown(false) {
    this->readFds = readFds;
    m_threads.reserve(threadNum);
    for (int i = 0; i < threadNum; i++) {
        m_threads.emplace_back([this] {
            while (true) {
                int fd = -1;
                // unique_lock<mutex> lock(m_cond_mutex);
                if (m_shutdown && m_tasks.empty()) {
                    return;
                }
                if (!m_tasks.empty()) {
                    fd = m_tasks.poll();
                }
                if (fd != -1) {
                    handleClient(fd);
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    m_shutdown = true;
    m_cond.notify_all();
    for (thread& t : m_threads) {
        t.join();
    }
}

void ThreadPool::submit(int fd) {
    m_tasks.add(fd);
    m_cond.notify_one();
}

void ThreadPool::handleClient(int fd) {
    // read and send msg to client;
    vector<char> buffer(2048);
    int dataIdx = 0;
    int dataLen = 0;
    while ((dataLen = recv(fd, &buffer.data()[dataIdx], buffer.size() - dataIdx, 0)) > 0) {
        dataIdx += dataLen;
        // enlarge when meet half
        if (dataIdx >= (int)buffer.size() / 2) {
            buffer.resize(buffer.size() * 2);
        }
        cout << "Received message from client " << fd << " : \n";
        string requestStr(buffer[0], buffer[dataIdx]);
        for (int i = 0; i < dataIdx; i ++) {
            cout << buffer[i];
        } 
        // 没找到可用的解析函数？
        
        // rend req and get msg to google
        struct addrinfo hints, *targetAddress;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        const char* hostname = "example.com";
        if (getaddrinfo(hostname, "80", &hints, &targetAddress) != 0) {
            throw ProxyHostAddressException();
        }  
        Client proxyClient(targetAddress);
        string request(buffer.begin(), buffer.begin() + dataIdx + 1);
        proxyClient.contactWithRemoteServer(request);

        
        //
        char sendBuffer[2048];
        // http::response<http::string_body> response;
        // response.version(11);
        // response.result(http::status::ok);
        // response.set(http::field::server, "MyServer");
        // response.set(http::field::content_type, "text/plain");
        // response.keep_alive(true);
        // response.body() = "Hello, world!\n";
        // response.prepare_payload();
        // stringstream ss;
        // ss << response;
        // string responseStr = ss.str();
        // strncpy(sendBuffer, responseStr.c_str(), responseStr.length());
        string message = "Hello, world!\n";
            string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: " + to_string(message.length()) + "\r\n"
                            "\r\n"
                            + message;
        strncpy(sendBuffer, response.c_str(), sizeof(sendBuffer));
        if (send(fd, sendBuffer, response.length(), 0) == -1) {
            throw SendException();
        }
    }

    if (dataLen == -1) {
        throw RecvException();
    }

    FD_CLR(fd, readFds);
    close(fd);
    cout << "Client " << fd << " has disconnected." << endl;
}


// send request and get msg back from server, cache and return 
void ThreadPool::handleRemoteServer(int fd) {
    // connect socket here, use new fd and send and get back from target.

}