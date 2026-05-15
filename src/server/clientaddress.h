#pragma once

#include <netinet/in.h>
#include <iosfwd>

#include "ipv4.h"
#include "port.h"


namespace will {


class ClientAddress {
public:
    ClientAddress(const ClientAddress&) = default;
    ClientAddress& operator=(const ClientAddress&) = default;
    ClientAddress(ClientAddress&&) = default;
    ClientAddress& operator=(ClientAddress&&) = default;

private:
    friend class ClientConnection;
    friend class ConnectionAcceptor;
    friend std::ostream& operator<<(std::ostream& os, const ClientAddress& address);

    explicit ClientAddress(sockaddr_in addr);

    IPv4 ip_;
    Port port_;

    sockaddr_in address_{};
};


std::ostream& operator<<(std::ostream& os, const ClientAddress& address);


} // namespace will
