#pragma once

#include "clientconfig.h"

#include <cstddef>


namespace will {


/** Load generator configuration (connection invariants validated separately). */
struct LoadClientsConfig {
	static constexpr std::size_t DefaultClients = 100;
	static constexpr std::size_t DefaultMessagesPerClient = 0;
	static constexpr int DefaultHoldSeconds = 30;

	ClientConfig connection;
	std::size_t clients = DefaultClients;
	std::size_t messages_per_client = DefaultMessagesPerClient;
	int hold_seconds = DefaultHoldSeconds;
};


} // namespace will
