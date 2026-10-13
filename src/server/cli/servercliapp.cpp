#include "servercliapp.h"

#include "serverconfig.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {


ServerCliApp::ServerCliApp(const ServerConfig& defaults)
	: app_{"will-server"}
	, port_(static_cast<int>(defaults.listen_port))
	, max_clients_(defaults.max_connections)
	, db_path_(defaults.db_path)
	, keepalive_interval_seconds_(defaults.keepalive_interval_seconds)
	, keepalive_timeout_seconds_(defaults.keepalive_timeout_seconds)
	, tls_cert_(defaults.tls_cert)
	, tls_key_(defaults.tls_key)
	, tls_port_(static_cast<int>(defaults.tls_port))
	, no_plain_(!defaults.plain)
{
	app_.allow_extras(false);

	app_.add_option("--port", port_)->description("Listen port");
	app_.add_option("--max-clients", max_clients_)->description("Max concurrent connections");
	app_.add_option("--db-path", db_path_)->description("Directory of the SQLite databases (will.eternity/.space/.time.db)");
	app_.add_option("--keepalive-interval", keepalive_interval_seconds_)
		->description("gRPC HTTP/2 keepalive ping interval in seconds");
	app_.add_option("--keepalive-timeout", keepalive_timeout_seconds_)
		->description("gRPC HTTP/2 keepalive ping timeout in seconds");
	app_.add_option("--tls-cert", tls_cert_)->description("TLS certificate chain (PEM)");
	app_.add_option("--tls-key", tls_key_)->description("TLS private key (PEM)");
	app_.add_option("--tls-port", tls_port_)->description("TLS listen port");
	app_.add_flag("--no-plain", no_plain_)->description("Do not listen in the open (TLS only)");
}


void ServerCliApp::print_help(std::ostream& os) const
{
	os << std::format(
		"Usage: will-server [options]\n"
		"\n"
		"Options:\n"
		"  -h, --help                      Print usage and exit\n"
		"  --port PORT                     Listen port (default {})\n"
		"  --max-clients N                 Max concurrent connections (default {})\n"
		"  --db-path DIR                   Directory of the databases (default {})\n"
		"  --keepalive-interval SECONDS    gRPC keepalive ping interval (default {})\n"
		"  --keepalive-timeout SECONDS     gRPC keepalive ping timeout (default {})\n"
		"  --tls-cert FILE                 TLS certificate chain, PEM (with --tls-key: listen with TLS too)\n"
		"  --tls-key FILE                  TLS private key, PEM\n"
		"  --tls-port PORT                 TLS listen port (default {})\n"
		"  --no-plain                      Do not listen in the open: TLS only\n",
		ServerConfig::DefaultListenPort,
		ServerConfig::DefaultMaxConnections,
		ServerConfig::DefaultDbPath,
		ServerConfig::DefaultKeepaliveIntervalSeconds,
		ServerConfig::DefaultKeepaliveTimeoutSeconds,
		ServerConfig::DefaultTlsPort);
}


void ServerCliApp::exit_on_help(const CLI::CallForHelp& error) const
{
	print_help(std::cerr);
	std::exit(error.get_exit_code());
}


void ServerCliApp::exit_on_parse_error(const CLI::ParseError& error) const
{
	std::exit(app_.exit(error, std::cerr));
}


void ServerCliApp::apply_to(ServerConfig& config) const
{
	config.listen_port = static_cast<std::uint16_t>(port_);
	config.max_connections = max_clients_;
	config.db_path = db_path_;
	config.keepalive_interval_seconds = keepalive_interval_seconds_;
	config.keepalive_timeout_seconds = keepalive_timeout_seconds_;
	config.tls_cert = tls_cert_;
	config.tls_key = tls_key_;
	config.tls_port = static_cast<std::uint16_t>(tls_port_);
	config.plain = !no_plain_;
}


ServerConfig ServerCliApp::parse(int argc, char* argv[])
{
	ServerConfig config;
	app_.parse(argc, argv);
	apply_to(config);
	return config;
}


} // namespace will
