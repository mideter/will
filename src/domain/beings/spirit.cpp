#include "spirit.h"

#include "beings/soul.h"

#include <stdexcept>


namespace will::domain {


void Spirit::present() const
{
	heaven().present(static_cast<const Soul&>(*this));
}


void Spirit::say(const Word&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


} // namespace will::domain
