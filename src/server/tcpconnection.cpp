#include "tcpconnection.h"

#include "tcpframe.h"
#include "wiremessage_codec.h"
#include "wiremessage_server.h"

#include <format>
#include <utility>


namespace will {


std::atomic<std::uint64_t> TcpConnection::next_id_{1};


TcpConnection::TcpConnection(asio::io_context& ioc, TcpStreamSocket socket,
                             asio::ip::tcp::endpoint peer_endpoint, TcpConnectionHandlers handlers,
                             HeartbeatSettings heartbeat)
    : id_(next_id_.fetch_add(1, std::memory_order_relaxed))
    , handlers_(std::move(handlers))
    , heartbeat_(heartbeat)
    , socket_(std::move(socket))
    , strand_(asio::make_strand(ioc))
    , heartbeat_timer_(strand_)
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
        self->cancel_heartbeat();

        if (self->channel_)
            self->channel_->stop();

        self->socket_.shutdown_and_close();
    });
}


void TcpConnection::handle_payload(std::vector<char> payload)
{
    if (closed_)
        return;

    note_inbound_activity();

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


void TcpConnection::start_heartbeat()
{
    asio::post(strand_, [self = shared_from_this()] {
        if (self->closed_)
            return;

        self->heartbeat_started_ = true;
        self->awaiting_pong_ = false;
        self->schedule_heartbeat_interval();
    });
}


void TcpConnection::note_inbound_activity()
{
    if (!heartbeat_started_ || closed_)
        return;

    awaiting_pong_ = false;
    schedule_heartbeat_interval();
}


void TcpConnection::schedule_heartbeat_interval()
{
    heartbeat_timer_.expires_after(heartbeat_.interval);
    heartbeat_timer_.async_wait(
        [self = shared_from_this()](const asio::error_code& ec) { self->on_heartbeat_interval(ec); });
}


void TcpConnection::on_heartbeat_interval(const asio::error_code& ec)
{
    if (ec == asio::error::operation_aborted || closed_ || !heartbeat_started_)
        return;

    if (!channel_)
        return;

    channel_->enqueue_wire_frame(TcpFrame::encode(WireMessageCodec::encode(PingMessage{})));
    awaiting_pong_ = true;
    heartbeat_timer_.expires_after(heartbeat_.timeout);
    heartbeat_timer_.async_wait(
        [self = shared_from_this()](const asio::error_code& wait_ec) { self->on_heartbeat_timeout(wait_ec); });
}


void TcpConnection::on_heartbeat_timeout(const asio::error_code& ec)
{
    if (ec == asio::error::operation_aborted || closed_)
        return;

    if (awaiting_pong_)
        request_close();
}


void TcpConnection::cancel_heartbeat()
{
    heartbeat_started_ = false;
    awaiting_pong_ = false;
    asio::error_code ignored;
    heartbeat_timer_.cancel(ignored);
}


void TcpConnection::request_close()
{
    if (closed_)
        return;

    closed_ = true;
    cancel_heartbeat();

    if (handlers_.on_closed)
        handlers_.on_closed(id_);
}


} // namespace will
