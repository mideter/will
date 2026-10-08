#pragma once

#include "values/weight.h"

#include <cstdint>


namespace will::domain {


/// Approach (Подход) — one go at an exercise: so many repetitions with this weight,
/// and the rest before it, in seconds: how rested one comes to it. The first effort of
/// a training comes fully rested whatever is willed.
class Approach {
public:
	static constexpr std::uint32_t MaxRepetitions = 10'000;
	/// At most an hour.
	static constexpr std::uint32_t MaxRestSeconds = 3'600;

	/// Throws std::invalid_argument unless 1 <= repetitions <= MaxRepetitions and
	/// the rest is at most MaxRestSeconds.
	Approach(Weight weight, std::uint32_t repetitions, std::uint32_t rest_seconds = 0);

	Weight weight() const noexcept { return weight_; }
	std::uint32_t repetitions() const noexcept { return repetitions_; }
	std::uint32_t rest_seconds() const noexcept { return rest_seconds_; }

	bool operator==(const Approach&) const = default;

private:
	Weight weight_;
	std::uint32_t repetitions_;
	std::uint32_t rest_seconds_;
};


} // namespace will::domain
