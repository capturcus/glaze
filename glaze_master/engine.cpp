#include "engine.hpp"
#include "glaze.hpp"

#include <mutex>
#include <queue>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

std::mutex queue_mutex;
std::queue<std::pair<const player&, const std::string&>> queue;
boost::interprocess::interprocess_semaphore semaphore(0);

void enqueue_message(const player& p, const std::string& message) {
	std::unique_lock<std::mutex> ulock(queue_mutex);
	queue.push({p, message});
	semaphore.post();
}

void engine_thread() {
	for (;;) {
		semaphore.wait();
		auto p = queue.front();
		{
			std::unique_lock<std::mutex> ulock(queue_mutex);
			queue.pop();
		}
		// do something with the pair
	}
}
