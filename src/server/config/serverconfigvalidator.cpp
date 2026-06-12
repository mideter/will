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


std::optional<std::string_view> ServerConfigValidator::otp_hash_salt_reason(const ServerConfig& config)
{
    if (!config.dev_fixed_otp && config.otp_hash_salt.empty())
        return "must be set unless --dev-fixed-otp is provided";

    return std::nullopt;
}


void ServerConfigValidator::validate(const ServerConfig& config)
{
    require("listen_port", listen_port_reason(config.listen_port));
    require("io_threads", positive_reason(config.io_threads));
    require("listen_backlog", positive_reason(config.listen_backlog));
    require("max_connections", positive_reason(config.max_connections));
    require("db_path", db_path_reason(config.db_path));
    
    require("otp_ttl_sec", positive_reason(config.otp_ttl_sec));
    require("otp_length", positive_reason(config.otp_length));
    require("max_verify_attempts", positive_reason(config.max_verify_attempts));
    require("otp_cooldown_sec", positive_reason(config.otp_cooldown_sec));
    require("max_requests_per_ip", positive_reason(config.max_requests_per_ip));
    require("ip_rate_window_sec", positive_reason(config.ip_rate_window_sec));
    require("auth_pending_timeout_sec", positive_reason(config.auth_pending_timeout_sec));
    require("otp_hash_salt", otp_hash_salt_reason(config));
}


ServerConfig ServerConfigValidator::accept(ServerConfig config)
{
    validate(config);
    return config;
}


} // namespace will
