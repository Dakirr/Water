#include "Thread.h"
#include <iostream>
mt19937 rnd(1337);
ThreadSafeQueue RandQueue;

bool flg_rnd = false;


void ThreadSafeQueue::push(size_t value) {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->queue.push(value);
    this->cv.notify_one();  // Notify waiting consumer
}

size_t ThreadSafeQueue::pop() {
    std::unique_lock<std::mutex> lock(this->mutex);
    this->cv.wait(lock, [this] { return !this->queue.empty(); }); // Wait until queue not empty
    size_t value = this->queue.front();
    this->queue.pop();
    return value;
}

bool ThreadSafeQueue::empty() {
    std::unique_lock<std::mutex> lock(this->mutex);
    return this->queue.empty();
}


void rand_process() {
    while (true) {
        RandQueue.push(rnd());
        while ((RandQueue.queue.size() > 1000) && !flg_rnd);
        if (flg_rnd) return;
    }
};