#pragma once

#include <string>
#include <memory>
#include <vector>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

typedef boost::beast::websocket::stream<boost::asio::ip::tcp::socket> boost_websocket;
struct player {
	std::string name;
	std::unique_ptr<boost_websocket> socket;

	inline player() {}
	inline player(std::string n, std::unique_ptr<boost_websocket> ws)
		: name(n), socket(std::move(ws))
		{}
};

void send_to_player_by_name(const std::string& name, std::string data);

extern std::vector<std::unique_ptr<player>> players;
