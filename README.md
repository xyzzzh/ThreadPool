# C++高性能线程池

基于C++17实现的高性能线程池

实现多线程安全的任务队列，线程池使用异步操作，提交(submit)使用与thread相同。

内部利用完美转发获取可调用对象的函数签名，lambda与function包装任务，使用RAII管理线程池的生命周期。