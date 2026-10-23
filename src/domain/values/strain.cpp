#include "strain.h"

#include <stdexcept>


namespace will::domain {


Strain::Strain(const std::uint32_t marks)
	: marks_(marks)
{
	if (marks_ > MaxMarks)
		throw std::invalid_argument("a strain is at most MaxMarks marks");
}


} // namespace will::domain
