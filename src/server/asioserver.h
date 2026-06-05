#pragma once

#include <asio.hpp>

#include <atomic>

#include "iocontextthreadpool.h"
#include "willprotocoladapter.h"
#include "sessionregistry.h"

#include "ports/messenger_persistence.h"
#include "serverconfig.h"


namespace will {


class AsioMessengerServer {
public:
    AsioMessengerServer(ServerConfig config, domain::MessengerPersistence persistence);

    void run();

    void request_stop();

private:
    void open_acceptor();

    void do_accept();

    void on_accept(const asio::error_code& ec, asio::ip::tcp::socket socket);

    void setup_signals();

    void on_signal(const asio::error_code& ec, int signal_number);

    ServerConfig config_;

    asio::io_context ioc_;
    asio::ip::tcp::acceptor acceptor_;
    asio::signal_set signals_;

    SessionRegistry registry_;
    WillProtocolAdapter protocol_adapter_;

    std::atomic<bool> stopping_{false};
};


} // namespace will
