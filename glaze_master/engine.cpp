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
std::queue<std::pair<player*, std::string>> queue;
boost::interprocess::interprocess_semaphore semaphore(0);

void enqueue_message(player* p, const std::string& message) {
	std::unique_lock<std::mutex> ulock(queue_mutex);
	queue.push({p, message});
	semaphore.post();
}

json::object test_world = {
	{ "type", "world_update" },
	{ "world", {
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
		} } }
	}
};

json::object make_test_log() {
	static int num = 0;
	return {
		{ "type", "log_message" },
		{ "log", "siemansko" + std::to_string(num++) },
	};
}

void process_message(player* p, const std::string& message) {
	std::cout << "message |" << message << "|\n";
	if (message == "l") {
		std::cout << "sending test log\n";
		for (auto& p : players) {
			send_text(*p->socket, json::serialize(make_test_log()));
		}
		return;
	}
	for (auto& p : players) {
		send_text(*p->socket, json::serialize(test_world));
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
