#pragma once

#include <thread>
#include <vector>

#include "connectionacceptor.h"
#include "clienthub.h"


namespace will {


class MessengerServer {
public:
    MessengerServer();
    ~MessengerServer();

    void run();

private:
    struct Peers {
        ClientHub clients;
        std::vector<std::jthread> threads;
    };

    ConnectionAcceptor acceptor_;
    Peers peers_;
};


} // namespace will
