#include "engine.hpp"
#include "glaze.hpp"
#include "websocket.hpp"

#include <mutex>
#include <queue>
#include <iostream>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/json.hpp>

namespace json = boost::json;

std::mutex queue_mutex;
std::queue<std::pair<player*, const std::string&>> queue;
boost::interprocess::interprocess_semaphore semaphore(0);

void enqueue_message(player* p, const std::string& message) {
	std::unique_lock<std::mutex> ulock(queue_mutex);
	queue.push({p, message});
	semaphore.post();
}

json::object test_object = {
	{ "sand left", 42 },
	{ "goblin1", {
		{ "hp", 100 },
		{ "mana", 50 },
		{ "inventory", {
			{ "sword", { { "atk", 5} } },
			{ "gold", 5 }
		} }
	} },
	{ "goblin2", {
		{ "hp", 34 },
		{ "mana", 40 },
	} }
};

void process_message(player* p, const std::string& message) {
	for (auto& p : players) {
		send_text(*p->socket, json::serialize(test_object));
	}
}

void engine_thread() {
	for (;;) {
		semaphore.wait();
		auto p = queue.front();
		{
			std::unique_lock<std::mutex> ulock(queue_mutex);
			queue.pop();
		}
		process_message(p.first, p.second);
	}
}
