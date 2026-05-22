#pragma once

#include <asio.hpp>

#include <cstdint>
#include <format>
#include <iosfwd>
#include <sstream>
#include <string_view>


namespace will {


/** IPv4 host and TCP port as {@link asio::ip::tcp::endpoint}. */
class HostAddress {
public:
    HostAddress(std::string_view host, std::uint16_t port);
    HostAddress(const HostAddress&) = default;
    HostAddress& operator=(const HostAddress&) = default;
    HostAddress(HostAddress&&) = default;
    HostAddress& operator=(HostAddress&&) = default;

    static HostAddress from_endpoint(asio::ip::tcp::endpoint endpoint);
    static HostAddress any(std::uint16_t port);

    [[nodiscard]] const asio::ip::tcp::endpoint& endpoint() const noexcept { return endpoint_; }

private:
    friend std::ostream& operator<<(std::ostream& os, const HostAddress& address);

    explicit HostAddress(asio::ip::tcp::endpoint endpoint);

    asio::ip::tcp::endpoint endpoint_;
};


std::ostream& operator<<(std::ostream& os, const HostAddress& address);


} // namespace will


template <>
struct std::formatter<will::HostAddress> {
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(const will::HostAddress& address, std::format_context& ctx) const
    {
        std::ostringstream os;
        os << address;
        return std::format_to(ctx.out(), "{}", os.str());
    }
};
