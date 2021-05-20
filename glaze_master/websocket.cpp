#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/json/src.hpp>

#include "glaze.hpp"
#include "engine.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;

std::string next_message(websocket::stream<tcp::socket>& ws) {
	beast::flat_buffer buffer;
	ws.read(buffer);
	return beast::buffers_to_string(buffer.data());
}

void send_text(websocket::stream<tcp::socket>& ws, std::string text) {
	ws.text(true);
	ws.write(net::buffer(text));
}

void
do_session(tcp::socket socket) {
	try
	{
		auto ws = std::make_unique<websocket::stream<tcp::socket>>(std::move(socket));

		ws->set_option(websocket::stream_base::decorator(
			[](websocket::response_type& res)
			{
				res.set(http::field::server,
					std::string(BOOST_BEAST_VERSION_STRING) +
						" websocket-server-sync");
			}));

		ws->accept();

		auto first_message = next_message(*ws);
		json::value value = json::parse(first_message);
		auto incoming_name = value.as_object()["name"].as_string();
		auto it = std::find_if(players.begin(), players.end(),
			[&incoming_name](player& p) { return p.name == incoming_name; });
		if (it != players.end()) {
			// this name is already taken
			std::cout << "name " << incoming_name << " already taken\n";
			ws->close(boost::beast::websocket::close_reason());
			return;
		}

		player new_player;
		new_player.name = std::string(incoming_name.c_str());
		new_player.socket = std::move(ws);
		players.push_back(std::move(new_player));

		for (;;)
		{
			auto message = next_message(*ws);
			enqueue_message(new_player, message);
		}
	}
	catch(beast::system_error const& se)
	{
		// This indicates that the session was closed
		if(se.code() != websocket::error::closed)
			std::cerr << "Error: " << se.code().message() << std::endl;
	}
	catch(std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

void websocket_thread() {
	try
	{
		auto const address = net::ip::make_address("0.0.0.0");
		auto const port = 8080;

		net::io_context ioc{1};

		tcp::acceptor acceptor{ioc, {address, port}};
		for (;;)
		{
			tcp::socket socket{ioc};

			acceptor.accept(socket);

			std::thread(
				&do_session,
				std::move(socket)).detach();
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return;
	}
}
