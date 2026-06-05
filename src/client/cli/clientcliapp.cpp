#include "clientcliapp.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {


ClientCliApp::ClientCliApp(const ClientConfig& defaults)
    : app_{"will-client"}
    , host_(defaults.host)
    , port_(static_cast<int>(defaults.port))
    , login_(defaults.login)
    , password_(defaults.password)
    , quiet_receipts_(defaults.quiet_receipts)
    , history_limit_(defaults.history_limit)
{
    app_.allow_extras(false);

    app_.add_option("--host", host_)->description("Server IPv4 address");
    app_.add_option("--port", port_)->description("Server TCP port");
    app_.add_option("--user", login_)->description("Login for server authentication");
    app_.add_option("--password", password_)->description("Password for server authentication");
    app_.add_flag("--quiet", quiet_receipts_)->description("Suppress server receipt messages on stderr");
    app_.add_option("--history", history_limit_)
        ->description("Request last N messages on connect");
    app_.add_flag_callback("--no-history", [this]() { history_limit_ = std::nullopt; })
        ->description("Do not request chat history on connect");
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
        "  --user LOGIN                    Login (default {})\n"
        "  --password PASSWORD             Password (default: dev seed)\n"
        "  --quiet                         Suppress server receipt messages on stderr\n"
        "  --history N                     Request last N messages on connect (default {})\n"
        "  --no-history                    Do not request chat history on connect\n",
        ClientConfig::DefaultHost,
        ClientConfig::NovosibirskHost,
        ClientConfig::DefaultPort,
        ClientConfig::DefaultLogin,
        ClientConfig::DefaultHistoryLimit);
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
    config.host = host_;
    config.port = static_cast<std::uint16_t>(port_);
    config.login = login_;
    config.password = password_;
    config.quiet_receipts = quiet_receipts_;
    config.history_limit = history_limit_;
}


ClientConfig ClientCliApp::parse(int argc, char* argv[])
{
    ClientConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace will
