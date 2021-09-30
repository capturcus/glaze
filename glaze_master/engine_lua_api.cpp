#include "engine_lua_api.hpp"
#include "glaze.hpp"
#include "websocket.hpp"

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

void lua_api::prompt_text(sol::this_state s, std::string player, std::string prompt) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    send_prompt_object(uuid, player, "text", prompt);
}

void lua_api::prompt_choice(sol::this_state s, std::string player, std::string prompt, sol::table choice) {
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

void lua_api::prompt_text_response(sol::this_state s, std::string player, std::string prompt) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    send_prompt_object(uuid, player, "text_response", prompt);
}

void lua_api::prompt_number_response(sol::this_state s, std::string player, std::string prompt) {
    sol::state_view lua(s);
    std::string uuid = lua[COROUTINE_ID];

    send_prompt_object(uuid, player, "number_response", prompt);
}

void lua_api::log(std::string message) {
    json::object log_obj = {
        { "type", "log_message" },
        { "log", message },
    };
    for (auto& p : players) {
        send_text(*p->socket, json::serialize(log_obj));
    }
    std::cout << "[log] " << message << "\n";
}

std::vector<std::string> lua_api::get_players() {
    std::vector<std::string> ret;
    for (auto& p : players) {
        ret.push_back(p->name);
    }
    return ret;
}

void lua_api::run_in_background(sol::this_state s, sol::coroutine coroutine, std::vector<sol::table> tables) {
    auto state_view = sol::state_view(s);
    for (auto& table : tables) {
        run_coroutine_as_resumable(state_view, coroutine, table);
    }
}
