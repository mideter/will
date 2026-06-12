#pragma once

#include <asio.hpp>

#include <cstdint>
#include <string>


namespace will {


/** Connected-or-connectable TCP stream with Will defaults (SO_KEEPALIVE). */
class TcpStreamSocket {
public:
    explicit TcpStreamSocket(asio::io_context& ioc);

    /** Accept path: move socket in, apply connection defaults. */
    explicit TcpStreamSocket(asio::ip::tcp::socket socket);

    TcpStreamSocket(TcpStreamSocket&&) noexcept = default;
    TcpStreamSocket& operator=(TcpStreamSocket&&) noexcept = default;

    TcpStreamSocket(const TcpStreamSocket&) = delete;
    TcpStreamSocket& operator=(const TcpStreamSocket&) = delete;

    void connect(const std::string& host, std::uint16_t port);

    asio::ip::tcp::socket& asio_socket() noexcept;
    const asio::ip::tcp::socket& asio_socket() const noexcept;

    asio::ip::tcp::endpoint remote_endpoint() const;

    void close_quietly() noexcept;
    void shutdown_and_close() noexcept;

private:
    static void apply_connection_defaults(asio::ip::tcp::socket& socket);

    asio::ip::tcp::socket socket_;
};


} // namespace will
