#include "engine.hpp"

#include <mutex>
#include <queue>
#include <semaphore>

std::mutex queue_mutex;
std::queue<std::pair<player&, std::string>> queue;
std::counting_semaphore<100> semaphore;

void enqueue_message(player& p, const std::string& message) {
    std::unique_lock<std::mutex> ulock(queue_mutex);
    queue.push({p, message});
    semaphore.release();
}

void engine_thread() {
    for (;;) {
        semaphore.acquire();
        std::unique_lock<std::mutex> ulock(queue_mutex);
        auto p = queue.pop();
        // do something with the pair
    }
}
