#include <iostream>
#include <condition_variable>
#include <thread>

void print_time() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&t_c);
}

class Game{
    const size_t farm_coins = 15;
    const size_t farms_limit = 5;
    const size_t coin_time = 3;
    const size_t building_time = 60;

    std::condition_variable cv;

    size_t coins = 0;
    size_t already_built = 0;
    size_t will_built = 0;

public:
    void operator()(){
        std::mutex m;
        std::unique_lock<std::mutex> ul(m);

        while(will_built < farms_limit){
            while(coins < farm_coins){
                std::this_thread::sleep_for(std::chrono::seconds(coin_time));
                ++coins;
            }

            std::cout << "New farm should be soon! ";
            print_time();

            will_built += 1;
            coins = 0;
            cv.notify_all();
        }
    }

    void operator()(bool){
        std::mutex m;
        std::unique_lock<std::mutex> ul(m);

        while(already_built < farms_limit){
            cv.wait(ul, [&](){return already_built < will_built;});
            std::this_thread::sleep_for(std::chrono::seconds(building_time));

            std::cout << "The new farm has been successfully built! ";
            print_time();

            already_built += 1;
            cv.notify_all();
        };
        std::cout << "Built " << already_built << " farms!" << std::endl;
    }

};


class Builder{

};


int main() {
    Game game;
    std::cout << "Game has been started! ";
    print_time();
    std::cout << std::endl;

    std::thread t1(std::ref(game));
    std::thread t2(std::ref(game), true);

    t1.join();
    t2.join();

    return 0;
}
