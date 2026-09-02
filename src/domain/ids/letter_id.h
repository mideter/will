#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent letter identity assigned by storage.
class LetterId : public Id {
public:
    explicit LetterId(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const LetterId&) const noexcept = default;
    constexpr bool operator==(const LetterId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::LetterId> : will::domain::IdHash<will::domain::LetterId> {};
