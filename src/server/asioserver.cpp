#include "asioserver.h"

#include "tcpstreamsocket.h"

#include <chrono>
#include <iostream>


namespace will {


namespace {


void log_accept_error(const asio::error_code& ec)
{
    if (ec != asio::error::operation_aborted)
        std::cerr << "accept error: " << ec.message() << '\n';
}


} // namespace


AsioMessengerServer::AsioMessengerServer(ServerConfig config, domain::MessengerPersistence persistence)
    : config_(std::move(config))
    , io_(config_.listen_port, config_.listen_backlog,
          [this](const asio::error_code& ec, int signo) { handle_shutdown_signal(ec, signo); })
    , protocol_adapter_(persistence, registry_, account_store_)
{
    registry_.set_heartbeat_settings(std::chrono::seconds{config_.heartbeat_interval_seconds},
                                     std::chrono::seconds{config_.heartbeat_timeout_seconds});
    registry_.set_payload_handler([this](const std::uint64_t id, const std::vector<char>& payload) {
        protocol_adapter_.on_client_payload(id, payload);
    });
}


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
        TcpStreamSocket stream(std::move(socket));
        stream.close_quietly();
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
        TcpStreamSocket stream(std::move(socket));
        stream.close_quietly();
        return;
    }

    try {
        TcpStreamSocket stream(std::move(socket));
        const auto peer_endpoint = stream.remote_endpoint();
        registry_.accept_connection(io_.ioc(), std::move(stream), peer_endpoint);
    }
    catch (const std::exception& e) {
        std::cerr << "Accept connection error: " << e.what() << '\n';
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

    registry_.close_all_connections();

    io_.stop();
}


} // namespace will
