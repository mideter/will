#pragma once

#include <netinet/in.h>

#include <string>
#include <iosfwd>


namespace will {


class IPv4 {
public:
	explicit IPv4(std::string address);
	explicit IPv4(in_addr addr);

private:
	friend class ServerAddress;
	friend std::ostream& operator<<(std::ostream& os, const IPv4& ip); 

	std::string ip_;
	in_addr network_order_;
};


std::ostream& operator<<(std::ostream& os, const IPv4& ip); 


} // namespace will
