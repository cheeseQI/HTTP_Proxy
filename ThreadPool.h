#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/select.h>
#include <vector>
#include <mutex>
#include "thread"
#include "condition_variable"
#include "TaskQueue.h"
#include "ExceptionHandler.h"

using namespace std;

// todo: change to?
const int THREAD_NUM = 4;


class ThreadPool {
private:
    fd_set * readFds;
    vector<thread> m_threads;
    TaskQueue m_tasks;
    //mutex m_cond_mutex;
    condition_variable m_cond; // use to notify thread from sleeping
    bool m_shutdown;
    void handleClient(int fd) {
        // read and send msg to client;
        vector<char> buffer(1024);
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
            char sendBuffer[1024];
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

public:
    ThreadPool(int threadNum, fd_set * readFds) : m_shutdown(false) {
        this->readFds = readFds;
        m_threads.reserve(threadNum);
        for (int i = 0; i < threadNum; i++) {
            m_threads.emplace_back([this] {
                while (true) {
                    int fd = -1;
                    // unique_lock<mutex> lock(m_cond_mutex);
                    // brace to hold lock
                    if (m_shutdown && m_tasks.empty()) {
                        return;
                    }
                    // block at conditional variable
                    // if (m_tasks.empty()) {
                    //     m_cond.wait(lock);
                    // }
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

    ~ThreadPool() {
        m_shutdown = true;
        m_cond.notify_all();
        for (thread& t : m_threads) {
            t.join();
        }
    }

    void submit(int fd) {
        m_tasks.add(fd);
        m_cond.notify_one();
    }
};