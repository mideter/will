#include "serverconfig.h"

#include "clioption.h"
#include "serverconfigerror.h"

#include <format>
#include <optional>
#include <string_view>


namespace will {


namespace {


std::optional<std::string_view> ListenPortReason(const int port)
{
    if (port < ServerConfig::MinListenPort || port > ServerConfig::MaxListenPort)
        return "must be between 1 and 65535";

    return std::nullopt;
}


std::optional<std::string_view> PositiveIntReason(const int value)
{
    if (value < 1)
        return "must be at least 1";

    return std::nullopt;
}


std::optional<std::string_view> PositiveSizeReason(const std::size_t value)
{
    if (value < 1)
        return "must be at least 1";

    return std::nullopt;
}


} // namespace


ServerConfig::ServerConfig() = default;


std::uint16_t ServerConfig::listen_port() const noexcept
{
    return listen_port_;
}


int ServerConfig::io_threads() const noexcept
{
    return io_threads_;
}


int ServerConfig::listen_backlog() const noexcept
{
    return listen_backlog_;
}


std::size_t ServerConfig::max_connections() const noexcept
{
    return max_connections_;
}


std::size_t ServerConfig::max_outbound_queue_bytes() const noexcept
{
    return max_outbound_queue_bytes_;
}


void ServerConfig::set_listen_port(int port)
{
    if (const std::optional<std::string_view> reason = ListenPortReason(port))
        throw ServerConfigError("listen_port", *reason);

    listen_port_ = static_cast<std::uint16_t>(port);
}


void ServerConfig::set_io_threads(int threads)
{
    if (const std::optional<std::string_view> reason = PositiveIntReason(threads))
        throw ServerConfigError("io_threads", *reason);

    io_threads_ = threads;
}


void ServerConfig::set_listen_backlog(int backlog)
{
    if (const std::optional<std::string_view> reason = PositiveIntReason(backlog))
        throw ServerConfigError("listen_backlog", *reason);

    listen_backlog_ = backlog;
}


void ServerConfig::set_max_connections(std::size_t max_connections)
{
    if (const std::optional<std::string_view> reason = PositiveSizeReason(max_connections))
        throw ServerConfigError("max_connections", *reason);

    max_connections_ = max_connections;
}


void ServerConfig::set_max_outbound_queue_bytes(std::size_t max_bytes)
{
    if (const std::optional<std::string_view> reason = PositiveSizeReason(max_bytes))
        throw ServerConfigError("max_outbound_queue_bytes", *reason);

    max_outbound_queue_bytes_ = max_bytes;
}


void ServerConfig::apply_cli_option(const CliOptionMatch& option)
{
    if (const std::optional<int> value = option.int_value())
        apply_cli_option(option.primary_flag(), *value);
    else if (const std::optional<std::size_t> value = option.size_value())
        apply_cli_option(option.primary_flag(), *value);
}


void ServerConfig::apply_cli_option(std::string_view flag, int value)
{
    if (flag == "--port") {
        if (const std::optional<std::string_view> reason = ListenPortReason(value))
            throw CliInvalidOptionError(flag, std::format("listen_port: {}", *reason));

        listen_port_ = static_cast<std::uint16_t>(value);
        return;
    }

    if (flag == "--io-threads") {
        if (const std::optional<std::string_view> reason = PositiveIntReason(value))
            throw CliInvalidOptionError(flag, std::format("io_threads: {}", *reason));

        io_threads_ = value;
        return;
    }

    if (flag == "--listen-backlog") {
        if (const std::optional<std::string_view> reason = PositiveIntReason(value))
            throw CliInvalidOptionError(flag, std::format("listen_backlog: {}", *reason));

        listen_backlog_ = value;
    }
}


void ServerConfig::apply_cli_option(std::string_view flag, std::size_t value)
{
    if (flag == "--max-clients") {
        if (const std::optional<std::string_view> reason = PositiveSizeReason(value))
            throw CliInvalidOptionError(flag, std::format("max_connections: {}", *reason));

        max_connections_ = value;
        return;
    }

    if (flag == "--max-outbound-queue-bytes") {
        if (const std::optional<std::string_view> reason = PositiveSizeReason(value))
            throw CliInvalidOptionError(flag, std::format("max_outbound_queue_bytes: {}", *reason));

        max_outbound_queue_bytes_ = value;
    }
}


} // namespace will
