#pragma once

#include <cstddef>
#include <cstdint>
#include <string>


namespace will {


/// Server configuration (no invariants; validated by ServerConfigValidator). Given both a
/// TLS certificate chain and private key (PEM files), the server listens with TLS on
/// tls_port beside the open listen_port; given neither, it speaks only in the open. The open
/// port stays while old clients, which speak only in the open, are not all renewed.
struct ServerConfig {
	static constexpr int MinListenPort = 1;
	static constexpr int MaxListenPort = 65535;
	static constexpr std::uint16_t DefaultListenPort = 7770;
	static constexpr std::size_t DefaultMaxConnections = 4096;
	static constexpr const char* DefaultDbPath = ".";
	static constexpr int DefaultKeepaliveIntervalSeconds = 30;
	static constexpr int DefaultKeepaliveTimeoutSeconds = 10;
	static constexpr std::uint16_t DefaultTlsPort = 17771;

	std::uint16_t listen_port = DefaultListenPort;
	std::size_t max_connections = DefaultMaxConnections;
	std::string db_path = DefaultDbPath;
	int keepalive_interval_seconds = DefaultKeepaliveIntervalSeconds;
	int keepalive_timeout_seconds = DefaultKeepaliveTimeoutSeconds;

	std::string tls_cert;
	std::string tls_key;
	std::uint16_t tls_port = DefaultTlsPort;
	bool plain = true;

	bool tls() const { return !tls_cert.empty() || !tls_key.empty(); }
};


} // namespace will
