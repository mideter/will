#include "spirit.h"

#include "beings/immanents/abode.h"
#include "beings/immanents/soul.h"

#include <stdexcept>


namespace will::domain {


void Spirit::say(const Saying&) const
{
	throw std::logic_error("Spirit outside the living world cannot say");
}


void Spirit::keep_contemplation(const Soul& soul, const Abode& abode)
{
	heaven().contemplate(soul, abode);
}


const Contemplation& Spirit::contemplation_of(const Soul& soul)
{
	return heaven().contemplation(soul.id());
}


} // namespace will::domain
