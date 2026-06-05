#pragma once

#include <atomic>

#include "iocontextthreadpool.h"
#include "ioresources.h"
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
    void start_accept();

    void handle_accept(const asio::error_code& ec, asio::ip::tcp::socket socket);

    void accept_client(asio::ip::tcp::socket socket);

    void handle_shutdown_signal(const asio::error_code& ec, int signal_number);

    ServerConfig config_;

    IoResources io_;
    SessionRegistry registry_;
    WillProtocolAdapter protocol_adapter_;

    std::atomic<bool> stopping_{false};
};


} // namespace will
