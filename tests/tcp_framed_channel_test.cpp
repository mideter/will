#include "tcpframe.h"
#include "tcpframedchannel.h"
#include "tcpstreamsocket.h"

#include <asio.hpp>

#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>


namespace {


using namespace std::chrono_literals;


void wait_for_flag(std::mutex& mutex, std::condition_variable& cv, const bool& flag)
{
    std::unique_lock lock(mutex);
    assert(cv.wait_for(lock, 2s, [&] { return flag; }));
}


std::pair<will::TcpStreamSocket, will::TcpStreamSocket> connect_loopback_pair(asio::io_context& ioc)
{
    asio::ip::tcp::acceptor acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
    const auto port = acceptor.local_endpoint().port();

    asio::ip::tcp::socket server_raw(ioc);
    asio::ip::tcp::socket client_raw(ioc);

    std::thread accept_thread([&] { acceptor.accept(server_raw); });

    client_raw.connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4("127.0.0.1"), port));
    accept_thread.join();

    return {will::TcpStreamSocket(std::move(server_raw)), will::TcpStreamSocket(std::move(client_raw))};
}


} // namespace


int main()
{
    asio::io_context ioc;

    auto [server_stream, client_stream] = connect_loopback_pair(ioc);

    will::TcpFramedChannel::Strand server_strand = asio::make_strand(ioc);
    will::TcpFramedChannel::Strand client_strand = asio::make_strand(ioc);

    will::TcpFramedChannel server_channel(server_stream, server_strand);
    will::TcpFramedChannel client_channel(client_stream, client_strand);

    std::mutex mutex;
    std::condition_variable cv;

    const std::vector<char> client_payload{'c', 'l', 'i', 'e', 'n', 't'};
    const std::vector<char> server_payload{'s', 'e', 'r', 'v', 'e', 'r'};
    const std::vector<char> wire_payload{'w', 'i', 'r', 'e'};

    bool server_received = false;
    bool client_received = false;
    std::vector<char> server_got;
    std::vector<char> client_got;

    server_channel.start(
        [&](std::vector<char> payload) {
            std::lock_guard lock(mutex);
            server_got = std::move(payload);
            server_received = true;
            cv.notify_one();
        },
        [] {});

    client_channel.start(
        [&](std::vector<char> payload) {
            std::lock_guard lock(mutex);
            client_got = std::move(payload);
            client_received = true;
            cv.notify_one();
        },
        [] {});

    std::jthread io_thread([&ioc] { ioc.run(); });

    client_channel.send_payload(client_payload);
    wait_for_flag(mutex, cv, server_received);
    assert(server_got == client_payload);

    server_channel.send_payload(server_payload);
    wait_for_flag(mutex, cv, client_received);
    assert(client_got == server_payload);

    client_received = false;
    const std::vector<char> wire_frame = will::TcpFrame::encode(wire_payload);
    server_channel.enqueue_wire_frame(std::vector<char>(wire_frame));
    wait_for_flag(mutex, cv, client_received);
    assert(client_got == wire_payload);

    client_channel.stop();
    server_channel.stop();
    client_stream.shutdown_and_close();
    server_stream.shutdown_and_close();
    ioc.stop();
    io_thread.join();

    return EXIT_SUCCESS;
}
