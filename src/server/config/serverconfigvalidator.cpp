#include "serverconfigvalidator.h"

#include <cstddef>
#include <format>


namespace will {


ServerConfigError::ServerConfigError(const std::string_view field, const std::string_view reason)
    : std::invalid_argument(std::format("{} {}", field, reason))
{}


void ServerConfigValidator::require(const std::string_view field, const std::optional<std::string_view> reason)
{
    if (reason)
        throw ServerConfigError(field, *reason);
}


std::optional<std::string_view> ServerConfigValidator::listen_port_reason(const std::uint16_t port)
{
    if (port < ServerConfig::MinListenPort || port > ServerConfig::MaxListenPort)
        return "must be between 1 and 65535";

    return std::nullopt;
}


template<typename T>
std::optional<std::string_view> ServerConfigValidator::positive_reason(const T value)
{
    if (value < 1)
        return "must be at least 1";

    return std::nullopt;
}


void ServerConfigValidator::validate(const ServerConfig& config)
{
    require("listen_port", listen_port_reason(config.listen_port));
    require("io_threads", positive_reason(config.io_threads));
    require("listen_backlog", positive_reason(config.listen_backlog));
    require("max_connections", positive_reason(config.max_connections));
    require("max_outbound_queue_bytes", positive_reason(config.max_outbound_queue_bytes));
}


ServerConfig ServerConfigValidator::accept(ServerConfig config)
{
    validate(config);
    return config;
}


} // namespace will
