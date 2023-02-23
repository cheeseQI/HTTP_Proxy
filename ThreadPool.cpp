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
        for (int i = 0; i < dataIdx; i ++) {
            cout << buffer[i];
        } 
        cout << endl;
        // todo: will be changed to a variable response with not fixed length
        char sendBuffer[2048];
                // string message = "Hello, world!\n";
        // string response = "HTTP/1.1 200 OK\r\n"
        //                 "Content-Type: text/plain\r\n"
        //                 "Content-Length: " + to_string(message.length()) + "\r\n"
        //                 "\r\n"
        //                 + message;
        http::response<http::string_body> response;
        response.version(11);
        response.result(http::status::ok);
        response.set(http::field::server, "MyServer");
        response.set(http::field::content_type, "text/plain");
        response.keep_alive(true);
        response.body() = "Hello, world!\n";
        response.prepare_payload();
        stringstream ss;
        ss << response;
        string responseStr = ss.str();
        strncpy(sendBuffer, responseStr.c_str(), responseStr.length());
        if (send(fd, sendBuffer, responseStr.length(), 0) == -1) {
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