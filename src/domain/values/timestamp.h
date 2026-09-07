#pragma once

#include <compare>
#include <cstdint>


namespace will::domain {


/// Unix epoch instant; stored as nanoseconds since 1970-01-01 UTC. Must be non-negative.
/// A measure of an instant — not the act of capturing "now" (that is Time::instant).
class Timestamp {
public:
	/// From nanoseconds since Unix epoch. Throws std::invalid_argument if negative.
	explicit Timestamp(std::int64_t value);

	constexpr std::int64_t value() const noexcept { return value_; }

	constexpr auto operator<=>(const Timestamp&) const noexcept = default;
	constexpr bool operator==(const Timestamp&) const noexcept = default;

private:
	std::int64_t value_ = 0;
};


} // namespace will::domain
