#pragma once

#include <compare>
#include <cstdint>
#include <functional>


namespace will::domain {


/// Persistent user identity assigned by storage. Must be non-zero.
class UserId {
public:
    /// Throws std::invalid_argument if value is 0.
    explicit UserId(std::uint64_t value);

    constexpr std::uint64_t value() const noexcept { return value_; }

    constexpr auto operator<=>(const UserId&) const noexcept = default;
    constexpr bool operator==(const UserId&) const noexcept = default;

private:
    std::uint64_t value_;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::UserId> {
    std::size_t operator()(const will::domain::UserId id) const noexcept
    {
        return std::hash<std::uint64_t>{}(id.value());
    }
};
