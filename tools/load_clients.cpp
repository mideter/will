#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "clientconfigvalidator.h"
#include "loadclientsconfigparser.h"
#include "willmessage.h"
#include "willprotocol.h"


namespace {


int connect_tcp(const will::ClientConfig& connection)
{
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(connection.port);
    if (::inet_pton(AF_INET, connection.host.c_str(), &addr.sin_addr) != 1) {
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


void client_worker(const will::LoadClientsConfig& config, std::atomic<std::size_t>& connect_failures)
{
    const int fd = connect_tcp(config.connection);
    if (fd < 0) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    try {
        for (std::size_t i = 0; i < config.messages_per_client; ++i) {
            const std::string body = "load-" + std::to_string(i);
            send_frame(fd, will::WillMessage::encode_user_chat(body));
        }

        if (config.hold_seconds > 0)
            std::this_thread::sleep_for(std::chrono::seconds(config.hold_seconds));
    }
    catch (...) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
    }

    ::close(fd);
}


} // namespace


int main(int argc, char* argv[])
try {
    const will::cli::LoadClientsConfigParser cli(argc, argv);
    will::LoadClientsConfig config = cli.load_config();
    config.connection = will::ClientConfigValidator::accept(std::move(config.connection));

    std::atomic<std::size_t> failures{0};

    const auto started = std::chrono::steady_clock::now();

    std::vector<std::thread> threads;
    threads.reserve(config.clients);
    for (std::size_t i = 0; i < config.clients; ++i)
        threads.emplace_back(client_worker, std::cref(config), std::ref(failures));

    for (std::thread& t : threads)
        t.join();

    const auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() -
                                                              started);

    const std::size_t ok = config.clients - failures.load();
    std::cout << "load_clients: clients=" << config.clients << " ok=" << ok
              << " failures=" << failures.load() << " elapsed_ms=" << elapsed.count() << '\n';

    return failures.load() == 0 ? 0 : 1;
}
catch (const std::exception& e) {
    std::cerr << "load_clients error: " << e.what() << '\n';
    return 1;
}
