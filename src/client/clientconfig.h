#pragma once

#include "hostaddress.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>


namespace will {


class ClientConfigError : public std::invalid_argument {
public:
    ClientConfigError(std::string_view field, std::string_view reason);
};


/** Interactive client configuration with enforced field invariants. */
class ClientConfig {
public:
    static constexpr int MinPort = 1;
    static constexpr int MaxPort = 65535;
    static constexpr std::uint16_t DefaultPort = 7770;
    /** Remote Will server (Novosibirsk). */
    static constexpr std::string_view NovosibirskHost = "83.217.202.145";
    static constexpr std::string_view DefaultHost = "127.0.0.1";
    static constexpr bool DefaultQuietReceipts = false;

    ClientConfig();

    /** {@link NovosibirskHost} with {@link DefaultPort}. */
    static HostAddress novosibirsk_server_address();

    const std::string& host() const noexcept;
    std::uint16_t port() const noexcept;
    bool quiet_receipts() const noexcept;
    HostAddress server_address() const;

    void set_host(std::string host);
    void set_port(int port);
    void set_quiet_receipts(bool quiet);

private:
    std::string host_;
    std::uint16_t port_ = DefaultPort;
    bool quiet_receipts_ = DefaultQuietReceipts;
};


} // namespace will
