#include "cli.hpp"
#include "engine.hpp"

#include <iostream>
#include <algorithm>
#include <boost/algorithm/string/replace.hpp>

static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

void cli_thread() {
	for (;;) {
		std::string line;
		getline(std::cin, line);
        boost::replace_all(line, "\"", "\\\"");
        // json::object j = ;
		enqueue_message(nullptr, {
            { "type", "cli_input" },
            { "line",  trim(line) }
        });
	}
}
