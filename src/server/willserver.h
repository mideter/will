#pragma once

#include "asioserver.h"
#include "serverconfig.h"
#include "sqlite_persistence_bundle.h"


namespace will {


class WillServer {
public:
    static constexpr const char* Version = "5.1.0";

    explicit WillServer(ServerConfig config = {});

    void run();

private:
    static void log_startup(const ServerConfig& config);

    ServerConfig config_;
    SqlitePersistenceBundle persistence_;
    AsioMessengerServer server_;
};


} // namespace will
