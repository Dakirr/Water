#ifndef THREAD_Q
#define THREAD_Q
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
using namespace std;
extern mt19937 rnd;
extern bool flg;
class ThreadSafeQueue {
private:
    std::mutex mutex;
    std::condition_variable cv;

public:
    std::queue<int32_t> queue;

    void push(size_t value);

    size_t pop();

    bool empty();
};

extern ThreadSafeQueue RandQueue;

void rand_process();

#endif