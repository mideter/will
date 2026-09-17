#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent testament identity. Same numeric value as the underlying Letter id.
class Testament : public Id {
public:
	explicit Testament(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Testament&) const noexcept = default;
	constexpr bool operator==(const Testament&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Testament> : will::domain::id::IdHash<will::domain::id::Testament> {};
