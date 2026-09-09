#include "spirit.h"

#include <stdexcept>


namespace will::domain {


void Spirit::say(std::string_view) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


std::vector<Letter> Spirit::hear(std::uint32_t) const
{
	throw std::logic_error("Spirit outside the living world cannot hear");
}


} // namespace will::domain
