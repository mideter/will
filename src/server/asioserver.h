#pragma once

#include <asio.hpp>

#include <atomic>
#include <thread>
#include <vector>

#include "willprotocoladapter.h"
#include "sessionregistry.h"

#include "sqlite_auth_session_store_impl.h"
#include "sqlite_database.h"
#include "sqlite_message_repository_impl.h"
#include "sqlite_user_repository_impl.h"
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

    void start_io_threads();
    
    void join_io_threads();
    
    void do_accept();
    
    void on_accept(const asio::error_code& ec, asio::ip::tcp::socket socket);
    
    void setup_signals();
    
    void on_signal(const asio::error_code& ec, int signal_number);

    asio::io_context ioc_;
    asio::ip::tcp::acceptor acceptor_;
    asio::signal_set signals_;

    SqliteDatabase database_;
    SqliteMessageRepositoryImpl message_repository_;
    SqliteUserRepositoryImpl user_repository_;
    SqliteAuthSessionStoreImpl auth_session_store_;
    SessionRegistry registry_;
    WillProtocolAdapter protocol_adapter_;
    
    std::vector<std::thread> io_threads_;
    std::atomic<bool> stopping_{false};
};


} // namespace will
