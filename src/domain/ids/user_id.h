#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent user identity assigned by storage. Must be non-zero.
class UserId : public Id {
public:
    /// Throws std::invalid_argument if value is 0.
    explicit UserId(std::uint64_t value);

    constexpr auto operator<=>(const UserId&) const noexcept = default;
    constexpr bool operator==(const UserId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::UserId> : will::domain::IdHash<will::domain::UserId> {};
