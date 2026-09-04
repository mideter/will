#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent man identity assigned by storage.
class Man : public Id {
public:
    explicit Man(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const Man&) const noexcept = default;
    constexpr bool operator==(const Man&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Man> : will::domain::id::IdHash<will::domain::id::Man> {};
