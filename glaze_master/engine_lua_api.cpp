#include "engine_lua_api.hpp"

std::string prompt_choice(sol::this_state s, std::string prompt, sol::table choice) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    for (const auto& key_value_pair : choice ) {
		sol::object value = key_value_pair.second;

        auto value_str = value.as<std::string>();
        std::cout << "value_str" << value_str << "\n";
	}
    return "<test>";
}
