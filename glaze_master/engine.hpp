#pragma once

#include <string>

struct player;

void enqueue_message(const player& p, const std::string& message);
