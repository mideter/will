#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent vessel identity assigned by storage.
class Vessel : public Id {
public:
    explicit Vessel(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const Vessel&) const noexcept = default;
    constexpr bool operator==(const Vessel&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Vessel> : will::domain::id::IdHash<will::domain::id::Vessel> {};
