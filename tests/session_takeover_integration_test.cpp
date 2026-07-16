#include "wiremessage.h"
#include "wiremessage_codec.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"
#include "tcpframe.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>


namespace {


int connect_tcp(const char* host, std::uint16_t port)
{
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (::inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
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
        assert(n > 0);
        sent += static_cast<std::size_t>(n);
    }
}


void send_payload(int fd, const std::vector<char>& payload)
{
    const std::vector<char> wire_bytes = will::TcpFrame::encode(payload);
    send_all(fd, wire_bytes.data(), wire_bytes.size());
}


std::vector<char> read_payload(int fd)
{
    std::array<unsigned char, 4> header{};
    std::size_t got = 0;
    while (got < 4) {
        const ssize_t n = ::recv(fd, header.data() + got, 4 - got, 0);
        assert(n > 0);
        got += static_cast<std::size_t>(n);
    }

    const std::size_t plen = will::TcpFrame::read_u32_be(header);
    assert(plen > 0);

    std::vector<char> payload(plen);
    got = 0;
    while (got < plen) {
        const ssize_t n = ::recv(fd, payload.data() + got, plen - got, 0);
        assert(n > 0);
        got += static_cast<std::size_t>(n);
    }

    return payload;
}


void bind_device_token(int fd, const char* device_token)
{
    send_payload(fd, will::WireMessageCodec::encode(will::BindTokenMessage{device_token}));

    const auto auth_payload = read_payload(fd);
    const auto auth_message = will::WireMessageCodec::decode_server(auth_payload);
    assert(auth_message);
    assert(dynamic_cast<const will::AuthOkMessage*>(auth_message.get()) != nullptr);
}


void drain_receipt_ack(int fd)
{
    const auto payload = read_payload(fd);
    const auto message = will::WireMessageCodec::decode_server(payload);
    assert(message);
    assert(dynamic_cast<const will::ServerReceiptAckMessage*>(message.get()) != nullptr);
}


bool wait_for_eof(int fd, std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    const int flags = ::fcntl(fd, F_GETFL, 0);
    assert(flags >= 0);
    assert(::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);

    char buf[1];
    while (std::chrono::steady_clock::now() < deadline) {
        const ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n == 0) {
            ::fcntl(fd, F_SETFL, flags);
            return true;
        }
        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ::fcntl(fd, F_SETFL, flags);
        return false;
    }

    ::fcntl(fd, F_SETFL, flags);
    return false;
}


std::uint16_t pick_port()
{
    return static_cast<std::uint16_t>(21000 + (getpid() % 10000));
}


pid_t start_server(const char* server_exe, std::uint16_t port, const std::string& db_path)
{
    const pid_t pid = fork();
    if (pid != 0)
        return pid;

    const std::string port_str = std::to_string(port);
    execl(server_exe, server_exe, "--port", port_str.c_str(), "--io-threads", "1", "--db-path",
          db_path.c_str(), static_cast<char*>(nullptr));
    _exit(127);
}


void stop_server(pid_t pid)
{
    kill(pid, SIGTERM);
    int status = 0;
    waitpid(pid, &status, 0);
}


void wait_for_server(std::uint16_t port)
{
    for (int attempt = 0; attempt < 50; ++attempt) {
        const int fd = connect_tcp("127.0.0.1", port);
        if (fd >= 0) {
            ::close(fd);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    assert(false && "server did not start");
}


void print_usage(const char* program)
{
    std::cerr << "Usage: " << program << " <path-to-will-server>\n";
}


constexpr const char* DeviceToken = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";


} // namespace


int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* const server_exe = argv[1];
    const std::uint16_t port = pick_port();
    const std::string db_path = "/tmp/will-session-takeover-test-" + std::to_string(getpid()) + ".db";

    ::unlink(db_path.c_str());

    const pid_t server_pid = start_server(server_exe, port, db_path);
    assert(server_pid > 0);
    wait_for_server(port);

    const int first_fd = connect_tcp("127.0.0.1", port);
    assert(first_fd >= 0);
    bind_device_token(first_fd, DeviceToken);

    const int second_fd = connect_tcp("127.0.0.1", port);
    assert(second_fd >= 0);
    bind_device_token(second_fd, DeviceToken);

    assert(wait_for_eof(first_fd, std::chrono::seconds(2)));

    send_payload(second_fd, will::WireMessageCodec::encode(will::UserChatMessage{"after-takeover"}));
    drain_receipt_ack(second_fd);

    ::close(first_fd);
    ::close(second_fd);
    stop_server(server_pid);
    ::unlink(db_path.c_str());

    std::cout << "session takeover integration test passed\n";
    return EXIT_SUCCESS;
}
