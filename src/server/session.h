#pragma once

#include <asio.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace will {


class ChatService;
class SessionRegistry;
class TcpFrameReader;


class Session : public std::enable_shared_from_this<Session> {
public:
    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;

    std::uint64_t id() const noexcept { return id_; }
    std::string_view peer_label() const noexcept { return peer_label_; }
    std::string_view peer_ip() const noexcept { return peer_ip_; }

    bool operator==(const Session& other) const noexcept { return id_ == other.id_; }
    bool operator!=(const Session& other) const noexcept { return !(*this == other); }

    void send_will_payload(const std::vector<char>& payload);
    void fail_protocol(const char* message);

private:
    Session(asio::io_context& ioc, TcpSocket socket, asio::ip::tcp::endpoint peer_endpoint,
            SessionRegistry& registry, ChatService& chat_service,
            std::size_t max_outbound_queue_bytes);

    void begin();
    void shutdown();

    void on_frame(std::vector<char> payload);
    void on_read_error(const char* context, const asio::error_code& ec);
    void fail(const char* context, const asio::error_code& ec);

    void enqueue_frame_bytes(std::vector<char> frame_bytes);
    void enqueue_payload_broadcast(const std::vector<char>& payload);

    void pump_writes();
    void on_write(const asio::error_code& ec, std::size_t n);

    const std::uint64_t id_;
    SessionRegistry& registry_;
    ChatService& chat_service_;
    const std::size_t max_outbound_queue_bytes_;

    TcpSocket socket_;
    Strand strand_;
    std::string peer_ip_;
    std::string peer_label_;

    std::shared_ptr<TcpFrameReader> frame_reader_;

    std::deque<std::vector<char>> write_queue_;
    std::size_t queued_bytes_ = 0;
    bool write_in_progress_ = false;
    bool closed_ = false;

    static std::atomic<std::uint64_t> next_id_;

    friend class SessionRegistry;
};


} // namespace will
