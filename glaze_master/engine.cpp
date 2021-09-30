#include "engine.hpp"
#include "glaze.hpp"
#include "websocket.hpp"
#include "engine_lua_api.hpp"

#include <mutex>
#include <queue>
#include <iostream>
#include <fstream>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/json.hpp>

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
	std::string path = data.as_string().c_str();
	auto new_thread = sol::thread::create(lua_state.lua_state());
	sol::state_view runner_state = new_thread.state();
	sol::table lua_ret;
	try {
		lua_ret = runner_state["actions_for_node"](path);
	} catch (const sol::error& e) {
		std::cout << "failed to execute actions_for_node: " << e.what() << "\n";
		return json::array();
	}
	json::array ret;

    for (const auto& key_value_pair : lua_ret ) {
		sol::object value = key_value_pair.second;

        auto value_str = value.as<std::string>();
        ret.emplace_back(value_str);
	}
	return ret;
}

std::map<std::string, rpc_handler> rpc_handlers = {
	std::make_pair( "actions_for_node", rpc_actions_for_node ),
};

player* find_player_by_name(const std::string& name) {
	auto ret = std::find_if(players.begin(), players.end(), [name](auto& p){
		return p->name == name;
	});
	if (ret == players.end())
		return nullptr;
	return ret->get();
}

void send_to_player_by_name(const std::string& name, std::string data) {
	auto p = find_player_by_name(name);
	if (!p)
		throw std::runtime_error("couldn't find player with name: " + name);
	send_text(*p->socket, data);
}

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
std::map<std::string, std::unique_ptr<sol_resumable>> resumables;

void run_as_resumable(const std::string& lua_code) {
	auto full_script = "function __main()\n" + lua_code + "\nend"; // because I just like to watch the world burn
	lua_state.safe_script(full_script, [](lua_State*, sol::protected_function_result pfr) {
		sol::error err = pfr;
		std::cout << "run_as_resumable: lua error: " << err.what() << std::endl;
		return pfr;
	});
	sol::coroutine coroutine = lua_state["__main"];
	run_coroutine_as_resumable(lua_state, coroutine);
	lua_state["__main"] = nullptr;
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

std::string table_to_lua(const sol::table& table) {
	std::string ret;
	for (const auto& key_value_pair : table ) {
		sol::object key = key_value_pair.first;
		sol::object value = key_value_pair.second;

		auto key_str = key.as<std::string>();
		if (value.is<double>()) {
			ret += key_str + "=" + std::to_string(value.as<double>()) + ",";
		}
		if (value.is<std::string>()) {
			ret += key_str + "=\"" + value.as<std::string>() + "\",";
		}
		if (value.is<sol::table>()) {
			std::string child_lua = table_to_lua(value.as<sol::table>());
			ret += key_str + "=" + child_lua + ",";
		}
	}
	if (ret.size() > 0)
		ret.pop_back();
	return "{" + ret + "}";
}

std::string previous_json_world = "{}";

void push_world_if_necessary(const sol::table& world) {
	json::object json_world = table_to_json(world);
	json::object world_update = {
		{ "type", "world_update" },
		{ "world", json_world }
	};
	std::string json_world_str = json::serialize(json_world);
	std::string new_world_update_str = json::serialize(world_update);
	if (json_world_str != previous_json_world) {
		for (auto& p : players) {
			send_text(*p->socket, new_world_update_str);
		}
		std::string world_lua = table_to_lua(world);
		std::ofstream out("world.lua");
		out << "world=" + world_lua;
		out.close();
		std::cout << "saved world.lua\n";
		previous_json_world = json_world_str;
	}
}

void process_prompt_result(json::object j) {
	std::string prompt_key = j["prompt_key"].as_string().c_str();	
	auto it = resumables.find(prompt_key);
	std::unique_ptr<sol_resumable> resumable = std::move(it->second);
	resumables.erase(it);
	if (j["prompt_result"].is_string()) {
		std::string prompt_result = j["prompt_result"].as_string().c_str();
		continue_coroutine(std::move(resumable), prompt_result);
	} else if (j["prompt_result"].is_number()) {
		int64_t prompt_result = j["prompt_result"].as_int64();
		continue_coroutine(std::move(resumable), prompt_result);
	} else if (j["prompt_result"].is_null())
		continue_coroutine(std::move(resumable));
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
	if (j["type"] == "rpc_call")
		process_rpc_call(p, j["rpc_key"].as_string().c_str(),
			j["function_name"].as_string().c_str(), j["data"]);

	if (j["type"] == "cli_input") {
		std::string line = j["line"].as_string().c_str();
		std::cout << "cli_input line: |" << line << "|\n";
		if (line.size() > 0 && line[0] == '/') {
			process_cmd(line.substr(1));
		} else {
			run_as_resumable(line);
		}
	}
	if (j["type"] == "prompt_result")
		process_prompt_result(j);

	if (j["type"] == "action_taken") {
		if (j["action"].is_null() || j["target"].is_null())
			return;
		std::string action = j["action"].as_string().c_str();
		std::string target = j["target"].as_string().c_str();
		run_as_resumable("action_taken(\""+action+"\", \""+target+"\")");
	}

	if (lua_state["world"].get_type() == sol::type::table) {
		sol::table world = lua_state["world"];
		push_world_if_necessary(world);
	} else {
		std::cout << "error: _G.world is not a table\n";
	}
}

#define SET_LUA_FUNCTION(fn) lua_state.set_function(#fn, lua_api::fn)
#define SET_LUA_FUNCTION_YIELDING(fn) lua_state.set_function(#fn, sol::yielding(lua_api::fn))

void engine_thread() {
	lua_state.open_libraries(sol::lib::base, sol::lib::coroutine);

	lua_state["world"] = sol::new_table();
	SET_LUA_FUNCTION_YIELDING(prompt_choice);
	SET_LUA_FUNCTION_YIELDING(prompt_text);
	SET_LUA_FUNCTION_YIELDING(prompt_text_response);
	SET_LUA_FUNCTION_YIELDING(prompt_number_response);
	SET_LUA_FUNCTION(log);
	SET_LUA_FUNCTION(get_players);
	SET_LUA_FUNCTION(run_in_background);

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
