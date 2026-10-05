#include "creation.h"

#include "horizons/life.h"


namespace will::domain {


Creation::Creation(const Birth<Life> birth)
	: spatiality_(birth.parent().eternity().spatiality(Birth<Creation>{*this}))
	, temporality_(birth.parent().eternity().temporality(Birth<Creation>{*this}))
	, world_(birth.parent().eternity(), *temporality_, *spatiality_)
{
	world_.awaken();
}


} // namespace will::domain
