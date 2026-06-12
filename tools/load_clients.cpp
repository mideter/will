#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "clientconfigvalidator.h"
#include "loadclientsconfigparser.h"
#include "wiremessage.h"
#include "wiremessage_codec.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"
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


void send_payload(int fd, const std::vector<char>& payload)
{
    const auto header = will::TcpFrame::u32_be(static_cast<std::uint32_t>(payload.size()));
    send_all(fd, reinterpret_cast<const char*>(header.data()), header.size());
    if (!payload.empty())
        send_all(fd, payload.data(), payload.size());
}


std::vector<char> read_payload(int fd)
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


void otp_and_bind(int fd, const will::ClientConfig& connection)
{
    if (connection.otp.empty())
        throw std::runtime_error("otp required for load_clients (pass --otp)");

    send_payload(fd, will::WireMessageCodec::encode(will::OtpPhoneRequestMessage{connection.phone}));

    const auto otp_sent_payload = read_payload(fd);
    const auto otp_sent_message = will::WireMessageCodec::decode_server(otp_sent_payload);
    if (const auto* failure = dynamic_cast<const will::OtpVerifyResponseMessage*>(otp_sent_message.get())) {
        if (!failure->success())
            throw std::runtime_error("OTP request failed");
    } else if (dynamic_cast<const will::OtpSentMessage*>(otp_sent_message.get()) == nullptr) {
        throw std::runtime_error("OTP request failed");
    }

    send_payload(fd, will::WireMessageCodec::encode(will::OtpCodeSubmitMessage{connection.otp}));

    const auto verify_payload = read_payload(fd);
    const auto verify_message = will::WireMessageCodec::decode_server(verify_payload);
    const auto* parsed = dynamic_cast<const will::OtpVerifyResponseMessage*>(verify_message.get());
    if (!parsed || !parsed->success())
        throw std::runtime_error("OTP verification failed");

    send_payload(fd, will::WireMessageCodec::encode(will::BindTokenMessage{parsed->token()}));
}


will::ClientConfig connection_for_client(const will::LoadClientsConfig& config, const std::size_t client_index)
{
    will::ClientConfig connection = config.connection;
    if (config.clients > 1)
        connection.phone = std::format("+1555{:07d}", client_index + 1);
    return connection;
}


void client_worker(const will::LoadClientsConfig& config, const std::size_t client_index,
                   std::atomic<std::size_t>& connect_failures)
{
    const will::ClientConfig connection = connection_for_client(config, client_index);
    const int fd = connect_tcp(connection);
    if (fd < 0) {
        connect_failures.fetch_add(1, std::memory_order_relaxed);
        return;
    }

    try {
        otp_and_bind(fd, connection);

        for (std::size_t i = 0; i < config.messages_per_client; ++i) {
            const std::string body = "load-" + std::to_string(i);
            send_payload(fd, will::WireMessageCodec::encode(will::UserChatMessage{body}));
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
        threads.emplace_back(client_worker, std::cref(config), i, std::ref(failures));

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
