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
	static constexpr const char* DefaultDbPath = ".";
	static constexpr int DefaultKeepaliveIntervalSeconds = 30;
	static constexpr int DefaultKeepaliveTimeoutSeconds = 10;
	static constexpr std::uint16_t DefaultTlsPort = 17771;

	std::uint16_t listen_port = DefaultListenPort;
	std::size_t max_connections = DefaultMaxConnections;
	std::string db_path = DefaultDbPath;
	int keepalive_interval_seconds = DefaultKeepaliveIntervalSeconds;
	int keepalive_timeout_seconds = DefaultKeepaliveTimeoutSeconds;

	/// TLS: the certificate chain and private key (PEM files). Given both, the server
	/// listens with TLS on tls_port beside the open listen_port; given neither, it
	/// speaks only in the open.
	std::string tls_cert;
	std::string tls_key;
	std::uint16_t tls_port = DefaultTlsPort;
	/// Listen on the open port too. Old clients speak only in the open: it is closed
	/// once all of them are renewed.
	bool plain = true;

	bool tls() const { return !tls_cert.empty() || !tls_key.empty(); }
};


} // namespace will
