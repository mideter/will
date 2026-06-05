#include "asioserver.h"

#include <iostream>


namespace will {


AsioMessengerServer::AsioMessengerServer(ServerConfig config)
    : config_(config)
    , database_(config.db_path)
    , message_repository_(database_)
    , user_repository_(database_)
    , auth_session_store_(database_)
    , registry_()
    , protocol_adapter_(message_repository_, user_repository_, auth_session_store_, registry_)
    , acceptor_(ioc_)
    , signals_(ioc_)
{
    open_acceptor();
    setup_signals();
}


void AsioMessengerServer::open_acceptor()
{
    using asio::ip::tcp;
    tcp::endpoint endpoint(tcp::v4(), config_.listen_port);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(config_.listen_backlog);
}


void AsioMessengerServer::setup_signals()
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait([this](const asio::error_code& ec, int signo) { on_signal(ec, signo); });
}


void AsioMessengerServer::start_io_threads()
{
    const int thread_count = config_.io_threads;
    io_threads_.reserve(static_cast<std::size_t>(thread_count));

    for (int i = 0; i < thread_count; ++i) {
        io_threads_.emplace_back([this] {
            try {
                ioc_.run();
            }
            catch (const std::exception& e) {
                std::cerr << "io_context worker error: " << e.what() << '\n';
            }
        });
    }
}


void AsioMessengerServer::join_io_threads()
{
    for (std::thread& t : io_threads_)
        t.join();

    io_threads_.clear();
}


void AsioMessengerServer::run()
{
    do_accept();
    start_io_threads();
    join_io_threads();
}


void AsioMessengerServer::do_accept()
{
    acceptor_.async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket socket) {
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

                registry_.accept_session(ioc_, std::move(socket), socket.remote_endpoint(),
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

    asio::error_code ignored;
    acceptor_.close(ignored);

    registry_.close_all_sessions();

    ioc_.stop();
}


} // namespace will
