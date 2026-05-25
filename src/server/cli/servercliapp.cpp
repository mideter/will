#include "servercliapp.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {
namespace cli {


ServerCliApp::ServerCliApp(const ServerConfig& defaults)
    : app_{"will-server"}
    , port_(static_cast<int>(defaults.listen_port()))
    , io_threads_(defaults.io_threads())
    , listen_backlog_(defaults.listen_backlog())
    , max_clients_(defaults.max_connections())
    , max_outbound_queue_bytes_(defaults.max_outbound_queue_bytes())
{
    app_.allow_extras(false);

    app_.add_option("--port", port_)->description("Listen port");
    app_.add_option("--io-threads", io_threads_)->description("io_context worker threads");
    app_.add_option("--listen-backlog", listen_backlog_)->description("listen() backlog");
    app_.add_option("--max-clients", max_clients_)->description("Max concurrent connections");
    app_.add_option("--max-outbound-queue-bytes", max_outbound_queue_bytes_)
        ->description("Per-session write queue cap");
}


void ServerCliApp::print_help(std::ostream& os) const
{
    os << app_.help();
}


void ServerCliApp::exit_on_help(const CLI::CallForHelp& error) const
{
    std::exit(app_.exit(error, std::cerr));
}


void ServerCliApp::exit_on_parse_error(const CLI::ParseError& error) const
{
    std::exit(app_.exit(error, std::cerr));
}


void ServerCliApp::apply_to(ServerConfig& config) const
{
    try {
        config.set_listen_port(port_);
    } catch (const ServerConfigError& error) {
        throw std::runtime_error(std::format("Invalid --port: {}", error.what()));
    }

    try {
        config.set_io_threads(io_threads_);
    } catch (const ServerConfigError& error) {
        throw std::runtime_error(std::format("Invalid --io-threads: {}", error.what()));
    }

    try {
        config.set_listen_backlog(listen_backlog_);
    } catch (const ServerConfigError& error) {
        throw std::runtime_error(std::format("Invalid --listen-backlog: {}", error.what()));
    }

    try {
        config.set_max_connections(max_clients_);
    } catch (const ServerConfigError& error) {
        throw std::runtime_error(std::format("Invalid --max-clients: {}", error.what()));
    }

    try {
        config.set_max_outbound_queue_bytes(max_outbound_queue_bytes_);
    } catch (const ServerConfigError& error) {
        throw std::runtime_error(
            std::format("Invalid --max-outbound-queue-bytes: {}", error.what()));
    }
}


ServerConfig ServerCliApp::parse(int argc, char* argv[])
{
    ServerConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace cli
} // namespace will
