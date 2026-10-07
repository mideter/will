#pragma once

#include "values/weight.h"

#include <cstdint>


namespace will::domain {


/// Approach (Подход) — one go at an exercise: so many repetitions with this weight.
class Approach {
public:
	static constexpr std::uint32_t MaxRepetitions = 10'000;

	/// Throws std::invalid_argument unless 1 <= repetitions <= MaxRepetitions.
	Approach(Weight weight, std::uint32_t repetitions);

	Weight weight() const noexcept { return weight_; }
	std::uint32_t repetitions() const noexcept { return repetitions_; }

	bool operator==(const Approach&) const = default;

private:
	Weight weight_;
	std::uint32_t repetitions_;
};


} // namespace will::domain
