#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent supplication identity assigned by storage.
class Supplication : public Id {
public:
	explicit Supplication(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Supplication&) const noexcept = default;
	constexpr bool operator==(const Supplication&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Supplication>
	: will::domain::id::IdHash<will::domain::id::Supplication> {};
