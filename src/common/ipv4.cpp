#include "ipv4.h"

#include <arpa/inet.h>

#include <cerrno>
#include <stdexcept>
#include <system_error>


namespace will {


IPv4::IPv4(std::string address)
	: ip_{std::move(address)}
	, network_order_{}
{
	const int result = ::inet_pton(AF_INET, ip_.c_str(), &network_order_);

	if (result == -1)
		throw std::system_error(errno, std::generic_category(), "inet_pton failed");

	if (result == 0)
		throw std::invalid_argument("invalid IPv4 address format: " + ip_);
}


IPv4::IPv4(in_addr addr)
	: ip_{}
	, network_order_{addr}
{
	char ip[INET_ADDRSTRLEN] = {0};
	if (::inet_ntop(AF_INET, &network_order_, ip, sizeof(ip)) == nullptr)
		throw std::runtime_error("failed to format IPv4 address");

	ip_ = std::string(ip);
}


std::ostream& operator<<(std::ostream& os, const IPv4& ip) 
{
	return os << ip.ip_;
} 


} // namespace will
