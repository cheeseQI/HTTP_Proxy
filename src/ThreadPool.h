#ifndef THREAD_H
#define THREAD_H 
#include "iostream"
#include "sys/socket.h"
#include "sys/types.h"
#include "unistd.h"
#include "fcntl.h"
#include "cstring"
#include "sys/select.h"
#include "vector"
#include "mutex"
#include "thread"
#include "condition_variable"
#include "TaskQueue.h"
#include "ExceptionHandler.h"
#include "HttpRequest.h"
#include "Client.h"
#include "uuid/uuid.h"
#include "SafeLog.h"
using namespace std;
const int THREAD_NUM = 50;

class ThreadPool {
private:
    vector<thread> m_threads;
    TaskQueue m_tasks;
    mutex m_cond_mutex;
    condition_variable m_cond; // use to notify thread from sleeping
    bool m_shutdown;
    void handleClient(int fd, string uuidStr, shared_ptr<SafeLog>& logFilePtr);

public:
    ThreadPool(int threadNum, shared_ptr<SafeLog>& logFilePtr);

    ~ThreadPool();

    void submit(int fd);
};
#endif