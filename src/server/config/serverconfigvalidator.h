#pragma once

#include "serverconfig.h"

#include <optional>
#include <stdexcept>
#include <string_view>


namespace will {


class ServerConfigError : public std::invalid_argument {
public:
    ServerConfigError(std::string_view field, std::string_view reason);
};


class ServerConfigValidator {
public:
    static void validate(const ServerConfig& config);
    static ServerConfig accept(ServerConfig config);

private:
    static void require(std::string_view field, std::optional<std::string_view> reason);
    static std::optional<std::string_view> listen_port_reason(std::uint16_t port);
    static std::optional<std::string_view> db_path_reason(std::string_view db_path);

    template<typename T>
    static std::optional<std::string_view> positive_reason(T value);
};


} // namespace will
