#pragma once

#include <compare>
#include <string>


namespace will::domain {


/// Opaque session token; generation and persistence live in infrastructure.
struct AuthToken {
    std::string value;

    constexpr AuthToken() noexcept = default;
    explicit AuthToken(std::string v) : value(std::move(v)) {}

    [[nodiscard]] bool empty() const noexcept { return value.empty(); }

    auto operator<=>(const AuthToken&) const = default;
};


} // namespace will::domain
