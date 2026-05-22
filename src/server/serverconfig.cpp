#include "serverconfig.h"

#include "serverconfigerror.h"


namespace will {


ServerConfig::ServerConfig() = default;


void ServerConfig::set_listen_port(std::uint16_t port)
{
    if (port == 0)
        throw ServerConfigError("listen_port", "must be between 1 and 65535");

    listen_port_ = port;
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


} // namespace will
