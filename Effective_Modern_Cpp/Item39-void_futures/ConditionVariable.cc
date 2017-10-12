/*
 * refer to wiz笔记 << C++11并发指南五.md >>
 */

#include <iostream>     // std::cout
#include <thread>       // std::thread
#include <mutex>        // std::mutex, std::unique_lock
#include <condition_variable> // std::condition_variable

///////////////////////////////////////////////////
std::mutex mtx;             // 全局互斥锁.
std::condition_variable cv; // 全局条件变量.
bool ready = false;         // 全局标志位.
void do_print_id(int id)// 线程函数
{
    std::unique_lock<std::mutex> lck(mtx);
    while (!ready)      // 如果标志位不为 true, 则等待…
        cv.wait(lck);   // 当前线程被阻塞, 当全局标志位变为 true 之后,

    // 线程被唤醒, 继续往下执行打印线程编号id.
    std::cout << "thread " << id << '\n';
}
// 唤醒所有线程
void go()
{
    std::unique_lock<std::mutex> lck(mtx);
    ready = true; // 设置全局标志位为 true.
    cv.notify_all(); // 唤醒所有线程.
}

///////////////////////////////////////////////////
std::mutex mtx2;
std::condition_variable cv2;
int cargo = 0;
bool shipment_available()
{
    return cargo != 0;
}
void consume(int n)// 消费者线程.
{
    for (int i = 0; i < n; ++i) {
        std::unique_lock <std::mutex> lck(mtx2);
        cv2.wait(lck, shipment_available);
        std::cout << cargo << '\n';
        cargo = 0;
    }
}


///////////////////////////////////////////////////
std::condition_variable cv3;
int value;
void do_read_value()
{
    std::cin >> value;
    cv3.notify_one();
}

////////////////////////////////////////////////////
std::mutex mtx4;
std::condition_variable cv4;
int cargo4 = 0; // shared value by producers and consumers
void consumer()
{
    std::unique_lock < std::mutex > lck(mtx4);
    while (cargo4 == 0)
        cv4.wait(lck);
    std::cout << cargo4 << " consumed!"<< '\n';
    cargo4 = 0;
}
void producer(int id)
{
    std::unique_lock < std::mutex > lck(mtx4);
    cargo4 = id;
    std::cout << cargo4 << " produced!"<< '\n';
    cv4.notify_one();
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
int main()
{
    ///////////////////////////////////////////////////
    {
        std::thread threads[10];

        // spawn 10 threads:
        for (int i = 0; i < 10; ++i)
            threads[i] = std::thread(do_print_id, i);
        std::cout << "10 threads ready to race…\n";

        go(); // go!

        // make all threads unjoinable
        for (auto & th:threads)
            th.join();
    }



    ///////////////////////////////////////////////////
    {
        std::thread consumer_thread(consume, 10); // 消费者线程.
        // 主线程为生产者线程, 生产 10 个物品.
        for (int i = 0; i < 10; ++i) {
            while (shipment_available())
                std::this_thread::yield();
            std::unique_lock <std::mutex> lck(mtx2);
            cargo = i + 1;
            cv2.notify_one();
        }
        consumer_thread.join();
    }

    ///////////////////////////////////////////////////
    {
        std::cout << "Please, enter an integer (I'll be printing dots): \n";
        std::thread th(do_read_value);
        std::mutex mtx;
        std::unique_lock<std::mutex> lck(mtx);
        while (cv3.wait_for(lck,std::chrono::seconds(1)) == std::cv_status::timeout) {
            std::cout << '.';
            std::cout.flush();
        }
        std::cout << "You entered: " << value << '\n';
        th.join();
    }

    ///////////////////////////////////////////////////
    {
        std::thread consumers[10], producers[10];
        // spawn 10 consumers and 10 producers:
        for (int i = 0; i < 10; ++i) {
            consumers[i] = std::thread(consumer);
            producers[i] = std::thread(producer, i + 1);
        }
        // join them back:
        for (int i = 0; i < 10; ++i) {
            producers[i].join();
            consumers[i].join();
        }
    }

    return 0;
}
