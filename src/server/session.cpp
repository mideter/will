#include "session.h"

#include "sessionregistry.h"
#include "tcpframereader.h"

#include <format>
#include <iostream>

#include "willmessage.h"
#include "willprotocol.h"


namespace will {


std::atomic<std::uint64_t> Session::next_id_{1};


Session::Session(asio::io_context& ioc, TcpSocket socket, HostAddress address, SessionRegistry& registry,
                 std::size_t max_outbound_queue_bytes)
    : id_(next_id_.fetch_add(1, std::memory_order_relaxed))
    , registry_(registry)
    , max_outbound_queue_bytes_(max_outbound_queue_bytes)
    , socket_(std::move(socket))
    , strand_(asio::make_strand(ioc))
    , address_(std::move(address))
{}


void Session::begin()
{
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
        const std::string msg = std::format("Protocol error: invalid frame from {}", address_);
        fail_protocol(msg.c_str());
        return;
    }

    enqueue_frame_bytes(encode_frame(WillMessage::encode_server_receipt_ack()));

    registry_.broadcast_except(*this, payload);
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
    asio::post(strand_, [self = shared_from_this(), frame = encode_frame(payload)]() mutable {
        self->enqueue_frame_bytes(std::move(frame));
    });
}


std::vector<char> Session::encode_frame(const std::vector<char>& payload)
{
    unsigned char header_buf[4];
    TcpFrame::append_u32_be(header_buf, payload.size());

    std::vector<char> frame;
    frame.reserve(4 + payload.size());
    frame.insert(frame.end(), reinterpret_cast<char*>(header_buf),
                 reinterpret_cast<char*>(header_buf) + 4);
    frame.insert(frame.end(), payload.begin(), payload.end());
    return frame;
}


void Session::enqueue_frame_bytes(std::vector<char> frame_bytes)
{
    if (closed_)
        return;

    const std::size_t frame_len = frame_bytes.size();
    if (queued_bytes_ + frame_len > max_outbound_queue_bytes_) {
        std::cerr << "Write queue limit exceeded for " << address_ << ", disconnecting\n";
        registry_.close_session(id_);
        return;
    }

    queued_bytes_ += frame_len;
    write_queue_.push_back(std::move(frame_bytes));

    if (!write_in_progress_)
        pump_writes();
}


void Session::pump_writes()
{
    if (closed_ || write_queue_.empty()) {
        write_in_progress_ = false;
        return;
    }

    write_in_progress_ = true;
    const std::vector<char>& front = write_queue_.front();

    asio::async_write(
        socket_, asio::buffer(front),
        asio::bind_executor(strand_, [self = shared_from_this()](const asio::error_code& ec, std::size_t n) {
            self->on_write(ec, n);
        }));
}


void Session::on_write(const asio::error_code& ec, std::size_t n)
{
    if (closed_)
        return;

    if (ec) {
        fail("write", ec);
        registry_.close_session(id_);
        return;
    }

    if (write_queue_.empty())
        return;

    const std::size_t written = write_queue_.front().size();
    if (n != written) {
        fail_protocol("partial write");
        return;
    }

    queued_bytes_ -= written;
    write_queue_.pop_front();

    if (write_queue_.empty())
        write_in_progress_ = false;
    else
        pump_writes();
}


void Session::fail(const char* context, const asio::error_code& ec)
{
    if (ec == asio::error::operation_aborted)
        return;
    std::cerr << "Session " << address_ << " " << context << ": " << ec.message() << '\n';
}


void Session::fail_protocol(const char* message)
{
    std::cerr << "Session " << address_ << ": " << message << '\n';
    registry_.close_session(id_);
}


} // namespace will
