#pragma once

#include "values/weight.h"

#include <cstdint>


namespace will::domain {


/// So many repetitions with this weight, and the rest before it: how
/// rested one comes to it. The first effort of a training comes fully rested.
class Approach {
public:
	static constexpr std::uint32_t MaxRepetitions = 10'000;
	static constexpr std::uint32_t MaxRestSeconds = 3'600;

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
