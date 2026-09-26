#include "spirit.h"

#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"

#include <stdexcept>


namespace will::domain {


void Spirit::say(const Saying&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


void Spirit::contemplate(const Abode& abode) const
{
	const auto& soul = static_cast<const Soul&>(*this);
	heaven().contemplate(soul, abode);
}


const Contemplation& Spirit::contemplation() const
{
	const auto& soul = static_cast<const Soul&>(*this);
	return heaven().contemplation(soul.id());
}


} // namespace will::domain
