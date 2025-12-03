#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>

std::mutex cout_mutex;


unsigned long long factorial(int n) {
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}


void Func(std::string name) {

    unsigned long long fact = factorial(20);


    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << name << " computed factorial(20) = " << fact << std::endl;
    }
}

int main() {
    auto start_parallel = std::chrono::high_resolution_clock::now();

    std::thread thread1(Func, "t1");
    std::thread thread2(Func, "t2");

    thread1.join();
    thread2.join();

    auto end_parallel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_parallel = end_parallel - start_parallel;

    std::cout << "Parallel execution time: " << duration_parallel.count() << " seconds" << std::endl;

    std::cout << "---------------------------" << std::endl;


    auto start_sequential = std::chrono::high_resolution_clock::now();

    Func("seq1");
    Func("seq2");

    auto end_sequential = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_sequential = end_sequential - start_sequential;

    std::cout << "Sequential execution time: " << duration_sequential.count() << " seconds" << std::endl;

    std::cout << "Speedup: " << (duration_sequential.count() / duration_parallel.count()) << "x" << std::endl;

    system("pause");
    return 0;
