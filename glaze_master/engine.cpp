#include "engine.hpp"
#include "glaze.hpp"
#include "websocket.hpp"

#include <mutex>
#include <queue>
#include <iostream>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/json.hpp>

#define SOL_ALL_SAFETIES_ON 1
#include "deps/sol/sol.hpp"

namespace json = boost::json;

std::mutex queue_mutex;
std::queue<std::pair<player*, std::string>> queue;
boost::interprocess::interprocess_semaphore semaphore(0);
sol::state lua_state;

void enqueue_message(player* p, const std::string& message) {
	std::unique_lock<std::mutex> ulock(queue_mutex);
	queue.push({p, message});
	semaphore.post();
}

typedef json::value (*rpc_handler)(json::value);

json::value rpc_actions_for_node(json::value data) {
	return json::array();
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

void process_cmd(const std::string& line) {

}

void push_world(const sol::table& world) {
	for (const auto& key_value_pair : world ) {
		sol::object key = key_value_pair.first;
		// sol::object value = key_value_pair.second;

		std::cout << "key: " << key.as<std::string>() << "\n";
		// std::cout << "key: " << sol::to_string(value) << "\n";
	}
}

void process_message(player* p, const std::string& message) {
	auto j = json::parse(message).as_object();
	std::cout << message << "\n";
	if (j["type"] == "rpc_call") {
		process_rpc_call(p, j["rpc_key"].as_string().c_str(),
			j["function_name"].as_string().c_str(), j["data"]);
	}
	if (j["type"] == "init_world") {
		// send_text(*p->socket, json::serialize(test_world));
	}
	if (j["type"] == "cli_input") {
		std::string line = j["line"].as_string().c_str();
		std::cout << "cli_input line: |" << line << "|\n";
		if (line.size() > 0 && line[0] == '/') {
			process_cmd(line.substr(1));
		} else {
			lua_state.safe_script(line);
		}
	}
	if (lua_state["world"].get_type() == sol::type::table) {
		sol::table world = lua_state["world"];
		push_world(world);
	} else {
		std::cout << "error: _G.world is not a table\n";
	}
}

void engine_thread() {
	lua_state.open_libraries(sol::lib::base);

	lua_state["world"] = sol::new_table();

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
