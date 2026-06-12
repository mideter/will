#pragma once

#include <asio.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <string_view>
#include <vector>


namespace will {


/** Async inbound TCP stream parser: 4-byte BE length prefix + payload. */
class TcpFrameReader : public std::enable_shared_from_this<TcpFrameReader> {
public:
    using TcpSocket = asio::ip::tcp::socket;
    using Strand = asio::strand<asio::io_context::executor_type>;
    using FrameHandler = std::function<void(std::vector<char> payload)>;
    using ProtocolErrorHandler = std::function<void(std::string_view message)>;
    using ReadErrorHandler =
        std::function<void(std::string_view context, const asio::error_code& ec)>;

    TcpFrameReader(TcpSocket& socket, Strand& strand, FrameHandler on_frame,
                   ProtocolErrorHandler on_protocol_error, ReadErrorHandler on_read_error);

    void start();
    void stop();

private:
    void start_read_header();
    void handle_read_header(const asio::error_code& ec, std::size_t n);
    void start_read_body();
    void handle_read_body(const asio::error_code& ec, std::size_t n);

    TcpSocket& socket_;
    Strand& strand_;
    FrameHandler on_frame_;
    ProtocolErrorHandler on_protocol_error_;
    ReadErrorHandler on_read_error_;

    bool stopped_ = false;

    std::array<unsigned char, 4> header_buf_{};
    std::vector<char> body_buf_;
    std::size_t expected_body_len_ = 0;
};


} // namespace will
