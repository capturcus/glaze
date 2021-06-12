#pragma once

#include <string>

#define SOL_ALL_SAFETIES_ON 1
#include "deps/sol/sol.hpp"

static const std::string COROUTINE_ID = "__coroutine_id";

std::string prompt_choice(sol::this_state s, std::string prompt, sol::table choice);
