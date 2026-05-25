#pragma once

#include "clientconfig.h"

#include <optional>
#include <stdexcept>
#include <string_view>


namespace will {


class ClientConfigError : public std::invalid_argument {
public:
    ClientConfigError(std::string_view field, std::string_view reason);
};


class ClientConfigValidator {
public:
    static void validate(const ClientConfig& config);
    static ClientConfig accept(ClientConfig config);

private:
    static void require(std::string_view field, std::optional<std::string_view> reason);
    static std::optional<std::string_view> host_reason(const std::string& host);
    static std::optional<std::string_view> port_reason(std::uint16_t port);
};


} // namespace will
