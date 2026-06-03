#include "session.h"

#include "chatservice.h"
#include "sessionregistry.h"
#include "tcpframereader.h"
#include "tcpframewriter.h"

#include <format>
#include <iostream>

#include "willmessage.h"
#include "willprotocol.h"


namespace will {


std::atomic<std::uint64_t> Session::next_id_{1};


Session::Session(asio::io_context& ioc, TcpSocket socket, asio::ip::tcp::endpoint peer_endpoint,
                 SessionRegistry& registry, ChatService& chat_service,
                 std::size_t max_outbound_queue_bytes)
    : id_(next_id_.fetch_add(1, std::memory_order_relaxed))
    , registry_(registry)
    , chat_service_(chat_service)
    , max_outbound_queue_bytes_(max_outbound_queue_bytes)
    , socket_(std::move(socket))
    , strand_(asio::make_strand(ioc))
    , peer_ip_(peer_endpoint.address().to_string())
    , peer_label_(std::format("{}:{}", peer_ip_, peer_endpoint.port()))
{}


void Session::begin()
{
    frame_writer_ = std::make_shared<TcpFrameWriter>(
        socket_, strand_, max_outbound_queue_bytes_,
        [self = shared_from_this()] { self->on_write_queue_full(); },
        [self = shared_from_this()](const char* message) { self->fail_protocol(message); },
        [self = shared_from_this()](const char* context, const asio::error_code& ec) {
            self->on_write_error(context, ec);
        });

    frame_reader_ = std::make_shared<TcpFrameReader>(
        socket_, strand_,
        [self = shared_from_this()](std::vector<char> payload) { self->on_frame(std::move(payload)); },
        [self = shared_from_this()](const char* message) { self->fail_protocol(message); },
        [self = shared_from_this()](const char* context, const asio::error_code& ec) {
            self->on_read_error(context, ec);
        });

    frame_reader_->start();
}


void Session::shutdown()
{
    asio::post(strand_, [self = shared_from_this()] {
        if (self->closed_)
            return;
        self->closed_ = true;

        if (self->frame_reader_)
            self->frame_reader_->stop();
        if (self->frame_writer_)
            self->frame_writer_->stop();

        asio::error_code ignored;
        self->socket_.shutdown(TcpSocket::shutdown_both, ignored);
        self->socket_.close(ignored);
    });
}


void Session::on_frame(std::vector<char> payload)
{
    if (closed_)
        return;

    if (!WillMessage::is_valid_client_to_server_payload(payload)) {
        const std::string msg = std::format("Protocol error: invalid frame from {}", peer_label_);
        fail_protocol(msg.c_str());
        return;
    }

    if (WillMessage::is_user_chat(payload)) {
        chat_service_.handle_user_chat(*this, payload);
        return;
    }

    if (WillMessage::is_history_request(payload)) {
        chat_service_.handle_history_request(*this, payload);
    }
}


void Session::send_will_payload(const std::vector<char>& payload)
{
    if (closed_ || !frame_writer_)
        return;

    frame_writer_->enqueue(TcpFrame::encode(payload));
}


void Session::on_read_error(const char* context, const asio::error_code& ec)
{
    if (closed_)
        return;

    if (ec != asio::error::eof)
        fail(context, ec);

    registry_.close_session(id_);
}


void Session::enqueue_payload_broadcast(const std::vector<char>& payload)
{
    asio::post(strand_, [self = shared_from_this(), frame = TcpFrame::encode(payload)]() mutable {
        if (self->closed_ || !self->frame_writer_)
            return;
        self->frame_writer_->enqueue(std::move(frame));
    });
}


void Session::on_write_queue_full()
{
    std::cerr << "Write queue limit exceeded for " << peer_label_ << ", disconnecting\n";
    registry_.close_session(id_);
}


void Session::on_write_error(const char* context, const asio::error_code& ec)
{
    if (closed_)
        return;

    fail(context, ec);
    registry_.close_session(id_);
}


void Session::fail(const char* context, const asio::error_code& ec)
{
    if (ec == asio::error::operation_aborted)
        return;
    std::cerr << "Session " << peer_label_ << " " << context << ": " << ec.message() << '\n';
}


void Session::fail_protocol(const char* message)
{
    std::cerr << "Session " << peer_label_ << ": " << message << '\n';
    registry_.close_session(id_);
}


} // namespace will
