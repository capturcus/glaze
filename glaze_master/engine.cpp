#include "engine.hpp"
#include "glaze.hpp"
#include "websocket.hpp"
#include "engine_lua_api.hpp"

#include <mutex>
#include <queue>
#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>

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

boost::uuids::random_generator generator;

struct sol_resumable {
	sol::thread runner;
	sol::coroutine coroutine;
};

std::map<std::string, std::unique_ptr<sol_resumable>> resumables;

void run_as_resumable(const std::string& lua_code) {
	auto full_script = "function __main()\n" + lua_code + "\nend"; // because I just like to watch the world burn
	auto new_resumable = std::make_unique<sol_resumable>();
	new_resumable->runner = sol::thread::create(lua_state.lua_state());
	sol::state_view runner_state = new_resumable->runner.state();
	runner_state.safe_script(full_script, [](lua_State*, sol::protected_function_result pfr) {
		sol::error err = pfr;
		std::cout << "run_as_resumable: lua error: " << err.what() << std::endl;
		return pfr;
	});
	new_resumable->coroutine = runner_state["__main"];
	std::string coroutine_id = boost::lexical_cast<std::string>(generator());
	runner_state[COROUTINE_ID] = coroutine_id;
	auto result = new_resumable->coroutine();
	if (!result.valid()) {
		sol::error err = result;
		std::string what = err.what();
		std::cout << what << std::endl;
	}
	if (new_resumable->coroutine.runnable())
		resumables.insert({coroutine_id, std::move(new_resumable)});
}

void process_cmd(const std::string& line) {
	if (line.rfind("run", 0) == 0) {
		auto script_path = line.substr(4);
		std::cout << "running lua script: " << script_path << "\n";
		std::ifstream t(script_path);
		std::stringstream buffer;
		buffer << t.rdbuf();
		run_as_resumable(buffer.str());
	}
}

json::object table_to_json(const sol::table& table) {
	json::object ret;
	for (const auto& key_value_pair : table ) {
		sol::object key = key_value_pair.first;
		sol::object value = key_value_pair.second;

		auto key_str = key.as<std::string>();
		if (value.is<double>()) {
			ret[key_str] = value.as<double>();
		}
		if (value.is<std::string>()) {
			ret[key_str] = value.as<std::string>();
		}
		if (value.is<sol::table>()) {
			auto child_json = table_to_json(value.as<sol::table>());
			ret[key_str] = child_json;
		}
	}
	return ret;
}

std::string previous_world_update_str = "{}";

void push_world_if_necessary(const sol::table& world) {
	json::object json_world = table_to_json(world);
	json::object world_update = {
		{ "type", "world_update" },
		{ "world", json_world }
	};
	std::string new_world_update_str = json::serialize(world_update);
	if (new_world_update_str != previous_world_update_str) {
		std::cout << "sending world update: " << new_world_update_str << "\n";
		for (auto& p : players) {
			send_text(*p->socket, new_world_update_str);
		}
		previous_world_update_str = new_world_update_str;
	}
}

void process_message(player* p, const std::string& message) {
	json::object j;
	try {
		j = json::parse(message).as_object();
	} catch(const boost::exception& e) {
		std::cout << "engine: parsing json failed\n";
		return;
	}
	std::cout << message << "\n";
	if (j["type"] == "rpc_call") {
		process_rpc_call(p, j["rpc_key"].as_string().c_str(),
			j["function_name"].as_string().c_str(), j["data"]);
	}
	if (j["type"] == "cli_input") {
		std::string line = j["line"].as_string().c_str();
		std::cout << "cli_input line: |" << line << "|\n";
		if (line.size() > 0 && line[0] == '/') {
			process_cmd(line.substr(1));
		} else {
			lua_state.safe_script(line, [](lua_State*, sol::protected_function_result pfr) {
				sol::error err = pfr;
				std::cout << "engine: lua error: " << err.what() << std::endl;
				return pfr;
			});
		}
	}
	if (lua_state["world"].get_type() == sol::type::table) {
		sol::table world = lua_state["world"];
		push_world_if_necessary(world);
	} else {
		std::cout << "error: _G.world is not a table\n";
	}
}

void engine_thread() {
	lua_state.open_libraries(sol::lib::base, sol::lib::coroutine);

	lua_state["world"] = sol::new_table();
	lua_state.set_function("prompt_choice", sol::yielding(prompt_choice));

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
