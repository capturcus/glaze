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

json::value test_actions = { "attack", "cast spell", "distract"};

typedef json::value (*rpc_handler)(json::value);

json::value rpc_actions_for_node(json::value data) {
	return test_actions;
}

std::map<std::string, rpc_handler> rpc_handlers = {
	std::make_pair( "actions_for_node", rpc_actions_for_node ),
};

void process_rpc_call(player* p, std::string rpc_key,
	std::string function_name, json::value data) {
	json::value ret = rpc_handlers[function_name](data);
	json::object ret_j = {
		{ "type", "rpc_result" },
		{ "rpc_key", rpc_key },
		{ "data", ret },
	};
	send_text(*p->socket, json::serialize(ret_j));
}

void process_message(player* p, const std::string& message) {
	auto j = json::parse(message).as_object();
	if (j["type"] == "cli_input") {
	}
	if (j["type"] == "rpc_call") {
		process_rpc_call(p, j["rpc_key"].as_string().c_str(),
			j["function_name"].as_string().c_str(), j["data"]);
	}
	if (j["type"] == "init_world") {
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
