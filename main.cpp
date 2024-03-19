//
// Created by xyzzzh on 24-3-18.
//
#include <iostream>
#include <random>
#include "ThreadPool.h"

std::random_device rd;      // 真实随机数产生器
std::mt19937 mt(rd());  //生成计算随机数mt
std::uniform_int_distribution<int> dist(-1000, 1000);   //生成-1000到1000之间的离散均匀分布数

auto rnd = std::bind(dist, mt);

// 设置线程睡眠时间
void simulate_hard_computation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

// 添加两个数字的简单函数并打印结果
void task1(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout<< "task1\t"  << a << " * " << b << "=" << res << std::endl;

}

// 添加并输出结果
void task2(int &res, const int a, const int b) {
    simulate_hard_computation();
    res = a * b;
    std::cout << "task2\t"  << a << " * " << b << "=" << res << std::endl;
}

// 结果返回
int task3(const int a, const int b) {
    simulate_hard_computation();
    const int res = a * b;
    std::cout << "task3\t" << a << " * " << b << "=" << res << std::endl;
    return res;
}

void example() {
    ThreadPool pool(16);
    pool.init();

    // task1
    for(int i=1;i<10;i++){
        for(int j=1;j<10;j++){
            pool.submit(task1, i, j);
        }
    }

    // task2
    int res;
    auto future1 = pool.submit(task2, std::ref(res), 5, 6);
    // 等待乘法输出完成
    future1.get();
    std::cout << "task2: " << res << std::endl;

    // task3
    auto future2 = pool.submit(task3, 7, 8);
    // 等待乘法输出完成
    res = future2.get();
    std::cout << "task3: " << res << std::endl;

    // 关闭线程池
    pool.shutdown();
}

int main() {
    example();
    return 0;
}