#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

int dish1 = 3000, dish2 = 3000, dish3 = 3000;
bool cook_quit = false, cook_fired = false, all_fat_dead = false;
int fat_deaths = 0;
mutex dish_mutex, state_mutex;

void fat_eater(int id, int gluttony) {
    int* dish;
    if (id == 1) dish = &dish1;
    else if (id == 2) dish = &dish2;
    else dish = &dish3;

    int eaten = 0;
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(10));

        lock_guard<mutex> lock(dish_mutex);
        lock_guard<mutex> state_lock(state_mutex);

        if (cook_quit || cook_fired || all_fat_dead) return;

        if (*dish >= gluttony) {
            *dish -= gluttony;
            eaten += gluttony;

            if (eaten >= 10000) {
                fat_deaths++;
                if (fat_deaths == 3) all_fat_dead = true;
                return;
            }
        }
        else {
            if (*dish <= 0) cook_fired = true;
            return;
        }
    }
}

void cook(int efficiency_factor) {
    auto start = chrono::steady_clock::now();
    while (true) {
        {
            lock_guard<mutex> state_lock(state_mutex);
            if (cook_fired || all_fat_dead) return;

            if (chrono::duration_cast<chrono::seconds>(
                chrono::steady_clock::now() - start).count() >= 5) {
                cout << "Кук уволился сам!" << endl;
                cook_quit = true;
                return;
            }
        }

        lock_guard<mutex> lock(dish_mutex);
        {
            lock_guard<mutex> state_lock(state_mutex);
            if (dish1 <= 0 || dish2 <= 0 || dish3 <= 0) {
                cook_fired = true;
                return;
            }
        }

        dish1 += efficiency_factor / 3;
        dish2 += efficiency_factor / 3;
        dish3 += efficiency_factor / 3 + efficiency_factor % 3;
    }
}

int main() {
    setlocale(0, "");

    // Кука уволили
    cout << "Сценарий 1: Кука уволили" << endl;
    cout << "Обжорство: 100, Производительность: 50" << endl;
    dish1 = dish2 = dish3 = 3000;
    cook_quit = cook_fired = all_fat_dead = false; fat_deaths = 0;

    thread t1(fat_eater, 1, 100), t2(fat_eater, 2, 100), t3(fat_eater, 3, 100);
    thread c(cook, 50);
    t1.join(); t2.join(); t3.join(); c.join();
    if (cook_fired) cout << "Результат: Кука уволили" << endl;

    // Не получил зарплату
    cout << "\nСценарий 2: Не получил зарплату" << endl;
    cout << "Обжорство: 200, Производительность: 600" << endl;
    dish1 = dish2 = dish3 = 3000;
    cook_quit = cook_fired = all_fat_dead = false; fat_deaths = 0;

    thread t4(fat_eater, 1, 200), t5(fat_eater, 2, 200), t6(fat_eater, 3, 200);
    thread c2(cook, 600);
    t4.join(); t5.join(); t6.join(); c2.join();
    if (all_fat_dead) cout << "Результат: Кук не получил зарплату" << endl;

    // Уволился сам
    cout << "\nСценарий 3: Уволился сам " << endl;
    cout << "Обжорство: 10, Производительность: 30" << endl;
    dish1 = dish2 = dish3 = 3000;
    cook_quit = cook_fired = all_fat_dead = false; fat_deaths = 0;

    thread t7(fat_eater, 1, 10), t8(fat_eater, 2, 10), t9(fat_eater, 3, 10);
    thread c3(cook, 30);
    t7.join(); t8.join(); t9.join(); c3.join();
    if (cook_quit && !cook_fired && !all_fat_dead) cout << "Результат: Кук уволился сам" << endl;

    return 0;
}
