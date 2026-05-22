#pragma once

#include <stdexcept>


namespace will {


class ServerConfigError : public std::invalid_argument {
public:
    ServerConfigError(const char* field, const char* reason);
};


} // namespace will
