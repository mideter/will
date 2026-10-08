#include "effort.h"

#include <stdexcept>


namespace will::domain::matter {


Effort::Effort(const id::Word training, const std::uint32_t exercise, const std::uint32_t approach,
			   const Weight weight, const std::uint32_t repetitions, const Timestamp begun,
			   const Timestamp finished)
	: training_(training)
	, exercise_(exercise)
	, approach_(approach)
	, weight_(weight)
	, repetitions_(repetitions)
	, begun_(begun)
	, finished_(finished)
{
	if (repetitions_ < 1)
		throw std::invalid_argument("an effort is at least one repetition");
	if (finished_ < begun_)
		throw std::invalid_argument("an effort does not finish before it begins");
}


} // namespace will::domain::matter
