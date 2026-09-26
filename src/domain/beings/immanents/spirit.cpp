#include "spirit.h"

#include <stdexcept>


namespace will::domain {


void Spirit::say(const Saying&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


} // namespace will::domain
