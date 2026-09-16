#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent obedience identity. Same numeric value as the obedience's Place id.
class Obedience : public Id {
public:
	explicit Obedience(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Obedience&) const noexcept = default;
	constexpr bool operator==(const Obedience&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Obedience> : will::domain::id::IdHash<will::domain::id::Obedience> {};
