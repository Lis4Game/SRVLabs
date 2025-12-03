#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>

std::mutex m;

void Func(std::string name)
{
    long double i = 0;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = start_time + std::chrono::seconds(1);

    while (std::chrono::high_resolution_clock::now() < end_time) {
        i += 1e-9; // 10^(-9)
    }

    m.lock();
    std::cout << name << ": " << i << std::endl;
    m.unlock();
}

int main()
{
    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");
    std::thread thread3(Func, "t3");

    thread1.join();
    thread2.join();
    thread3.join();

    system("pause");


    return 0;
}
