#include <iostream>
#include <list>
#include <future>
#include <mutex>

size_t factorial(const size_t& n){
    if (n <= 1){
        return 1;
    }

    size_t res = 1;
    for (size_t i = 2;i <= n; i++){
        res *= i;
    }
    return res;
}

namespace task{
    class complete_task{
        std::mutex pt_list_mutex;
        std::list<std::pair<std::packaged_task<size_t(const size_t&)>, size_t>> pt_list;
        std::list<std::pair<size_t, size_t>> res;

    public:
        void operator()(size_t n){
            std::mutex m;
            std::unique_lock<std::mutex> ul(m);
            std::condition_variable cv;
            std::packaged_task<size_t(const size_t&)> task(factorial);

            pt_list_mutex.lock();
            pt_list.emplace_back(std::move(task), n);

            cv.notify_all();
            std::cout << "The task ("  << n << "!) has been successfully added to the queue!" << std::endl;
            pt_list_mutex.unlock();
        }

        void operator()(size_t num_of_values, bool){
            std::mutex m;
            std::unique_lock<std::mutex> ul(m);
            std::condition_variable cv;

            size_t idx = 0;
            while(idx < num_of_values){
                cv.wait(ul, [&]()->bool {return !pt_list.empty();});
                pt_list_mutex.lock();
                std::packaged_task<size_t(const size_t&)> task = std::move(pt_list.begin()->first);
                size_t value = pt_list.begin()->second;
                pt_list.erase(pt_list.begin(), ++pt_list.begin());
                pt_list_mutex.unlock();

                std::future<size_t> future = task.get_future();
                std::thread thread(std::move(task), value);
                thread.detach();
                //Можно создать список из std::future, но тут возникает проблема как его достать оттуда
                res.emplace_back(future.get(), value);

                idx++;
            }
        }
        void print_values(){
            std::cout << std::endl;
            std::for_each(res.cbegin(), res.cend(), [&](const std::pair<size_t, size_t>& p){
                std::cout << p.second << "! = " << p.first << std::endl;
            });
        }
    };
}

void solution(){
    task::complete_task c_task;

    std::thread t1(std::ref(c_task), 7);
    std::thread t2(std::ref(c_task), 8);
    std::thread t3(std::ref(c_task), 12);
    std::thread t4(std::ref(c_task), 4);
    std::thread do_thread(std::ref(c_task), 4, true);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    do_thread.join();

    c_task.print_values();
}

int main() {
    solution();
    return 0;
}
