#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "clientconfigvalidator.h"
#include "loadclientsconfigparser.h"
#include "wiremessage.h"
#include "tcpframe.h"


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
    const auto header = will::TcpFrame::u32_be(static_cast<std::uint32_t>(payload.size()));
    send_all(fd, reinterpret_cast<const char*>(header.data()), header.size());
    if (!payload.empty())
        send_all(fd, payload.data(), payload.size());
}


std::vector<char> read_frame(int fd)
{
    std::array<unsigned char, 4> header{};
    std::size_t got = 0;
    while (got < 4) {
        const ssize_t n = ::recv(fd, header.data() + got, 4 - got, 0);
        if (n <= 0)
            throw std::runtime_error("read frame header failed");
        got += static_cast<std::size_t>(n);
    }

    const std::size_t plen = will::TcpFrame::read_u32_be(header);
    if (plen == 0)
        throw std::runtime_error("empty payload");

    std::vector<char> payload(plen);
    got = 0;
    while (got < plen) {
        const ssize_t n = ::recv(fd, payload.data() + got, plen - got, 0);
        if (n <= 0)
            throw std::runtime_error("read frame payload failed");
        got += static_cast<std::size_t>(n);
    }

    return payload;
}


void login_and_bind(int fd, const will::ClientConfig& connection)
{
    send_frame(fd, will::encode(will::LoginRequestMessage{connection.login, connection.password}));

    const auto response = read_frame(fd);
    const auto message = will::decode_server_message(response);
    if (!message)
        throw std::runtime_error("login failed");

    const auto* parsed = dynamic_cast<const will::LoginResponseMessage*>(message.get());
    if (!parsed || !parsed->success())
        throw std::runtime_error("login failed");

    send_frame(fd, will::encode(will::BindTokenMessage{parsed->token()}));
}


void client_worker(const will::LoadClientsConfig& config, std::atomic<std::size_t>& connect_failures)
{
    const int fd = connect_tcp(config.connection);
    if (fd < 0) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    try {
        login_and_bind(fd, config.connection);

        for (std::size_t i = 0; i < config.messages_per_client; ++i) {
            const std::string body = "load-" + std::to_string(i);
            send_frame(fd, will::encode(will::UserChatMessage{body}));
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
    const will::LoadClientsConfigParser cli(argc, argv);
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
