#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Persistent Deed (Дело) identity assigned by storage.
class Deed : public Id {
public:
	explicit Deed(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Deed&) const noexcept = default;
	constexpr bool operator==(const Deed&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Deed> : will::domain::id::IdHash<will::domain::id::Deed> {};
