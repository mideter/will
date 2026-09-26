#include "man.h"

#include <utility>


namespace will::domain {


Man::Man(Embodiment embodiment)
	: Soul(embodiment.soul(), embodiment.name())
	, Vessel(embodiment.vessel(), embodiment.token())
{
	Immanent<Heaven>::present<Soul>();
	Immanent<Earth>::present<Vessel>();
}


} // namespace will::domain
