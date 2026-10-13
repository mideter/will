#pragma once

#include <compare>
#include <cstdint>


namespace will::domain {


/// Nanoseconds since the Unix epoch; "now" is Time::instant.
class Timestamp {
public:
	explicit Timestamp(std::int64_t value);

	constexpr std::int64_t value() const noexcept { return value_; }

	constexpr auto operator<=>(const Timestamp&) const noexcept = default;
	constexpr bool operator==(const Timestamp&) const noexcept = default;

private:
	std::int64_t value_ = 0;
};


} // namespace will::domain
