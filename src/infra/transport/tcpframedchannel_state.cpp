#include "tcpframedchannel_state.h"

#include <iostream>
#include <utility>


namespace will {


TcpFramedChannel::State::State(TcpStreamSocket& stream, Strand& strand,
                               const std::size_t max_outbound_queue_bytes)
    : stream_(stream)
    , strand_(strand)
    , max_outbound_queue_bytes_(max_outbound_queue_bytes)
{}


void TcpFramedChannel::State::request_close()
{
    if (closed_)
        return;

    closed_ = true;

    if (on_closed_)
        on_closed_();
}


void TcpFramedChannel::State::handle_framing_error(const std::string_view message)
{
    if (closed_)
        return;

    std::cerr << "TcpFramedChannel: " << message << '\n';
    request_close();
}


void TcpFramedChannel::State::handle_read_error(const std::string_view context,
                                                const asio::error_code& ec)
{
    if (closed_)
        return;

    if (ec != asio::error::eof)
        std::cerr << "TcpFramedChannel " << context << ": " << ec.message() << '\n';

    request_close();
}


void TcpFramedChannel::State::handle_write_queue_full()
{
    if (closed_)
        return;

    std::cerr << "TcpFramedChannel: outbound queue limit exceeded, closing\n";
    request_close();
}


void TcpFramedChannel::State::handle_write_error(const std::string_view context,
                                                 const asio::error_code& ec)
{
    if (closed_)
        return;

    if (ec != asio::error::operation_aborted)
        std::cerr << "TcpFramedChannel " << context << ": " << ec.message() << '\n';

    request_close();
}


void TcpFramedChannel::State::start_on_strand(FrameHandler on_frame, ClosedHandler on_closed)
{
    if (started_ || stopped_)
        return;

    started_ = true;
    on_frame_ = std::move(on_frame);
    on_closed_ = std::move(on_closed);

    writer_ = std::make_shared<TcpFrameWriter>(
        stream_.asio_socket(), strand_, max_outbound_queue_bytes_,
        [state = shared_from_this()] { state->handle_write_queue_full(); },
        [state = shared_from_this()](const std::string_view message) {
            state->handle_framing_error(message);
        },
        [state = shared_from_this()](const std::string_view context, const asio::error_code& ec) {
            state->handle_write_error(context, ec);
        });

    reader_ = std::make_shared<TcpFrameReader>(
        stream_.asio_socket(), strand_,
        [state = shared_from_this()](std::vector<char> payload) {
            if (state->closed_)
                return;

            if (state->on_frame_)
                state->on_frame_(std::move(payload));
        },
        [state = shared_from_this()](const std::string_view message) {
            state->handle_framing_error(message);
        },
        [state = shared_from_this()](const std::string_view context, const asio::error_code& ec) {
            state->handle_read_error(context, ec);
        });

    reader_->start();
}


void TcpFramedChannel::State::stop_on_strand()
{
    if (stopped_)
        return;

    stopped_ = true;

    if (reader_)
        reader_->stop();

    if (writer_)
        writer_->stop();
}


void TcpFramedChannel::State::enqueue_wire_frame_on_strand(std::vector<char> wire_bytes)
{
    if (closed_ || stopped_ || !writer_)
        return;

    writer_->enqueue(std::move(wire_bytes));
}


} // namespace will
