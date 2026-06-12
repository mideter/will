#include "tcpstreamsocket.h"


namespace will {


void TcpStreamSocket::apply_connection_defaults(asio::ip::tcp::socket& socket)
{
    socket.set_option(asio::socket_base::keep_alive(true));
}


TcpStreamSocket::TcpStreamSocket(asio::io_context& ioc)
    : socket_(ioc)
{}


TcpStreamSocket::TcpStreamSocket(asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
{
    apply_connection_defaults(socket_);
}


void TcpStreamSocket::connect(const std::string& host, const std::uint16_t port)
{
    socket_.connect(asio::ip::tcp::endpoint(asio::ip::make_address_v4(host), port));
    apply_connection_defaults(socket_);
}


asio::ip::tcp::socket& TcpStreamSocket::asio_socket() noexcept
{
    return socket_;
}


const asio::ip::tcp::socket& TcpStreamSocket::asio_socket() const noexcept
{
    return socket_;
}


asio::ip::tcp::endpoint TcpStreamSocket::remote_endpoint() const
{
    return socket_.remote_endpoint();
}


void TcpStreamSocket::close_quietly() noexcept
{
    asio::error_code ignored;
    socket_.close(ignored);
}


void TcpStreamSocket::shutdown_and_close() noexcept
{
    asio::error_code ignored;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    socket_.close(ignored);
}


} // namespace will
