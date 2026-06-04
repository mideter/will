#include "clientconfigvalidator.h"

#include <asio.hpp>

#include <format>


namespace will {


ClientConfigError::ClientConfigError(const std::string_view field, const std::string_view reason)
    : std::invalid_argument(std::format("{} {}", field, reason))
{}


void ClientConfigValidator::require(const std::string_view field,
                                    const std::optional<std::string_view> reason)
{
    if (reason)
        throw ClientConfigError(field, *reason);
}


std::optional<std::string_view> ClientConfigValidator::host_reason(const std::string& host)
{
    if (host.empty())
        return "must not be empty";

    try {
        (void)asio::ip::make_address_v4(host);
    } catch (...) {
        return "must be a valid IPv4 address";
    }

    return std::nullopt;
}


std::optional<std::string_view> ClientConfigValidator::port_reason(const std::uint16_t port)
{
    if (port < ClientConfig::MinPort || port > ClientConfig::MaxPort)
        return "must be between 1 and 65535";

    return std::nullopt;
}


std::optional<std::string_view> ClientConfigValidator::credentials_reason(const std::string& value)
{
    if (value.empty())
        return "must not be empty";

    return std::nullopt;
}


void ClientConfigValidator::validate(const ClientConfig& config)
{
    require("host", host_reason(config.host));
    require("port", port_reason(config.port));
    require("login", credentials_reason(config.login));
    require("password", credentials_reason(config.password));
}


ClientConfig ClientConfigValidator::accept(ClientConfig config)
{
    validate(config);
    return config;
}


} // namespace will
