#include "asioserver.h"

#include <iostream>

#include "session.h"


namespace will {


AsioMessengerServer::AsioMessengerServer(ServerConfig config)
    : config_(config)
    , work_guard_(asio::make_work_guard(ioc_))
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
    acceptor_.listen(static_cast<int>(config_.listen_backlog));
}


void AsioMessengerServer::setup_signals()
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.async_wait([this](const asio::error_code& ec, int signo) { on_signal(ec, signo); });
}


void AsioMessengerServer::run()
{
    hub_.reset();

    do_accept();

    const int thread_count = config_.io_threads > 0 ? config_.io_threads : 1;
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

    for (std::thread& t : io_threads_)
        if (t.joinable())
            t.join();

    io_threads_.clear();
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
        if (hub_.at_capacity(config_.max_connections)) {
            std::cerr << "Max connections (" << config_.max_connections << ") reached, rejecting peer\n";
            asio::error_code ignored;
            socket.close(ignored);
        }
        else {
            try {
                socket.set_option(asio::socket_base::keep_alive(true));

                const ClientAddress peer_address = address_from_socket(socket);
                auto session = std::make_shared<Session>(ioc_, std::move(socket), peer_address, hub_,
                                                           config_.max_outbound_queue_bytes);
                session->start();
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


ClientAddress AsioMessengerServer::address_from_socket(const asio::ip::tcp::socket& socket)
{
    const asio::ip::tcp::endpoint remote = socket.remote_endpoint();
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(remote.port());
    addr.sin_addr.s_addr = htonl(remote.address().to_v4().to_uint());
    return ClientAddress::from_sockaddr_in(addr);
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
    close_all_sessions();
    work_guard_.reset();
    ioc_.stop();
}


void AsioMessengerServer::close_all_sessions()
{
    hub_.shutdown_all();
}


} // namespace will
