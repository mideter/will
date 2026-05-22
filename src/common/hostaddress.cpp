#include "hostaddress.h"

#include <ostream>


namespace will {


HostAddress::HostAddress(std::string_view host, std::uint16_t port)
    : endpoint_(asio::ip::tcp::endpoint(asio::ip::make_address_v4(host), port))
{}


HostAddress::HostAddress(asio::ip::tcp::endpoint endpoint)
    : endpoint_(std::move(endpoint))
{}


HostAddress HostAddress::from_endpoint(asio::ip::tcp::endpoint endpoint)
{
    return HostAddress(std::move(endpoint));
}


HostAddress HostAddress::any(std::uint16_t port)
{
    return HostAddress("0.0.0.0", port);
}


std::ostream& operator<<(std::ostream& os, const HostAddress& address)
{
    return os << address.endpoint_.address().to_string() << ':' << address.endpoint_.port();
}


} // namespace will
