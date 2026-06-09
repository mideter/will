#include "tcpconnection.h"

#include "tcpframereader.h"
#include "tcpframewriter.h"

#include <format>
#include <iostream>
#include <utility>


namespace will {


std::atomic<std::uint64_t> TcpConnection::next_id_{1};


TcpConnection::TcpConnection(asio::io_context& ioc, TcpSocket socket, asio::ip::tcp::endpoint peer_endpoint,
                             TcpConnectionHandlers handlers)
    : id_(next_id_.fetch_add(1, std::memory_order_relaxed))
    , handlers_(std::move(handlers))
    , socket_(std::move(socket))
    , strand_(asio::make_strand(ioc))
    , peer_label_(std::format("{}:{}", peer_endpoint.address().to_string(), peer_endpoint.port()))
{}


void TcpConnection::begin(const std::size_t max_outbound_queue_bytes)
{
    frame_writer_ = std::make_shared<TcpFrameWriter>(
        socket_, strand_, max_outbound_queue_bytes,
        [self = shared_from_this()] { self->handle_write_queue_full(); },
        [self = shared_from_this()](const std::string_view message) { self->handle_framing_error(message); },
        [self = shared_from_this()](const std::string_view context, const asio::error_code& ec) {
            self->handle_write_error(context, ec);
        });

    frame_reader_ = std::make_shared<TcpFrameReader>(
        socket_, strand_,
        [self = shared_from_this()](std::vector<char> payload) { self->handle_frame(std::move(payload)); },
        [self = shared_from_this()](const std::string_view message) { self->handle_framing_error(message); },
        [self = shared_from_this()](const std::string_view context, const asio::error_code& ec) {
            self->handle_read_error(context, ec);
        });

    frame_reader_->start();
}


void TcpConnection::shutdown()
{
    asio::post(strand_, [self = shared_from_this()] {
        if (self->shutdown_done_)
            return;

        self->shutdown_done_ = true;

        if (self->frame_reader_)
            self->frame_reader_->stop();

        if (self->frame_writer_)
            self->frame_writer_->stop();

        asio::error_code ignored;
        self->socket_.shutdown(TcpSocket::shutdown_both, ignored);
        self->socket_.close(ignored);
    });
}


void TcpConnection::handle_frame(std::vector<char> payload)
{
    if (closed_)
        return;

    if (handlers_.on_frame)
        handlers_.on_frame(id_, std::move(payload));
}


void TcpConnection::send_frame(std::vector<char> wire_bytes)
{
    if (closed_ || !frame_writer_)
        return;

    frame_writer_->enqueue(std::move(wire_bytes));
}


void TcpConnection::enqueue_frame(std::vector<char> wire_bytes)
{
    asio::post(strand_, [self = shared_from_this(), frame = std::move(wire_bytes)]() mutable {
        if (self->closed_ || !self->frame_writer_)
            return;

        self->frame_writer_->enqueue(std::move(frame));
    });
}


void TcpConnection::handle_read_error(const std::string_view context, const asio::error_code& ec)
{
    if (closed_)
        return;

    if (ec != asio::error::eof)
        fail(context, ec);

    request_close();
}


void TcpConnection::handle_write_queue_full()
{
    std::cerr << "Write queue limit exceeded for " << peer_label_ << ", disconnecting\n";
    request_close();
}


void TcpConnection::handle_write_error(const std::string_view context, const asio::error_code& ec)
{
    if (closed_)
        return;

    fail(context, ec);
    request_close();
}


void TcpConnection::handle_framing_error(const std::string_view message)
{
    if (closed_)
        return;

    std::cerr << "TcpConnection " << peer_label_ << ": " << message << std::endl;
    request_close();
}


void TcpConnection::fail(const std::string_view context, const asio::error_code& ec)
{
    if (ec == asio::error::operation_aborted)
        return;

    std::cerr << "TcpConnection " << peer_label_ << " " << context << ": " << ec.message() << '\n';
}


void TcpConnection::request_close()
{
    if (closed_)
        return;
    
    closed_ = true;

    if (handlers_.on_closed)
        handlers_.on_closed(id_);
}


} // namespace will
