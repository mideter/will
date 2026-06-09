#pragma once

#include <asio.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>


namespace will {


class TcpFrameReader;
class TcpFrameWriter;


struct TcpConnectionHandlers {
    std::function<void(std::uint64_t connection_id, std::vector<char> payload)> on_frame;
    std::function<void(std::uint64_t connection_id)> on_closed;
};


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;

    std::uint64_t id() const noexcept { return id_; }
    std::string_view peer_label() const noexcept { return peer_label_; }

    void send_frame(std::vector<char> wire_bytes);
    void enqueue_frame(std::vector<char> wire_bytes);

private:
    TcpConnection(asio::io_context& ioc, TcpSocket socket, asio::ip::tcp::endpoint peer_endpoint,
                  TcpConnectionHandlers handlers);

    void begin(std::size_t max_outbound_queue_bytes);
    void shutdown();

    void handle_frame(std::vector<char> payload);
    void handle_read_error(std::string_view context, const asio::error_code& ec);
    void handle_write_queue_full();
    void handle_write_error(std::string_view context, const asio::error_code& ec);
    void handle_framing_error(std::string_view message);
    void fail(std::string_view context, const asio::error_code& ec);
    void request_close();

    const std::uint64_t id_;
    TcpConnectionHandlers handlers_;

    TcpSocket socket_;
    Strand strand_;

    std::string peer_label_;

    std::shared_ptr<TcpFrameReader> frame_reader_;
    std::shared_ptr<TcpFrameWriter> frame_writer_;

    bool closed_ = false;
    bool shutdown_done_ = false;

    static std::atomic<std::uint64_t> next_id_;

    friend class TcpConnectionRegistry;
};


} // namespace will
