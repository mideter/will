#pragma once

#include "tcpframedchannel.h"
#include "tcpstreamsocket.h"

#include <asio.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace will {


struct TcpConnectionHandlers {
    std::function<void(std::uint64_t connection_id, std::vector<char> payload)> on_payload;
    std::function<void(std::uint64_t connection_id)> on_closed;
};


struct HeartbeatSettings {
    std::chrono::milliseconds interval{std::chrono::seconds{30}};
    std::chrono::milliseconds timeout{std::chrono::seconds{10}};
};


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using Strand = asio::strand<asio::io_context::executor_type>;

    std::uint64_t id() const noexcept { return id_; }
    std::string_view peer_address() const noexcept { return peer_address_; }

    void enqueue_wire_frame(std::vector<char> wire_bytes);

    void schedule_on_strand(std::function<void(asio::any_io_executor)> fn);

    void start_heartbeat();

private:
    TcpConnection(asio::io_context& ioc, TcpStreamSocket socket, asio::ip::tcp::endpoint peer_endpoint,
                  TcpConnectionHandlers handlers, HeartbeatSettings heartbeat);

    void begin();
    void shutdown();

    void handle_payload(std::vector<char> payload);
    void request_close();

    void note_inbound_activity();
    void schedule_heartbeat_interval();
    void on_heartbeat_interval(const asio::error_code& ec);
    void on_heartbeat_timeout(const asio::error_code& ec);
    void cancel_heartbeat();

    const std::uint64_t id_;
    TcpConnectionHandlers handlers_;
    HeartbeatSettings heartbeat_;

    TcpStreamSocket socket_;
    Strand strand_;
    asio::steady_timer heartbeat_timer_;

    std::string peer_address_;

    std::shared_ptr<TcpFramedChannel> channel_;

    bool closed_ = false;
    bool shutdown_done_ = false;
    bool heartbeat_started_ = false;
    bool awaiting_pong_ = false;

    static std::atomic<std::uint64_t> next_id_;

    friend class TcpConnectionRegistry;
};


} // namespace will
