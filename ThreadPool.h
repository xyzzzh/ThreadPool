//
// Created by xyzzzh on 24-3-18.
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H


#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include "SafeQueue.h"

// 线程池最重要的方法就是负责向任务队列添加任务。我们的提交函数应该做到以下两点：
// 1. 接收任何参数的任何函数。（普通函数，Lambda，成员函数……）
// 2. 立即返回任务结束的结果，避免阻塞主线程。

class ThreadPool {
public:
    ThreadPool(int num_threads = 4) :
            m_threads(std::vector<std::thread>(num_threads)), m_shutdown(false) {}

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

    ThreadPool &operator=(ThreadPool &&) = delete;

    // 内置线程工作类
    class ThreadWorker {
    public:
        ThreadWorker(const int id, ThreadPool *pool) : m_id(id), m_pool(pool) {}

        // 重载()操作
        void operator()() {
            // 定义基础函数类func
            std::function<void()> func;
            // 是否正在取出队列中元素
            bool popped;

            while (!m_pool->m_shutdown) {
                {
                    // 为线程环境加锁，互访问工作线程的休眠和唤醒
                    std::unique_lock<std::mutex> lock(m_pool->m_mutex);

                    // 如果任务队列为空，阻塞当前线程
                    if (m_pool->m_tasks.empty()) {
                        m_pool->m_cond.wait(lock);  // 等待条件变量通知，开启线程
                    }

                    popped = m_pool->m_tasks.pop(func);
                }
                if (popped) {
                    func();
                }
            }

        }

    private:
        int m_id;               // 线程id
        ThreadPool *m_pool;     // 所属线程池
    };

public:
    void init() {
        for (int i = 0; i < m_threads.size(); i++) {
            m_threads[i] = std::thread(ThreadPool::ThreadWorker(i, this));
        }
        m_shutdown = false;
    }

    void shutdown() {
        m_shutdown = true;
        m_cond.notify_all();    // 通知，唤醒所有工作线程
        for (int i = 0; i < m_threads.size(); i++) {
            if (m_threads[i].joinable()) {    // 判断线程是否在等待
                m_threads[i].join();        // 将线程加入到等待队列
            }
        }
    }

    // 使用auto而非decltype(f)是因为此时编译期仍未知道到f的类型，利用auto配合C++11引入的尾返回类型，可以实现返回类型后置
    template<typename F, typename... Args>
    auto submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))> {
        // 创建 std::function 对象
        // 使用 std::bind 和 std::forward 来绑定函数 f 和参数 args，并将它们封装成一个 std::function 对象。
        // 这样做可以确保参数的正确转发（即左值或右值）。
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        // 创建 std::packaged_task 对象
        // 使用 std::packaged_task 将之前创建的 std::function 对象封装成一个可以异步执行的任务，
        // 并将其包装成一个 std::shared_ptr 指针，以便后续可以安全地共享和复制。
        auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

        // 创建一个 lambda 函数 wrapper_func，它不接受任何参数，并调用 std::packaged_task 对象。
        // 这个包装器是为了适应线程池可能需要的无参任务接口。
        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)();
        };

        // 队列通用安全封包函数，并压入安全队列
        m_tasks.push(wrapper_func);

        // 唤醒一个等待中的线程
        m_cond.notify_one();

        // 返回先前注册的任务指针
        // 返回 std::packaged_task 对象关联的 std::future 对象，以便调用者可以异步获取任务的结果。
        return task_ptr->get_future();
    }

private:
    std::vector<std::thread> m_threads;         // 工作线程队列
    SafeQueue<std::function<void()>> m_tasks;  // 执行函数安全队列，即任务队列
    std::mutex m_mutex;                         // 线程休眠锁互斥变量
    std::condition_variable m_cond;             // 线程环境锁，可以让线程处于休眠或者唤醒状态
    bool m_shutdown;                            // 线程池是否关闭
};


#endif //THREADPOOL_THREADPOOL_H
