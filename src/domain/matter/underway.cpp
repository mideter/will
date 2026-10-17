#include "underway.h"


namespace will::domain::matter {


Underway::Underway(const id::Word training, const std::uint32_t exercise, const std::uint32_t approach,
				   const Timestamp begun)
	: training_(training)
	, exercise_(exercise)
	, approach_(approach)
	, begun_(begun)
{}


} // namespace will::domain::matter
