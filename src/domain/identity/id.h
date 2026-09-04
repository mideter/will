#pragma once

#include <compare>
#include <concepts>
#include <cstdint>
#include <functional>


namespace will::domain::id {


/// Common base for strongly-typed persistent identifiers.
class Id {
public:
	constexpr std::uint64_t value() const noexcept { return value_; }

	constexpr auto operator<=>(const Id&) const noexcept = default;
	constexpr bool operator==(const Id&) const noexcept = default;

protected:
	/// Throws std::invalid_argument if value is 0.
	explicit Id(std::uint64_t value);

private:
	std::uint64_t value_;
};


template<typename T>
concept DomainId = std::derived_from<T, Id>;


template<DomainId IdType>
struct IdHash {
	std::size_t operator()(const IdType& id) const noexcept
	{
		return std::hash<std::uint64_t>{}(id.value());
	}
};


} // namespace will::domain::id
