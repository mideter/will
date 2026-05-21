#pragma once

#include "asioserver.h"
#include "serverconfig.h"


namespace will {


class WillServer {
public:
    static constexpr const char* Version = "4.0.0";

    explicit WillServer(ServerConfig config = {});

    void run();

private:
    static void log_startup(const ServerConfig& config);

    AsioMessengerServer server_;
};


} // namespace will
