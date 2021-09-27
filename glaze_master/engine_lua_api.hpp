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
void run_for_players(sol::this_state s, std::vector<std::string> player_names, std::string function_name);

} // lua_api
