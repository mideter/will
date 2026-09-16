#include "spirit.h"

#include "entities/soul.h"

#include <stdexcept>


namespace will::domain {


void Spirit::present(const Soul& soul) const
{
	heaven().present(soul);
}


void Spirit::say(const Word&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


} // namespace will::domain
