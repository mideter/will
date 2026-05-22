#pragma once

#include <asio.hpp>

#include <atomic>
#include <thread>
#include <vector>

#include "clientaddress.h"
#include "sessionregistry.h"
#include "serverconfig.h"


namespace will {


class AsioMessengerServer {
public:
    explicit AsioMessengerServer(ServerConfig config = {});

    void run();
    void request_stop();

protected:
    ServerConfig config_;

private:
    void open_acceptor();
    void do_accept();
    void on_accept(const asio::error_code& ec, asio::ip::tcp::socket socket);
    void setup_signals();
    void on_signal(const asio::error_code& ec, int signal_number);

    static ClientAddress address_from_socket(const asio::ip::tcp::socket& socket);
    
    asio::io_context ioc_;
    asio::ip::tcp::acceptor acceptor_;
    asio::signal_set signals_;

    SessionRegistry registry_;
    
    std::vector<std::thread> io_threads_;
    std::atomic<bool> stopping_{false};
};


} // namespace will
