#include "serverconfigparser.h"

#include "serverconfigerror.h"

#include <charconv>
#include <cstdlib>
#include <iostream>


namespace will {


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


ServerConfig ServerConfigParser::parse(int argc, char* argv[])
{
    ServerConfig config;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};

        auto need_value = [&](const char* flag) -> std::string_view {
            if (i + 1 >= argc) {
                std::cerr << flag << " requires a value\n";
                print_usage();
                std::exit(2);
            }
            return std::string_view{argv[++i]};
        };

        auto fail = [](const char* message) {
            std::cerr << message << '\n';
            std::exit(2);
        };

        try {
            if (arg == "--port") {
                const auto port = parse_int(need_value("--port"));
                
                if (!port || *port <= 0 || *port > 65535)
                    fail("Invalid --port");

                config.set_listen_port(static_cast<std::uint16_t>(*port));
            }
            else if (arg == "--io-threads") {
                const auto n = parse_int(need_value("--io-threads"));
                
                if (!n)
                    fail("Invalid --io-threads");

                config.set_io_threads(*n);
            }
            else if (arg == "--listen-backlog") {
                const auto n = parse_int(need_value("--listen-backlog"));

                if (!n)
                    fail("Invalid --listen-backlog");
                
                config.set_listen_backlog(*n);
            }
            else if (arg == "--max-clients") {
                const auto n = parse_size(need_value("--max-clients"));

                if (!n)
                    fail("Invalid --max-clients");
                
                config.set_max_connections(*n);
            }
            else if (arg == "--max-outbound-queue-bytes") {
                const auto n = parse_size(need_value("--max-outbound-queue-bytes"));

                if (!n)
                    fail("Invalid --max-outbound-queue-bytes");
                
                config.set_max_outbound_queue_bytes(*n);
            }
            else if (arg == "--help" || arg == "-h") {
                print_usage();
                std::exit(0);
            }
            else {
                std::cerr << "Unknown option: " << arg << '\n';
                print_usage();
                std::exit(2);
            }
        }
        catch (const ServerConfigError& e) {
            std::cerr << "Invalid " << arg << ": " << e.what() << '\n';
            std::exit(2);
        }
    }

    return config;
}


} // namespace will
