#include "engine_lua_api.hpp"
#include "glaze.hpp"

#include <boost/json.hpp>

namespace json = boost::json;

std::string prompt_choice(sol::this_state s, std::string player, std::string prompt, sol::table choice) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    json::array choices;

    for (const auto& key_value_pair : choice ) {
		sol::object value = key_value_pair.second;

        auto value_str = value.as<std::string>();
        choices.emplace_back(value_str);
	}
    json::object prompt_obj = {
        { "type", "prompt" },
        { "prompt_type", "choice" },
        { "prompt_text", prompt },
        { "prompt_key", uuid },
        { "prompt_data", choices }
    };

    send_to_player_by_name(player, json::serialize(prompt_obj));

    return "<test>";
}
