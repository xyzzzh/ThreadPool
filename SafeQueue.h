//
// Created by xyzzzh on 24-3-18.
//

#ifndef THREADPOOL_SAFEQUEUE_H
#define THREADPOOL_SAFEQUEUE_H

#include <queue>
#include <mutex>

template<typename T>
class SafeQueue {
public:
    SafeQueue() {}

    SafeQueue(SafeQueue &&other) {}

    ~SafeQueue() {}

    bool empty() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    auto size() {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

    void push(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.emplace(t);
    }

    bool pop(T &t) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_queue.empty()) {
            return false;
        }
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }


public:
    std::queue<T> m_queue;      // 利用模板函数构造队列
    std::mutex m_mutex;         // 访问互斥锁
};

#endif //THREADPOOL_SAFEQUEUE_H
