#include <iostream>
#include <thread>
#include <mutex>

int coins = 101;
int Bob_coins = 0;
int Tom_coins = 0;
std::mutex mtx;

void coin_sharing(std::string name, int& thief_coins, int& companion_coins) {
    while (true) {
        std::lock_guard<std::mutex> lock(mtx);
        int coins_to_divide = (coins % 2 == 1) ? coins - 1 : coins;
        if (coins_to_divide <= 0) {
            break;
        }
        if (thief_coins <= companion_coins) {
            coins--;
            thief_coins++;

            std::cout << name << " : " << thief_coins << " " << companion_coins << " |Left: " << coins << std::endl;
        }

    }
}
int main() {
    std::thread bob_thread(coin_sharing, "Bob", std::ref(Bob_coins), std::ref(Tom_coins));
    std::thread tom_thread(coin_sharing, "Tom", std::ref(Tom_coins), std::ref(Bob_coins));

    bob_thread.join();
    tom_thread.join();

    std::cout << "\nSummary:\n";
    std::cout << "Bob: " << Bob_coins << "\n";
    std::cout << "Tom: " << Tom_coins << "\n";
    std::cout << "Dead: " << coins << "\n";
    std::cout << "All: " << Bob_coins + Tom_coins + coins << "\n";
    return 0;
}

