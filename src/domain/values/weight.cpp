#include "weight.h"

#include <stdexcept>


namespace will::domain {


Weight::Weight(const std::uint32_t grams)
	: grams_(grams)
{
	if (grams_ > MaxGrams)
		throw std::invalid_argument("Weight exceeds MaxGrams");
}


} // namespace will::domain
