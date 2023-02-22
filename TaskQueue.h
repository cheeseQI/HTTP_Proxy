#ifndef TASKQUEUE_H
#define TASKQUEUE_H 
#include "mutex"
#include "queue"
using namespace std;

/**
 * encapsulate a class which support thread-safe queue
*/
class TaskQueue {
private:
  queue<int> m_queue;
  mutex m_mutex;

public:
  // TaskQueue() {}
  // ~TaskQueue() {}
  bool empty() {
    unique_lock<mutex> lock(m_mutex); 
    return m_queue.empty();
  }
  
  int size() {
    unique_lock<mutex> lock(m_mutex);
    return m_queue.size();
  }

  void add(int fd) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.push(fd);
  }
//队列取出元素

  int poll() {
    unique_lock<mutex> lock(m_mutex); //队列加锁
    if (m_queue.empty()) {
      return -1;
    }
    int i = m_queue.front();
    m_queue.pop();
    return i;
  }
};

#endif