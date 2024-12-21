#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <iostream>

int function_example(std::vector<int>& vec) {
    int sum = 0;
    for (int i = 0; i < vec.size(); i++) {
        sum += vec[i];
    }
    vec.push_back(sum);
    std::cerr << sum << " ";
    return sum;
}


template <typename T>
class ThreadPool {
public:
    ThreadPool(size_t numThreads) : numThreads(numThreads) {
        threads.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        this->queueCondition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                        if (this->stop && this->tasks.empty()) {
                            return;
                        }
                        task = std::move(this->tasks.front());
                        this->tasks.pop_front();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        queueCondition.notify_all();
        for (auto& thread : threads) {
            thread.join();
        }
    }

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto future = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push_back([task] { (*task)(); });
        }
        queueCondition.notify_one();
        return future;
    }

private:
    size_t numThreads;
    std::vector<std::thread> threads;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::deque<std::function<void()>> tasks;
    bool stop = false;
};


void sum_vec() {
    ThreadPool<int> pool(std::thread::hardware_concurrency());
    std::vector<std::future<int>> futures;
    std::vector<int> vec = {0, 1, 2};
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool.enqueue(function_example, vec));
    }
    for (auto& future : futures) {
        future.get();
    }
    std::cerr << "\n";
}
