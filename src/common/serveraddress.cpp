#include "serveraddress.h"

#include <arpa/inet.h>


namespace will {


ServerAddress::ServerAddress(IPv4 ipv4, Port port)
    : ip_(ipv4), port_(port)
{
    address_.sin_family = AF_INET;
    address_.sin_addr = ip_.network_order_;
    address_.sin_port = port_.network_order_;
}


ServerAddress ServerAddress::any(Port port)
{
    return ServerAddress(IPv4("0.0.0.0"), port);
}


const ServerAddress ServerAddress::Local(IPv4("127.0.0.1"), Port(7770));

const ServerAddress ServerAddress::Novosibirsk(IPv4("83.217.202.145"), Port(7770));


} // namespace will
