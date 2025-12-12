#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <cstdlib>

const int EXPLOSION_LIMIT = 10000;
const int INITIAL_DISH = 3000;
const int NUM_FAT_MEN = 3;
const int SIMULATION_TIME_SEC = 5;

std::mutex mtx;

struct Result {
    std::string outcome;
    std::vector<int> eaten;
    std::vector<int> dishes;
};

Result run_simulation(int gluttony, int efficiency) {
    std::vector<int> dishes(NUM_FAT_MEN, INITIAL_DISH);
    std::vector<int> eaten(NUM_FAT_MEN, 0);
    bool running = true;
    std::string outcome = "Кук уволился сам";

    auto cook = std::thread([&]() {
        while (running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock(mtx);
            if (!running) break;
            for (int i = 0; i < NUM_FAT_MEN; ++i) {
                dishes[i] += efficiency;
            }
        }
        });

    std::vector<std::thread> eaters;
    for (int id = 0; id < NUM_FAT_MEN; ++id) {
        eaters.emplace_back([&, id]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std::lock_guard<std::mutex> lock(mtx);
                if (!running) break;

                if (eaten[id] >= EXPLOSION_LIMIT) continue;

                int room = EXPLOSION_LIMIT - eaten[id];
                int take = std::min({ dishes[id], gluttony, room });

                if (take > 0) {
                    dishes[id] -= take;
                    eaten[id] += take;

                    if (dishes[id] <= 0) {
                        outcome = "Кука уволили";
                        running = false;
                    }
                }

                bool all_exploded = true;
                for (int i = 0; i < NUM_FAT_MEN; ++i) {
                    if (eaten[i] < EXPLOSION_LIMIT) {
                        all_exploded = false;
                        break;
                    }
                }
                if (all_exploded) {
                    outcome = "Кук не получил зарплату";
                    running = false;
                }
            }
            });
    }

    std::this_thread::sleep_for(std::chrono::seconds(SIMULATION_TIME_SEC));
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
    }

    cook.join();
    for (auto& t : eaters) t.join();

    return { outcome, eaten, dishes };
}

int main() {
    system("chcp 1251>nul");

    struct Test {
        int gluttony;
        int efficiency;
    };

    std::vector<Test> tests = {
        {80, 5},    //Увольнение
        {100, 1000}, //Взрыв
        {1, 2}     //рейджквит
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "\nВариант " << (i + 1) << ":\n";
        std::cout << "GLUTTONY = " << tests[i].gluttony
            << ", EFFICIENCY = " << tests[i].efficiency << "\n";

        Result res = run_simulation(tests[i].gluttony, tests[i].efficiency);

        std::cout << "Итог: " << res.outcome << "\n";
        std::cout << "Сводка:\n";
        for (int j = 0; j < NUM_FAT_MEN; ++j) {
            std::cout << "Толстяк #" << (j + 1)
                << ": съел " << res.eaten[j]
                << ", на тарелке " << res.dishes[j] << "\n";
        }
    }

    return 0;
}