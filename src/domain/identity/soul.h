#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent soul identity assigned by storage.
class Soul : public Id {
public:
	explicit Soul(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Soul&) const noexcept = default;
	constexpr bool operator==(const Soul&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Soul> : will::domain::id::IdHash<will::domain::id::Soul> {};
