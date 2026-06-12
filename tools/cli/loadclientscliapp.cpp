#include "loadclientscliapp.h"

#include <cstdlib>
#include <format>
#include <iostream>


namespace will {


LoadClientsCliApp::LoadClientsCliApp(const LoadClientsConfig& defaults)
    : app_{"will-load-clients"}
    , host_(defaults.connection.host)
    , port_(static_cast<int>(defaults.connection.port))
    , phone_(defaults.connection.phone)
    , otp_(defaults.connection.otp)
    , clients_(defaults.clients)
    , messages_per_client_(defaults.messages_per_client)
    , hold_seconds_(defaults.hold_seconds)
{
    app_.allow_extras(false);

    app_.add_option("--host", host_)->description("Server IPv4 address");
    app_.add_option("--port", port_)->description("Server TCP port");
    app_.add_option("--phone", phone_)->description("Phone number in E.164 for OTP authentication");
    app_.add_option("--otp", otp_)->description("OTP code (required for non-interactive load test)");
    app_.add_option("--clients", clients_)->description("Concurrent connections");
    app_.add_option("--messages", messages_per_client_)->description("Chat messages per client (0 = idle only)");
    app_.add_option("--hold-seconds", hold_seconds_)->description("Keep connections open (seconds)");
}


void LoadClientsCliApp::print_help(std::ostream& os) const
{
    os << std::format(
        "Usage: will-load-clients [options]\n"
        "\n"
        "Options:\n"
        "  -h, --help                      Print usage and exit\n"
        "  --host HOST                     Server IPv4 address (default {})\n"
        "  --port PORT                     Server TCP port (default {})\n"
        "  --phone PHONE                   E.164 phone for OTP auth (default {})\n"
        "  --otp CODE                      OTP code (required for non-interactive runs)\n"
        "  --clients N                     Concurrent connections (default {})\n"
        "  --messages N                    Chat messages per client (default {} = idle only)\n"
        "  --hold-seconds N                Keep connections open (default {})\n",
        ClientConfig::DefaultHost,
        ClientConfig::DefaultPort,
        ClientConfig::DefaultPhone,
        LoadClientsConfig::DefaultClients,
        LoadClientsConfig::DefaultMessagesPerClient,
        LoadClientsConfig::DefaultHoldSeconds);
}


void LoadClientsCliApp::exit_on_help(const CLI::CallForHelp& error) const
{
    print_help(std::cerr);
    std::exit(error.get_exit_code());
}


void LoadClientsCliApp::exit_on_parse_error(const CLI::ParseError& error) const
{
    std::exit(app_.exit(error, std::cerr));
}


void LoadClientsCliApp::apply_to(LoadClientsConfig& config) const
{
    config.connection.host = host_;
    config.connection.port = static_cast<std::uint16_t>(port_);
    config.connection.phone = phone_;
    config.connection.otp = otp_;
    config.clients = clients_;
    config.messages_per_client = messages_per_client_;
    config.hold_seconds = hold_seconds_;
}


LoadClientsConfig LoadClientsCliApp::parse(int argc, char* argv[])
{
    LoadClientsConfig config;
    app_.parse(argc, argv);
    apply_to(config);
    return config;
}


} // namespace will
