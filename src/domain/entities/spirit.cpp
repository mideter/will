#include "spirit.h"

#include <stdexcept>


namespace will::domain {


void Spirit::say(std::string_view) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


} // namespace will::domain
