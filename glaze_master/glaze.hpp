#pragma once

#include <string>
#include <memory>
#include <vector>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

struct player {
	std::string name;
	std::unique_ptr<boost::beast::websocket::stream<boost::asio::ip::tcp::socket>> socket;
};

extern std::vector<player> players;
