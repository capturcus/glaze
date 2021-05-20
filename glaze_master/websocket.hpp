#pragma once

#include "glaze.hpp"

void websocket_thread();
void send_text(boost_websocket& ws, std::string text);
