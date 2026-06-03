#include "tcpframewriter.h"


namespace will {


TcpFrameWriter::TcpFrameWriter(TcpSocket& socket, Strand& strand, std::size_t max_queued_bytes,
                               QueueFullHandler on_queue_full,
                               ProtocolErrorHandler on_protocol_error,
                               WriteErrorHandler on_write_error)
    : socket_(socket)
    , strand_(strand)
    , max_queued_bytes_(max_queued_bytes)
    , on_queue_full_(std::move(on_queue_full))
    , on_protocol_error_(std::move(on_protocol_error))
    , on_write_error_(std::move(on_write_error))
{}


void TcpFrameWriter::stop()
{
    stopped_ = true;
}


void TcpFrameWriter::enqueue(std::vector<char> frame_bytes)
{
    if (stopped_)
        return;

    const std::size_t frame_len = frame_bytes.size();
    if (queued_bytes_ + frame_len > max_queued_bytes_) {
        on_queue_full_();
        return;
    }

    queued_bytes_ += frame_len;
    write_queue_.push_back(std::move(frame_bytes));

    if (!write_in_progress_)
        pump_writes();
}


void TcpFrameWriter::pump_writes()
{
    if (stopped_ || write_queue_.empty()) {
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


void TcpFrameWriter::on_write(const asio::error_code& ec, std::size_t n)
{
    if (stopped_)
        return;

    if (ec) {
        on_write_error_("write", ec);
        return;
    }

    if (write_queue_.empty())
        return;

    const std::size_t written = write_queue_.front().size();
    if (n != written) {
        on_protocol_error_("partial write");
        return;
    }

    queued_bytes_ -= written;
    write_queue_.pop_front();

    if (write_queue_.empty())
        write_in_progress_ = false;
    else
        pump_writes();
}


} // namespace will
