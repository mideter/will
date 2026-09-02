#pragma once

#include "id.h"

#include <compare>


namespace will::domain {


/// Persistent god identity assigned by storage.
class GodId : public Id {
public:
    explicit GodId(std::uint64_t value) : Id(value) {}

    constexpr auto operator<=>(const GodId&) const noexcept = default;
    constexpr bool operator==(const GodId&) const noexcept = default;
};


} // namespace will::domain


template <>
struct std::hash<will::domain::GodId> : will::domain::IdHash<will::domain::GodId> {};
