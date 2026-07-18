#include "servercliapp.h"

#include "serverconfig.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {


ServerCliApp::ServerCliApp(const ServerConfig& defaults)
    : app_{"will-server"}
    , port_(static_cast<int>(defaults.listen_port))
    , io_threads_(defaults.io_threads)
    , listen_backlog_(defaults.listen_backlog)
    , max_clients_(defaults.max_connections)
    , db_path_(defaults.db_path)
    , heartbeat_interval_seconds_(defaults.heartbeat_interval_seconds)
    , heartbeat_timeout_seconds_(defaults.heartbeat_timeout_seconds)
{
    app_.allow_extras(false);

    app_.add_option("--port", port_)->description("Listen port");
    app_.add_option("--io-threads", io_threads_)->description("io_context worker threads");
    app_.add_option("--listen-backlog", listen_backlog_)->description("listen() backlog");
    app_.add_option("--max-clients", max_clients_)->description("Max concurrent connections");
    app_.add_option("--db-path", db_path_)->description("SQLite database path");
    app_.add_option("--heartbeat-interval", heartbeat_interval_seconds_)
        ->description("Seconds between server Ping messages after auth");
    app_.add_option("--heartbeat-timeout", heartbeat_timeout_seconds_)
        ->description("Seconds to wait for Pong (or any inbound) after Ping");
}


void ServerCliApp::print_help(std::ostream& os) const
{
    os << std::format(
        "Usage: will-server [options]\n"
        "\n"
        "Options:\n"
        "  -h, --help                      Print usage and exit\n"
        "  --port PORT                     Listen port (default {})\n"
        "  --io-threads N                  io_context worker threads (default {})\n"
        "  --listen-backlog N              listen() backlog (default {})\n"
        "  --max-clients N                 Max concurrent connections (default {})\n"
        "  --db-path PATH                  SQLite database path (default {})\n"
        "  --heartbeat-interval SECONDS    Seconds between Ping after auth (default {})\n"
        "  --heartbeat-timeout SECONDS     Seconds to wait for Pong after Ping (default {})\n",
        ServerConfig::DefaultListenPort,
        ServerConfig::DefaultIoThreads,
        ServerConfig::DefaultListenBacklog,
        ServerConfig::DefaultMaxConnections,
        ServerConfig::DefaultDbPath,
        ServerConfig::DefaultHeartbeatIntervalSeconds,
        ServerConfig::DefaultHeartbeatTimeoutSeconds);
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
    config.io_threads = io_threads_;
    config.listen_backlog = listen_backlog_;
    config.max_connections = max_clients_;
    config.db_path = db_path_;
    config.heartbeat_interval_seconds = heartbeat_interval_seconds_;
    config.heartbeat_timeout_seconds = heartbeat_timeout_seconds_;
}


ServerConfig ServerCliApp::parse(int argc, char* argv[])
{
    ServerConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace will
