#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <charconv>
#include <optional>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "willmessage.h"
#include "willprotocol.h"


namespace {


struct Options {
    std::string host = "127.0.0.1";
    std::uint16_t port = 7770;
    std::size_t clients = 100;
    std::size_t messages_per_client = 0;
    int hold_seconds = 30;
};


std::optional<std::size_t> parse_size(std::string_view text)
{
    std::size_t value = 0;
    const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (ec != std::errc{} || ptr != text.data() + text.size())
        return std::nullopt;
    return value;
}


void usage()
{
    std::cerr
        << "Usage: will-load-clients [options]\n"
        << "  --host HOST           Server host (default 127.0.0.1)\n"
        << "  --port PORT           Server port (default 7770)\n"
        << "  --clients N           Concurrent connections (default 100)\n"
        << "  --messages N          Chat messages per client (default 0 = idle only)\n"
        << "  --hold-seconds N      Keep connections open (default 30)\n";
}


Options parse_args(int argc, char* argv[])
{
    Options opts;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};
        auto need = [&](const char* flag) {
            if (i + 1 >= argc) {
                std::cerr << flag << " requires a value\n";
                usage();
                std::exit(2);
            }
            return std::string_view{argv[++i]};
        };

        if (arg == "--host")
            opts.host = std::string(need("--host"));
        else if (arg == "--port") {
            int port = 0;
            const auto text = need("--port");
            const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), port);
            if (ec != std::errc{} || port <= 0 || port > 65535) {
                std::cerr << "Invalid --port\n";
                std::exit(2);
            }
            opts.port = static_cast<std::uint16_t>(port);
        }
        else if (arg == "--clients") {
            const auto n = parse_size(need("--clients"));
            if (!n) {
                std::cerr << "Invalid --clients\n";
                std::exit(2);
            }
            opts.clients = *n;
        }
        else if (arg == "--messages") {
            const auto n = parse_size(need("--messages"));
            if (!n) {
                std::cerr << "Invalid --messages\n";
                std::exit(2);
            }
            opts.messages_per_client = *n;
        }
        else if (arg == "--hold-seconds") {
            const auto n = parse_size(need("--hold-seconds"));
            if (!n) {
                std::cerr << "Invalid --hold-seconds\n";
                std::exit(2);
            }
            opts.hold_seconds = static_cast<int>(*n);
        }
        else if (arg == "--help" || arg == "-h") {
            usage();
            std::exit(0);
        }
        else {
            std::cerr << "Unknown option: " << arg << '\n';
            usage();
            std::exit(2);
        }
    }
    return opts;
}


int connect_tcp(const Options& opts)
{
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(opts.port);
    if (::inet_pton(AF_INET, opts.host.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return -1;
    }

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return -1;
    }

    return fd;
}


void send_all(int fd, const char* data, std::size_t len)
{
    std::size_t sent = 0;
    while (sent < len) {
        const ssize_t n = ::send(fd, data + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0)
            throw std::runtime_error("send failed");
        sent += static_cast<std::size_t>(n);
    }
}


void send_frame(int fd, const std::vector<char>& payload)
{
    unsigned char header[4];
    will::TcpFrame::append_u32_be(header, payload.size());
    send_all(fd, reinterpret_cast<const char*>(header), 4);
    if (!payload.empty())
        send_all(fd, payload.data(), payload.size());
}


void client_worker(const Options& opts, std::atomic<std::size_t>& connect_failures)
{
    const int fd = connect_tcp(opts);
    if (fd < 0) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    try {
        for (std::size_t i = 0; i < opts.messages_per_client; ++i) {
            const std::string body = "load-" + std::to_string(i);
            send_frame(fd, will::WillMessage::encode_user_chat(body));
        }

        if (opts.hold_seconds > 0)
            std::this_thread::sleep_for(std::chrono::seconds(opts.hold_seconds));
    }
    catch (...) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
    }

    ::close(fd);
}


} // namespace


int main(int argc, char* argv[])
try {
    const Options opts = parse_args(argc, argv);
    std::atomic<std::size_t> failures{0};

    const auto started = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(opts.clients);
    for (std::size_t i = 0; i < opts.clients; ++i)
        threads.emplace_back(client_worker, std::cref(opts), std::ref(failures));

    for (std::thread& t : threads)
        t.join();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                              started);

    const std::size_t ok = opts.clients - failures.load();
    std::cout << "load_clients: clients=" << opts.clients << " ok=" << ok
              << " failures=" << failures.load() << " elapsed_ms=" << elapsed.count() << '\n';

    return failures.load() == 0 ? 0 : 1;
}
catch (const std::exception& e) {
    std::cerr << "load_clients error: " << e.what() << '\n';
    return 1;
}
