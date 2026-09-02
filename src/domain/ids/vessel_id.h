#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent vessel identity assigned by storage.
class VesselId : public Id {
public:
    explicit VesselId(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const VesselId&) const noexcept = default;
    constexpr bool operator==(const VesselId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::VesselId> : will::domain::IdHash<will::domain::VesselId> {};
