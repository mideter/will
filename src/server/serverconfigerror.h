#pragma once

#include <stdexcept>
#include <string_view>


namespace will {


class ServerConfigError : public std::invalid_argument {
public:
    ServerConfigError(std::string_view field, std::string_view reason);
};


} // namespace will
