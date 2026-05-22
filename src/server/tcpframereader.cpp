#include "tcpframereader.h"

#include "willprotocol.h"


namespace will {


TcpFrameReader::TcpFrameReader(TcpSocket& socket, Strand& strand, FrameHandler on_frame,
                               ProtocolErrorHandler on_protocol_error,
                               ReadErrorHandler on_read_error)
    : socket_(socket)
    , strand_(strand)
    , on_frame_(std::move(on_frame))
    , on_protocol_error_(std::move(on_protocol_error))
    , on_read_error_(std::move(on_read_error))
{}


void TcpFrameReader::start()
{
    asio::dispatch(strand_, [self = shared_from_this()] { self->do_read_header(); });
}


void TcpFrameReader::stop()
{
    stopped_ = true;
}


void TcpFrameReader::do_read_header()
{
    if (stopped_)
        return;

    asio::async_read(socket_, asio::buffer(header_buf_),
                     asio::bind_executor(strand_,
                                         [self = shared_from_this()](const asio::error_code& ec,
                                                                     std::size_t n) {
                                             self->on_read_header(ec, n);
                                         }));
}


void TcpFrameReader::on_read_header(const asio::error_code& ec, std::size_t n)
{
    if (stopped_)
        return;

    if (ec) {
        on_read_error_("read header", ec);
        return;
    }

    if (n != header_buf_.size()) {
        on_protocol_error_("Will frame: incomplete header");
        return;
    }

    const unsigned char* const header_u = reinterpret_cast<const unsigned char*>(header_buf_.data());
    const std::uint32_t len_u32 = TcpFrame::read_u32_be(header_u);
    expected_body_len_ = static_cast<std::size_t>(len_u32);

    if (expected_body_len_ > TcpFrame::MaxPayloadBytes) {
        on_protocol_error_("Will frame: frame exceeds TcpFrame::MaxPayloadBytes");
        return;
    }

    body_buf_.assign(expected_body_len_, '\0');
    if (expected_body_len_ == 0) {
        on_frame_(body_buf_);
        do_read_header();
        return;
    }

    do_read_body();
}


void TcpFrameReader::do_read_body()
{
    if (stopped_)
        return;

    asio::async_read(socket_, asio::buffer(body_buf_),
                     asio::bind_executor(strand_,
                                         [self = shared_from_this()](const asio::error_code& ec,
                                                                     std::size_t n) {
                                             self->on_read_body(ec, n);
                                         }));
}


void TcpFrameReader::on_read_body(const asio::error_code& ec, std::size_t n)
{
    if (stopped_)
        return;

    if (ec) {
        on_read_error_("read body", ec);
        return;
    }

    if (n != body_buf_.size()) {
        on_protocol_error_("Will frame: connection closed mid-frame");
        return;
    }

    on_frame_(body_buf_);
    do_read_header();
}


} // namespace will
