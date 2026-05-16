#include "clientaddress.h"

#include <ostream>


namespace will {


ClientAddress::ClientAddress(sockaddr_in addr)
    : ip_{addr.sin_addr}
    , port_{addr.sin_port}
    , address_(addr)
{}


ClientAddress ClientAddress::from_sockaddr_in(sockaddr_in addr)
{
    return ClientAddress(addr);
}


std::ostream& operator<<(std::ostream& os, const ClientAddress& address)
{
    return os << address.ip_ << ":" << address.port_;
}


} // namespace will
