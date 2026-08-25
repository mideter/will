#pragma once

#include <cstdint>
#include <compare>


namespace will::domain {


struct UserId {
    std::uint64_t value = 0;

    constexpr UserId() noexcept = default;
    constexpr explicit UserId(std::uint64_t v) noexcept : value(v) {}

    constexpr auto operator<=>(const UserId&) const noexcept = default;
    constexpr bool operator==(const UserId&) const noexcept = default;
};


} // namespace will::domain
