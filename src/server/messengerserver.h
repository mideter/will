#pragma once

#include "asioserver.h"
#include "serverconfig.h"


namespace will {


class MessengerServer {
public:
    explicit MessengerServer(ServerConfig config = {});

    void run();

private:
    AsioMessengerServer server_;
};


} // namespace will
