#pragma once

#include <string>
#include <memory>
#include <map>
#include <limits>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid.hpp>

#include <boost/json.hpp>

namespace json = boost::json;

struct player;

void enqueue_message(player* p, const json::object& message);
void engine_thread();

#define SOL_ALL_SAFETIES_ON 1
#include "deps/sol/sol.hpp"

static const std::string COROUTINE_ID = "__coroutine_id";

struct sol_resumable {
	sol::thread runner;
	sol::coroutine coroutine;
};

extern boost::uuids::random_generator generator;
extern std::map<std::string, std::unique_ptr<sol_resumable>> resumables;

template<typename... Args>
void continue_coroutine(std::unique_ptr<sol_resumable>&& resumable, Args... args) {
	std::string coroutine_id = boost::lexical_cast<std::string>(generator());
	sol::state_view runner_state = resumable->runner.state();
	runner_state[COROUTINE_ID] = coroutine_id;
	auto result = resumable->coroutine(args...);
	if (!result.valid()) {
		sol::error err = result;
		std::string what = err.what();
		std::cout << what << std::endl;
	}
	if (resumable->coroutine.runnable())
		resumables.insert({coroutine_id, std::move(resumable)});
}

template<typename... Args>
void run_coroutine_as_resumable(sol::state_view& lua_state, sol::coroutine coroutine, Args... args) {
	auto new_resumable = std::make_unique<sol_resumable>();
	new_resumable->runner = sol::thread::create(lua_state.lua_state());
	sol::state_view runner_state = new_resumable->runner.state();
	new_resumable->coroutine = sol::coroutine(runner_state, coroutine);
	continue_coroutine(std::move(new_resumable), args...);
}
