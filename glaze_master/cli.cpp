#include "cli.hpp"
#include "engine.hpp"

#include <iostream>

void cli_thread() {
	for (;;) {
		std::string line;
		getline(std::cin, line);
		std::cout << "input: " << line << "\n";
		enqueue_message(nullptr, line);
	}
}
