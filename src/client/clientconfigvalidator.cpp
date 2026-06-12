#include "clientconfigvalidator.h"

#include "support/phone_number.h"

#include <asio.hpp>

#include <cctype>
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


std::optional<std::string_view> ClientConfigValidator::phone_reason(const std::string& phone)
{
    if (phone.empty())
        return "must not be empty";

    if (!domain::PhoneNumber::parse(phone))
        return "must be a valid E.164 phone number";

    return std::nullopt;
}


std::optional<std::string_view> ClientConfigValidator::otp_reason(const std::string& otp)
{
    if (otp.empty())
        return std::nullopt;

    if (otp.size() < 4u || otp.size() > 8u)
        return "must be 4-8 ASCII digits when provided";

    for (const char c : otp) {
        if (!std::isdigit(static_cast<unsigned char>(c)))
            return "must be 4-8 ASCII digits when provided";
    }

    return std::nullopt;
}


void ClientConfigValidator::validate(const ClientConfig& config)
{
    require("host", host_reason(config.host));
    require("port", port_reason(config.port));
    require("phone", phone_reason(config.phone));
    require("otp", otp_reason(config.otp));
}


ClientConfig ClientConfigValidator::accept(ClientConfig config)
{
    validate(config);

    if (const auto parsed = domain::PhoneNumber::parse(config.phone))
        config.phone = parsed->e164();

    return config;
}


} // namespace will
