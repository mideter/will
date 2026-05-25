#include "willserver.h"
#include "asioserver.h"

#include <format>
#include <iostream>
#include <optional>
#include <string_view>


namespace will {


WillServerConfigError::WillServerConfigError(const std::string_view field, const std::string_view reason)
    : std::invalid_argument(std::format("{} {}", field, reason))
{}


namespace {


void require(const std::string_view field, const std::optional<std::string_view> reason)
{
    if (reason)
        throw WillServerConfigError(field, *reason);
}


std::optional<std::string_view> listen_port_reason(const std::uint16_t port)
{
    if (port < ServerConfig::MinListenPort || port > ServerConfig::MaxListenPort)
        return "must be between 1 and 65535";

    return std::nullopt;
}


template<typename T>
std::optional<std::string_view> positive_reason(const T value)
{
    if (value < 1)
        return "must be at least 1";

    return std::nullopt;
}


} // namespace


void WillServer::validate_config(const ServerConfig& config)
{
    require("listen_port", listen_port_reason(config.listen_port));
    require("io_threads", positive_reason(config.io_threads));
    require("listen_backlog", positive_reason(config.listen_backlog));
    require("max_connections", positive_reason(config.max_connections));
    require("max_outbound_queue_bytes", positive_reason(config.max_outbound_queue_bytes));
}


ServerConfig WillServer::accept_config(ServerConfig config)
{
    validate_config(config);
    return config;
}


WillServer::WillServer(ServerConfig config)
    : AsioMessengerServer(accept_config(std::move(config)))
{}


void WillServer::run()
{
    log_startup(config_);
    AsioMessengerServer::run();
}


void WillServer::log_startup(const ServerConfig& config)
{
    std::cout << "Starting Will Messenger Server v" << Version << " on port " << config.listen_port
              << " (max " << config.max_connections << " clients, " << config.io_threads
              << " io threads)\n";
}


} // namespace will
