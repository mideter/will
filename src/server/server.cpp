#include <charconv>
#include <iostream>
#include <optional>
#include <string_view>

#include "messengerserver.h"
#include "serverconfig.h"


namespace {


std::optional<std::size_t> parse_size(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;
    return value;
}


std::optional<int> parse_int(std::string_view text)
{
    int value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;
    return value;
}


void print_usage()
{
    std::cerr
        << "Usage: will-server [options]\n"
        << "  --port PORT                 Listen port (default "
        << will::ServerConfig::DefaultListenPort << ")\n"
        << "  --io-threads N              io_context worker threads (default "
        << will::ServerConfig::DefaultIoThreads << ")\n"
        << "  --listen-backlog N          listen() backlog (default "
        << will::ServerConfig::DefaultListenBacklog << ")\n"
        << "  --max-clients N             Max concurrent connections (default "
        << will::ServerConfig::DefaultMaxConnections << ")\n"
        << "  --max-outbound-queue-bytes N  Per-session write queue cap (default "
        << will::ServerConfig::DefaultMaxOutboundQueueBytes << ")\n";
}


will::ServerConfig parse_args(int argc, char* argv[])
{
    will::ServerConfig config;

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

        if (arg == "--port") {
            const auto port = parse_int(need_value("--port"));
            if (!port || *port <= 0 || *port > 65535) {
                std::cerr << "Invalid --port\n";
                std::exit(2);
            }
            config.listen_port = static_cast<std::uint16_t>(*port);
        }
        else if (arg == "--io-threads") {
            const auto n = parse_int(need_value("--io-threads"));
            if (!n || *n < 1) {
                std::cerr << "Invalid --io-threads\n";
                std::exit(2);
            }
            config.io_threads = *n;
        }
        else if (arg == "--listen-backlog") {
            const auto n = parse_int(need_value("--listen-backlog"));
            if (!n || *n < 1) {
                std::cerr << "Invalid --listen-backlog\n";
                std::exit(2);
            }
            config.listen_backlog = *n;
        }
        else if (arg == "--max-clients") {
            const auto n = parse_size(need_value("--max-clients"));
            if (!n || *n < 1) {
                std::cerr << "Invalid --max-clients\n";
                std::exit(2);
            }
            config.max_connections = *n;
        }
        else if (arg == "--max-outbound-queue-bytes") {
            const auto n = parse_size(need_value("--max-outbound-queue-bytes"));
            if (!n || *n < 1) {
                std::cerr << "Invalid --max-outbound-queue-bytes\n";
                std::exit(2);
            }
            config.max_outbound_queue_bytes = *n;
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

    return config;
}


} // namespace


int main(int argc, char* argv[])
try {
    constexpr const char* version = "4.0.0";
    const will::ServerConfig config = parse_args(argc, argv);

    std::cout << "Starting Will Messenger Server v" << version << " on port " << config.listen_port
              << " (max " << config.max_connections << " clients, " << config.io_threads
              << " io threads)\n";

    will::MessengerServer server(config);
    server.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << '\n';
    return 1;
}
