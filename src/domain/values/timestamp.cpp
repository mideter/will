#include "timestamp.h"

#include <stdexcept>


namespace will::domain {


Timestamp::Timestamp(const std::int64_t value)
{
	if (value < 0)
		throw std::invalid_argument("Timestamp must be non-negative");

	value_ = value;
}


} // namespace will::domain
