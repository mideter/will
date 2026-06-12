#include "tcpconnection.h"

#include <format>
#include <utility>


namespace will {


std::atomic<std::uint64_t> TcpConnection::next_id_{1};


TcpConnection::TcpConnection(asio::io_context& ioc, TcpStreamSocket socket,
                             asio::ip::tcp::endpoint peer_endpoint, TcpConnectionHandlers handlers)
    : id_(next_id_.fetch_add(1, std::memory_order_relaxed))
    , handlers_(std::move(handlers))
    , socket_(std::move(socket))
    , strand_(asio::make_strand(ioc))
    , peer_address_(std::format("{}:{}", peer_endpoint.address().to_string(), peer_endpoint.port()))
{}


void TcpConnection::begin()
{
    channel_ = std::make_shared<TcpFramedChannel>(socket_, strand_);
    channel_->start(
        [self = shared_from_this()](std::vector<char> payload) { self->handle_payload(std::move(payload)); },
        [self = shared_from_this()] { self->request_close(); });
}


void TcpConnection::shutdown()
{
    asio::post(strand_, [self = shared_from_this()] {
        if (self->shutdown_done_)
            return;

        self->shutdown_done_ = true;

        if (self->channel_)
            self->channel_->stop();

        self->socket_.shutdown_and_close();
    });
}


void TcpConnection::handle_payload(std::vector<char> payload)
{
    if (closed_)
        return;

    if (handlers_.on_payload)
        handlers_.on_payload(id_, std::move(payload));
}


void TcpConnection::enqueue_wire_frame(std::vector<char> wire_bytes)
{
    if (closed_ || !channel_)
        return;

    channel_->enqueue_wire_frame(std::move(wire_bytes));
}


void TcpConnection::schedule_on_strand(std::function<void(asio::any_io_executor)> fn)
{
    asio::post(strand_, [self = shared_from_this(), fn = std::move(fn)]() mutable {
        if (self->closed_)
            return;

        fn(self->strand_);
    });
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
