#include "approach.h"

#include <stdexcept>


namespace will::domain {


Approach::Approach(const Weight weight, const std::uint32_t repetitions)
	: weight_(weight)
	, repetitions_(repetitions)
{
	if (repetitions_ < 1 || repetitions_ > MaxRepetitions)
		throw std::invalid_argument("Approach needs 1 to MaxRepetitions repetitions");
}


} // namespace will::domain
