#include "serverconfig.h"

#include "clioption.h"
#include "serverconfigerror.h"


namespace will {


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
    if (port < MinListenPort || port > MaxListenPort)
        throw ServerConfigError("listen_port", "must be between 1 and 65535");

    listen_port_ = static_cast<std::uint16_t>(port);
}


void ServerConfig::set_io_threads(int threads)
{
    if (threads < 1)
        throw ServerConfigError("io_threads", "must be at least 1");

    io_threads_ = threads;
}


void ServerConfig::set_listen_backlog(int backlog)
{
    if (backlog < 1)
        throw ServerConfigError("listen_backlog", "must be at least 1");

    listen_backlog_ = backlog;
}


void ServerConfig::set_max_connections(std::size_t max_connections)
{
    if (max_connections < 1)
        throw ServerConfigError("max_connections", "must be at least 1");

    max_connections_ = max_connections;
}


void ServerConfig::set_max_outbound_queue_bytes(std::size_t max_bytes)
{
    if (max_bytes < 1)
        throw ServerConfigError("max_outbound_queue_bytes", "must be at least 1");

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
        set_listen_port(value);
        return;
    }

    if (flag == "--io-threads") {
        set_io_threads(value);
        return;
    }

    if (flag == "--listen-backlog") {
        set_listen_backlog(value);
        return;
    }
}


void ServerConfig::apply_cli_option(std::string_view flag, std::size_t value)
{
    if (flag == "--max-clients") {
        set_max_connections(value);
        return;
    }

    if (flag == "--max-outbound-queue-bytes") {
        set_max_outbound_queue_bytes(value);
        return;
    }
}


} // namespace will
