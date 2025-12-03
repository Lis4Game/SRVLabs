#include <iostream>
#include <time.h>

long long factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    const int iterations = 10000000;
    clock_t start = clock();

    long long total = 0;
    for (int i = 0; i < iterations; ++i) {
        total += factorial(10);
    }

    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;

    std::cout << "Вычислено " << iterations << " раз, результат суммы = " << total << std::endl;
    std::cout << "Время выполнения: " << seconds << " секунд" << std::endl;

    return 0;
}
