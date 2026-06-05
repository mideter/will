#include "asioserver.h"

#include <iostream>


namespace will {


namespace {


void close_quietly(asio::ip::tcp::socket& socket)
{
    asio::error_code ignored;
    socket.close(ignored);
}


void log_accept_error(const asio::error_code& ec)
{
    if (ec != asio::error::operation_aborted)
        std::cerr << "accept error: " << ec.message() << '\n';
}


} // namespace


AsioMessengerServer::AsioMessengerServer(ServerConfig config, domain::MessengerPersistence persistence)
    : config_(config)
    , io_(config_.listen_port, config_.listen_backlog,
          [this](const asio::error_code& ec, int signo) { handle_shutdown_signal(ec, signo); })
    , protocol_adapter_(persistence, registry_)
{}


void AsioMessengerServer::run()
{
    start_accept();
    const IoContextThreadPool io_threads(io_.ioc(), config_.io_threads);
}


void AsioMessengerServer::start_accept()
{
    io_.acceptor().async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
        handle_accept(ec, std::move(socket));
    });
}


void AsioMessengerServer::handle_accept(const asio::error_code& ec, asio::ip::tcp::socket socket)
{
    if (stopping_) {
        close_quietly(socket);
        return;
    }

    if (ec)
        log_accept_error(ec);
    else
        accept_client(std::move(socket));

    if (!stopping_)
        start_accept();
}


void AsioMessengerServer::accept_client(asio::ip::tcp::socket socket)
{
    if (registry_.at_capacity(config_.max_connections)) {
        std::cerr << "Max connections (" << config_.max_connections << ") reached, rejecting peer\n";
        close_quietly(socket);
        return;
    }

    try {
        socket.set_option(asio::socket_base::keep_alive(true));

        registry_.accept_session(io_.ioc(), std::move(socket), socket.remote_endpoint(),
                                 protocol_adapter_, config_.max_outbound_queue_bytes);
    }
    catch (const std::exception& e) {
        std::cerr << "Accept session error: " << e.what() << '\n';
    }
}


void AsioMessengerServer::handle_shutdown_signal(const asio::error_code& ec, int signal_number)
{
    if (ec == asio::error::operation_aborted)
        return;

    std::cout << "Shutdown signal (" << signal_number << ") received\n";
    request_stop();
}


void AsioMessengerServer::request_stop()
{
    if (stopping_.exchange(true))
        return;

    io_.close_acceptor();

    registry_.close_all_sessions();

    io_.stop();
}


} // namespace will
