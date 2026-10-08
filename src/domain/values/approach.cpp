#include "approach.h"

#include <stdexcept>


namespace will::domain {


Approach::Approach(const Weight weight, const std::uint32_t repetitions, const std::uint32_t rest_seconds)
	: weight_(weight)
	, repetitions_(repetitions)
	, rest_seconds_(rest_seconds)
{
	if (repetitions_ < 1 || repetitions_ > MaxRepetitions)
		throw std::invalid_argument("Approach needs 1 to MaxRepetitions repetitions");
	if (rest_seconds_ > MaxRestSeconds)
		throw std::invalid_argument("Approach rest exceeds MaxRestSeconds");
}


} // namespace will::domain
