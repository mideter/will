#include "creation.h"

#include "horizons/life.h"


namespace will::domain {


Creation::Creation(Birth<Life>, Eternity& eternity)
	: spatiality_(eternity.spatiality(Birth<Creation>{}))
	, temporality_(eternity.temporality(Birth<Creation>{}))
	, world_(eternity, *temporality_, *spatiality_)
{
	present<Creation>();
	world_.awaken();
}


Creation::~Creation()
{
	depart<Creation>();
}


} // namespace will::domain
