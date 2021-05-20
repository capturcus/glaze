#include <thread>

#include "glaze.hpp"
#include "websocket.hpp"
#include "cli.hpp"
#include "engine.hpp"

std::vector<player> players;

int main() {
	std::vector<std::thread> threads;
	auto thread_fns = {
		websocket_thread,
		engine_thread,
		cli_thread,
	};
	for (auto t : thread_fns) {
		threads.emplace_back(t);
	}
	for (auto& t : threads) {
		t.join();
	}
}
