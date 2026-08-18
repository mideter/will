module;

#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>

module will.server.serverconfigvalidator;


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


std::optional<std::string_view> ServerConfigValidator::db_path_reason(const std::string_view db_path)
{
    if (db_path.empty())
        return "must not be empty";

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
    require("max_connections", positive_reason(config.max_connections));
    require("db_path", db_path_reason(config.db_path));
    require("keepalive_interval_seconds", positive_reason(config.keepalive_interval_seconds));
    require("keepalive_timeout_seconds", positive_reason(config.keepalive_timeout_seconds));
}


ServerConfig ServerConfigValidator::accept(ServerConfig config)
{
    validate(config);
    return config;
}


} // namespace will
