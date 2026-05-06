#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include "serveraddress.h"
#include "sockethandle.h"


// TCP transport: uint32 big-endian length + payload[length]. Empty payload is allowed.
// The relay server reads and forwards aligned frames while logging header and body separately.
class MessengerClient {
public:
	// Maximum payload size per frame (excluding the 4-byte length prefix).
	static constexpr std::size_t max_payload_bytes = 1u << 20; // 1 MiB

	MessengerClient();

	void connect(ServerAddress server);
	void send(std::string_view message) const;

	// std::nullopt = peer closed cleanly before the next frame header; otherwise one payload (may be "").
	std::optional<std::string> receiveMessage() const;

	void shutdown() const;

private:
	SocketHandle socket_;
};
