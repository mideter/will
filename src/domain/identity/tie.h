#pragma once

#include "id.h"
#include "place.h"

#include <compare>


namespace will::domain::id {


/// Persistent id of a Tie (Узы) — the shared Obedience/Shepherding place.
/// Same numeric value as that place's Place id (from Space::point).
class Tie : public Id {
public:
	explicit Tie(Place place) : Id(place.value()) {}
	explicit Tie(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Tie&) const noexcept = default;
	constexpr bool operator==(const Tie&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Tie> : will::domain::id::IdHash<will::domain::id::Tie> {};
