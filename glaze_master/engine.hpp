#pragma once

#include <string>

struct player;

void enqueue_message(player* p, const std::string& message);
void engine_thread();
