#pragma once

#include <format>
#include <iosfwd>
#include <netinet/in.h>
#include <sstream>

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


template <>
struct std::formatter<will::ClientAddress> {
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const will::ClientAddress& address, std::format_context& ctx) const
    {
        std::ostringstream os;
        os << address;
        return std::format_to(ctx.out(), "{}", os.str());
    }
};
