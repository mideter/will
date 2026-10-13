#pragma once

#include "id.h"

#include <compare>


namespace will::domain::id {


/// Shared by all the parts of a word's matter.
class Word : public Id {
public:
	explicit Word(std::uint64_t value) : Id(value) {}

	constexpr auto operator<=>(const Word&) const noexcept = default;
	constexpr bool operator==(const Word&) const noexcept = default;
};


} // namespace will::domain::id


template <>
struct std::hash<will::domain::id::Word> : will::domain::id::IdHash<will::domain::id::Word> {};
