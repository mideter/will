#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent place identity — where a Word may be fixed in time.
/// Personal abode place id matches the host man id.
/// Id 1 may remain as an orphan letter tail from the former global abode.
class Place : public Id {
public:
	explicit Place(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Place&) const noexcept = default;
	constexpr bool operator==(const Place&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Place> : will::domain::id::IdHash<will::domain::id::Place> {};
