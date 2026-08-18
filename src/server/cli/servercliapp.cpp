module;

#include <CLI/CLI.hpp>

#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <string>

module will.server.servercliapp;


namespace will {


ServerCliApp::ServerCliApp(const ServerConfig& defaults)
    : app_{"will-server"}
    , port_(static_cast<int>(defaults.listen_port))
    , max_clients_(defaults.max_connections)
    , db_path_(defaults.db_path)
    , keepalive_interval_seconds_(defaults.keepalive_interval_seconds)
    , keepalive_timeout_seconds_(defaults.keepalive_timeout_seconds)
{
    app_.allow_extras(false);

    app_.add_option("--port", port_)->description("Listen port");
    app_.add_option("--max-clients", max_clients_)->description("Max concurrent connections");
    app_.add_option("--db-path", db_path_)->description("SQLite database path");
    app_.add_option("--keepalive-interval", keepalive_interval_seconds_)
        ->description("gRPC HTTP/2 keepalive ping interval in seconds");
    app_.add_option("--keepalive-timeout", keepalive_timeout_seconds_)
        ->description("gRPC HTTP/2 keepalive ping timeout in seconds");
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
        "  --db-path PATH                  SQLite database path (default {})\n"
        "  --keepalive-interval SECONDS    gRPC keepalive ping interval (default {})\n"
        "  --keepalive-timeout SECONDS     gRPC keepalive ping timeout (default {})\n",
        ServerConfig::DefaultListenPort,
        ServerConfig::DefaultMaxConnections,
        ServerConfig::DefaultDbPath,
        ServerConfig::DefaultKeepaliveIntervalSeconds,
        ServerConfig::DefaultKeepaliveTimeoutSeconds);
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
}


ServerConfig ServerCliApp::parse(int argc, char* argv[])
{
    ServerConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace will
