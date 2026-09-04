#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent letter identity assigned by storage.
class Letter : public Id {
public:
    explicit Letter(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const Letter&) const noexcept = default;
    constexpr bool operator==(const Letter&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Letter> : will::domain::id::IdHash<will::domain::id::Letter> {};
