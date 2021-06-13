#pragma once

#include <string>

#include <limits>

#define SOL_ALL_SAFETIES_ON 1
#include "deps/sol/sol.hpp"

static const std::string COROUTINE_ID = "__coroutine_id";

void prompt_text(sol::this_state s, std::string player, std::string prompt);
void prompt_choice(sol::this_state s, std::string player, std::string prompt, sol::table choice);
void prompt_text_response(sol::this_state s, std::string player, std::string prompt);
void prompt_number_response(sol::this_state s, std::string player, std::string prompt);
