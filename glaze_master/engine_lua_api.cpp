#include "engine_lua_api.hpp"
#include "glaze.hpp"

#include <boost/json.hpp>

namespace json = boost::json;

void send_prompt_object(std::string uuid, std::string player, std::string prompt_type, std::string prompt) {
    json::object prompt_obj = {
        { "type", "prompt" },
        { "prompt_type", prompt_type },
        { "prompt_text", prompt },
        { "prompt_key", uuid },
    };

    send_to_player_by_name(player, json::serialize(prompt_obj));
}

void prompt_text(sol::this_state s, std::string player, std::string prompt) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    send_prompt_object(uuid, player, "text", prompt);
}

void prompt_choice(sol::this_state s, std::string player, std::string prompt, sol::table choice) {
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
}

void prompt_text_response(sol::this_state s, std::string player, std::string prompt) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    send_prompt_object(uuid, player, "text_response", prompt);
}

void prompt_number_response(sol::this_state s, std::string player, std::string prompt) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    send_prompt_object(uuid, player, "number_response", prompt);
}
