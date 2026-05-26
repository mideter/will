#pragma once

#include "asioserver.h"
#include "serverconfig.h"

namespace will {


class WillServer : private AsioMessengerServer {
public:
    static constexpr const char* Version = "4.1.0";

    explicit WillServer(ServerConfig config = {});

    void run();

private:
    static void log_startup(const ServerConfig& config);
};


} // namespace will
