#pragma once

#include <asio.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <vector>

#include "clientaddress.h"


namespace will {


class SessionRegistry;
class TcpFrameReader;


class Session : public std::enable_shared_from_this<Session> {
public:
    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;

    std::uint64_t id() const noexcept { return id_; }
    const ClientAddress& address() const noexcept { return address_; }

    bool operator==(const Session& other) const noexcept { return id_ == other.id_; }
    bool operator!=(const Session& other) const noexcept { return !(*this == other); }

private:
    Session(asio::io_context& ioc, TcpSocket socket, ClientAddress address, SessionRegistry& registry,
            std::size_t max_outbound_queue_bytes);

    void begin();
    void shutdown();

    void on_frame(std::vector<char> payload);
    void on_read_error(const char* context, const asio::error_code& ec);
    void fail(const char* context, const asio::error_code& ec);
    void fail_protocol(const char* message);

    void enqueue_frame_bytes(std::vector<char> frame_bytes);
    void enqueue_payload_broadcast(const std::vector<char>& payload);

    friend class SessionRegistry;
    void pump_writes();
    void on_write(const asio::error_code& ec, std::size_t n);

    static std::vector<char> encode_frame(const std::vector<char>& payload);

    const std::uint64_t id_;
    SessionRegistry& registry_;
    const std::size_t max_outbound_queue_bytes_;

    TcpSocket socket_;
    Strand strand_;
    ClientAddress address_;

    std::shared_ptr<TcpFrameReader> frame_reader_;

    std::deque<std::vector<char>> write_queue_;
    std::size_t queued_bytes_ = 0;
    bool write_in_progress_ = false;
    bool closed_ = false;

    static std::atomic<std::uint64_t> next_id_;
};


} // namespace will
