#include <iostream>
#include <thread>
#include <mutex>

int coins = 101;
int Bob_coins = 0;
int Tom_coins = 0;
int Dead_coins = 0;
std::mutex mtx;

void coin_sharing(std::string name, int& thief_coins, int& companion_coins) {
    while (true) {
        std::lock_guard<std::mutex> lock(mtx);

        if (coins == 1 && Bob_coins == Tom_coins) {
            break;
        }

        if (coins <= 0) {
            break;
        }

        if (thief_coins <= companion_coins) {
            coins--;
            thief_coins++;

            std::cout << name << " :" << thief_coins << " " << companion_coins
                << " Осталось: " << coins << std::endl;
        }
    }
}

int main() {
    system("chcp 1251>nul");
    std::thread bob_thread(coin_sharing, "Bob", std::ref(Bob_coins), std::ref(Tom_coins));
    std::thread tom_thread(coin_sharing, "Tom", std::ref(Tom_coins), std::ref(Bob_coins));

    bob_thread.join();
    tom_thread.join();

    if (coins > 0 && Bob_coins == Tom_coins) {
        Dead_coins = coins;
        coins = 0;
    }

    std::cout << "Итого: Bob=" << Bob_coins << ", Tom=" << Tom_coins
        << ", Покойник=" << Dead_coins << ", Осталось=" << coins << std::endl;
    std::cout << "Сумма: " << (Bob_coins + Tom_coins + Dead_coins + coins) << std::endl;

    return 0;
}