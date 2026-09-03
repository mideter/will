#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent god identity assigned by storage.
class God : public Id {
public:
    explicit God(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const God&) const noexcept = default;
    constexpr bool operator==(const God&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::God> : will::domain::id::IdHash<will::domain::id::God> {};
