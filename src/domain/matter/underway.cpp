#include "underway.h"


#include <stdexcept>


namespace will::domain::matter {


Underway::Underway(const id::Word training, const std::uint32_t exercise, const std::uint32_t approach,
				   const Timestamp begun, const std::optional<Timestamp> ended)
	: training_(training)
	, exercise_(exercise)
	, approach_(approach)
	, begun_(begun)
	, ended_(ended)
{
	if (ended_ && ended_->value() < begun_.value())
		throw std::invalid_argument("an approach ends after it begins");
}


} // namespace will::domain::matter
