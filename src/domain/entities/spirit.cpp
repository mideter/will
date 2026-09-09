#include "spirit.h"

#include <stdexcept>


namespace will::domain {


void Spirit::say(const Word&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


Word Spirit::hear() const
{
	throw std::logic_error("Spirit outside the living world cannot hear");
}


} // namespace will::domain
