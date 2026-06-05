#include "asioserver.h"

#include <iostream>


namespace will {


AsioMessengerServer::AsioMessengerServer(ServerConfig config, domain::MessengerPersistence persistence)
    : config_(config)
    , io_(config_.listen_port, config_.listen_backlog,
          [this](const asio::error_code& ec, int signo) { on_signal(ec, signo); })
    , protocol_adapter_(persistence, registry_)
{
}


void AsioMessengerServer::run()
{
    do_accept();
    const IoContextThreadPool io_threads(io_.ioc(), config_.io_threads);
}


void AsioMessengerServer::do_accept()
{
    io_.acceptor().async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
        on_accept(ec, std::move(socket));
    });
}


void AsioMessengerServer::on_accept(const asio::error_code& ec, asio::ip::tcp::socket socket)
{
    if (stopping_) {
        asio::error_code ignored;
        socket.close(ignored);
        return;
    }

    if (!ec) {
        if (registry_.at_capacity(config_.max_connections)) {
            std::cerr << "Max connections (" << config_.max_connections << ") reached, rejecting peer\n";
            asio::error_code ignored;
            socket.close(ignored);
        }
        else {
            try {
                socket.set_option(asio::socket_base::keep_alive(true));

                registry_.accept_session(io_.ioc(), std::move(socket), socket.remote_endpoint(),
                                         protocol_adapter_, config_.max_outbound_queue_bytes);
            }
            catch (const std::exception& e) {
                std::cerr << "Accept session error: " << e.what() << '\n';
            }
        }
    }
    else if (ec != asio::error::operation_aborted) {
        std::cerr << "accept error: " << ec.message() << '\n';
    }

    if (!stopping_)
        do_accept();
}


void AsioMessengerServer::on_signal(const asio::error_code& ec, int signal_number)
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
