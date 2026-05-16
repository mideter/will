#include "session.h"

#include "clienthub.h"

#include <format>
#include <iostream>
#include <stdexcept>

#include "willmessage.h"
#include "willprotocol.h"


namespace will {


std::atomic<std::uint64_t> Session::next_id_{1};
std::mutex Session::frame_log_mutex_{};


Session::Session(asio::io_context& ioc, TcpSocket socket, ClientAddress address, ClientHub& hub,
                 std::size_t max_outbound_queue_bytes)
    : id_(next_id_.fetch_add(1, std::memory_order_relaxed))
    , hub_(hub)
    , max_outbound_queue_bytes_(max_outbound_queue_bytes)
    , socket_(std::move(socket))
    , strand_(asio::make_strand(ioc))
    , address_(std::move(address))
{}


void Session::start()
{
    hub_.add(shared_from_this());
    asio::dispatch(strand_, [self = shared_from_this()] { self->do_read_header(); });
}


void Session::close()
{
    asio::post(strand_, [self = shared_from_this()] {
        if (self->closed_)
            return;
        self->closed_ = true;
        asio::error_code ignored;
        self->socket_.shutdown(TcpSocket::shutdown_both, ignored);
        self->socket_.close(ignored);
        self->hub_.remove(self->id_);
    });
}


void Session::do_read_header()
{
    if (closed_)
        return;

    asio::async_read(socket_, asio::buffer(header_buf_),
                     asio::bind_executor(strand_,
                                         [self = shared_from_this()](const asio::error_code& ec,
                                                                     std::size_t n) {
                                             self->on_read_header(ec, n);
                                         }));
}


void Session::on_read_header(const asio::error_code& ec, std::size_t n)
{
    if (closed_)
        return;

    if (ec) {
        if (ec != asio::error::eof)
            fail("read header", ec);
        close();
        return;
    }

    if (n != header_buf_.size()) {
        fail_protocol("Will frame: incomplete header");
        return;
    }

    const unsigned char* const header_u = reinterpret_cast<const unsigned char*>(header_buf_.data());
    const std::uint32_t len_u32 = TcpFrame::read_u32_be(header_u);
    expected_body_len_ = static_cast<std::size_t>(len_u32);

    if (expected_body_len_ > TcpFrame::MaxPayloadBytes) {
        fail_protocol("Will frame: frame exceeds TcpFrame::MaxPayloadBytes");
        return;
    }

    body_buf_.assign(expected_body_len_, '\0');
    if (expected_body_len_ == 0) {
        handle_complete_payload();
        return;
    }

    do_read_body();
}


void Session::do_read_body()
{
    if (closed_)
        return;

    asio::async_read(socket_, asio::buffer(body_buf_),
                     asio::bind_executor(strand_,
                                         [self = shared_from_this()](const asio::error_code& ec,
                                                                     std::size_t n) {
                                             self->on_read_body(ec, n);
                                         }));
}


void Session::on_read_body(const asio::error_code& ec, std::size_t n)
{
    if (closed_)
        return;

    if (ec) {
        if (ec != asio::error::eof)
            fail("read body", ec);
        close();
        return;
    }

    if (n != body_buf_.size()) {
        fail_protocol("Will frame: connection closed mid-frame");
        return;
    }

    handle_complete_payload();
}


void Session::handle_complete_payload()
{
    if (!WillMessage::is_valid_client_to_server_payload(body_buf_)) {
        const std::string msg = std::format("Protocol error: invalid frame from {}", address_);
        fail_protocol(msg.c_str());
        return;
    }

    const std::vector<char> payload = body_buf_;
    enqueue_frame_bytes(encode_frame(WillMessage::encode_server_receipt_ack()));

    hub_.broadcast_except(id_, payload, [payload](const std::shared_ptr<Session>& peer) {
        peer->enqueue_payload_broadcast(payload);
    });

    {
        std::lock_guard io_lock(frame_log_mutex_);
        std::cout << "Broadcast from " << address_ << ": "
                  << WillMessage::format_payload_for_log(payload) << std::endl;
    }

    do_read_header();
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
        close();
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
        close();
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
    close();
}


} // namespace will
