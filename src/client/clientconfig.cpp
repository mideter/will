#include "clientconfig.h"

#include <format>
#include <optional>
#include <string_view>


namespace will {


ClientConfigError::ClientConfigError(const std::string_view field, const std::string_view reason)
    : std::invalid_argument(std::format("{}: {}", field, reason))
{}


namespace {


std::optional<std::string_view> PortReason(const int port)
{
    if (port < ClientConfig::MinPort || port > ClientConfig::MaxPort)
        return "must be between 1 and 65535";

    return std::nullopt;
}


} // namespace


ClientConfig::ClientConfig()
    : host_(DefaultHost)
{}


const std::string& ClientConfig::host() const noexcept
{
    return host_;
}


std::uint16_t ClientConfig::port() const noexcept
{
    return port_;
}


bool ClientConfig::quiet_receipts() const noexcept
{
    return quiet_receipts_;
}


HostAddress ClientConfig::server_address() const
{
    return HostAddress{host_, port_};
}


void ClientConfig::set_host(std::string host)
{
    if (host.empty())
        throw ClientConfigError("host", "must not be empty");

    host_ = std::move(host);
}


void ClientConfig::set_port(const int port)
{
    if (const auto& reason = PortReason(port))
        throw ClientConfigError("port", *reason);

    port_ = static_cast<std::uint16_t>(port);
}


void ClientConfig::set_quiet_receipts(const bool quiet)
{
    quiet_receipts_ = quiet;
}


} // namespace will
