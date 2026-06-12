#pragma once

#include <asio.hpp>

#include <cstddef>
#include <deque>
#include <functional>
#include <string_view>
#include <vector>


namespace will {


/** Async outbound TCP frame writer: bounded queue + async_write on a strand. */
class TcpFrameWriter : public std::enable_shared_from_this<TcpFrameWriter> {
public:
    static constexpr std::size_t MaxQueuedBytes = 1u << 20; // 1 MiB per session

    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;
    using QueueFullHandler = std::function<void()>;
    using ProtocolErrorHandler = std::function<void(std::string_view message)>;
    using WriteErrorHandler =
        std::function<void(std::string_view context, const asio::error_code& ec)>;

    TcpFrameWriter(TcpSocket& socket, Strand& strand, QueueFullHandler on_queue_full,
                   ProtocolErrorHandler on_protocol_error, WriteErrorHandler on_write_error);

    /** Must be called on {@code strand_}. */
    void enqueue(std::vector<char> frame_bytes);
    void stop();

private:
    void pump_writes();
    void handle_write(const asio::error_code& ec, std::size_t n);

    TcpSocket& socket_;
    Strand& strand_;
    QueueFullHandler on_queue_full_;
    ProtocolErrorHandler on_protocol_error_;
    WriteErrorHandler on_write_error_;

    bool stopped_ = false;

    std::deque<std::vector<char>> write_queue_;
    std::size_t queued_bytes_ = 0;
    bool write_in_progress_ = false;
};


} // namespace will
