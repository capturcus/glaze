#pragma once

#include <string>

#include <limits>

#include "engine.hpp"

#define SOL_ALL_SAFETIES_ON 1
#include "deps/sol/sol.hpp"

namespace lua_api {

void prompt_text(sol::this_state s, std::string player, std::string prompt);
void prompt_choice(sol::this_state s, std::string player, std::string prompt, sol::table choice);
void prompt_text_response(sol::this_state s, std::string player, std::string prompt);
void prompt_number_response(sol::this_state s, std::string player, std::string prompt);
void log(std::string message);
std::vector<std::string> get_players();
void run_in_background(sol::this_state s, sol::coroutine coroutine, std::vector<sol::table> tables);

} // lua_api
