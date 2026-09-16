#pragma once

#include "id.h"
#include "place.h"

#include <compare>


namespace will::domain::id {


/// Persistent abode identity. Personal abode id matches the host man id.
/// Same numeric value as the abode's Place id.
/// Id 1 may remain as an orphan letter tail from the former global abode.
class Abode : public Id {
public:
	explicit Abode(std::uint64_t value) : Id(value) {}

	static Abode global() { return Abode{1}; }

	/// The place id of this abode (same value).
	Place as_place() const noexcept { return Place{value()}; }

	constexpr auto operator<=>(const Abode&) const noexcept = default;
	constexpr bool operator==(const Abode&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Abode> : will::domain::id::IdHash<will::domain::id::Abode> {};
