#include "clientcliapp.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {


ClientCliApp::ClientCliApp()
    : app_{"will-client"}
{
    app_.allow_extras(false);

    app_.add_option("--host", config_.host)->description("Server IPv4 address");
    app_.add_option("--port", config_.port)->description("Server TCP port");
    app_.add_option("--user", config_.login)->description("Login for server authentication");
    app_.add_option("--password", config_.password)->description("Password for server authentication");
    app_.add_flag("--quiet", config_.quiet_receipts)
        ->description("Suppress server receipt messages on stderr");
    app_.add_option("--history", config_.history_limit)
        ->description("Request last N messages on connect");
    app_.add_flag_callback("--no-history", [this]() { config_.history_limit = 0; })
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


ClientConfig ClientCliApp::parse(int argc, char* argv[])
{
    app_.parse(argc, argv);
    return config_;
}


} // namespace will
