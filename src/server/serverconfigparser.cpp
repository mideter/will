#include "serverconfigparser.h"

#include <charconv>
#include <cstdlib>
#include <iostream>


namespace will {


ServerConfigParser::ServerConfigParser(int argc, char* argv[])
    : argc_(argc)
    , argv_(argv)
{
    parse_command_line();
}


void ServerConfigParser::print_usage()
{
    std::cerr
        << "Usage: will-server [options]\n"
        << "  --port PORT                 Listen port "
        << ServerConfig::MinListenPort << "-" << ServerConfig::MaxListenPort << " (default "
        << ServerConfig::DefaultListenPort << ")\n"
        << "  --io-threads N              io_context worker threads (default "
        << ServerConfig::DefaultIoThreads << ")\n"
        << "  --listen-backlog N          listen() backlog (default "
        << ServerConfig::DefaultListenBacklog << ")\n"
        << "  --max-clients N             Max concurrent connections (default "
        << ServerConfig::DefaultMaxConnections << ")\n"
        << "  --max-outbound-queue-bytes N  Per-session write queue cap (default "
        << ServerConfig::DefaultMaxOutboundQueueBytes << ")\n";
}


void ServerConfigParser::parse_command_line()
{
    for (index_ = 1; index_ < argc_; ++index_) {
        const std::string_view option_text{argv_[index_]};
        const Option option = classify_option(option_text);

        if (option == Option::Help) {
            print_usage();
            std::exit(0);
        }
        if (option == Option::Unknown) {
            std::cerr << "Unknown option: " << option_text << '\n';
            print_usage();
            std::exit(2);
        }

        apply_option(option);
    }
}


const char* ServerConfigParser::option_flag(Option option)
{
    switch (option) {
    case Option::Port:
        return "--port";
    case Option::IoThreads:
        return "--io-threads";
    case Option::ListenBacklog:
        return "--listen-backlog";
    case Option::MaxClients:
        return "--max-clients";
    case Option::MaxOutboundQueueBytes:
        return "--max-outbound-queue-bytes";
    case Option::Help:
    case Option::Unknown:
        break;
    }

    return "option";
}


std::string_view ServerConfigParser::need_value(Option option)
{
    const char* flag = option_flag(option);

    if (index_ + 1 >= argc_) {
        std::cerr << flag << " requires a value\n";
        print_usage();
        std::exit(2);
    }

    return std::string_view{argv_[++index_]};
}


std::optional<std::size_t> ServerConfigParser::parse_size(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


std::optional<int> ServerConfigParser::parse_int(std::string_view text)
{
    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);

    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;

    return value;
}


void ServerConfigParser::cli_fail_flag(Option option) const
{
    std::cerr << "Invalid " << option_flag(option) << '\n';
    std::exit(2);
}


void ServerConfigParser::cli_fail_option(Option option, const ServerConfigError& error) const
{
    std::cerr << "Invalid " << option_flag(option) << ": " << error.what() << '\n';
    std::exit(2);
}


int ServerConfigParser::require_int(Option option)
{
    const auto value = parse_int(need_value(option));

    if (!value)
        cli_fail_flag(option);

    return *value;
}


std::size_t ServerConfigParser::require_size(Option option)
{
    const auto value = parse_size(need_value(option));

    if (!value)
        cli_fail_flag(option);

    return *value;
}


ServerConfigParser::Option ServerConfigParser::classify_option(std::string_view option)
{
    if (option == "--port")
        return Option::Port;
    if (option == "--io-threads")
        return Option::IoThreads;
    if (option == "--listen-backlog")
        return Option::ListenBacklog;
    if (option == "--max-clients")
        return Option::MaxClients;
    if (option == "--max-outbound-queue-bytes")
        return Option::MaxOutboundQueueBytes;
    if (option == "--help" || option == "-h")
        return Option::Help;

    return Option::Unknown;
}


void ServerConfigParser::apply_option(Option option)
try {
    switch (option) {
    case Option::Port:
        config_.set_listen_port(require_int(option));
        break;
    case Option::IoThreads:
        config_.set_io_threads(require_int(option));
        break;
    case Option::ListenBacklog:
        config_.set_listen_backlog(require_int(option));
        break;
    case Option::MaxClients:
        config_.set_max_connections(require_size(option));
        break;
    case Option::MaxOutboundQueueBytes:
        config_.set_max_outbound_queue_bytes(require_size(option));
        break;
    case Option::Help:
    case Option::Unknown:
        break;
    }
}
catch (const ServerConfigError& error) {
    cli_fail_option(option, error);
}


} // namespace will
