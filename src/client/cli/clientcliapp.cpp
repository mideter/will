#include "clientcliapp.h"

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
    } catch (const ClientConfigError& error) {
        throw CLI::ValidationError(std::format("Invalid {}: {}", flag, error.what()), 2);
    }
}


} // namespace


ClientCliApp::ClientCliApp(const ClientConfig& defaults)
    : app_{"will-client"}
    , host_(defaults.host())
    , port_(static_cast<int>(defaults.port()))
    , quiet_receipts_(defaults.quiet_receipts())
{
    app_.allow_extras(false);

    app_.add_option("--host", host_)->description("Server IPv4 address");
    app_.add_option("--port", port_)->description("Server TCP port");
    app_.add_flag("--quiet", quiet_receipts_)->description("Suppress server receipt messages on stderr");
}


void ClientCliApp::print_help(std::ostream& os) const
{
    os << std::format(
        "Usage: will-client [options]\n"
        "\n"
        "Options:\n"
        "  -h, --help                      Print usage and exit\n"
        "  --host HOST                     Server IPv4 address (default {}; Novosibirsk: {})\n"
        "  --port PORT                     Server TCP port (default {})\n"
        "  --quiet                         Suppress server receipt messages on stderr\n",
        ClientConfig::DefaultHost,
        ClientConfig::NovosibirskHost,
        ClientConfig::DefaultPort);
}


void ClientCliApp::exit_on_help(const CLI::CallForHelp& error) const
{
    print_help(std::cerr);
    std::exit(error.get_exit_code());
}


void ClientCliApp::exit_on_parse_error(const CLI::ParseError& error) const
{
    std::exit(app_.exit(error, std::cerr));
}


void ClientCliApp::apply_to(ClientConfig& config) const
{
    apply_cli_field("--host", [&] { config.set_host(host_); });
    apply_cli_field("--port", [&] { config.set_port(port_); });
    apply_cli_field("--quiet", [&] { config.set_quiet_receipts(quiet_receipts_); });
}


ClientConfig ClientCliApp::parse(int argc, char* argv[])
{
    ClientConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace cli
} // namespace will
