#pragma once

#include <atomic>

#include "connectionaccountstore.h"
#include "iocontextthreadpool.h"
#include "iocontext.h"
#include "otpconnectionstatestore.h"
#include "protocoladapter.h"
#include "tcpconnectionregistry.h"

#include "logging_sms_sender.h"
#include "ports/messenger_persistence.h"
#include "sha256_otp_hasher.h"
#include "serverconfig.h"


namespace will {


class AsioMessengerServer {
public:
    AsioMessengerServer(ServerConfig config, domain::MessengerPersistence persistence, domain::OtpStore& otp_store);

    void run();

    void request_stop();

private:
    void start_accept();

    void handle_accept(const asio::error_code& ec, asio::ip::tcp::socket socket);

    void accept_client(asio::ip::tcp::socket socket);

    void handle_shutdown_signal(const asio::error_code& ec, int signal_number);

    ServerConfig config_;

    ConnectionAccountStore account_store_;
    IoContext io_;
    TcpConnectionRegistry registry_{account_store_};
    OtpConnectionStateStore otp_state_{registry_, config_.auth_pending_timeout_sec};
    LoggingSmsSender sms_sender_;
    Sha256OtpHasher otp_hasher_;
    ProtocolAdapter protocol_adapter_;

    std::atomic<bool> stopping_{false};
};


} // namespace will
