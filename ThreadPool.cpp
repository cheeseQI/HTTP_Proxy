#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadNum, fd_set * readFds) : m_shutdown(false) {
    this->readFds = readFds;
    m_threads.reserve(threadNum);
    for (int i = 0; i < threadNum; i++) {
        m_threads.emplace_back([this] {
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
                    cout << "start work as thread " << this_thread::get_id() << endl;
                    if (!m_tasks.empty()) {
                        fd = m_tasks.poll();
                    }
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
    vector<char> buffer(65536);
    int dataLen = 0; 
    dataLen = recv(fd, &buffer.data()[0], buffer.size(), 0);
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
    if (method == "CONNECT" || method == "POST") {
        FD_CLR(fd, readFds);
        close(fd);
        cout << "Client " << fd << " has disconnected." << endl;
        return;
    }
    struct addrinfo hints, *targetAddress;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    int status = getaddrinfo(hostName.c_str(), "80", &hints, &targetAddress); 
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << " errono: " << errno << '\n';
        throw ProxyHostAddressException();
    }
    // communicate with real server as proxy client 
    Client proxyClient(fd, targetAddress);
    proxyClient.contactWithRemoteServer(requestStr);

    FD_CLR(fd, readFds);
    close(fd);
    cout << "Client " << fd << " has disconnected." << endl;
}
