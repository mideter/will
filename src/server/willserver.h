#pragma once

#include "asioserver.h"
#include "serverconfig.h"

#include <stdexcept>
#include <string_view>


namespace will {


class WillServerConfigError : public std::invalid_argument {
public:
    WillServerConfigError(std::string_view field, std::string_view reason);
};


class WillServer : private AsioMessengerServer {
public:
    static constexpr const char* Version = "4.0.2";

    explicit WillServer(ServerConfig config = {});

    void run();

private:
    static void validate_config(const ServerConfig& config);
    static ServerConfig accept_config(ServerConfig config);
    static void log_startup(const ServerConfig& config);
};


} // namespace will
