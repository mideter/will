#include "servercliapp.h"

#include "serverconfig.h"

#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>


namespace will {
namespace cli {


namespace {


template<typename Fn>
void apply_cli_field(std::string_view flag, Fn&& apply)
{
    try {
        apply();
    } catch (const ServerConfigError& error) {
        throw CLI::ValidationError(std::format("Invalid {}: {}", flag, error.what()), 2);
    }
}


} // namespace


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
    os << std::format(
        "Usage: will-server [options]\n"
        "\n"
        "Options:\n"
        "  -h, --help                      Print usage and exit\n"
        "  --port PORT                     Listen port (default {})\n"
        "  --io-threads N                  io_context worker threads (default {})\n"
        "  --listen-backlog N              listen() backlog (default {})\n"
        "  --max-clients N                 Max concurrent connections (default {})\n"
        "  --max-outbound-queue-bytes N    Per-session write queue cap (default {})\n",
        ServerConfig::DefaultListenPort,
        ServerConfig::DefaultIoThreads,
        ServerConfig::DefaultListenBacklog,
        ServerConfig::DefaultMaxConnections,
        ServerConfig::DefaultMaxOutboundQueueBytes);
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
    apply_cli_field("--port", [&] { config.set_listen_port(port_); });
    apply_cli_field("--io-threads", [&] { config.set_io_threads(io_threads_); });
    apply_cli_field("--listen-backlog", [&] { config.set_listen_backlog(listen_backlog_); });
    apply_cli_field("--max-clients", [&] { config.set_max_connections(max_clients_); });
    apply_cli_field("--max-outbound-queue-bytes",
                    [&] { config.set_max_outbound_queue_bytes(max_outbound_queue_bytes_); });
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
