#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadNum, fd_set * readFds, shared_ptr<SafeLog>& logFilePtr) : m_shutdown(false) {
    this->readFds = readFds;
    m_threads.reserve(threadNum);
    for (int i = 0; i < threadNum; i++) {
        m_threads.emplace_back([this, &logFilePtr] {
            thread_local uuid_t uuid;
            uuid_generate(uuid);
            char uuidStr[37]; // 37 是 UUID 字符串形式的长度
            uuid_unparse(uuid, uuidStr);
            while (true) {
                int fd = -1;
                {
                    unique_lock<mutex> lock(m_cond_mutex);
                    if (m_shutdown && m_tasks.empty()) {
                        return;
                    }
                    if (m_tasks.empty()) {
                        m_cond.wait(lock);
                    }
                    cout << "start work as thread " << this_thread::get_id() 
                        << ", UUID: " << uuidStr << endl;
                    if (!m_tasks.empty()) {
                        fd = m_tasks.poll();
                    }
                }
                if (fd != -1) {
                    handleClient(fd, uuidStr, logFilePtr);
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

void ThreadPool::handleClient(int fd, string uuidStr, shared_ptr<SafeLog>& logFilePtr) {
    // read and send msg to client;
    vector<char> buffer(65536);
    int dataLen = 0; 
    dataLen = recv(fd, &buffer.data()[0], buffer.size(), 0);
    if (dataLen == 0) {
        return;
    }
    if (dataLen == -1) {
        throw RecvException();
    }
    cout << "Received request from socket fd: " << fd << " : \n";
    string requestStr(buffer.begin(), buffer.begin() + dataLen);
    cout << requestStr << endl;

    HttpRequest httpRequest(requestStr);
    string method = httpRequest.getMethod();
    string hostName = httpRequest.getHost();
    // todo: will handle it in the future
    struct addrinfo hints, *targetAddress;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME; // todo: still may delete, need double check
    const char * port = (method == "CONNECT") ? "443" : "80"; 
    int status = getaddrinfo(hostName.c_str(), port, &hints, &targetAddress); 
    if (status != 0) {
        freeaddrinfo(targetAddress);
        throw ProxyHostAddressException(hostName + ": " + gai_strerror(status));
    }
    Client proxyClient(fd, targetAddress, uuidStr, logFilePtr);
    if (method == "CONNECT") {
        proxyClient.contactInTunnel();
    } else if (method == "GET" || method == "POST") {
        proxyClient.contactWithRemoteServer(requestStr);
    } 
    FD_CLR(fd, readFds);
    close(fd);
    cout << "Client " << fd << " has disconnected." << endl;
}
