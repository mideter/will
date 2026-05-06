#pragma once

#include <cerrno>
#include <string>
#include <system_error>


namespace will {


class SocketError : public std::system_error {
public:
	explicit SocketError(const std::string& message)
		: std::system_error(errno, std::generic_category(), message)
	{}

	SocketError(int error_code, const std::string& message)
		: std::system_error(error_code, std::generic_category(), message)
	{}
};


} // namespace will
