#pragma once

#include <netinet/in.h>

#include "ipv4.h"
#include "port.h"


namespace will {


class WillServer;
class WillClient;


class ServerAddress {
public:
    ServerAddress(IPv4 ipv4, Port port);
    
    static ServerAddress any(Port port);

    /** Local Will server (loopback). */
    static const ServerAddress Local;

    /** Remote Will server (Novosibirsk). */
    static const ServerAddress Novosibirsk;

private:
    friend class WillServer;
    friend class WillClient;

    IPv4 ip_;
    Port port_;

    sockaddr_in address_{};
};


} // namespace will
