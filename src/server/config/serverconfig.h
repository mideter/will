#pragma once

#include <cstddef>
#include <cstdint>
#include <string>


namespace will {


/** Server configuration (no invariants; validated by ServerConfigValidator). */
struct ServerConfig {
	static constexpr int MinListenPort = 1;
	static constexpr int MaxListenPort = 65535;
	static constexpr std::uint16_t DefaultListenPort = 7770;
	static constexpr std::size_t DefaultMaxConnections = 4096;
	static constexpr const char* DefaultDbPath = "will.db";
	static constexpr int DefaultKeepaliveIntervalSeconds = 30;
	static constexpr int DefaultKeepaliveTimeoutSeconds = 10;

	std::uint16_t listen_port = DefaultListenPort;
	std::size_t max_connections = DefaultMaxConnections;
	std::string db_path = DefaultDbPath;
	int keepalive_interval_seconds = DefaultKeepaliveIntervalSeconds;
	int keepalive_timeout_seconds = DefaultKeepaliveTimeoutSeconds;
};


} // namespace will
