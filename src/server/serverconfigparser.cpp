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
        << "  --port PORT                 Listen port (default "
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
        const std::string_view option{argv_[index_]};

        try {
            apply_option(option);
        }
        catch (const ServerConfigError& error) {
            cli_fail_option(option, error);
        }
    }
}


std::string_view ServerConfigParser::need_value(const char* flag)
{
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


void ServerConfigParser::cli_fail(const char* message) const
{
    std::cerr << message << std::endl;
    std::exit(2);
}


void ServerConfigParser::cli_fail_option(std::string_view option, const ServerConfigError& error) const
{
    std::cerr << "Invalid " << option << ": " << error.what() << '\n';
    std::exit(2);
}


void ServerConfigParser::apply_port()
{
    const auto port = parse_int(need_value("--port"));

    if (!port || *port <= 0 || *port > 65535)
        cli_fail("Invalid --port");

    config_.set_listen_port(static_cast<std::uint16_t>(*port));
}


void ServerConfigParser::apply_io_threads()
{
    const auto n = parse_int(need_value("--io-threads"));

    if (!n)
        cli_fail("Invalid --io-threads");

    config_.set_io_threads(*n);
}


void ServerConfigParser::apply_listen_backlog()
{
    const auto n = parse_int(need_value("--listen-backlog"));

    if (!n)
        cli_fail("Invalid --listen-backlog");

    config_.set_listen_backlog(*n);
}


void ServerConfigParser::apply_max_clients()
{
    const auto n = parse_size(need_value("--max-clients"));

    if (!n)
        cli_fail("Invalid --max-clients");

    config_.set_max_connections(*n);
}


void ServerConfigParser::apply_max_outbound_queue_bytes()
{
    const auto n = parse_size(need_value("--max-outbound-queue-bytes"));

    if (!n)
        cli_fail("Invalid --max-outbound-queue-bytes");

    config_.set_max_outbound_queue_bytes(*n);
}


void ServerConfigParser::apply_option(std::string_view option)
{
    if (option == "--port") {
        apply_port();
        return;
    }
    if (option == "--io-threads") {
        apply_io_threads();
        return;
    }
    if (option == "--listen-backlog") {
        apply_listen_backlog();
        return;
    }
    if (option == "--max-clients") {
        apply_max_clients();
        return;
    }
    if (option == "--max-outbound-queue-bytes") {
        apply_max_outbound_queue_bytes();
        return;
    }
    if (option == "--help" || option == "-h") {
        print_usage();
        std::exit(0);
    }

    std::cerr << "Unknown option: " << option << '\n';
    print_usage();
    std::exit(2);
}


} // namespace will
