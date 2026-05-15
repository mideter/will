#pragma once

#include <netinet/in.h>

#include "ipv4.h"
#include "port.h"


namespace will {


class MessengerServer;
class MessengerClient;
class ConnectionAcceptor;


class ServerAddress {
public:
    ServerAddress(IPv4 ipv4, Port port);
    
    static ServerAddress any(Port port);

private:
    friend class MessengerServer;
    friend class MessengerClient;
    friend class ConnectionAcceptor;

    IPv4 ip_;
    Port port_;

    sockaddr_in address_{};
};


} // namespace will
