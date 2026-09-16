#include "spirit.h"

#include "entities/soul.h"

#include <stdexcept>


namespace will::domain {


void Spirit::index(const Soul& soul) const
{
	heaven().index(soul);
}


void Spirit::say(const Word&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


} // namespace will::domain
