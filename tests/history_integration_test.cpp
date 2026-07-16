#include "wiremessage.h"
#include "wiremessage_codec.h"
#include "wiremessage_client.h"
#include "wiremessage_server.h"
#include "wiremessage_user_chat.h"
#include "tcpframe.h"

#include <arpa/inet.h>
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


void drain_receipt_ack(int fd)
{
    const auto payload = read_payload(fd);
    const auto message = will::WireMessageCodec::decode_server(payload);
    assert(message);
    assert(dynamic_cast<const will::ServerReceiptAckMessage*>(message.get()) != nullptr);
}


constexpr const char* SenderToken = "11111111111111111111111111111111";
constexpr const char* ViewerToken = "22222222222222222222222222222222";


void bind_device_token(int fd, const char* device_token)
{
    send_payload(fd, will::WireMessageCodec::encode(will::BindTokenMessage{device_token}));

    const auto auth_payload = read_payload(fd);
    const auto auth_message = will::WireMessageCodec::decode_server(auth_payload);
    assert(auth_message);
    assert(dynamic_cast<const will::AuthOkMessage*>(auth_message.get()) != nullptr);
}


std::uint16_t pick_port()
{
    return static_cast<std::uint16_t>(20000 + (getpid() % 10000));
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


} // namespace


int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* const server_exe = argv[1];
    const std::uint16_t port = pick_port();
    const std::string db_path = "/tmp/will-history-test-" + std::to_string(getpid()) + ".db";

    ::unlink(db_path.c_str());

    const pid_t server_pid = start_server(server_exe, port, db_path);
    assert(server_pid > 0);
    wait_for_server(port);

    const int sender_fd = connect_tcp("127.0.0.1", port);
    assert(sender_fd >= 0);
    bind_device_token(sender_fd, SenderToken);
    send_payload(sender_fd, will::WireMessageCodec::encode(will::UserChatMessage{"hello-from-sender"}));
    drain_receipt_ack(sender_fd);

    const int viewer_fd = connect_tcp("127.0.0.1", port);
    assert(viewer_fd >= 0);
    bind_device_token(viewer_fd, ViewerToken);
    send_payload(viewer_fd, will::WireMessageCodec::encode(will::HistoryRequestMessage{10}));

    const auto first_item = read_payload(viewer_fd);
    const auto parsed_first_message = will::WireMessageCodec::decode_server(first_item);
    assert(parsed_first_message);
    const auto* parsed_first = dynamic_cast<const will::HistoryItemMessage*>(parsed_first_message.get());
    assert(parsed_first);
    assert(parsed_first->body() == "hello-from-sender");
    assert(!parsed_first->is_mine());

    const auto end = read_payload(viewer_fd);
    const auto end_message = will::WireMessageCodec::decode_server(end);
    assert(end_message);
    assert(dynamic_cast<const will::HistoryEndMessage*>(end_message.get()) != nullptr);

    send_payload(sender_fd, will::WireMessageCodec::encode(will::UserChatMessage{"hello-again"}));
    drain_receipt_ack(sender_fd);
    send_payload(sender_fd, will::WireMessageCodec::encode(will::HistoryRequestMessage{10}));

    const auto own_item = read_payload(sender_fd);
    const auto parsed_own_message = will::WireMessageCodec::decode_server(own_item);
    assert(parsed_own_message);
    const auto* parsed_own = dynamic_cast<const will::HistoryItemMessage*>(parsed_own_message.get());
    assert(parsed_own);
    assert(parsed_own->body() == "hello-from-sender");
    assert(parsed_own->is_mine());

    const auto own_second = read_payload(sender_fd);
    const auto parsed_second_message = will::WireMessageCodec::decode_server(own_second);
    assert(parsed_second_message);
    const auto* parsed_second = dynamic_cast<const will::HistoryItemMessage*>(parsed_second_message.get());
    assert(parsed_second);
    assert(parsed_second->body() == "hello-again");
    assert(parsed_second->is_mine());

    const auto sender_end = read_payload(sender_fd);
    const auto sender_end_message = will::WireMessageCodec::decode_server(sender_end);
    assert(sender_end_message);
    assert(dynamic_cast<const will::HistoryEndMessage*>(sender_end_message.get()) != nullptr);

    ::close(sender_fd);
    ::close(viewer_fd);
    stop_server(server_pid);
    ::unlink(db_path.c_str());

    std::cout << "history integration test passed\n";
    return EXIT_SUCCESS;
}
